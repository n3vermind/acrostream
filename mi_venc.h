#pragma once

#include <sys/types.h>

static const int32_t VENC_DEV_ID = 0;
static const int32_t VENC_CHANNEL_ID = 0;
static const int32_t VENC_PORT_ID = 0;

typedef enum {
  E_MI_VENC_MODE_VENC = 1,
  E_MI_VENC_MODE_H264,
  E_MI_VENC_MODE_H265,
  E_MI_VENC_MODE_MJPEG,
  E_MI_VENC_MODE_MAX,
} mi_venc_mode;

typedef enum {
  E_MI_VENC_RC_MODE_H264CBR = 1,
  E_MI_VENC_RC_MODE_H264VBR,
  E_MI_VENC_RC_MODE_H264ABR,
  E_MI_VENC_RC_MODE_H264FIXQP,
  E_MI_VENC_RC_MODE_H264AVBR,
  E_MI_VENC_RC_MODE_MJPEGCBR,
  E_MI_VENC_RC_MODE_MJPEGQP,
  E_MI_VENC_RC_MODE_H265CBR,
  E_MI_VENC_RC_MODE_H265VBR,
  E_MI_VENC_RC_MODE_H265FIXQP,
  E_MI_VENC_RC_MODE_H265AVBR,
  E_MI_VENC_RC_MODE_MAX,
} mi_venc_ratemode;

typedef enum {
  E_MI_VENC_H265_NALU_PSLICE = 1,
  E_MI_VENC_H265_NALU_ISLICE = 19,
  E_MI_VENC_H265_NALU_VPS = 32,
  E_MI_VENC_H265_NALU_SPS = 33,
  E_MI_VENC_H265_NALU_PPS = 34,
  E_MI_VENC_H265_NALU_SEI = 39,
  E_MI_VENC_H265_NALU_MAX,
} mi_venc_nalu;

typedef struct {
  u_int32_t max_width;
  u_int32_t max_height;
  u_int32_t buf_size;
  u_int32_t profile;
  u_int8_t by_frame;
  u_int32_t width;
  u_int32_t height;
  u_int32_t frame_num;
  u_int32_t ref_num;
} mi_venc_attr_h265;

typedef struct {
  mi_venc_mode mode;
  union {
    mi_venc_attr_h265 h265;
  };
} mi_venc_attr;

typedef struct {
  u_int32_t gop;
  u_int32_t stat_time;
  u_int32_t src_frame_rate_num;
  u_int32_t src_frame_rate_den;
  u_int32_t bitrate;
  u_int32_t fluctuate;
} mi_venc_rate_h265cbr;

typedef struct {
  mi_venc_ratemode mode;
  union {
    mi_venc_rate_h265cbr h265_cbr;
  };
  void* p_rc_attr;
} mi_venc_rc_attr;

typedef struct {
  mi_venc_attr ve_attr;
  mi_venc_rc_attr rc_attr;
} mi_venc_channel;

typedef struct {
  u_int32_t left_pics;
  u_int32_t left_stream_bytes;
  u_int32_t left_stream_frames;
  u_int32_t left_stream_millis;
  u_int32_t cur_packs;
  u_int32_t left_recv_pics;
  u_int32_t left_enc_pics;
  u_int32_t fps_num;
  u_int32_t fps_den;
  u_int32_t bitrate;
} mi_venc_channel_stat;

typedef struct {
  mi_venc_nalu nalu;
  u_int32_t offset;
  u_int32_t length;
  u_int32_t slice_id;
} mi_venc_packet_info;

typedef struct {
  u_int64_t phy_addr;
  u_int8_t *ptr;
  u_int32_t len;
  u_int64_t timestamp;
  u_int8_t frame_end;
  mi_venc_nalu nalu;
  u_int32_t offset;
  u_int32_t packet_num;
  mi_venc_packet_info info[8];
} mi_venc_packet;

typedef struct {
  u_int32_t size;
  u_int32_t inter_cu_64x64;
  u_int32_t inter_cu_32x32;
  u_int32_t inter_cu_16x16;
  u_int32_t inter_cu_8x8;
  u_int32_t intra_cu_32x32;
  u_int32_t intra_cu_16x16;
  u_int32_t intra_cu_8x8;
  u_int32_t intra_cu_4x4;
  u_int32_t ref;
  u_int32_t update_attr_cnt;
  u_int32_t start_qp;
} mi_venc_stream_info_h265;

typedef struct {
 mi_venc_packet *packet; 
 u_int32_t count;
 u_int32_t seq;
 int32_t fd;
 union {
   mi_venc_stream_info_h265 h265;
   u_int32_t padding[14];
 };
} mi_venc_stream;

typedef struct {
  u_int8_t enable;
  u_int32_t slice_row_count;
} mi_venc_h265_slice_split_attr;

typedef struct {
  u_int8_t enable;
  u_int32_t refresh_line_num;
  u_int32_t req_iq;
} mi_venc_intra_refresh_attr;

typedef struct {
  u_int32_t max_qp;
  u_int32_t min_qp;
  int32_t ipqp_delta;
  u_int32_t max_iqp;
  u_int32_t min_iqp;
  u_int32_t max_ip_prop;
  u_int32_t min_p_size;
  u_int32_t max_p_size;
} mi_venc_param_h265_cbr;

typedef struct {
  u_int32_t thrd_i;
  u_int32_t thrd_p;
  u_int32_t row_qp_delta;
  union {
    mi_venc_param_h265_cbr h265_cbr;
    u_int32_t padding[10];
  };
  void *rc_param;
} mi_venc_rc_param;

typedef struct {
  u_int32_t (*create_channel)(u_int32_t channel, mi_venc_channel *config);
  u_int32_t (*destroy_channel)(u_int32_t channel);
  u_int32_t (*start_recv_pic)(u_int32_t channel);
  u_int32_t (*stop_recv_pic)(u_int32_t channel);
  u_int32_t (*get_fd)(u_int32_t channel);
  u_int32_t (*query)(u_int32_t channel, mi_venc_channel_stat *stat);
  u_int32_t (*get_stream)(u_int32_t channel, mi_venc_stream *stream, int32_t timeout_ms);
  u_int32_t (*release_stream)(u_int32_t channel, mi_venc_stream *stream);
  u_int32_t (*set_h265_slice_split)(u_int32_t channel, mi_venc_h265_slice_split_attr *attr);
  u_int32_t (*set_intra_refresh)(u_int32_t channel, mi_venc_intra_refresh_attr *attr);
  u_int32_t (*get_rc_param)(u_int32_t channel, mi_venc_rc_param *attr);
  u_int32_t (*set_rc_param)(u_int32_t channel, mi_venc_rc_param *attr);
} mi_venc_impl;
