#pragma once

#include <sys/types.h>

#include "config.h"
#include "mi_venc.h"

typedef struct {
  u_int8_t cc:4;
  u_int8_t x:1;
  u_int8_t p:1;
  u_int8_t version:2;
  u_int8_t pt:7;
  u_int8_t m:1;
  u_int16_t seq;
  u_int32_t ts;
  u_int32_t ssrc;
} rtp_header;

typedef struct {
  int fd;
  u_int32_t seq;
  u_int64_t param_sent;
} rtp_connection;

int init_rtp(config *cfg, rtp_connection *conn);
int send_rtp(config *cfg, rtp_connection *conn, mi_venc_stream *stream);
int send_single_nalu(config *cfg, rtp_connection *conn, u_int8_t *start, u_int8_t *end, u_int8_t marker);
void set_header(rtp_header *hdr, u_int32_t seq, u_int8_t marker);
