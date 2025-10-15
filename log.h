#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

static void error(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  write(STDERR_FILENO, "\n", 2);
  va_end(args);
}
