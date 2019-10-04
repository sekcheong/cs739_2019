#ifndef DEBUG_H
#define DEBUG_H

#define MAX_DEBUG_STR_SIZE 256
//#define NDEBUG

void debug_print(const char *fmt, ...);

#if !defined(NDEBUG) | defined(_DEBUG)
#define DEBUG_PRINT(format, ...) debug_print(format, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(format, ...)
#endif //_DEBUG

#endif
