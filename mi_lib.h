#pragma once

#include "mi_isp.h"
#include "mi_snr.h"
#include "mi_venc.h"
#include "mi_vif.h"
#include "mi_vpe.h"

typedef struct {
  mi_sys_impl mi_sys;
  mi_isp_impl mi_isp;
  mi_snr_impl mi_snr;
  mi_vif_impl mi_vif;
  mi_vpe_impl mi_vpe;
  mi_venc_impl mi_venc;
} mi_lib;

int mi_create(mi_lib *lib);

int mi_isp_load(mi_isp_impl *impl);
int mi_sys_load(mi_sys_impl *impl);
int mi_snr_load(mi_snr_impl *impl);
int mi_vif_load(mi_vif_impl *impl);
int mi_vpe_load(mi_vpe_impl *impl);
int mi_venc_load(mi_venc_impl *impl);
