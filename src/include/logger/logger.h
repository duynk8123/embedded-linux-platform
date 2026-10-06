#ifndef LOGGER_H
#define LOGGER_H

typedef enum{
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_TEST
}log_level_t;

void logger_init(void);
void logger_deinit(void);

void logger_log(
    log_level_t level,
    const char *file,
    int line,
    const char *fmt,
    ...
);

#define LOG_DEBUG(...) \
    logger_log(LOG_LEVEL_DEBUG,__FILE__,__LINE__,__VA_ARGS__)

#define LOG_INFO(...) \
    logger_log(LOG_LEVEL_INFO,__FILE__,__LINE__,__VA_ARGS__)

#define LOG_WARN(...) \
    logger_log(LOG_LEVEL_WARN,__FILE__,__LINE__,__VA_ARGS__)

#define LOG_ERROR(...) \
    logger_log(LOG_LEVEL_ERROR,__FILE__,__LINE__,__VA_ARGS__)

#define LOG_TEST(...) \
    logger_log(LOG_LEVEL_TEST,__FILE__,__LINE__,__VA_ARGS__)
#endif