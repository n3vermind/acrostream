#include "rtp.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "log.h"
#include "mi_venc.h"

static u_int8_t buf[10 * 1024]; // Needs to be larger than sizeof(rtp_header) + cfg->max_packet_size + 3
static const size_t RTP_HEADER_SIZE = sizeof(rtp_header);

int init_rtp(config *cfg, rtp_connection *conn) {
  if ((conn->fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
      error("failed to create a socket");
      return -1;
  }

  u_int32_t buffer_size = 16 * 1024 * 1024;
  if (setsockopt(conn->fd, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size)) < 0) {
    error("failed to set the buffer size on the socket");
    return -1;
  }

  struct sockaddr_in addr;
  addr.sin_addr.s_addr = inet_addr(cfg->address);
  addr.sin_port = htons(cfg->port);
  addr.sin_family = AF_INET;

  if (connect(conn->fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    error("failed to connect the socket");
    return -1;
  }

  conn->seq = 0x2137;
  conn->param_sent = 0;

  return 0;
}

int send_rtp(config *cfg, rtp_connection *conn, mi_venc_stream *stream) {
  u_int32_t starting_seq = conn->seq;

  for (u_int32_t i=0; i<stream->count; i++) {

    slice nalu[256];
    int nalu_idx = 0;
    int nalu_sum = 0;
    mi_venc_packet *pkt = &stream->packet[i];

    for (u_int32_t j=0; j<pkt->packet_num; ++j) {
      printf("NALU=%d size=%d offset=%d\n", pkt->info[j].nalu, pkt->info[j].length, pkt->info[j].offset);
    }

    u_int8_t *start = pkt->ptr+pkt->offset;
    u_int8_t *end = start;
    do {
      end += 1;
      if (end < pkt->ptr+pkt->len-3 && !(end[0] == 0x00 && end[1] == 0x00 && end[2] == 0x00 && end[3] == 0x01)) {
        continue;
      } else if (end >= pkt->ptr+pkt->len-3) {
        // Last NALU, set the end pointer to the end of the data.
        end = pkt->ptr+pkt->len;
      }

      mi_venc_nalu type = start[4] >> 1 & 0x3f;
      if (type >= E_MI_VENC_H265_NALU_VPS && type <= E_MI_VENC_H265_NALU_PPS) {
        u_int64_t mark = 1llu << type;
        if (conn->param_sent & mark) {
          // Skip sending the configuration nalu again.
          start = end;
          continue;
        }
        conn->param_sent |= mark;
      }

      printf("Appending NALU %d, len %d\n", type, end-start-4);


      if (RTP_HEADER_SIZE + nalu_sum + (end-start-4) + 2*nalu_idx > cfg->max_packet_size) {
        flush_nalu(cfg, conn, nalu, nalu_idx, 0);
        nalu_idx = 0;
        nalu_sum = 0;
      }

      nalu[nalu_idx].start = start+4;
      nalu[nalu_idx++].end = end;
      nalu_sum += end-start-4;

      start = end;
    } while (end < pkt->ptr+pkt->len);

    flush_nalu(cfg, conn, nalu, nalu_idx, pkt->frame_end);
  }

  printf("Full frame sent in %d packets\n", conn->seq - starting_seq);

  return 0;
}

int flush_nalu(config *cfg, rtp_connection *conn, slice *nalu, int nalu_len, u_int8_t marker) {
    if (nalu_len == 1 && nalu[0].end-nalu[0].start > cfg->max_packet_size) {
      send_fragmented_nalu(cfg, conn, &nalu[0], marker);
    } else if (nalu_len == 1) {
      send_single_nalu(cfg, conn, &nalu[0], marker);
    } else if (nalu_len > 1) {
      send_aggregated_nalu(cfg, conn, nalu, nalu_len, marker);
    }
    return 0;
}

int send_single_nalu(config *cfg, rtp_connection *conn, slice *nalu, u_int8_t marker) {
  u_int32_t len = nalu->end-nalu->start;

  set_header((rtp_header*)buf, conn->seq++, marker);

  memcpy(buf+RTP_HEADER_SIZE, nalu->start, len);

  sendto(conn->fd, buf, RTP_HEADER_SIZE + len, 0, NULL, sizeof(struct sockaddr_in));
  return 0;
}

int send_fragmented_nalu(config *cfg, rtp_connection *conn, slice *nalu, u_int8_t marker) {
  mi_venc_nalu type = nalu->start[0] >> 1 & 0x3F;

  rtp_header *hdr = (rtp_header*)buf;
  set_header(hdr, 0, marker);

  buf[RTP_HEADER_SIZE] = (49 << 1) | (nalu->start[0] & 0x81);
  buf[RTP_HEADER_SIZE+1] = nalu->start[1];
  buf[RTP_HEADER_SIZE+2] = type | 0x80; // FU start bit

  for(u_int8_t *ptr=nalu->start+2; ptr<nalu->end; ptr+=cfg->max_packet_size) {
    u_int32_t len = cfg->max_packet_size;

    hdr->seq = htons(conn->seq++);

    if (ptr+cfg->max_packet_size >= nalu->end) {
      buf[RTP_HEADER_SIZE+2] |= 0x40; // FU end bit.
      len = nalu->end-ptr;
    }

    memcpy(buf+RTP_HEADER_SIZE+3, ptr, len);

    sendto(conn->fd, buf, RTP_HEADER_SIZE + 3 + len, 0, NULL, sizeof(struct sockaddr_in));

    buf[RTP_HEADER_SIZE+2] &= 0xFF ^ 0x80; // Clear the FU start bit
  }

  return 0;
}

int send_aggregated_nalu(config *cfg, rtp_connection *conn, slice *nalu, int nalu_len, u_int8_t marker) {

  set_header((rtp_header*)buf, conn->seq++, marker);

  u_int8_t *ptr = buf+RTP_HEADER_SIZE+2;
  u_int16_t f = 0, layer_id = 0xffff, t_id = 0xffff;

  for (int i=0; i<nalu_len; i++) {
    u_int8_t *start = nalu[i].start;
    u_int8_t *end = nalu[i].end;

    u_int16_t new_hdr = *start;
    f |= new_hdr & 1;
    u_int16_t new_layer_id  = (new_hdr >> 7) & 0x7f;
    u_int16_t new_t_id = (new_hdr >> 13);
    if (new_layer_id < layer_id) {
      layer_id = new_layer_id;
    }
    if (new_t_id < t_id) {
      t_id = new_t_id;
    }

    u_int16_t nalu_size = end-start;
    *(u_int16_t*)ptr = htons(nalu_size);
    memcpy(ptr+2, start, nalu_size);

    ptr += 2 + nalu_size;
  }

  // Setting the payload header.
  *(u_int16_t*)(buf+RTP_HEADER_SIZE) = f | (48 << 1) | (layer_id << 7) | (t_id << 13);

  sendto(conn->fd, buf, ptr-buf, 0, NULL, sizeof(struct sockaddr_in));

  return 0;
}

void set_header(rtp_header *hdr, u_int32_t seq, u_int8_t marker) {
  hdr->cc = 0;
  hdr->x = 0;
  hdr->p = 0;
  hdr->version = 2;
  hdr->pt = 97 & 0x7F;
  hdr->m = marker;
  hdr->seq = htons(seq);
  hdr->ssrc = htonl(0xAAAA);

  struct timeval t;
  gettimeofday(&t, NULL);
  hdr->ts = htonl(t.tv_sec * 1000 + t.tv_usec / 1000) * 90;
}
