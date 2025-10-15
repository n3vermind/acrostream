#pragma once

#include "mi_sys.h"
#include <sys/types.h>

static const int32_t VIF_DEV_ID = 0;
static const int32_t VIF_CHANNEL_ID = 0;
static const int32_t VIF_PORT_ID = 0;

typedef enum {
  E_MI_VIF_PIN_POLAR_POS,
  E_MI_VIF_PIN_POLAR_NEG,
} mi_vif_polar;

typedef struct {
  mi_vif_polar vsync_polarity;
  mi_vif_polar hsync_polarity;
  mi_vif_polar pclk_polarity;
  u_int32_t vsync_delay;
  u_int32_t hsync_delay;
  u_int32_t pclk_delay;
} mi_vif_sync_attr;

typedef struct {
  u_int32_t dev_id;
  u_int8_t *data;
} mi_vif_init_param;

typedef enum {
  E_MI_VIF_MODE_BT656,
  E_MI_VIF_MODE_DIGITAL_CAMERA,
  E_MI_VIF_MODE_BT1120_STANDARD,
  E_MI_VIF_MODE_BT1120_INTERLEAVED,
  E_MI_VIF_MODE_MIPI,
  E_MI_VIF_MODE_MAX
} mi_vif_intf_mode;


typedef enum {
  E_MI_VIF_WORK_MODE_1MULTIPLEX,
  E_MI_VIF_WORK_MODE_2MULTIPLEX,
  E_MI_VIF_WORK_MODE_4MULTIPLEX,

  E_MI_VIF_WORK_MODE_RGB_REALTIME,
  E_MI_VIF_WORK_MODE_RGB_FRAMEMODE,
  E_MI_VIF_WORK_MODE_RGB_MAX,
} mi_vif_work_mode;

typedef enum {
  E_MI_VIF_HDR_TYPE_OFF,
  E_MI_VIF_HDR_TYPE_VC, 
  E_MI_VIF_HDR_TYPE_DOL,
  E_MI_VIF_HDR_TYPE_EMBEDDED, 
  E_MI_VIF_HDR_TYPE_LI,
  E_MI_VIF_HDR_TYPE_MAX,
} mi_vif_hdr_type;

typedef enum {
  E_MI_VIF_INPUT_DATA_VUVU = 0,
  E_MI_VIF_INPUT_DATA_UVUV,
  E_MI_VIF_INPUT_DATA_UYVY = 0,
  E_MI_VIF_INPUT_DATA_VYUY,
  E_MI_VIF_INPUT_DATA_YUYV,
  E_MI_VIF_INPUT_DATA_YVYU,
  E_MI_VIF_DATA_YUV_MAX,
} mi_vif_data_yuv_seq;

typedef enum {
  E_MI_VIF_CLK_EDGE_SINGLE_UP,
  E_MI_VIF_CLK_EDGE_SINGLE_DOWN,
  E_MI_VIF_CLK_EDGE_DOUBLE,
  E_MI_VIF_CLK_EDGE_MAX
} mi_vif_clk_edge;

typedef enum {
  E_MI_VIF_BITORDER_NORMAL,
  E_MI_VIF_BITORDER_REVERSED,
} mi_vif_bit_order;

typedef enum {
  E_MI_VIF_FRAMERATE_FULL,
  E_MI_VIF_FRAMERATE_HALF,
  E_MI_VIF_FRAMERATE_QUARTER,
  E_MI_VIF_FRAMERATE_OCTANT,
  E_MI_VIF_FRAMERATE_THREE_QUARTERS,
  E_MI_VIF_FRAMERATE_MAX
} mi_vif_framerate;

typedef struct {
  mi_vif_intf_mode intf_mode;
  mi_vif_work_mode work_mode;
  mi_vif_hdr_type hdr_type;
  mi_vif_clk_edge clk_edge;
  mi_vif_data_yuv_seq data_seq;
  mi_vif_bit_order bit_order;
  mi_vif_sync_attr sync_attr;
  u_int32_t dev_stich_mask;
} mi_vif_dev_attr;

typedef struct {
  mi_sys_window_rect cap_rect;
  mi_sys_window_size dest_size;
  mi_sys_field_type cap_sel;
  mi_sys_frame_scan_mode scan_mode;
  mi_sys_pixel_format pix_format;
  mi_vif_framerate framerate;
  u_int32_t frame_mode_line_count;
} mi_vif_chn_port_attr;

typedef struct {
  u_int32_t (*init_dev)(mi_vif_init_param *init);
  u_int32_t (*deinit_dev)();
  u_int32_t (*set_dev_attr)(u_int32_t dev_id, mi_vif_dev_attr *attr);
  u_int32_t (*enable_dev)(u_int32_t dev_id);
  u_int32_t (*disable_dev)(u_int32_t dev_id);
  u_int32_t (*set_chn_port_attr)(u_int32_t chn, u_int32_t port, mi_vif_chn_port_attr *attr);
  u_int32_t (*enable_chn_port)(u_int32_t chn, u_int32_t port);
  u_int32_t (*disable_chn_port)(u_int32_t chn, u_int32_t port);
} mi_vif_impl;
