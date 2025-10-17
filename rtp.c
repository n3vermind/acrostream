#include "rtp.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "log.h"
#include "mi_venc.h"

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

  for (u_int32_t i=0; i<stream->count; i++) {

    u_int8_t *start, *end;
    mi_venc_packet *pkt = &stream->packet[i];

    for (u_int32_t j=0; j<pkt->packet_num; ++j) {
      printf("NALU=%d size=%d offset=%d\n", pkt->info[j].nalu, pkt->info[j].length, pkt->info[j].offset);
    }

    for (start=pkt->ptr+pkt->offset, end=pkt->ptr+pkt->offset+4; end < pkt->ptr+pkt->len-3; end++) {
      if (!(end[0] == 0x00 && end[1] == 0x00 && end[2] == 0x00 && end[3] == 0x01)) {
        continue;
      }
      if (send_single_nalu(cfg, conn, start+4, end, 0)) {
        error("failed to send a single NALU");
        return -1;
      }
      start = end;
    }
    if (send_single_nalu(cfg, conn, start+4, pkt->ptr+pkt->len, pkt->frame_end)) {
      error("failed to send the last NALU");
      return -1;
    }
  }
  return 0;
}

int send_single_nalu(config *cfg, rtp_connection *conn, u_int8_t *start, u_int8_t *end, u_int8_t marker) {
  static u_int8_t buf[10 * 1024]; // Needs to be larger than sizeof(rtp_header) + cfg->max_packet_size + 3
  u_int32_t len = end-start;

  mi_venc_nalu type = start[0] >> 1 & 0x3F;

  if (type >= E_MI_VENC_H265_NALU_VPS && type <= E_MI_VENC_H265_NALU_PPS) {
    if (conn->param_sent & (1 << type)) {
      return 0;
    }
    conn->param_sent |= (1 << type);
  }

  printf("Sending NALU %d, len %d\n", type, len);

  if (len < cfg->max_packet_size) {
    set_header((rtp_header*)buf, conn->seq++, marker);

    memcpy(buf+sizeof(rtp_header), start, len);

    sendto(conn->fd, buf, sizeof(rtp_header) + len, 0, NULL, sizeof(struct sockaddr_in));
    return 0;
  }

  rtp_header *hdr = (rtp_header*)buf;
  set_header(hdr, 0, marker);

  buf[sizeof(rtp_header)] = (49 << 1) | (start[0] & 0x81);
  buf[sizeof(rtp_header)+1] = start[1];
  buf[sizeof(rtp_header)+2] = type | 0x80; // FU start bit

  for(u_int8_t *ptr=start+2; ptr<end; ptr+=cfg->max_packet_size) {
    u_int32_t len = cfg->max_packet_size;

    hdr->seq = htons(conn->seq++);

    if (ptr+cfg->max_packet_size >= end) {
      buf[sizeof(rtp_header)+2] |= 0x40; // FU end bit.
      hdr->m = marker;
      len = end-ptr;
    }

    memcpy(buf+sizeof(rtp_header)+3, ptr, len);

    sendto(conn->fd, buf, sizeof(rtp_header) + 3 + len, 0, NULL, sizeof(struct sockaddr_in));

    buf[sizeof(rtp_header)+2] &= 0xFF ^ 0x80; // Clear the FU start bit
  }

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
