#pragma once

#include <sys/types.h>

#include "config.h"
#include "mi_lib.h"
#include "rtp.h"

static const u_int32_t WIDTH = 1920;
static const u_int32_t HEIGHT = 1080;
static const u_int32_t FPS = 90;

int setup_pipeline(config *cfg, mi_lib *lib, u_int32_t *venc_fd);
int cleanup_pipeline(mi_lib *lib);
int process_frame(config *cfg, mi_lib *lib, rtp_connection *conn, u_int32_t venc_fd);
