#include "mi_lib.h"

#include "symbols.h"

int mi_create(mi_lib *lib) {
  if (mi_sys_load(&lib->mi_sys)) {
    error("failed to load mi_sys");
    return -1;
  }

  if (mi_isp_load(&lib->mi_isp)) {
    error("failed to load mi_isp");
    return -1;
  }

  if (mi_snr_load(&lib->mi_snr)) {
    error("failed to load mi_snr");
    return -1;
  }

  if (mi_vif_load(&lib->mi_vif)) {
    error("failed to load mi_vif");
    return -1;
  }

  if (mi_vpe_load(&lib->mi_vpe)) {
    error("failed to load mi_vpe");
    return -1;
  }

  if (mi_venc_load(&lib->mi_venc)) {
    error("failed to load mi_venc");
    return -1;
  }
  return 0;
}

int mi_isp_load(mi_isp_impl *impl) {
  // Load the libs to allow transient dependencies.
  dlopen("libcam_os_wrapper.so", RTLD_LAZY | RTLD_GLOBAL);
  dlopen("libispalgo.so", RTLD_LAZY | RTLD_GLOBAL);
  dlopen("libcus3a.so", RTLD_LAZY | RTLD_GLOBAL);
  void *handle = dlopen("libmi_isp.so", RTLD_LAZY | RTLD_GLOBAL);
  if (!(impl->load_config = symbol_load(handle, "MI_ISP_API_CmdLoadBinFile"))) {
    return -1;
  }
  if (!(impl->get_exposure_limit = symbol_load(handle, "MI_ISP_AE_GetExposureLimit"))) {
    return -1;
  }
  if (!(impl->set_exposure_limit = symbol_load(handle, "MI_ISP_AE_SetExposureLimit"))) {
    return -1;
  }
  return 0;
}

int mi_sys_load(mi_sys_impl *impl) {
  void *handle = dlopen("libmi_sys.so", RTLD_LAZY | RTLD_GLOBAL);
  if (!(impl->init = symbol_load(handle, "MI_SYS_Init"))) {
      return -1;
  }
  if (!(impl->bind_chn_port_2 = symbol_load(handle, "MI_SYS_BindChnPort2"))) {
      return -1;
  }
  return 0;
}

int mi_snr_load(mi_snr_impl *impl) {
  void* handle = dlopen("libmi_sensor.so", RTLD_LAZY | RTLD_GLOBAL);
  if (!(impl->enable = symbol_load(handle, "MI_SNR_Enable"))) {
    return -1;
  }
  if (!(impl->disable = symbol_load(handle, "MI_SNR_Enable"))) {
    return -1;
  }
  if (!(impl->get_plane_info = symbol_load(handle, "MI_SNR_GetPlaneInfo"))) {
    return -1;
  }
  if (!(impl->set_plane_mode = symbol_load(handle, "MI_SNR_SetPlaneMode"))) {
    return -1;
  }
  if (!(impl->query_res_count = symbol_load(handle, "MI_SNR_QueryResCount"))) {
    return -1;
  }
  if (!(impl->get_res = symbol_load(handle, "MI_SNR_GetRes"))) {
    return -1;
  }
  if (!(impl->set_res = symbol_load(handle, "MI_SNR_SetRes"))) {
    return -1;
  }
  if (!(impl->cust_function = symbol_load(handle, "MI_SNR_CustFunction"))) {
    return -1;
  }
  if (!(impl->set_fps = symbol_load(handle, "MI_SNR_SetFps"))) {
    return -1;
  }
  if (!(impl->get_pad_info = symbol_load(handle, "MI_SNR_GetPadInfo"))) {
    return -1;
  }
  return 0;
}

