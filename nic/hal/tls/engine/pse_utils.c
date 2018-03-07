#include <stdio.h>
#include <stdarg.h>
#include "pse_utils.h"

static FILE* fp = NULL;
const char* logfile_name = "pse.log";

int pse_init_log(void) 
{
    if(fp) {
        return 0; 
    }

    fp = fopen(logfile_name, "w");
    if(!fp) {
        printf("Failed to open log file");
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
