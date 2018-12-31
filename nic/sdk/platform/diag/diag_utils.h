// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DIAG_UTILS_H_
#define _DIAG_UTILS_H_

#include <spdlog/spdlog.h>
#include <getopt.h>

typedef enum {
    OFFLINE_DIAG,
    ONLINE_DIAG,
    POST,
    TEST_MODE_MAX
}test_mode_e;

typedef enum {
    TEST_PASS,
    TEST_FAIL,
    TEST_INVALID_OPTION,
    TEST_SHOW_HELP
}diag_ret_e;

typedef std::shared_ptr<spdlog::logger> Logger;
// GetCurrentLogger returns the current logger instance
Logger GetCurrentLogger();
void SetCurrentLogger(Logger logger);

int diag_error_logger (const char *format, ...);
int diag_info_logger (const char *format, ...);


void print_diag_result(const char *format, ...);
void LOG_TEST_BANNER(const char* test_name);
void LOG_TEST_RESULT(const char* test_name, int retval);
void PRINT_TEST_REPORT_BANNER(const char* diag_type);

//#define DEBUG_ENABLE
#ifdef DEBUG_ENABLE
#define DEBUG(args...) GetCurrentLogger()->debug(args)
#else
#define DEBUG(args...) while (0) { GetCurrentLogger()->debug(args); }
#endif // DEBUG_ENABLE

#define INFO       diag_info_logger 
#define ERR        diag_error_logger 

#endif