int mi_vif_load(mi_vif_impl *impl) {
  void* handle = dlopen("libmi_vif.so", RTLD_LAZY | RTLD_GLOBAL);
  if (!(impl->init_dev = symbol_load(handle, "MI_VIF_InitDev"))) {
    return -1;
  }
  if (!(impl->deinit_dev = symbol_load(handle, "MI_VIF_DeInitDev"))) {
    return -1;
  }
  if (!(impl->set_dev_attr = symbol_load(handle, "MI_VIF_SetDevAttr"))) {
    return -1;
  }
  if (!(impl->enable_dev = symbol_load(handle, "MI_VIF_EnableDev"))) {
    return -1;
  }
  if (!(impl->disable_dev = symbol_load(handle, "MI_VIF_DisableDev"))) {
    return -1;
  }
  if (!(impl->set_chn_port_attr = symbol_load(handle, "MI_VIF_SetChnPortAttr"))) {
    return -1;
  }
  if (!(impl->enable_chn_port = symbol_load(handle, "MI_VIF_EnableChnPort"))) {
    return -1;
  }
  if (!(impl->disable_chn_port = symbol_load(handle, "MI_VIF_DisableChnPort"))) {
    return -1;
  }
  return 0;
}

int mi_vpe_load(mi_vpe_impl *impl) {
  void* handle = dlopen("libmi_vpe.so", RTLD_LAZY | RTLD_GLOBAL);
  if (!(impl->init_dev = symbol_load(handle, "MI_VPE_InitDev"))) {
    return -1;
  }
  if (!(impl->deinit_dev = symbol_load(handle, "MI_VPE_DeInitDev"))) {
    return -1;
  }
  if (!(impl->create_channel = symbol_load(handle, "MI_VPE_CreateChannel"))) {
    return -1;
  }
  if (!(impl->destroy_channel = symbol_load(handle, "MI_VPE_DestroyChannel"))) {
    return -1;
  }
  if (!(impl->get_channel_param = symbol_load(handle, "MI_VPE_GetChannelParam"))) {
    return -1;
  }
  if (!(impl->set_channel_param = symbol_load(handle, "MI_VPE_SetChannelParam"))) {
    return -1;
  }
  if (!(impl->start_channel = symbol_load(handle, "MI_VPE_StartChannel"))) {
    return -1;
  }
  if (!(impl->stop_channel = symbol_load(handle, "MI_VPE_StopChannel"))) {
    return -1;
  }
  if (!(impl->set_port_mode = symbol_load(handle, "MI_VPE_SetPortMode"))) {
    return -1;
  }
  if (!(impl->enable_port = symbol_load(handle, "MI_VPE_EnablePort"))) {
    return -1;
  }
  if (!(impl->disable_port = symbol_load(handle, "MI_VPE_DisablePort"))) {
    return -1;
  }
  return 0;
}

int mi_venc_load(mi_venc_impl *impl) {
  void *handle = dlopen("libmi_venc.so", RTLD_LAZY | RTLD_GLOBAL);
  if (!(impl->create_channel = symbol_load(handle, "MI_VENC_CreateChn"))) {
    return -1;
  }
  if (!(impl->destroy_channel = symbol_load(handle, "MI_VENC_DestroyChn"))) {
    return -1;
  }
  if (!(impl->start_recv_pic = symbol_load(handle, "MI_VENC_StartRecvPic"))) {
    return -1;
  }
  if (!(impl->stop_recv_pic = symbol_load(handle, "MI_VENC_StopRecvPic"))) {
    return -1;
  }
  if (!(impl->get_fd = symbol_load(handle, "MI_VENC_GetFd"))) {
    return -1;
  }
  if (!(impl->query = symbol_load(handle, "MI_VENC_Query"))) {
    return -1;
  }
  if (!(impl->get_stream = symbol_load(handle, "MI_VENC_GetStream"))) {
    return -1;
  }
  if (!(impl->release_stream = symbol_load(handle, "MI_VENC_ReleaseStream"))) {
    return -1;
  }
  if (!(impl->set_h265_slice_split = symbol_load(handle, "MI_VENC_SetH265SliceSplit"))) {
    return -1;
  }
  if (!(impl->set_intra_refresh = symbol_load(handle, "MI_VENC_SetIntraRefresh"))) {
    return -1;
  }
  if (!(impl->get_rc_param = symbol_load(handle, "MI_VENC_GetRcParam"))) {
    return -1;
  }
  if (!(impl->set_rc_param = symbol_load(handle, "MI_VENC_SetRcParam"))) {
    return -1;
  }
  return 0;
}
