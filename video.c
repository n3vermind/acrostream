#include "video.h"

#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

#include "log.h"
#include "mi_isp.h"
#include "mi_lib.h"
#include "mi_snr.h"
#include "mi_sys.h"
#include "mi_venc.h"
#include "mi_vif.h"
#include "mi_vpe.h"
#include "rtp.h"

int setup_pipeline(config *cfg, mi_lib *lib, u_int32_t *venc_fd) {
  if (lib->mi_sys.init()) {
    error("failed to init mi_sys");
    return -1;
  }

  // Magic. Without this call setting fps on the sensor fails.
  u_int32_t what = 0x80300a;
  if (lib->mi_snr.cust_function(SENSOR_PAD, 0x23, 4, &what, 0)) {
    error("failed to set the custom function");
  }

  if (lib->mi_snr.set_plane_mode(SENSOR_PAD, 0)) {
    error("failed to set plane mode");
    return -1;
  }

  u_int32_t res_count;
  if (lib->mi_snr.query_res_count(SENSOR_PAD, &res_count)) {
    error("failed to query res count");
    return -1;
  }


  u_int32_t found_idx;
  mi_snr_res res;
  for (found_idx=0; found_idx<res_count; found_idx++) {
    if (lib->mi_snr.get_res(SENSOR_PAD, found_idx, &res)) {
      error("failed to get res");
      return -1;
    }
    if (res.size.width == WIDTH && res.size.height == HEIGHT && res.max_fps == FPS) {
      break;
    }
  }

  if (found_idx == res_count) {
    error("failed to find the resolution");
    return -1;
  }

  printf("resolution is %dx%d, FPS:(%d,%d)\n", res.size.width, res.size.height, res.min_fps, res.max_fps);

  if (lib->mi_snr.set_res(SENSOR_PAD, found_idx)) {
    error("failed to set resolution");
    return -1;
  }

  if (lib->mi_snr.set_fps(SENSOR_PAD, FPS)) {
    error("failed to set fps");
    return -1;
  }
  
  mi_snr_pad_info pad_info;
  if (lib->mi_snr.get_pad_info(SENSOR_PAD, &pad_info)) {
    error("failed to get pad info");
    return -1;
  }

  printf("pad info: intf=%d, hdr_mode=%d, early_init=%d\n", pad_info.intf_mode, pad_info.hdr_mode, pad_info.early_init);

  mi_snr_plane_info plane_info;
  if (lib->mi_snr.get_plane_info(SENSOR_PAD, 0, &plane_info)) {
    error("failed to get plane info");
    return -1;
  }

  printf("plane info: name=%s bayer_id=%d, pix_precision=%d, hdr_src=%d, shutter=%d, sensor_gain=%d, comp_gain=%d\n", plane_info.sensor_name, plane_info.bayer_id, plane_info.pix_precision, plane_info.hdr_src, plane_info.shutter_us, plane_info.sensor_gain, plane_info.comp_gain);


  if (lib->mi_snr.enable(SENSOR_PAD)) {
    error("failed to enable the sensor");
    return -1;
  }

  // Done initializing the sensor

  mi_vif_init_param vif_dev_init_param = {0};
  if (lib->mi_vif.init_dev(&vif_dev_init_param)) {
    error("failed to initialize the vif device");
    return -1;
  }

  mi_vif_dev_attr dev_attr = {0};
  dev_attr.intf_mode = pad_info.intf_mode;
  dev_attr.work_mode = E_MI_VIF_WORK_MODE_RGB_REALTIME;
  dev_attr.hdr_type = E_MI_VIF_HDR_TYPE_OFF;
  dev_attr.clk_edge = E_MI_VIF_CLK_EDGE_DOUBLE;
  dev_attr.data_seq = pad_info.intf_attr.mipi_attr.data_yuv_order;
  dev_attr.bit_order = E_MI_VIF_BITORDER_NORMAL;

  if (lib->mi_vif.set_dev_attr(VIF_DEV_ID, &dev_attr)) {
    error("failed to set the vif device attributes");
    return -1;
  }

  if (lib->mi_vif.enable_dev(VIF_DEV_ID)) {
    error("failed to enable the vif device");
    return -1;
  }

  mi_vif_chn_port_attr dev_chn_port_attr = {0};
  dev_chn_port_attr.cap_rect = plane_info.cap_rect;
  dev_chn_port_attr.dest_size.width = plane_info.cap_rect.width;
  dev_chn_port_attr.dest_size.height = plane_info.cap_rect.height;
  dev_chn_port_attr.pix_format = E_MI_SYS_PIXEL_FRAME_RGB_BAYER_BASE + plane_info.pix_precision * E_MI_SYS_PIXEL_BAYERID_MAX + plane_info.bayer_id;
  dev_chn_port_attr.framerate = E_MI_VIF_FRAMERATE_FULL;

  if (lib->mi_vif.set_chn_port_attr(VIF_CHANNEL_ID, VIF_PORT_ID, &dev_chn_port_attr)) {
    error("failed to set channel/port attributes");
    return -1;
  }

  if (lib->mi_vif.enable_chn_port(VIF_CHANNEL_ID, VIF_PORT_ID)) {
    error("failed to enable channel/port");
    return -1;
  }

  mi_vpe_init_param vpe_init_param = {0};
  if (lib->mi_vpe.init_dev(&vpe_init_param)) {
    error("failed to init vpe");
    return -1;
  }

  mi_vpe_channel_attr vpe_channel_attr = {0};
  vpe_channel_attr.capt.width = plane_info.cap_rect.width;
  vpe_channel_attr.capt.height = plane_info.cap_rect.height;
  vpe_channel_attr.pixel_format = dev_chn_port_attr.pix_format;
  vpe_channel_attr.hdr = E_MI_VPE_HDR_TYPE_OFF;
  vpe_channel_attr.sensor_bind_id = SENSOR_PAD + 1;
  vpe_channel_attr.running_mode = E_MI_VPE_RUN_REALTIME_MODE;
  if (lib->mi_vpe.create_channel(VPE_CHANNEL_ID, &vpe_channel_attr)) {
    error("failed to create vpe channel");
    return -1;
  }
  
  mi_vpe_channel_param vpe_channel_param = {0};
  if (lib->mi_vpe.get_channel_param(VPE_CHANNEL_ID, &vpe_channel_param)) {
    error("failed to get vpe channel params");
    return -1;
  }

  vpe_channel_param.hdr_type = E_MI_VPE_HDR_TYPE_OFF;
  vpe_channel_param.noise = E_MI_VPE_3DNR_LEVEL_OFF;

  if (lib->mi_vpe.set_channel_param(VPE_CHANNEL_ID, &vpe_channel_param)) {
    error("failed to set vpe channel params");
    return -1;
  }

  if (lib->mi_vpe.start_channel(VPE_CHANNEL_ID)) {
    error("failed to start vpe channel");
    return -1;
  }

  mi_vpe_port_mode vpe_port_mode = {
    .out = {
      .width = plane_info.cap_rect.width,
      .height = plane_info.cap_rect.height,
    },
    .mirror = 0,
    .flip = 0,
    .pixel_format = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420,
    .compress_mode = E_MI_SYS_COMPRESS_MODE_NONE,
  };
  if (lib->mi_vpe.set_port_mode(VPE_CHANNEL_ID, VPE_PORT_ID, &vpe_port_mode)) {
    error("failed to set vpe port mode");
    return -1;
  }

  if (lib->mi_vpe.enable_port(VPE_CHANNEL_ID, VPE_PORT_ID)) {
    error("failed to enable vpe port");
    return -1;
  }


  mi_sys_chn_port vif_port = {
    .module_id = E_MI_MODULE_ID_VIF,
    .dev_id = VIF_DEV_ID,
    .chn_id = VIF_CHANNEL_ID,
    .port_id = VIF_PORT_ID,
  };
  mi_sys_chn_port vpe_port = {
    .module_id = E_MI_MODULE_ID_VPE,
    .dev_id = VPE_DEV_ID,
    .chn_id = VPE_CHANNEL_ID,
    .port_id = VPE_CHANNEL_ID,
  };

  if (lib->mi_sys.bind_chn_port_2(&vif_port, &vpe_port, FPS, FPS, E_MI_SYS_BIND_TYPE_REALTIME, 0)) {
    error("failed to bind vif to vpe");
    return -1;
  }


  mi_venc_channel venc_channel = {
    .ve_attr = {
      .mode = E_MI_VENC_MODE_H265,
      .h265 = {
        .max_width = plane_info.cap_rect.width,
        .max_height = plane_info.cap_rect.height,
        .buf_size = plane_info.cap_rect.width * plane_info.cap_rect.height,
        .profile = 2,
        .by_frame = 1,
        .width = plane_info.cap_rect.width,
        .height = plane_info.cap_rect.height,
        .frame_num = 0,
        .ref_num = 1,
      },
    },
    .rc_attr = {
      .mode = E_MI_VENC_RC_MODE_H265CBR,
      .h265_cbr = {
        .gop = cfg->gop,
        .stat_time = 1,
        .src_frame_rate_num = FPS,
        .src_frame_rate_den = 1,
        .bitrate = cfg->bitrate,
        .fluctuate = 1,
      },
    },
  };

  if (lib->mi_venc.create_channel(0, &venc_channel)) {
    error("failed to create the venc channel");
    return -1;
  }

  mi_sys_chn_port venc_port = {
    .module_id = E_MI_MODULE_ID_VENC,
    .dev_id = VENC_DEV_ID,
    .chn_id = VENC_CHANNEL_ID,
    .port_id = VENC_PORT_ID,
  };

  if (lib->mi_sys.bind_chn_port_2(&vpe_port, &venc_port, FPS, FPS, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0)) {
    error("failed to bind vpe to venc");
    return -1;
  }

  if (cfg->slice_split) {
    mi_venc_h265_slice_split_attr split_attr = {
      .enable = 1,
      .slice_row_count = cfg->slice_split,
    };
    if (lib->mi_venc.set_h265_slice_split(VENC_CHANNEL_ID, &split_attr)) {
      error("failed to set the slice split on the venc channel");
    }
  }

  if (cfg->intra_refresh) {
    mi_venc_intra_refresh_attr refresh_attr = {
      .enable = 1,
      .refresh_line_num = cfg->intra_refresh,
      .req_iq = 1,
    };
    if (lib->mi_venc.set_intra_refresh(VENC_CHANNEL_ID, &refresh_attr)) {
      error("failed to set intra refresh parameters on venc");
      return -1;
    }
  }

  mi_venc_rc_param rc_param = {0};
  if (lib->mi_venc.get_rc_param(VENC_CHANNEL_ID, &rc_param)) {
    error("failed to get the rc param");
    return -1;
  }

  rc_param.h265_cbr.max_ip_prop = 1;

  if (lib->mi_venc.set_rc_param(VENC_CHANNEL_ID, &rc_param)) {
    error("failed to set the rc param");
    return -1;
  }

  if (lib->mi_venc.start_recv_pic(VENC_CHANNEL_ID)) {
    error("failed to start receiving from the venc channel");
    return -1;
  }

  *venc_fd = lib->mi_venc.get_fd(VENC_CHANNEL_ID);

  // Have to wait for the isp to initialize :/
  sleep(1);

  if (lib->mi_isp.load_config(ISP_CHANNEL_ID, "/etc/sensors/imx415_fpv.bin", 0x4d2)) {
    error("failed to set the isp config");
    return -1;
  }

  mi_isp_ae_exposure_limit expo_limit = {0};
  if (lib->mi_isp.get_exposure_limit(ISP_CHANNEL_ID, &expo_limit)) {
    error("failed to get the exposure limit");
    return -1;
  }

  expo_limit.max_shutter_us = 1000000 / FPS;
  if (lib->mi_isp.set_exposure_limit(ISP_CHANNEL_ID, &expo_limit)) {
    error("failed to set the exposure limit");
    return -1;
  }
  printf("EXPO: Shutter (%d ,%d)\n FN (%d, %d)\n Sensor (%d, %d)\n ISP (%d %d)\n", expo_limit.min_shutter_us, expo_limit.max_shutter_us, expo_limit.min_fn, expo_limit.max_fn, expo_limit.min_sensor_gain, expo_limit.max_sensor_gain, expo_limit.min_isp_gain, expo_limit.max_isp_gain);

  return 0;
}

