#include "client_lib.h"
#include "cl_smart_appliance.h"
#include "cl_priv.h"
#include "udp_ctrl.h"
#include "smart_appliance_priv.h"
#include "lc_furnace_priv.h"
#include "cl_lamp.h"

CLIB_API RS cl_lamp_ctrl_power(cl_handle_t dev_handle, u_int8_t is_on)
{
	if(is_on)
		return cl_send_u32_notify(&cl_priv->thread_main, dev_handle,CLNE_IA_CTRL, GX_LED_ON, 0);
	else
		return cl_send_u32_notify(&cl_priv->thread_main, dev_handle,CLNE_IA_CTRL, GX_LED_OFF, 0);
}

CLIB_API RS cl_lamp_ctrl_full_light(cl_handle_t dev_handle)
{
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle,CLNE_IA_CTRL, GX_LED_FULL_LIGHT, 0);
}

CLIB_API RS cl_lamp_ctrl_white(cl_handle_t dev_handle)
{
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle,CLNE_IA_CTRL, GX_LED_WHITE, 0);
}

CLIB_API RS cl_lamp_ctrl_warm(cl_handle_t dev_handle)
{
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle,CLNE_IA_CTRL, GX_LED_WARM, 0);
}

CLIB_API RS cl_lamp_ctrl_mix(cl_handle_t dev_handle)
{
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle,CLNE_IA_CTRL, GX_LED_MIX, 0);
}

CLIB_API RS cl_lamp_ctrl_brightness(cl_handle_t dev_handle, u_int8_t brightness)
{
	if(brightness > 100)
		return RS_INVALID_PARAM;
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle,CLNE_IA_CTRL, GX_LED_LIGHT_OR_DARK,
		BUILD_U32(0, brightness, 0, 0));
}

CLIB_API RS cl_lamp_ctrl_warmness(cl_handle_t dev_handle, u_int8_t warmness)
{
	if(warmness > 100)
		return RS_INVALID_PARAM;
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle,CLNE_IA_CTRL, GX_LED_WHITE_OR_WARM,
		BUILD_U32(0, warmness, 0, 0));
}

CLIB_API RS cl_lamp_ctrl_rgb(cl_handle_t dev_handle, u_int8_t r, u_int8_t g, u_int8_t b)
{
	u_int32_t rgb;

	rgb = BUILD_U32(0, r, g, b);
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle,CLNE_IA_CTRL, GX_LED_RGB, rgb);
	
}

CLIB_API RS cl_lamp_ctrl_auto(cl_handle_t dev_handle)
{
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle,CLNE_IA_CTRL, GX_LED_AUTO, 0);
}

CLIB_API RS cl_lamp_ctrl_energy_saving(cl_handle_t dev_handle)
{
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle,CLNE_IA_CTRL, GX_LED_SAVE, 0);
}

CLIB_API RS cl_lamp_ctrl_night(cl_handle_t dev_handle, u_int8_t is_on)
{
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle,CLNE_IA_CTRL, GX_LED_NIGHT, 
		BUILD_U32(0, is_on, 0, 0));
}

CLIB_API RS cl_lamp_ctrl_model1(cl_handle_t dev_handle)
{
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle,CLNE_IA_CTRL, GX_LED_MODEL1, 0);
}

CLIB_API RS cl_lamp_ctrl_model2(cl_handle_t dev_handle)
{
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle,CLNE_IA_CTRL, GX_LED_MODEL2, 0);
}

CLIB_API RS cl_lamp_query_all(cl_handle_t dev_handle)
{
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle,CLNE_IA_CTRL, GX_LED_QUERY_ALL, 0);
}

