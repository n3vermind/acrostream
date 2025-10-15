#pragma once

#include <sys/types.h>

#include "mi_vif.h"
#include "mi_sys.h"

static const int32_t SENSOR_PAD = 0;

typedef struct {
  mi_sys_window_rect crop;
  mi_sys_window_size size;
  u_int32_t max_fps;
  u_int32_t min_fps;
  int8_t desc[32];
} __attribute__((packed, aligned(4))) mi_snr_res;

typedef struct {
  mi_vif_sync_attr sync_attr; 
} mi_snr_attr_parallel;

typedef enum {
  E_MI_SNR_HDR_HW_MODE_NONE,
  E_MI_SNR_HDR_HW_MODE_SONY_DOL,
  E_MI_SNR_HDR_HW_MODE_DCG,
  E_MI_SNR_HDR_HW_MODE_EMBEDDED_RAW8,
  E_MI_SNR_HDR_HW_MODE_EMBEDDED_RAW10,
  E_MI_SNR_HDR_HW_MODE_EMBEDDED_RAW12,
  E_MI_SNR_HDR_HW_MODE_EMBEDDED_RAW14,
  E_MI_SNR_HDR_HW_MODE_EMBEDDED_RAW16,
} mi_snr_hdr_hw_mode;

typedef enum {
  E_MI_SNR_HDR_SOURCE_VC0,
  E_MI_SNR_HDR_SOURCE_VC1,
  E_MI_SNR_HDR_SOURCE_VC2,
  E_MI_SNR_HDR_SOURCE_VC3,
  E_MI_SNR_HDR_SOURCE_MAX,
} mi_snr_hdr_src;

typedef struct {
  u_int32_t lane_num;
  u_int32_t data_format; // 0: YUV 422. 1: RGB
  mi_vif_data_yuv_seq data_yuv_order;
  u_int32_t hsync_mode;
  u_int32_t samplling_delay;
  mi_snr_hdr_hw_mode hdr_hw_mode;
  u_int32_t hdr_virchn_num;
  u_int32_t long_packet_type[2];
} mi_snr_attr_mipi;

typedef struct {
  u_int32_t multiplex_num;
  mi_vif_sync_attr sync_attr;
  mi_vif_clk_edge clk_edge;
  mi_vif_bit_order bit_order;
} mi_snr_attr_bt656;

typedef union {
  mi_snr_attr_parallel parallel_attr;
  mi_snr_attr_mipi mipi_attr;
  mi_snr_attr_bt656 bt656_attr;
} mi_snr_intf_attr;

typedef struct {
  u_int32_t planeCount;
  mi_vif_intf_mode intf_mode;
  mi_vif_hdr_type hdr_mode;
  mi_snr_intf_attr intf_attr;
  int8_t early_init;
} mi_snr_pad_info;

typedef struct {
  u_int32_t plane_id;
  char sensor_name[32];
  mi_sys_window_rect cap_rect;
  mi_sys_bayer_id bayer_id;
  mi_sys_data_precision pix_precision;
  mi_snr_hdr_src hdr_src;
  u_int32_t shutter_us;
  u_int32_t sensor_gain; // * 1024
  u_int32_t comp_gain;
  mi_sys_pixel_format pixel_format;
} mi_snr_plane_info;

typedef struct {
  u_int32_t (*enable)(int32_t pad_id);
  u_int32_t (*disable)(int32_t pad_id);
  u_int32_t (*get_plane_info)(int32_t pad_id, u_int32_t plane_id, mi_snr_plane_info *info);
  u_int32_t (*set_plane_mode)(int32_t pad_id, u_int8_t enable);
  u_int32_t (*query_res_count)(int32_t pad_id, u_int32_t *count);
  u_int32_t (*get_res)(int32_t pad_id, u_int8_t idx, mi_snr_res *res);
  u_int32_t (*set_res)(int32_t pad_id, u_int8_t idx);
  u_int32_t (*cust_function)(int32_t pad_id, u_int32_t cmd_id, u_int32_t data_size, void *cust_data, u_int8_t dir);
  u_int32_t (*set_fps)(int32_t pad_id, u_int32_t fps);
  u_int32_t (*get_pad_info)(int32_t pad_id, mi_snr_pad_info *info);
} mi_snr_impl;
