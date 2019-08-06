#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "pse_utils.h"

static FILE* fp = NULL;
const char* logfile_name = "pse.log";

int pse_init_log(void) 
{
    char *log_dir;
    char final_logfile_name[256];

    if(fp) {
        return 0; 
    }

    log_dir = getenv("ENGINE_LOG_DIR");
    if (log_dir) {
        snprintf(final_logfile_name, sizeof(final_logfile_name),
                 "%s%s%s", log_dir, "/", logfile_name);
    } else {
        snprintf(final_logfile_name, sizeof(final_logfile_name),
                 "%s", logfile_name);
    }

    fp = fopen(final_logfile_name, "w");
    if(!fp) {
        printf("Failed to open log file %s\n", final_logfile_name);
        return 0;
    }

    return 1;
}

void pse_log(const char* format, ...) 
{
    va_list args;
    va_start(args, format);
    vfprintf(fp, format, args);
    fflush(fp);
    va_end(args);
}
