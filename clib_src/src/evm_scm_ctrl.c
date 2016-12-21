#include "client_lib.h"
#include "cl_priv.h"
#include "uc_client.h"
#include "cl_smart_appliance.h"
#include "smart_appliance_priv.h"
#include "udp_scm_direct_ctrl_priv.h"

// 各种类型虚拟机控制头文件统一放这里
#include "evm_tt_ctrl.h"
#include "evm_yuyuan_ctrl.h"
#include "evm_zkcleanner_ctrl.h"
#include "evm_hythermostat_ctrl.h"
#include "evm_bpuair_ctrl.h"
#include "evm_jrxheater_ctrl.h"
#include "evm_cjthermostat_ctrl.h"
#include "evm_drkzq_ctrl.h"
#if 0
typedef struct {
	u_int32_t evm_type;
	u_int8_t sub_type;
	u_int8_t ext_type;
} evm_map_t;
evm_map_t evm_type_map[] = {
	{100, IJ_EVM, EYPE_EVM_YUYUAN_WATER_CLEANNER},	// 御源净水器
};
#endif
/**
	通过TLV里面的UCT_DEVICE_NEW_DEV_EXT_TYPE，改变设备证书里面的sub_type 和ext_type
*/
void evm_set_type_by_tlv(u_int8_t *sub_type, u_int8_t *ext_type, uc_tlv_t* tlv)
{
	u_int8_t *ptr;
//	int i;

	log_info("evm set type tlv, type %u len %u\n", tlv->type, tlv->len);

	if (tlv->type != UCT_DEVICE_NEW_DEV_EXT_TYPE || tlv->len < sizeof(u_int16_t)) {
		return ;
	}

	ptr = (u_int8_t *)tlv_val(tlv);
	log_debug("get tvl set type data dev_type %u ext_type %u\n", ptr[0], ptr[1]);

	*sub_type = ptr[0];
	*ext_type = ptr[1];
	
	log_info("evm convert to sub_type %u ext_type %u\n", *sub_type, *ext_type);
}


/**
	构造私有属性给上层
*/
bool evm_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
	user_t* user = ac->sac->user;

	if (!user) {
		log_err(false, "evm not fund user\n");
		return false;
	}
		
// 如果IJ_EVM已经用满了255个类型，这里需要修改
	if (user->sub_type != IJ_EVM) {
		return false;
	}

	switch (user->ext_type) {
		case EYPE_EVM_DEFAULT:
			return tt_bulid_priv_dev_info(ac, di);
			break;
		case EYPE_EVM_YUYUAN_WATER_CLEANNER:
			return yuyuan_bulid_priv_dev_info(ac, di);
		case EYPE_EVM_ZKCLEANNER:
			return zkcleanner_bulid_priv_dev_info(ac, di);
        //case EYPE_EVM_HYTHERMOSTAT:
        case EYPE_EVM_HYTHERMOSTAT_AC:
        case EYPE_EVM_HYTHERMOSTAT_HT:
			return hythermostat_bulid_priv_dev_info(ac, di);
		case EYPE_EVM_BPUAIR_1:
		case EYPE_EVM_BPUAIR_2:
		case EYPE_EVM_BPUAIR_3:
		case EYPE_EVM_BPUAIR_4:
			return bpuair_bulid_priv_dev_info(ac, di);
		case EYPE_EVM_JRXHEATER:
			return jrxheater_bulid_priv_dev_info(ac, di);
		case EYPE_EVM_CJTHERMOSTAT:
			return cjthermostat_bulid_priv_dev_info(ac, di);
		case EYPE_EVM_DRKZQ:
			return drkzq_bulid_priv_dev_info(ac, di);			
	}

	return false;
}



