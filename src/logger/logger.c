#ifdef __linux__
    #define _GNU_SOURCE
#endif // __linux__
#include "logger.h"

#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"
#define TURQUOISE "\033[0;36m"
#define WHITE "\033[0m"
#define ORANGE "\033[0;38:2:220:165:0m"

static const char* log_level_str[] = {
#define X(A) [A] = #A,
    LOG_LVL(X)
#undef X
};

static FILE* outfile = NULL;
static struct logger_options logger_options = {
    .output_logs = true, .show_date = false,
#ifdef __linux__
    .show_thread_id = false,
#endif // __linux__
};

void logger_set_options(struct logger_options options) {
    logger_options = options;
}

bool logger_set_outfile(const char* filename) {
    if (outfile != NULL) return false;
    FILE* f = fopen(filename, "w");
    if (!f) return false;
    outfile = f;
    return true;
}

void logger_close_outfile(void) {
    if (outfile) {
        fclose(outfile);
        outfile = NULL;
    }
}

void logger_log(enum log_level level, const char* file_name, size_t line, const char* func_name, char* message) {
#ifdef __linux__
    pid_t id = gettid();
#endif // __linux__
    time_t t = time(NULL);
    struct tm* tt = localtime(&t); 
    char date[100] = { 0 };
    sprintf(date, "[%d-%d-%d %d:%d:%d]", tt->tm_year + 1900, tt->tm_mon + 1, tt->tm_mday, tt->tm_hour, tt->tm_min, tt->tm_sec);
    if (outfile) {
        fprintf(outfile, "[%s] %s ", log_level_str[level], date);
#ifdef __linux__
        if (id == getpid())
            fprintf(outfile, "[main thread] ");
        else
            fprintf(outfile, "[thread: %d] ", id);
#endif // __linux__
        fprintf(outfile, "%s:%zu in %s(): %s\n", file_name, line, func_name, message);
    }
    if (logger_options.output_logs || level == FATAL || level == ERROR) {
        const char* color;
        FILE* out;
        switch (level) {
            case INFO:
                color = TURQUOISE;
                out = stdout;
                break;
            case WARNING:
                color = YELLOW;
                out = stdout;
                break;
            case ERROR:
                color = ORANGE;
                out = stderr;
                break;
            case TIMEOUT:
                color = BLUE;
                out = stderr;
                break;
            case FATAL:
                color = RED;
                out = stderr;
                break;
        }
        fprintf(out, "%s[%s]%s ", color, log_level_str[level], WHITE);
        if (logger_options.show_date)
            fprintf(out, "%s ", date);
#ifdef __linux__
        if (logger_options.show_thread_id)
        {
            if (id == getpid())
                fprintf(out, "[main thread] ");
            else
                fprintf(out, "[thread: %d] ", id);
        }
#endif // __linux__
        fprintf(out, "%s:%zu in %s(): %s%s%s\n", file_name, line, func_name, color, message, WHITE);
    }
}
