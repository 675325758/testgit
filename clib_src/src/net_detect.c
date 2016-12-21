#include "cl_priv.h"


static void net_detect_init(net_detect_t *nd)
{
	nd->m_Cookie.nd_ptr = (u_int32_t)nd;
	nd->m_Cookie.time = get_msec();
}

RS send_net_detect_q(video_t *video, net_detect_t *nd)
{
	pkt_t *pkt;
	network_detect_q_t *ndq;
	struct sockaddr_in addr;
	user_t *user = video->slave->user;

	user_create_udp(user);
	
	log_debug("video %s send_net_detect_q To %u.%u.%u.%u port %u, priority=%u, type=%u\n",
		video->slave->str_sn, IP_SHOW(nd->m_nIp), nd->m_nPort, nd->m_nPriority, nd->m_nType);

	if (nd->m_nIp == 0 || nd->m_nPort == 0 || nd->m_nIp == 0xFFFFFFFF)
		return RS_ERROR;

	if (nd->m_Cookie.nd_ptr == 0) {
		net_detect_init(nd);
	}

	pkt = pkt_new_v2(CMD_NETWORK_DETECT_Q, sizeof(network_detect_q_t), NHF_TRANSPARENT, video->slave->sn, TP_USER);
	PKT_HANDLE(pkt) = video->slave->handle;
	ndq = get_pkt_payload(pkt, network_detect_q_t);
	memcpy(ndq->cookie, &nd->m_Cookie, sizeof(network_detect_q_t));

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(nd->m_nIp);
	addr.sin_port = htons(nd->m_nPort);

	sendto(user->sock_udp, pkt->data, pkt->total, 0, (struct sockaddr *)&addr, sizeof(addr));

#ifdef SUPPORT_TRAFFIC_STAT
    UDP_PKT_STAT(false, pkt->total);
#endif
	
	nd->m_liQTime = get_usec();

	free(pkt);

	return RS_OK;
}


void OnP2PDetect(video_t *video)
{
#if 1
	log_err(false, "Please write function OnP2PDetect\n");

#else
	int i;
	CVideoWnd *vw;

	for (i = 0; i < m_arVideoWnd.GetCount(); i++) {
		vw = (CVideoWnd *)m_arVideoWnd[i];
		if (vw->m_nSlaveSN == slave->m_nSN) {
			if ((vw->m_nStatus == VS_SYN_A_RCV || vw->m_nStatus == VS_ESTABLISH) 
				&& vw->m_pVideoTry->m_pTcpVideo == NULL
				&& (! vw->m_bVtap) && !vw->m_bBackGround)
			{
				vw->SetStatErr("尝试更高视频质量");
				vw->SetErrorStr("尝试直接连接以提高视频效果。。。");
				vw->Restart(100);
			}
			break;
		}
	}
#endif	
}

/* 
	如果是给自己的，返回TRUE。否则返回FALSE。
*/
RS recv_net_detect_a(user_t *user, pkt_t *pkt)
{
	int index = 0;
	slave_t *slave;
	video_t *video;
	nd_cookie_t *ndc;
	net_header_t *hdr;
	net_detect_t *nd;

	hdr = (net_header_t *)pkt->data;

	if ((slave = lookup_by_handle(HDLT_SLAVE, hdr->handle)) == NULL)
		return RS_ERROR;

	video = slave->video;
	
	ndc = get_pkt_payload(pkt, nd_cookie_t);

	stlc_list_for_each_entry(net_detect_t, nd, &video->net_detect_list, link) {
		if (memcmp(ndc, &nd->m_Cookie, sizeof(nd_cookie_t)) == 0)
			break;
		
		index++;
	}
	if (&nd->link == &video->net_detect_list) {
		log_err(false, "%s ignore CMD_NETWORK_DETECT_A pkt, nd_ptr=0x%x, time=%u\n",
			slave->str_sn, ndc->nd_ptr, ndc->time);
		return RS_ERROR;
	}

	if (nd->m_nPkts == 0) {
		nd->m_liFirstATime = get_usec();
		nd->m_Delay = (double)(nd->m_liFirstATime - nd->m_liQTime)/1000000.0;
		log_info("%s dealy=%lf\n", slave->str_sn, nd->m_Delay);		
	}
	
	nd->m_nPkts++;
	nd->m_nBytes += pkt->total + ETH_IP_UDP_LEN;

	// 收到3个报文以上再计算速率，否则误差很大
	if (nd->m_nPkts >= 3) {
		u_int32_t now;
		double dt;

		now = get_usec();
		dt = now - nd->m_liFirstATime;
		if (dt == 0.0)
			dt = 0.1;

		nd->m_Rate = nd->m_nBytes*8 / (dt/1000000);
		log_info("%s pkts=%d, rate=%lf\n", slave->str_sn, nd->m_nPkts, nd->m_Rate);		
	}

	if (video->slect_p2p < 0 || video->slect_p2p > index) {
		video->slect_p2p = index;
		log_info("Slave %s select P2P %d, ip=%u.%u.%u.%u, port=%u, priority=%u, type=%u\n", 
			slave->str_sn, video->slect_p2p, IP_SHOW(nd->m_nIp), nd->m_nPort,
			nd->m_nPriority, nd->m_nType);
		// 如果它在看视频，强制切换成这个链路。哈哈哈
		OnP2PDetect(video);
	}

	// 探测到了，本轮探测结束
	CL_THREAD_OFF(video->t_net_detect);
	CL_THREAD_TIMER_ON(&cl_priv->master, video->t_net_detect, timer_net_detect, video, TIME_NET_DETECT_FAIL_RETRY);
				
	return RS_OK;
}

int timer_net_detect(cl_thread_t *t)
{
	video_t *video = CL_THREAD_ARG(t);
	u_int32_t next_time;
	net_detect_t *nd;

	video->t_net_detect = NULL;

	if (stlc_list_empty(&video->net_detect_list) || video->slect_p2p >= 0)
		return 0;

	stlc_list_by_index(net_detect_t, nd, &video->net_detect_list, link, video->pos_next_query);
	video->pos_next_query++;

	// 下一次要很久以后再探测了，但是也不要失望，特别是无线连接情况下 
	if (nd == NULL) {
		next_time = TIME_NET_DETECT_FAIL_RETRY;
		video->pos_next_query = 0;
	} else {
		next_time = TIME_NET_DETECT_TIMEOUT;
        if(!cl_priv->run_in_background)
            send_net_detect_q(video, nd);
	}

	CL_THREAD_TIMER_ON(&cl_priv->master, video->t_net_detect, timer_net_detect, video, next_time);

	return 0;
}


