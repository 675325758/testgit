#ifndef __OSDEF_HEADER__
#define __OSDEF_HEADER__

#ifdef __cplusplus
extern "C" {
#endif 

void Gw_init_avlock();

void Gw_destroy_avlock();

void Gw_avlock();

void Gw_avunlock();

#ifdef __cplusplus
}
#endif 


#endif
