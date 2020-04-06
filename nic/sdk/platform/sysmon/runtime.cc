/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "sysmon_internal.hpp"

void
checkruntime(void)
{
    static int runtimecounter;
    static bool filerror = false;

    if (runtimecounter >= 360 &&
        runtimecounter % 360 == 0) {
        FILE *fptr = NULL;
        int uptime = 0;
        fptr = fopen (UPTIME_FILENAME, "r+");
        if (fptr == NULL) {
            fptr = fopen (UPTIME_FILENAME, "w");
            if (fptr == NULL) {
                if (filerror == false) {
                    filerror = true;
                    SDK_HMON_TRACE_INFO("Error creating file %s",
                                        UPTIME_FILENAME);
                }
                return;
            }
        } else {
            fscanf(fptr, "%d", &uptime);
            rewind(fptr);
        }
        uptime++;
        fprintf(fptr, "%d", uptime);
        fflush(fptr);
        fsync(fileno(fptr));
        fclose(fptr);
    }
    runtimecounter++;
    return;
}

// MONFUNC(checkruntime);
