#pragma once

#include <sys/types.h>

#include "mi_sys.h"

static const int32_t VPE_DEV_ID = 0;
static const int32_t VPE_CHANNEL_ID = 0;
static const int32_t VPE_PORT_ID = 0;

typedef enum {
  E_MI_VPE_HDR_TYPE_OFF,
  E_MI_VPE_HDR_TYPE_VC,
  E_MI_VPE_HDR_TYPE_DOL,
  E_MI_VPE_HDR_TYPE_EMBEDDED,
  E_MI_VPE_HDR_TYPE_LI,
  E_MI_VPE_HDR_TYPE_MAX,
} mi_vpe_hdr_type;

typedef enum {
  E_MI_VPE_SENSOR_INVALID,
  E_MI_VPE_SENSOR0,
  E_MI_VPE_SENSOR1,
  E_MI_VPE_SENSOR2,
  E_MI_VPE_SENSOR3,
  E_MI_VPE_SENSOR_MAX,
} mi_vpe_sensor_channel;

typedef struct {
  u_int32_t dev_id;
  u_int8_t *data;
} mi_vpe_init_param;

typedef enum {
  E_MI_VPE_RUN_INVALID              = 0x00,
  E_MI_VPE_RUN_DVR_MODE             = 0x01,
  E_MI_VPE_RUN_CAM_TOP_MODE         = 0x02,
  E_MI_VPE_RUN_CAM_BOTTOM_MODE      = 0x04,
  E_MI_VPE_RUN_CAM_MODE             = E_MI_VPE_RUN_CAM_TOP_MODE | E_MI_VPE_RUN_CAM_BOTTOM_MODE,
  E_MI_VPE_RUN_REALTIME_TOP_MODE    = 0x08,
  E_MI_VPE_RUN_REALTIME_BOTTOM_MODE = 0x10,
  E_MI_VPE_RUN_REALTIME_MODE        = E_MI_VPE_RUN_REALTIME_TOP_MODE | E_MI_VPE_RUN_REALTIME_BOTTOM_MODE,
  E_MI_VPE_RUNNING_MODE_MAX,
} mi_vpe_running_mode;

typedef enum {
  E_MI_VPE_3DNR_LEVEL_OFF,
  E_MI_VPE_3DNR_LEVEL1,
  E_MI_VPE_3DNR_LEVEL2,
  E_MI_VPE_3DNR_LEVEL3,
  E_MI_VPE_3DNR_LEVEL4,
  E_MI_VPE_3DNR_LEVEL5,
  E_MI_VPE_3DNR_LEVEL6,
  E_MI_VPE_3DNR_LEVEL7,
  E_MI_VPE_3DNR_LEVEL_MAX,
} mi_vpe_3dnr;

typedef struct {
  u_int32_t revision;
  u_int32_t size;
  u_int8_t data[64];
} mi_vpe_isp_init_para;

typedef struct {
  u_int32_t mode;
  int8_t bypassOn;
  int8_t proj3x3On;
  int32_t proj3x3[9];
  unsigned short userSliceNum;
  unsigned int focalLengthX;
  unsigned int focalLengthY;
  void *configAddr;
  unsigned int configSize;
  int mapType;
  union {
    struct {
      void *xMapAddr, *yMapAddr;
      unsigned int xMapSize, yMapSize;
    } dispInfo;
    struct {
      void *calibPolyBinAddr;
      unsigned int calibPolyBinSize;
    } calibInfo;
  };
  char lensAdjOn;
} mi_vpe_ldc;

typedef struct {
  mi_sys_window_size capt;
  mi_sys_pixel_format pixel_format;
  mi_vpe_hdr_type hdr;
  mi_vpe_sensor_channel sensor_bind_id;
  u_int8_t noise;
  u_int8_t edge;
  u_int8_t edge_smooth;
  u_int8_t contrast;
  u_int8_t uv_invert;
  u_int8_t rotation;
  mi_vpe_running_mode running_mode;
  mi_vpe_isp_init_para isp_init_para;
  mi_vpe_ldc lens_init;
  u_int8_t ldc;
  u_int32_t chn_port_mode;
} mi_vpe_channel_attr;

typedef struct {
  mi_sys_window_size out;
  int8_t mirror;
  int8_t flip;
  mi_sys_pixel_format pixel_format;
  mi_sys_compress_mode compress_mode;
} mi_vpe_port_mode;

typedef struct {
  u_int8_t u8NrcSfStr; //0 ~ 255;
  u_int8_t u8NrcTfStr; //0 ~ 255
  u_int8_t u8NrySfStr; //0 ~ 255
  u_int8_t u8NryTfStr; //0 ~ 255
  u_int8_t u8NryBlendMotionTh; //0 ~ 15
  u_int8_t u8NryBlendStillTh; //0 ~ 15
  u_int8_t u8NryBlendMotionWei; //0 ~ 31
  u_int8_t u8NryBlendOtherWei; //0 ~ 31
  u_int8_t u8NryBlendStillWei; //0 ~ 31
  u_int8_t u8EdgeGain[6]; //0~255
  u_int8_t u8Contrast; //0~255
} mi_vpe_pq_param;


typedef struct {
  mi_vpe_pq_param pq_param;
  mi_vpe_ldc lens;
  mi_vpe_hdr_type hdr_type;
  mi_vpe_3dnr noise;
  u_int8_t mirror;
  u_int8_t flip;
  u_int8_t wdr;
  u_int8_t ldc;
} mi_vpe_channel_param;

typedef struct {
  u_int32_t (*init_dev)(mi_vpe_init_param *init);
  u_int32_t (*deinit_dev)();
  u_int32_t (*create_channel)(u_int32_t channel_id, mi_vpe_channel_attr *attr);
  u_int32_t (*destroy_channel)(u_int32_t channel_id);
  u_int32_t (*get_channel_param)(u_int32_t channel_id, mi_vpe_channel_param *param);
  u_int32_t (*set_channel_param)(u_int32_t channel_id, mi_vpe_channel_param *param);
  u_int32_t (*start_channel)(u_int32_t channel_id);
  u_int32_t (*stop_channel)(u_int32_t channel_id);
  u_int32_t (*set_port_mode)(u_int32_t channel_id, u_int32_t port_id, mi_vpe_port_mode *mode);
  u_int32_t (*enable_port)(u_int32_t channel_id, u_int32_t port_id);
  u_int32_t (*disable_port)(u_int32_t channel_id, u_int32_t port_id);
} mi_vpe_impl;
