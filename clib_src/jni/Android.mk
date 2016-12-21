LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := clib

SRC_DIR := ../../src

LOCAL_C_INCLUDES += $(SRC_DIR)/../inc  $(SRC_DIR) $(SRC_DIR)/aes $(SRC_DIR)/h264 \
	 $(SRC_DIR)/ilbc  $(SRC_DIR)/ilbc/ilbc $(SRC_DIR)/ilbc/ilbc/interface \
	 $(SRC_DIR)/ilbc/signal_processing  $(SRC_DIR)/ilbc/signal_processing/include 
	 
LOCAL_CFLAGS += -DANDROID -DCLIB_HDR -DNO_COMMUNITY -O2 -Wimplicit#-Wall#-Werror #-DMUT_SERVER_ADAPT #-DAWS_SERVER 

LOCAL_SRC_FILES := \
	$(SRC_DIR)/misc_client.c \
	$(SRC_DIR)/app_pkt_priv.c \
	$(SRC_DIR)/uc_agent.c \
	$(SRC_DIR)/yinsu_scm_ctrl.c \
	$(SRC_DIR)/leis_scm_ctrl.c \
	$(SRC_DIR)/lanusers_priv.c \
	$(SRC_DIR)/evm_drkzq_ctrl.c \
	$(SRC_DIR)/rfgw_scm_ctrl.c \
	$(SRC_DIR)/evm_cjthermostat_ctrl.c \
	$(SRC_DIR)/cl_debug_agent.c \
	$(SRC_DIR)/ica_priv.c\
	$(SRC_DIR)/evm_hythermostat_ctrl.c \
	$(SRC_DIR)/evm_jrxheater_ctrl.c \
	$(SRC_DIR)/evm_bpuair_ctrl.c \
	$(SRC_DIR)/aes/tea.c \
	$(SRC_DIR)/evm_zkcleanner_ctrl.c \
	$(SRC_DIR)/cl_linkage.c \
	$(SRC_DIR)/linkage_priv.c \
	$(SRC_DIR)/linkage_client.c \
	$(SRC_DIR)/zkrsq_priv.c \
	$(SRC_DIR)/evm_tt_ctrl.c \
	$(SRC_DIR)/evm_priv.c \
	$(SRC_DIR)/yj_heater_scm_ctrl.c \
	$(SRC_DIR)/evm_scm_ctrl.c \
	$(SRC_DIR)/evm_yuyuan_ctrl.c \
	$(SRC_DIR)/zssx_priv.c \
	$(SRC_DIR)/sbt_ther_scm_ctrl.c \
	$(SRC_DIR)/kxm_scm_ctrl.c \
	$(SRC_DIR)/yt_priv.c \
	$(SRC_DIR)/tbb_priv.c \
	$(SRC_DIR)/bimar_scm_ctrl.c \
	$(SRC_DIR)/xy_priv.c \
	$(SRC_DIR)/udp_rf_dev.c \
	$(SRC_DIR)/dev_stat_priv.c \
	$(SRC_DIR)/cl_ia.c \
	$(SRC_DIR)/cl_lamp.c \
	$(SRC_DIR)/cl_smart_appliance.c \
	$(SRC_DIR)/smart_appliance_priv.c \
	$(SRC_DIR)/cl_health.c \
	$(SRC_DIR)/cl_cloud_ac.c \
	$(SRC_DIR)/cl_eb.c \
	$(SRC_DIR)/cl_ch_blanket.c \
	$(SRC_DIR)/cl_common_udp_device.c \
	$(SRC_DIR)/udp_device_common_priv.c \
	$(SRC_DIR)/amt_scm_ctrl.c \
	$(SRC_DIR)/chiffo_scm_ctrl.c \
	$(SRC_DIR)/ia_priv.c \
	$(SRC_DIR)/audio_priv.c \
	$(SRC_DIR)/intelligent_forward_priv.c \
	$(SRC_DIR)/udp_scm_direct_ctrl_priv.c \
	$(SRC_DIR)/lc_scm_ctrl.c \
	$(SRC_DIR)/cl_intelligent_forward.c \
	$(SRC_DIR)/cloud_ac_priv.c \
	$(SRC_DIR)/buffer.c \
	$(SRC_DIR)/cl_area.c \
	$(SRC_DIR)/cl_scene.c \
	$(SRC_DIR)/cl_lan_dev_probe.c \
	$(SRC_DIR)/cl_rfgw.c \
	$(SRC_DIR)/rfgw_priv.c \
	$(SRC_DIR)/cl_dns.c \
	$(SRC_DIR)/cl_equipment.c \
	$(SRC_DIR)/cl_notify_push.c \
	$(SRC_DIR)/cl_linux.c \
	$(SRC_DIR)/cl_log.c \
	$(SRC_DIR)/cl_main.c \
	$(SRC_DIR)/cl_mem.c \
	$(SRC_DIR)/cl_notify.c \
	$(SRC_DIR)/cl_priv.c \
	$(SRC_DIR)/cl_plug.c \
	$(SRC_DIR)/cl_server.c \
	$(SRC_DIR)/cl_thread.c \
	$(SRC_DIR)/cl_user.c \
	$(SRC_DIR)/cl_video.c \
	$(SRC_DIR)/client_lib.c \
	$(SRC_DIR)/cmd_misc.c \
	$(SRC_DIR)/data_exchg.c \
	$(SRC_DIR)/env_mon_priv.c \
	$(SRC_DIR)/equipment_priv.c \
	$(SRC_DIR)/h264_decode.c \
	$(SRC_DIR)/health_priv.c \
	$(SRC_DIR)/lc_furnace_priv.c \
	$(SRC_DIR)/cl_lc_furnace.c \
	$(SRC_DIR)/lbs.c \
	$(SRC_DIR)/lookup.c \
	$(SRC_DIR)/md5.c \
	$(SRC_DIR)/mp4_dec.c \
	$(SRC_DIR)/mp4_enc.c \
	$(SRC_DIR)/net_detect.c \
	$(SRC_DIR)/notify_push_priv.c \
	$(SRC_DIR)/pic_ring.c \
	$(SRC_DIR)/plug_priv.c \
	$(SRC_DIR)/sps.c \
	$(SRC_DIR)/user_priv.c \
	$(SRC_DIR)/video_priv.c \
	$(SRC_DIR)/video_record_timer.c \
	$(SRC_DIR)/video_try.c \
	$(SRC_DIR)/wait_server.c \
	$(SRC_DIR)/area_priv.c \
	$(SRC_DIR)/scene_priv.c \
	$(SRC_DIR)/lan_dev_probe_priv.c \
	$(SRC_DIR)/phone_user.c \
	$(SRC_DIR)/bindphone_priv.c \
	$(SRC_DIR)/http.c \
	$(SRC_DIR)/json.c \
	$(SRC_DIR)/ioext.c \
	$(SRC_DIR)/wget.c \
	$(SRC_DIR)/jpg.c \
	$(SRC_DIR)/smart_config_priv.c \
	$(SRC_DIR)/uc_client.c \
	$(SRC_DIR)/uas_client.c \
	$(SRC_DIR)/uasc_priv.c \
	$(SRC_DIR)/udp_ctrl.c \
	$(SRC_DIR)/eb_priv.c \
	$(SRC_DIR)/aes/aes.c \
	$(SRC_DIR)/cl_rfgw.c \
	$(SRC_DIR)/rfgw_priv.c \
	$(SRC_DIR)/qpcp_priv.c \
	$(SRC_DIR)/zh_jl_lamp_ctrl.c \
	$(SRC_DIR)/h264/allcodecs.c \
	$(SRC_DIR)/h264/bitstream.c \
	$(SRC_DIR)/h264/cabac.c \
	$(SRC_DIR)/h264/dsputil.c \
	$(SRC_DIR)/h264/error_resilience.c \
	$(SRC_DIR)/h264/golomb.c \
	$(SRC_DIR)/h264/h264.c \
	$(SRC_DIR)/h264/h264_parser.c \
	$(SRC_DIR)/h264/h264dspenc.c \
	$(SRC_DIR)/h264/h264idct.c \
	$(SRC_DIR)/h264/h264pred.c \
	$(SRC_DIR)/h264/imgconvert.c \
	$(SRC_DIR)/h264/jrevdct.c \
	$(SRC_DIR)/h264/log.c \
	$(SRC_DIR)/h264/mem.c \
	$(SRC_DIR)/h264/mini264lib.c \
	$(SRC_DIR)/h264/mpegvideo.c \
	$(SRC_DIR)/h264/opt.c \
	$(SRC_DIR)/h264/osdep.c \
	$(SRC_DIR)/h264/parser.c \
	$(SRC_DIR)/h264/rational.c \
	$(SRC_DIR)/h264/simple_idct.c \
	$(SRC_DIR)/h264/svq3.c \
	$(SRC_DIR)/h264/utils.c \
	$(SRC_DIR)/h264/vp3dsp.c \
	$(SRC_DIR)/h264/yuv2bmp.c \
	$(SRC_DIR)/ilbc/ilbc/abs_quant.c \
	$(SRC_DIR)/ilbc/ilbc/abs_quant_loop.c \
	$(SRC_DIR)/ilbc/ilbc/augmented_cb_corr.c \
	$(SRC_DIR)/ilbc/ilbc/bw_expand.c \
	$(SRC_DIR)/ilbc/ilbc/cb_construct.c \
	$(SRC_DIR)/ilbc/ilbc/cb_mem_energy.c \
	$(SRC_DIR)/ilbc/ilbc/cb_mem_energy_augmentation.c \
	$(SRC_DIR)/ilbc/ilbc/cb_mem_energy_calc.c \
	$(SRC_DIR)/ilbc/ilbc/cb_search.c \
	$(SRC_DIR)/ilbc/ilbc/cb_search_core.c \
	$(SRC_DIR)/ilbc/ilbc/cb_update_best_index.c \
	$(SRC_DIR)/ilbc/ilbc/chebyshev.c \
	$(SRC_DIR)/ilbc/ilbc/comp_corr.c \
	$(SRC_DIR)/ilbc/ilbc/constants.c \
	$(SRC_DIR)/ilbc/ilbc/create_augmented_vec.c \
	$(SRC_DIR)/ilbc/ilbc/decode.c \
	$(SRC_DIR)/ilbc/ilbc/decode_residual.c \
	$(SRC_DIR)/ilbc/ilbc/decoder_interpolate_lsf.c \
	$(SRC_DIR)/ilbc/ilbc/do_plc.c \
	$(SRC_DIR)/ilbc/ilbc/encode.c \
	$(SRC_DIR)/ilbc/ilbc/energy_inverse.c \
	$(SRC_DIR)/ilbc/ilbc/enh_upsample.c \
	$(SRC_DIR)/ilbc/ilbc/enhancer.c \
	$(SRC_DIR)/ilbc/ilbc/enhancer_interface.c \
	$(SRC_DIR)/ilbc/ilbc/filtered_cb_vecs.c \
	$(SRC_DIR)/ilbc/ilbc/frame_classify.c \
	$(SRC_DIR)/ilbc/ilbc/gain_dequant.c \
	$(SRC_DIR)/ilbc/ilbc/gain_quant.c \
	$(SRC_DIR)/ilbc/ilbc/get_cd_vec.c \
	$(SRC_DIR)/ilbc/ilbc/get_lsp_poly.c \
	$(SRC_DIR)/ilbc/ilbc/get_sync_seq.c \
	$(SRC_DIR)/ilbc/ilbc/hp_input.c \
	$(SRC_DIR)/ilbc/ilbc/hp_output.c \
	$(SRC_DIR)/ilbc/ilbc/ilbc.c \
	$(SRC_DIR)/ilbc/ilbc/index_conv_dec.c \
	$(SRC_DIR)/ilbc/ilbc/index_conv_enc.c \
	$(SRC_DIR)/ilbc/ilbc/init_decode.c \
	$(SRC_DIR)/ilbc/ilbc/init_encode.c \
	$(SRC_DIR)/ilbc/ilbc/interpolate.c \
	$(SRC_DIR)/ilbc/ilbc/interpolate_samples.c \
	$(SRC_DIR)/ilbc/ilbc/lpc_encode.c \
	$(SRC_DIR)/ilbc/ilbc/lsf_check.c \
	$(SRC_DIR)/ilbc/ilbc/lsf_interpolate_to_poly_dec.c \
	$(SRC_DIR)/ilbc/ilbc/lsf_interpolate_to_poly_enc.c \
	$(SRC_DIR)/ilbc/ilbc/lsf_to_lsp.c \
	$(SRC_DIR)/ilbc/ilbc/lsf_to_poly.c \
	$(SRC_DIR)/ilbc/ilbc/lsp_to_lsf.c \
	$(SRC_DIR)/ilbc/ilbc/my_corr.c \
	$(SRC_DIR)/ilbc/ilbc/nearest_neighbor.c \
	$(SRC_DIR)/ilbc/ilbc/pack_bits.c \
	$(SRC_DIR)/ilbc/ilbc/poly_to_lsf.c \
	$(SRC_DIR)/ilbc/ilbc/poly_to_lsp.c \
	$(SRC_DIR)/ilbc/ilbc/refiner.c \
	$(SRC_DIR)/ilbc/ilbc/simple_interpolate_lsf.c \
	$(SRC_DIR)/ilbc/ilbc/simple_lpc_analysis.c \
	$(SRC_DIR)/ilbc/ilbc/simple_lsf_dequant.c \
	$(SRC_DIR)/ilbc/ilbc/simple_lsf_quant.c \
	$(SRC_DIR)/ilbc/ilbc/smooth.c \
	$(SRC_DIR)/ilbc/ilbc/smooth_out_data.c \
	$(SRC_DIR)/ilbc/ilbc/sort_sq.c \
	$(SRC_DIR)/ilbc/ilbc/split_vq.c \
	$(SRC_DIR)/ilbc/ilbc/state_construct.c \
	$(SRC_DIR)/ilbc/ilbc/state_search.c \
	$(SRC_DIR)/ilbc/ilbc/swap_bytes.c \
	$(SRC_DIR)/ilbc/ilbc/unpack_bits.c \
	$(SRC_DIR)/ilbc/ilbc/vq3.c \
	$(SRC_DIR)/ilbc/ilbc/vq4.c \
	$(SRC_DIR)/ilbc/ilbc/window32_w32.c \
	$(SRC_DIR)/ilbc/ilbc/xcorr_coef.c \
	$(SRC_DIR)/ilbc/signal_processing/auto_corr_to_refl_coef.c \
	$(SRC_DIR)/ilbc/signal_processing/auto_correlation.c \
	$(SRC_DIR)/ilbc/signal_processing/complex_fft.c \
	$(SRC_DIR)/ilbc/signal_processing/copy_set_operations.c \
	$(SRC_DIR)/ilbc/signal_processing/division_operations.c \
	$(SRC_DIR)/ilbc/signal_processing/dot_product_with_scale.c \
	$(SRC_DIR)/ilbc/signal_processing/energy.c \
	$(SRC_DIR)/ilbc/signal_processing/filter_ar.c \
	$(SRC_DIR)/ilbc/signal_processing/filter_ma_fast_q12.c \
	$(SRC_DIR)/ilbc/signal_processing/get_hanning_window.c \
	$(SRC_DIR)/ilbc/signal_processing/get_scaling_square.c \
	$(SRC_DIR)/ilbc/signal_processing/ilbc_specific_functions.c \
	$(SRC_DIR)/ilbc/signal_processing/levinson_durbin.c \
	$(SRC_DIR)/ilbc/signal_processing/lpc_to_refl_coef.c \
	$(SRC_DIR)/ilbc/signal_processing/min_max_operations.c \
	$(SRC_DIR)/ilbc/signal_processing/randomization_functions.c \
	$(SRC_DIR)/ilbc/signal_processing/refl_coef_to_lpc.c \
	$(SRC_DIR)/ilbc/signal_processing/resample.c \
	$(SRC_DIR)/ilbc/signal_processing/resample_48khz.c \
	$(SRC_DIR)/ilbc/signal_processing/resample_by_2.c \
	$(SRC_DIR)/ilbc/signal_processing/resample_by_2_internal.c \
	$(SRC_DIR)/ilbc/signal_processing/resample_fractional.c \
	$(SRC_DIR)/ilbc/signal_processing/spl_sqrt.c \
	$(SRC_DIR)/ilbc/signal_processing/spl_version.c \
	$(SRC_DIR)/ilbc/signal_processing/splitting_filter.c \
	$(SRC_DIR)/ilbc/signal_processing/sqrt_of_one_minus_x_squared.c \
	$(SRC_DIR)/ilbc/signal_processing/vector_scaling_operations.c \
	$(SRC_DIR)/ilbc/signal_processing/cross_correlation.c \
	$(SRC_DIR)/ilbc/signal_processing/downsample_fast.c \
	$(SRC_DIR)/ilbc/signal_processing/filter_ar_fast_q12.c \
	$(SRC_DIR)/ilbc/signal_processing/complex_bit_reverse.c \
	$(SRC_DIR)/ilbc/signal_processing/spl_sqrt_floor.c \
	

