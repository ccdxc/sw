/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "sysmon_internal.hpp"

#define POST_TEST_FILENAME "/var/log/pensando/post_report_*.txt"
#define POST_TEST_COMPLETE_FILE "/tmp/.post_done"
#define SKIP_POST_TEST_FILE "/sysconfig/config0/post_disable"

void
checkpostdiag(void)
{
    static int runposttest;
    int result;
    if (runposttest == 0) {
        if (access(POST_TEST_COMPLETE_FILE, F_OK) == 0) {
            runposttest = 1;
            result = system("grep -s -q FAIL " POST_TEST_FILENAME"> /dev/null 2>&1");
            if (result == 0 && g_sysmon_cfg.postdiag_event_cb) {
                g_sysmon_cfg.postdiag_event_cb();
                SDK_HMON_TRACE_INFO("Post diag failed, event generated");
                return;
            }
            SDK_HMON_TRACE_INFO("Post diag completed Success");
            return;
        } else if (access(SKIP_POST_TEST_FILE, F_OK) == 0) {
            runposttest = 1;
            SDK_HMON_TRACE_INFO("Post skipped");
            return;
        }
    }
}
