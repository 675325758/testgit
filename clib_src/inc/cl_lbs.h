#ifndef	__CL_LBS_H__
#define	__CL_LBS_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"

typedef struct {
	u_int8_t is_east;
	u_int8_t is_north;
	u_int8_t is_gps;
	u_int8_t pad;
	u_int16_t speed;
	// γ��
	double latitude;
	// ����
	double longitude;
} lbs_pos_t;

#define MAX_BASE_CNT 8
typedef struct{
	int count; //��վ������ÿ�ο��ܻ�ȡ�������վ
	int base[MAX_BASE_CNT];
}lbs_base_t;

typedef struct{
	u_int64_t sn;
	double speed;
	double distance;
	u_int8_t is_in_lan;
	u_int8_t has_mark_home;
	u_int8_t status;
}lbs_gdb_info_t;

typedef struct {
	int count;
	lbs_gdb_info_t list[10];
} lbs_gdb_list_t;

// ��ǵ�ǰλ��
CLIB_API RS lbs_mark(lbs_pos_t *pos);
// ��ǻ�վλ��
CLIB_API RS lbs_mark_base(lbs_base_t *base);
// ����λ�þ��ȣ���λ��
CLIB_API void lbs_set_precision(int precision);
// �����ã���ʾ��ǰλ���Ǽҵ�λ�á�ֻ������һ�Σ��ڶ���Ҫɾ��home.txt�ļ���ɱ������
CLIB_API RS lbs_mark_home_test();
/* γ�Ⱦ��� */
CLIB_API double lbs_get_latitude_diff();
/* ���Ⱦ��� */
CLIB_API double lbs_get_longitude_diff();

/*�鿴������Ϣ*/
CLIB_API void lbs_get_info(lbs_gdb_list_t *info);

/*���½ӿڸ�iosʹ��
������ɨ�赽�յ���ʱ����ǻ���������λ��
3�������� -- > ����3�������� --> ����1��������ʱ����lbs_going_home_on
���ܻؼҿ����յ�*/
CLIB_API RS lbs_going_home_on(cl_handle_t handle);

#ifdef __cplusplus
}
#endif 


#endif



