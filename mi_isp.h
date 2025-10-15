#pragma once

#include <sys/types.h>

static const int32_t ISP_CHANNEL_ID = 0;

typedef enum {
  MI_ISP_AE_MODE_A,
  MI_ISP_AE_MODE_AV,
  MI_ISP_AE_MODE_SV,
  MI_ISP_AE_MODE_TV,
  MI_ISP_AE_MODE_Mj,
  MI_ISP_AE_MODE_MAX,
} mi_isp_ae_mode;

typedef struct {
  u_int32_t min_shutter_us;
  u_int32_t max_shutter_us;
  u_int32_t min_fn;
  u_int32_t max_fn;
  u_int32_t min_sensor_gain;
  u_int32_t min_isp_gain;
  u_int32_t max_sensor_gain;
  u_int32_t max_isp_gain;
} mi_isp_ae_exposure_limit;

typedef struct {
  u_int32_t fn;
  u_int32_t sensor_gain;
  u_int32_t isp_gain;
  u_int32_t shutter_us;
} mi_isp_ae_exposure;

typedef struct {
  u_int32_t (*load_config)(u_int32_t channel, char *path, u_int32_t key);
  u_int32_t (*get_exposure_limit)(u_int32_t channel, mi_isp_ae_exposure_limit *data);
  u_int32_t (*set_exposure_limit)(u_int32_t channel, mi_isp_ae_exposure_limit *data);
} mi_isp_impl;
