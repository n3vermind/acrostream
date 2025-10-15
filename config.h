#pragma once

#include <sys/types.h>

typedef struct {
  char *address;
  u_int32_t port;
  u_int32_t bitrate;
  u_int32_t gop;
  u_int32_t slice_split;
  u_int32_t intra_refresh;
  u_int32_t max_packet_size;
} config;
