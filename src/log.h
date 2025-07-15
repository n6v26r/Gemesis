#pragma once
#include "utils.h"
#include <cassert>
#include <cstdlib>
#include <stdarg.h>
#include <stdio.h>

enum LOGLVL {
  NONE,
  INFO,
  WARN,
  ERR,
};

#define ANSI_RED "\e[1;31m"
#define ANSI_YLLW "\e[1;33m"
#define ANSI_BLUE "\e[1;34m"
#define ANSI_GREEN "\e[0;32m"
#define ANSI_PINK "\e[38;5;212m"
#define ANSI_BLACK "\e[4;30m"
#define ANSI_WHITE "\e[4;37m"

#define ANSI_BACKGROUND_RED "\e[41m"
#define ANSI_BACKGROUND_YLLW "\e[43m"
#define ANSI_BACKGROUND_BLUE "\e[44m"
#define ANSI_BACKGROUND_GREEN "\e[42m"

#define ANSI_RESET "\e[0m"

template <class... Args> inline void logInfo(const char *s, Args... args) {
  fprintf(stderr, "[" ANSI_BLUE " Info " ANSI_RESET "]: ");
  fprintf(stderr, s, args...);
  fputc('\n', stderr);
}

template <class... Args> inline void logWarn(const char *s, Args... args) {
  fprintf(stderr, "[" ANSI_YLLW " Warn " ANSI_RESET "]: ");
  fprintf(stderr, s, args...);
  fputc('\n', stderr);
}

template <class... Args> inline void logErr(const char *s, Args... args) {
  fprintf(stderr, "[" ANSI_RED " Error " ANSI_RESET "]: ");
  fprintf(stderr, s, args...);
  fputc('\n', stderr);
  exit(1);
}

template <class... Args>
inline void log(LOGLVL lv, const char *s, Args... args) {
  switch (lv) {
  case INFO:
    logInfo(s, args...);
    break;
  case WARN:
    logWarn(s, args...);
    break;
  case ERR:
    logErr(s, args...);
    break;
  default:
    logErr("(log): invalid loglvl");
    break;
  }
}

template <class... Args>
inline void logAssert(bool expr, const char *s = "Assert Failed!",
                      Args... args) {
  if (!expr) {
    logErr(s, args...);
  }
}