int cleanup_pipeline(mi_lib *lib) {
  if (lib->mi_venc.stop_recv_pic(VENC_CHANNEL_ID)) {
    error("failed to stop receiving from the venc channel");
  }

  if (lib->mi_venc.destroy_channel(VENC_CHANNEL_ID)) {
    error("failed to destroy the venc channel");
  }

  if (lib->mi_vpe.stop_channel(VPE_CHANNEL_ID)) {
    error("failed to stop vpe channel");
    return -1;
  }

  if (lib->mi_vpe.disable_port(VPE_CHANNEL_ID, VPE_PORT_ID)) {
    error("failed to disable vpe port");
    return -1;
  }
  
  if (lib->mi_vpe.destroy_channel(VPE_CHANNEL_ID)) {
    error("failed to destroy vpe channel");
    return -1;
  }

  if (lib->mi_vpe.deinit_dev()) {
    error("failed to deinit vpe");
    return -1;
  }

  if (lib->mi_vif.disable_chn_port(VIF_DEV_ID, VIF_PORT_ID)) {
    error("failed to disable channel/port");
    return -1;
  }

  if (lib->mi_vif.disable_dev(VIF_DEV_ID)) {
    error("failed to disable the vif device");
    return -1;
  }

  if (lib->mi_vif.deinit_dev()) {
    error("failed to deinit the vif device");
    return -1;
  }

  if (lib->mi_snr.disable(SENSOR_PAD)) {
    error("failed to disable the sensor");
    return -1;
  }

  return 0;
}

