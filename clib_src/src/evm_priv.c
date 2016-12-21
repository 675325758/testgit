#include "evm_priv.h"
#include "cl_evm.h"

#include "md5.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>


extern RS ucc_request_add2(ucc_session_t *s, pkt_t *pkt);
extern RS ucc_request_add(ucc_session_t *s, pkt_t *pkt);


static void evm_block_order(evm_block_t *block)
{
	int i;
	evm_block_item_t *item;
	
	
	for (i = 0; i < block->n; i++, item++) {
		item = (evm_block_item_t*)(&block->item[i]);
		
		item->flash_addr = ntohl(item->flash_addr);
		item->valid = ntohl(item->valid);
		item->soft_ver = ntohl(item->soft_ver);
		item->svn = ntohl(item->svn);
		item->len = ntohl(item->len);
		item->crc = ntohl(item->crc);
		item->run = ntohl(item->run);

		
		log_debug("secion %d, pack_name[%s] base_vm_name[%s] run=%u valid=%u soft_ver=%u svn=%u crc=0x%08x\n", 
			i, item->pack_name, item->base_vm_name, item->run, item->valid, item->soft_ver, item->svn, item->crc);
	}
}

void sys_get_evm_info(smart_air_ctrl_t* air_ctrl, ucp_obj_t* uobj)
{
	memcpy(&air_ctrl->evm_block, uobj + 1, uobj->param_len);

	evm_block_order(&air_ctrl->evm_block);
}

pkt_t *sa_evm_new_upgrade_pkt(ucc_session_t *s, uc_upgrade_block_t **ppub, int block_sz)
{
	pkt_t *pkt;
	ucp_ctrl_t* uc;
	ucp_obj_t* uo;
	uc_upgrade_block_t *ub;
	
	pkt = uc_pkt_new(s, CMD_UDP_CTRL, sizeof(*uc)+sizeof(*uo)+sizeof(*ub)+block_sz,
                     true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id++);
	if(pkt == NULL)
		return NULL;
	
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->action = UCA_SET;
	uc->count = 1;
	uc->reserved = 0;

	uo = (ucp_obj_t *)(uc+1);
	fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_EVM_UPGRADE, (short)(sizeof(*ub) + block_sz));
	ub = (uc_upgrade_block_t*)(uo+1);
	*ppub = ub;
	return pkt;
}


RS sa_evm_check_upgrade_file(user_t *user, char *file, int len, struct stlc_list_head *pktlst)
{
	FILE *fp;
	int n;
	MD5_CTX ctx;
	pkt_t *pkt, *next;
	RS ret = RS_INVALID_PARAM;
	unsigned char digest[16];
	unsigned short total, current;
	uc_upgrade_block_t *ub;
	ucc_session_t *s = user->uc_session;
	char *data;

		
	fp = fopen(file, "rb");
	log_debug("enter %s %d\n", __FUNCTION__, __LINE__);	
	if(fp==NULL){
		return RS_INVALID_PARAM;
	}
#if 0
	if(fread(&hdr, 1, sizeof(hdr), fp) != sizeof(hdr)){		
		goto done;
	}
	if(ckeck_image_hdr(user, &hdr) != RS_OK)
		goto done;
#endif	
	MD5Init(&ctx);	
	total = 1 + len/IMAGE_BLOCK_SZ;
	if(len%IMAGE_BLOCK_SZ)
		total++;
	for(current = 1; current <= (total-1); current++){
		if(len >= IMAGE_BLOCK_SZ){
			n = IMAGE_BLOCK_SZ;
			len -= IMAGE_BLOCK_SZ;
		}else{
			n = len;
			len = 0;
		}
		
		pkt = sa_evm_new_upgrade_pkt(s, &ub, n);
		if(pkt == NULL)
			goto done;
		pkt->up_total = total;
		pkt->up_current = current;
		ub->total = ntohs(total);
		ub->current = ntohs(current);
		data = (char*)(ub+1);
		if(fread(data, 1, n, fp) != n) {
	log_debug("enter %s %d\n", __FUNCTION__, __LINE__);			
			goto done;
		}
		MD5Update(&ctx, (unsigned char *)data, (unsigned int)n);
		//dump_md5_ctx(&ctx, current);
		stlc_list_add_tail(&pkt->link, pktlst);
	}	
	
	MD5Final(digest, &ctx);
	#if 0
	if(memcmp(digest, hdr.checksum, 16) != 0)
		goto done;
	#endif
	pkt = sa_evm_new_upgrade_pkt(s, &ub, 16);
	if(pkt == NULL)
		goto done;
	pkt->up_total = total;
	pkt->up_current = current;
	ub->total = ntohs(total);
	ub->current = ntohs(current++);
	data = (char*)(ub+1);
	memcpy(data, digest, 16);
	stlc_list_add_tail(&pkt->link, pktlst);
	ret = RS_OK;
	log_debug("enter %s %d\n", __FUNCTION__, __LINE__);	
done:
	fclose(fp);
	if(ret != RS_OK){
		stlc_list_for_each_entry_safe(pkt_t, pkt, next, pktlst, link){
			stlc_list_del(&pkt->link);
			pkt_free(pkt);			
		}
	}
	return ret;		
}

