#ifndef	__CL_ENV_MON_PRIV_H__
#define	__CL_ENV_MON_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif 

extern RS em_get_city_list();

extern RS em_on_set_city(cl_notify_pkt_t *pkt);
extern RS em_on_get_city_list(cl_notify_pkt_t *pkt);

extern RS em_on_get_weather(cl_notify_pkt_t *pkt);
extern RS em_on_get_pm25(cl_notify_pkt_t *pkt);
extern RS em_on_get_suggest(cl_notify_pkt_t *pkt);
extern RS em_start(user_t *user);


#ifdef __cplusplus
}
#endif 

#endif