include $(BUILD_STATIC_LIBRARY)


# Build JNI wrapper
include $(CLEAR_VARS)

LOCAL_C_INCLUDES += ../../inc 

LOCAL_MODULE := clib_jni

LOCAL_SRC_FILES := clib_jni.c  \
	clib_jni_user.c  \
	clib_jni_ia.c  \
	clib_jni_video.c \
	clib_jni_plug.c \
	clib_jni_equipment.c \
	clib_jni_area.c \
	clib_jni_scene.c \
	clib_jni_probe.c \
	clib_jni_puser.c \
	clib_jni_jpeg.c\
	clib_jni_bind_phone.c\
	clib_jni_health.c\
	clib_jni_eb.c\
	clib_jni_dev_upgrade.c\
	clib_jni_cmt_notify.c\
	clib_jni_smart_socket_code_match.c \
	clib_jni_lbs.c \
	clib_jni_lc_air_heater.c \
	clib_jni_gx_lamp.c \
	clib_jni_blanket.c \
	clib_jni_udp_dev.c \
	clib_jni_utils.c \
	clib_jni_app_server.c \
	clib_jni_sdk.c \
	clib_jni_linkage.c 

LOCAL_STATIC_LIBRARIES := libclib

# ��Ӷ��og���֧��?
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog -lOpenSLES -lGLESv1_CM -ljnigraphics
#  ������tatic��.a��ֻ�����?LOCAL_LDLIBS:=-llog     

include $(BUILD_SHARED_LIBRARY)