static RS get_file_len(char *file, int *len)
{
	struct stat st;
	if(stat(file, &st) != 0)
		return RS_INVALID_PARAM;
	*len = (int)st.st_size;	
	return RS_OK;
}

RS sa_evm_upgrade_file(user_t *user, char *file)
{
	int len;	
	pkt_t *pkt, *next;
	ucc_session_t *s = user->uc_session;
	struct stlc_list_head pktlst;

	STLC_INIT_LIST_HEAD(&pktlst);
	if (get_file_len(file, &len) != RS_OK)
		return RS_INVALID_PARAM;
	if (len < (int)sizeof(evm_fw_hdr_t))
		return RS_INVALID_PARAM;
	log_debug("enter len=%d %s %d \n", len, __FUNCTION__, __LINE__);
	//len -= (int)sizeof(image_hdr_t);
	if (sa_evm_check_upgrade_file(user, file, len, &pktlst) != RS_OK)
		return RS_INVALID_PARAM;		
	log_debug("enter %s %d\n", __FUNCTION__, __LINE__);	
	s->up_total = 0;
	s->up_current = 0;
	s->up_first = NULL;
	stlc_list_for_each_entry_safe(pkt_t, pkt, next, &pktlst, link){
		stlc_list_del(&pkt->link);
		ucc_request_add2(s, pkt);
		s->up_total++;
		if(s->up_first == NULL)
			s->up_first = pkt;		
	}
	log_debug("enter %s %d\n", __FUNCTION__, __LINE__);
	
	return RS_OK;
}


RS sa_dev_upgrade_evm_pack(cl_notify_pkt_t *cln_pkt, user_t *user, char *file)
{
	RS ret = RS_OK;
	
	log_debug("enter %s %d\n", __FUNCTION__, __LINE__);
	
	ret = sa_evm_upgrade_file(user, file);

//done:
	log_debug("enter %s %d ret=%d\n", __FUNCTION__, __LINE__, ret);
	
	return ret;
}

pkt_t *sa_evm_query_section_pkt(ucc_session_t *s)
{
	pkt_t *pkt;
	ucp_ctrl_t* uc;
	ucp_obj_t* uo;
//	u_int8_t *pdata = NULL;
	
	pkt = uc_pkt_new(s, CMD_UDP_CTRL, sizeof(*uc)+sizeof(*uo),
					 true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id);
	if(pkt == NULL)
		return NULL;
	
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->action = UCA_GET;
	uc->count = 1;
	uc->reserved = 0;

	uo = (ucp_obj_t *)(uc+1);
	fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_EVM_INFO, 0);
	log_debug("sa_evm_query_section_pkt\n");
	
	return pkt;
}


RS sa_dev_query_evm_info(cl_notify_pkt_t *cln_pkt, user_t *user)
{
	RS ret = RS_OK;
	
	pkt_t *pkt = NULL;
	
	
	cl_lock(&cl_priv->mutex);
	log_debug("enter %s %d\n", __FUNCTION__, __LINE__);


	pkt = sa_evm_query_section_pkt(user->uc_session);
	if (pkt) {
		log_debug("enter %s %d ret=%d\n", __FUNCTION__, __LINE__, ret); 	
		ucc_request_add(user->uc_session, pkt);
	} 

//done:
	cl_unlock(&cl_priv->mutex);
	log_debug("enter %s %d ret=%d\n", __FUNCTION__, __LINE__, ret);
	
	return ret;
}