/**
	收到单片机透传命令，更新数据
*/
bool evm_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{	
	user_t* user = air_ctrl->sac->user;

	if (!user) {
		log_err(false, "evm not fund user\n");
		return false;
	}

	log_info("evm scm update data, sub_type %u ext_type %u\n", user->sub_type, user->ext_type);

	// 如果IJ_EVM已经用满了255个类型，这里需要修改
	if (user->sub_type != IJ_EVM) {
		return false;
	}

	switch (user->ext_type) {
		case EYPE_EVM_DEFAULT:
			return tt_do_update_scm_data(air_ctrl, action, obj);
			break;
		case EYPE_EVM_YUYUAN_WATER_CLEANNER:
			return yuyuan_do_update_scm_data(air_ctrl, action, obj);
		case EYPE_EVM_ZKCLEANNER:
			return zkcleanner_do_update_scm_data(air_ctrl, action, obj);
        //case EYPE_EVM_HYTHERMOSTAT:
        case EYPE_EVM_HYTHERMOSTAT_AC:
        case EYPE_EVM_HYTHERMOSTAT_HT:
            return hythermostat_do_update_scm_data(air_ctrl, action, obj);

		case EYPE_EVM_BPUAIR_1:
		case EYPE_EVM_BPUAIR_2:
		case EYPE_EVM_BPUAIR_3:
		case EYPE_EVM_BPUAIR_4:
			return bpuair_do_update_scm_data(air_ctrl, action, obj);
		case EYPE_EVM_JRXHEATER:
			return jrxheater_do_update_scm_data(air_ctrl, action, obj);
		case EYPE_EVM_CJTHERMOSTAT:
			return cjthermostat_do_update_scm_data(air_ctrl, action, obj);
		case EYPE_EVM_DRKZQ:
			return drkzq_do_update_scm_data(air_ctrl, action, obj);			
	}

	return false;
}


/**
	这里是更新设备的除透传状态以外的其他状态。比如情景，密码啥的
*/
bool evm_update_device_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{	
	user_t* user = air_ctrl->sac->user;

	// 暂时只处理EVM STAT相关的
	if (obj->sub_objct != UCSOT_EVM_STAT) {
		return false;
	}

	if (!user) {
		log_err(false, "evm not fund user\n");
		return false;
	}

	// 如果IJ_EVM已经用满了255个类型，这里需要修改
	if (user->sub_type != IJ_EVM) {
		return false;
	}

	switch (user->ext_type) {
		case EYPE_EVM_YUYUAN_WATER_CLEANNER:
			return yuyuan_do_update_device_data(air_ctrl, action, obj);
		case EYPE_EVM_ZKCLEANNER:
			return zkcleanner_do_update_device_data(air_ctrl, action, obj);
		case EYPE_EVM_BPUAIR_1:
		case EYPE_EVM_BPUAIR_2:
		case EYPE_EVM_BPUAIR_3:
		case EYPE_EVM_BPUAIR_4:
			return bpuair_do_update_device_data(air_ctrl, action, obj);
		case EYPE_EVM_JRXHEATER:
			return jrxheater_do_update_device_data(air_ctrl, action, obj);
		case EYPE_EVM_CJTHERMOSTAT:
			return cjthermostat_do_update_device_data(air_ctrl, action, obj);
		case EYPE_EVM_DRKZQ:
			return drkzq_do_update_device_data(air_ctrl, action, obj);
	}
	

	return false;
}

bool evm_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
	// 如果IJ_EVM已经用满了255个类型，这里需要修改
	if (user->sub_type != IJ_EVM) {
		return false;
	}

	switch (user->ext_type) {
		case EYPE_EVM_DEFAULT:
			return tt_scm_proc_notify(user, pkt, ret);
		case EYPE_EVM_YUYUAN_WATER_CLEANNER:
			return yuyuan_scm_proc_notify(user, pkt, ret);
		case EYPE_EVM_ZKCLEANNER:
			return zkcleanner_scm_proc_notify(user, pkt, ret);
        //case EYPE_EVM_HYTHERMOSTAT:
        case EYPE_EVM_HYTHERMOSTAT_AC:
        case EYPE_EVM_HYTHERMOSTAT_HT:
            return hythermostat_scm_proc_notify(user, pkt, ret);
		case EYPE_EVM_BPUAIR_1:
		case EYPE_EVM_BPUAIR_2:
		case EYPE_EVM_BPUAIR_3:
		case EYPE_EVM_BPUAIR_4:
			return bpuair_scm_proc_notify(user, pkt, ret);
		case EYPE_EVM_JRXHEATER:
			return jrxheater_scm_proc_notify(user, pkt, ret);
		case EYPE_EVM_CJTHERMOSTAT:
			return cjthermostat_scm_proc_notify(user, pkt, ret);
		case EYPE_EVM_DRKZQ:
			return drkzq_scm_proc_notify(user, pkt, ret);			
	}

	return true;
}

