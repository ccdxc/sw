#include "pal.hpp"
#include "cpld_test.h"
#include "logger.hpp"

#define CPLD_COMMON_TESTS_OPTION    \
    {"all", no_argument, NULL, 'a'}, /* Run all the tests */ \
    {"help", no_argument, NULL, 'h'}, /* Run all the tests */ \
    {"id", no_argument, NULL, 'i'}, /* Read and Verify CPLD ID */ \
 

static int arg_option;
struct option cpld_test_online_diag_options[] =
{
    CPLD_COMMON_TESTS_OPTION
    /* Add online diags specific options here  */
    {0, 0, NULL, 0}
};

struct option cpld_test_offline_diag_options[] =
{
    CPLD_COMMON_TESTS_OPTION
    /* Add offline diags specific options here  */
    {0, 0, NULL, 0}
};

struct option cpld_test_post_options[] =
{
    CPLD_COMMON_TESTS_OPTION
    /* Add post specific options here  */
    {0, 0, NULL, 0}
};


void cpld_common_tests_usage()
{
    printf (
"-all           Run all the CPLD test in given mode\n"
"-id            Verify CPLD ID\n"
"-help          Show this usage summary\n"
);

}

void cpld_online_diag_usage()
{
    /* Add online diag specific usage here  */
    //As of now nothing
}

void cpld_offline_diag_usage()
{
    /* Add offline diag specific usage here  */
    //As of now nothing
}

void cpld_post_usage()
{
    /* Add post specific usage here  */
    //As of now nothing
}


void cpld_test_usage(test_mode_e mode)
{
    cpld_common_tests_usage();
    
    if (mode == ONLINE_DIAG)
        cpld_online_diag_usage();
    else if (mode == OFFLINE_DIAG)
        cpld_offline_diag_usage();
    else if (mode == POST)
        cpld_post_usage();

    return;
}

int cpld_verify_id_test()
{
    int read_val, ret_val;
    read_val = sdk::lib::pal_get_cpld_id();

    if (read_val == NAPLES_CPLD_ID)
    {
        SDK_TRACE_PRINT("CPLD ID: 0x%x, Rev: 0x%x", read_val, sdk::lib::pal_get_cpld_rev());
        ret_val = 0;
    }
    else
    {
        SDK_TRACE_ERR("Wrong CPLD ID: 0x%x : Expected: 0x%x", read_val, NAPLES_CPLD_ID);
        ret_val = -1;
    }

    return ret_val;

}

diag_ret_e cpld_test(test_mode_e mode, int argc, char* argv[])
{
    int index, retval, cpld_read_id_test = 0, cpld_test_all = 0;
    struct option* option;

    if(mode == OFFLINE_DIAG)
        option = cpld_test_offline_diag_options;
    else if(mode == ONLINE_DIAG)
        option = cpld_test_online_diag_options;
    else if(mode == POST)
        option = cpld_test_post_options;
    else
    {
        return TEST_INVALID_OPTION;
    }

    if (argc < 2)
        cpld_test_all = 1;

    while ((arg_option = getopt_long_only(argc, argv, "", option, &index)) >= 0) /* Parse command line args */
    {
        switch (arg_option)
        {
            case 'i':
                cpld_read_id_test = 1;
                break;
            case 'h':
                cpld_test_usage(mode);
                return TEST_SHOW_HELP;;
            case 'a':
                cpld_test_all = 1;
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

   LOG_TEST_BANNER("CPLD Tests:");

   if (cpld_test_all)
   {
        cpld_read_id_test = 1;
       //set additionaltest flags here to run all the tests
   }

   if(cpld_read_id_test)
   {
       retval = cpld_verify_id_test();
       LOG_TEST_RESULT("CPLD ID Verify test", retval);
   }
   
   return (retval ? TEST_PASS : TEST_FAIL); 
}