pkt_t *sa_evm_upgrade_flash_erase_pkt(ucc_session_t *s, u_int8_t num)
{
	pkt_t *pkt;
	ucp_ctrl_t* uc;
	ucp_obj_t* uo;
	u_int8_t *pdata = NULL;
	
	pkt = uc_pkt_new(s, CMD_UDP_CTRL, sizeof(*uc)+sizeof(*uo) + sizeof(num),
                     true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id);
	if(pkt == NULL)
		return NULL;
	
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->action = UCA_SET;
	uc->count = 1;
	uc->reserved = 0;

	uo = (ucp_obj_t *)(uc+1);
	fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_EVM_ERASE, sizeof(num));
	pdata = (u_int8_t *)(uo+1);
	*pdata = num;
	log_debug("sa_evm_upgrade_flash_erase_pkt num=%u\n", num);
	
	return pkt;
}

static RS sa_dev_upgrade_evm_erase(cl_notify_pkt_t *cln_pkt, user_t *user, u_int32_t num)
{
	RS ret = RS_OK;
	pkt_t *pkt = NULL;
	
	log_debug("enter %s %d\n", __FUNCTION__, __LINE__);

	log_debug(" sa_evm_upgrade_erase num=%u\n", num);
	pkt = sa_evm_upgrade_flash_erase_pkt(user->uc_session, num);
	if (pkt) {
		log_debug("enter %s %d ret=%d\n", __FUNCTION__, __LINE__, ret); 	
		ucc_request_add(user->uc_session, pkt);
	} 

//done:
	log_debug("enter %s %d ret=%d\n", __FUNCTION__, __LINE__, ret);
	
	return ret;
}



RS sa_dev_upgrade_evm(cl_notify_pkt_t *pkt)
{
//	bool res = true;
	cln_common_info_t *info;
//	smart_air_ctrl_t* ac;
//	cl_yuyuan_info_t *pv;
//	ucc_session_t *session;
	u_int8_t *var_param;
	int var_param_len = 0;
	user_t *user;

	info = (cln_common_info_t *)&pkt->data[0];
	user = lookup_by_handle(HDLT_USER, info->handle);
	if (!user) {
		return false;
	}

	var_param = &info->u.u8_data[0];
	var_param_len = info->data_len;
	
	switch (info->action) {
		case ACT_EVM_ERASE:
			return sa_dev_upgrade_evm_erase(pkt, user, cci_u32_data(info));
		case ACT_EVM_QUERY_INFO:
			return sa_dev_query_evm_info(pkt, user);
		case ACT_EVM_UPGRADE:
			return sa_dev_upgrade_evm_pack(pkt, user, cci_pointer_data(info));
	}

	return false;
}



CLIB_API RS cl_dev_update_evm(cl_handle_t handle, char *filename)
{	
	CL_CHECK_INIT;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, handle, CLNE_DEV_UPGRADE_EVM, ACT_EVM_UPGRADE,
		(u_int8_t *)filename, (u_int32_t)(strlen(filename)) + 1);
}

CLIB_API RS cl_dev_update_evm_info(cl_handle_t handle)
{
	CL_CHECK_INIT;

	return cl_send_u32_notify(&cl_priv->thread_main, handle, CLNE_DEV_UPGRADE_EVM, ACT_EVM_QUERY_INFO, 0);
}

CLIB_API RS cl_dev_evm_info_get(cl_handle_t handle, evm_block_t *block)
{
	user_t *user;
//	user_t *dev;
	smart_air_ctrl_t* ac;
	smart_appliance_ctrl_t* sac;
	//int data_len = MONTH_PER_YEAR*sizeof(int);


	if ((user = (user_t *)lookup_by_handle(HDLT_USER, handle)) == NULL) {
		log_debug("not find user\n");
		return RS_ERROR;
	}

	if (user->smart_appliance_ctrl) {
		sac = user->smart_appliance_ctrl;
		ac = sac->sub_ctrl;
		if (ac) {
			memcpy(block, &(ac->evm_block), sizeof(*block));
		} else {
			log_debug("cl_dev_evm_info_get ac is null\n");
		}
	} else {
		log_debug("cl_dev_evm_info_get error !!!!!!!!!!!!!!!!!!!!!!!\n");
	}	

	return RS_OK;
}


CLIB_API RS cl_evm_flash_erase(cl_handle_t handle, u_int32_t num)
{
	CL_CHECK_INIT;

	return cl_send_u32_notify(&cl_priv->thread_main, handle, CLNE_DEV_UPGRADE_EVM, ACT_EVM_ERASE, num);
}


