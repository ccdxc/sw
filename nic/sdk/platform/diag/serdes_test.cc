// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#include "serdes_test.h"

namespace sdk {
namespace platform {
namespace diag {

#define SERDES_COMMON_TESTS_OPTION    \
    {"all", no_argument, NULL, 'a'}, /* Run all the tests */ \
    {"help", no_argument, NULL, 'h'}, /* Show usage */ \
    {"fw-check", no_argument, NULL, 'f'}, /* check serdes fw version*/ \

struct option serdes_online_diag_options[] =
{
    SERDES_COMMON_TESTS_OPTION
    /* Add online diags specific options here  */
    {"eye", no_argument, NULL, 'e'},
    {0, 0, NULL, 0}
};

struct option serdes_offline_diag_options[] =
{
    SERDES_COMMON_TESTS_OPTION
    /* Add offline diags specific options here  */
    {"prbs", required_argument, NULL, 'p'},
    {"bist", no_argument, NULL, 'b'},
    {0, 0, NULL, 0}
};

struct option serdes_post_options[] =
{
    SERDES_COMMON_TESTS_OPTION
    /* Add post specific options here  */
    {0, 0, NULL, 0}
};

char aapl_cmd[256];

static void serdes_common_tests_usage()
{
    printf (
"-all           Run all the temp sensor test in given mode\n"
"-fw-check      Run temp sensor test for Die\n"
"-help          Show this usage summary\n"
);

}

static void serdes_online_diag_usage()
{
    /* Add online diag specific usage here  */
    //As of now nothing
}

static void serdes_offline_diag_usage()
{
    /* Add offline diag specific usage here  */
    //As of now nothing
}

static void serdes_post_usage()
{
    /* Add post specific usage here  */
    //As of now nothing
}

static void serdes_test_usage(test_mode_e mode)
{
    serdes_common_tests_usage();

    if (mode == ONLINE_DIAG)
        serdes_online_diag_usage();
    else if (mode == OFFLINE_DIAG)
        serdes_offline_diag_usage();
    else if (mode == POST)
        serdes_post_usage();

    return;
}

static int run_serdes_fw_check_test(uint32_t sbus)
{
    snprintf(aapl_cmd, sizeof(aapl_cmd), "aapl dev -s localhost -p 9000 -a %d -v 0 | grep 0x1087", sbus);
    return system(aapl_cmd);
}

static int run_serdes_eye_test(uint32_t sbus)
{
    snprintf(aapl_cmd, sizeof(aapl_cmd), "aapl eye -print-ascii-eye -s localhost -p 9000 -a %d", sbus);
    return system(aapl_cmd);
}

static int run_serdes_prbs_test(uint32_t sbus, char* prbs_mode, uint32_t inject_errors)
{
    snprintf(aapl_cmd, sizeof(aapl_cmd), "aapl serdes -cmp-data PRBS31 -tx-data-sel PRBS31  -s localhost -p 9000 -a %d -error-reset -error-flag-reset -rx-input 1", sbus);
    return system(aapl_cmd);
}

static int run_serdes_bist_test(uint32_t sbus)
{
    snprintf(aapl_cmd, sizeof(aapl_cmd), "aapl bist -s localhost -p 9000 -a %d", sbus);
    return system(aapl_cmd);
}


diag_ret_e serdes_test(test_mode_e mode, int argc, char* argv[])
{
    int arg_option, sbus = -1;
    int index, retval;
    char prbs_pattern[8] ="PRBS31";
    struct option* option;

    /** test flags */
    int test_all=0;
    int serdes_fw_check_test=0, serdes_prbs_test=0, serdes_eye_test=0, serdes_bist_test=0;


    if(mode == OFFLINE_DIAG)
        option = serdes_offline_diag_options;
    else if(mode == ONLINE_DIAG)
        option = serdes_online_diag_options;
    else if(mode == POST)
        option = serdes_post_options;
    else
    {
        return TEST_INVALID_OPTION;
    }

    if (argc < 2)
        test_all = 1;

    while ((arg_option = getopt_long_only(argc, argv, "", option, &index)) >= 0) /* Parse command line args */
    {
        switch (arg_option)
        {
            case 'f':
                serdes_fw_check_test = 1;
                break;
            case 'p':
                serdes_prbs_test = 1;
                snprintf(prbs_pattern, sizeof(prbs_pattern), "%s", optarg);
                break;
            case 'e':
                serdes_eye_test = 1;
                break;
            case 'b':
                serdes_bist_test = 1;
                break;
            case 'h':
                serdes_test_usage(mode);
                return TEST_SHOW_HELP;;
            case 'a':
                test_all = 1;
                break;
            case 's':
                sbus = atoi(optarg);
                break;
            case '?':
            default:
                printf("Invalid option provoided!\n");
                printf("Please run with -h to see usage:\n");
                return TEST_INVALID_OPTION;
        }
    }

   if (optind < argc) {
        printf("Ignoring non-option argv-elements: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
    }

   if (sbus == -1)
   {
       sbus = 34;
   }
   LOG_TEST_BANNER("Serdes Tests:");

   if (test_all)
   {
       if (mode == OFFLINE_DIAG)
       {
           serdes_fw_check_test = 1;
           serdes_eye_test = 1;
           serdes_prbs_test = 1;
           serdes_bist_test = 1;
       }
       else if (mode == ONLINE_DIAG)
       {
           serdes_fw_check_test = 1;
           serdes_eye_test = 1;
       }
       else if (mode == POST)
       {
           serdes_fw_check_test = 1;
       }
   }

   if (serdes_fw_check_test)
   {
       retval = run_serdes_fw_check_test(sbus);
       LOG_TEST_RESULT("Serdes FW Check Test", retval);
   }

   if (serdes_eye_test)
   {
       retval = run_serdes_eye_test(sbus);
       LOG_TEST_RESULT("Serdes Eye Test", retval);
   }

   if (serdes_prbs_test)
   {
       retval = run_serdes_prbs_test(sbus, prbs_pattern, 21);
       LOG_TEST_RESULT("Serdes PRBS Test", retval);
   }

   if (serdes_bist_test)
   {
       retval = run_serdes_bist_test(sbus);
       LOG_TEST_RESULT("Serdes BIST Test", retval);
   }

   return (retval ? TEST_PASS : TEST_FAIL);
}
} // namespace diag
} // namespace platform
} // namespace sdk