int process_frame(config *cfg, mi_lib *lib, rtp_connection *conn, u_int32_t venc_fd) {
  fd_set read_fds;
  FD_ZERO(&read_fds);
  FD_SET(venc_fd, &read_fds);

  struct timeval tv = {
    .tv_sec = 1,
    .tv_usec = 0,
  };

  int ret = select(venc_fd+1, &read_fds, NULL, NULL, &tv);
  if (ret < 0) {
    error("failed to select on the venc fd");
    return -1;
  } else if (ret == 0) {
    error("nothing to read from the venc fd");
    return 0;
  }

  mi_venc_channel_stat stat = {0};
  if (lib->mi_venc.query(VENC_CHANNEL_ID, &stat)) {
    error("failed to query the venc channel");
    return -1;
  }

  if (stat.cur_packs == 0) {
    return 0;
  }

  mi_venc_stream stream;
  stream.packet = malloc(sizeof(mi_venc_packet) * stat.cur_packs);
  if (!stream.packet) {
    error("failed to allocate memory for the packet");
    return -1;
  }
  stream.count = stat.cur_packs;

  if (lib->mi_venc.get_stream(VENC_CHANNEL_ID, &stream, 100)) {
    free(stream.packet);

    error("failed to get the venc stream");
    return -1;
  }

  if (send_rtp(cfg, conn, &stream)) {
    error("failed to send the rtp packets");
  }

  if (lib->mi_venc.release_stream(VENC_CHANNEL_ID, &stream)) {
    error("failed to release the venc stream");
    return -1;
  }

  free(stream.packet);

  return 0;
}
