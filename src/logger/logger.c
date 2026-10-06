#include "logger/logger.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

static const char *level_string[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "TEST" /* test program*/
};

void logger_init(void)
{

}

void logger_deinit(void)
{

}

void logger_log(
    log_level_t level,
    const char *file,
    int line,
    const char *fmt,
    ...
){
    time_t now;
    struct tm *tm_info;

    time(&now);
    tm_info = localtime(&now);

    printf("[%02d:%02d:%02d] [%s] [%s:%d] ",
           tm_info->tm_hour,
           tm_info->tm_min,
           tm_info->tm_sec,
           level_string[level],
           file,
           line);

    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");
}