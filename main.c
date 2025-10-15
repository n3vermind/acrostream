#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "log.h"
#include "mi_lib.h"
#include "rtp.h"
#include "video.h"

bool finish = false;

void handle_error(int signo) {
  error("error occured (%d)\n", signo);
  exit(EXIT_FAILURE);
}

void handle_exit(int signo) {
  write(STDERR_FILENO, "Shutdown.\n", 10);
  finish = true;
}


int main(int argc, char *argv[]) {
  {
    char signal_error[] = {SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV};
    char signal_exit[] = {SIGINT, SIGQUIT, SIGTERM};

    for (char *s = signal_error; s < (&signal_error)[1]; s++)
      signal(*s, handle_error);
    for (char *s = signal_exit; s < (&signal_exit)[1]; s++)
      signal(*s, handle_exit);
  }

  config cfg = {
    .address = "127.0.0.1",
    .port = 5600,
    .bitrate = 8 * 1024 << 10,
    .gop = 18,
    .slice_split = 6,
    .intra_refresh = 6,
    .max_packet_size = 3800,
  };

  int opt;
  while((opt = getopt(argc, argv, "a:p:b:g:s:r:m:")) != -1) {
    switch (opt) {
      case 'a':
        cfg.address = optarg;
        break;
      case 'p':
        cfg.port = atoi(optarg);
        break;
      case 'b':
        cfg.bitrate = atoi(optarg) << 10;
        break;
      case 'g':
        cfg.gop = atoi(optarg);
        break;
      case 's':
        cfg.slice_split = atoi(optarg);
        break;
      case 'r':
        cfg.intra_refresh = atoi(optarg);
        break;
      case 'm':
        cfg.max_packet_size = atoi(optarg);
        break;

      default:
        printf("Usage: %s [-a address] [-p port] [-b bitrate in Kb/s] [-g gop] [-s slice_split] [-r intra_refresh] [-m max_packet_size]\n", argv[0]);
        return -1;
    }
  }

  mi_lib lib;
  if (mi_create(&lib)) {
    error("failed to create mi_lib");
    return -1;
  }

  u_int32_t venc_fd;
  if (setup_pipeline(&cfg, &lib, &venc_fd)) {
    error("failed to setup the video pipeline");
    return -1;
  }

  rtp_connection conn;
  if (init_rtp(&cfg, &conn)) {
    error("failed to init the connection");
    return -1;
  }

  printf("Started.\n");

  while(!finish) {
    process_frame(&cfg, &lib, &conn, venc_fd);
  }

  if (cleanup_pipeline(&lib)) {
    error("failed to cleanup the video pipeline");
    return -1;
  }

  return 0;
}
