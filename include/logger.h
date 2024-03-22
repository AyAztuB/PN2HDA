#ifndef LOGGER_H
#define LOGGER_H

#include <stddef.h>
#include <stdbool.h>

#define LOG_LVL(X) \
    X(INFO) \
    X(WARNING) \
    X(ERROR) \
    X(TIMEOUT) \
    X(FATAL)

enum log_level {
#define X(A) A,
    LOG_LVL(X)
#undef X
};

#ifndef SOURCE_PATH_SIZE
    #define SOURCE_PATH_SIZE 0
#endif // SOURCE_PATH_SIZE
#define __FILENAME__ ((__FILE__) + (SOURCE_PATH_SIZE))
#define LOG(LEVEL, MSG) \
    logger_log((LEVEL), __FILENAME__, __LINE__, __func__, (MSG))

struct logger_options {
    bool output_logs; // output log on stdout/stderr (in addition or not of logging in file)
    // if true, all logs INFO, and WARNING will be displayed on stdout and ERROR and TIMEOUT on stderr
    // all log marked as FATAL will be displayed on stderr even if output_logs if set to false
    bool show_date;
#ifdef __linux__
    bool show_thread_id;
#endif // __linux__
};

void logger_set_options(struct logger_options options);
bool logger_set_outfile(const char* filename);
void logger_close_outfile(void);
void logger_log(enum log_level level, const char* file_name, size_t line, const char* func_name, char* message);

#endif // LOGGER_H
