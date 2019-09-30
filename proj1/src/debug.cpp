#include <iostream>
#include <cstdarg>
#include <cstring>
#include <sys/time.h>
#include <unistd.h>

#include "debug.h"

void output_debug_string(const char *message) {
 	struct timeval tv;
 	time_t nowtime;
 	struct tm *nowtm;
 	char tmbuf[64]; 
 	char buf[64];
	pid_t pid = getpid();
 	gettimeofday(&tv, NULL);
 	nowtime = tv.tv_sec;
 	nowtm = localtime(&nowtime);
 	strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
 	snprintf(buf, sizeof buf, "%s.%06ld", tmbuf, (long) tv.tv_usec);
  	std::cout << "[" << buf << "]:[" << pid << "]:" << message ;
}


void debug_print(const char *fmt, ...) {
	char buff[MAX_DEBUG_STR_SIZE];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buff, MAX_DEBUG_STR_SIZE - 1, fmt, args);
	strncat(buff, "\n", MAX_DEBUG_STR_SIZE - 1);
	va_end(args);
	output_debug_string(buff);
}
