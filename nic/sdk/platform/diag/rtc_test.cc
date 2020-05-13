// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "rtc_test.h"

namespace sdk {
namespace platform {
namespace diag {

int duration = SLEEP_INTERVAL;

struct option rtc_test_options[] = 
{
    {"all", no_argument, NULL, 'a'}, /* Run all the tests */
    {"help", no_argument, NULL, 'h'}, /* Run all the tests */
    {"duration", required_argument, NULL, 'd'}, /* Rin the test for specific duration */
    {0, 0, NULL, 0}
};

void rtc_test_usage()
{
    printf (
"-all           Run all the RTC test in given mode\n"
"-duration      Run RTC test for given duration (seconds)\n"
"-help          Show this usage summary\n"
""
);

    return;
}

int do_rtc_test()
{
    int fd, retval, sec_diff, min_diff;
    rtc_time a, b;

    fd = open(RTC_DEV_NODE, O_RDONLY);

    if (fd ==  -1) {
        SDK_TRACE_ERR("Can't open rtc device %s\n", RTC_DEV_NODE);
        return fd;
    }

    retval = ioctl(fd, RTC_RD_TIME, &a);
    if (retval == -1) {
        SDK_TRACE_ERR("RTC_RD_TIME ioctl failed with error: %d\n", retval);
        goto error_out;
    }

    SDK_TRACE_PRINT("Initial time from RTC: %d:%d (mm:ss)\n", a.tm_min, a.tm_sec);
    sleep(duration);

    retval = ioctl(fd, RTC_RD_TIME, &b);
    if (retval == -1) {
        SDK_TRACE_ERR("RTC_RD_TIME ioctl failed with error: %d\n", retval);
        goto error_out;
    }

    sec_diff = b.tm_sec - a.tm_sec;
    min_diff = b.tm_min - a.tm_min;
    SDK_TRACE_PRINT("Time from RTC after %d:%d (mm:ss) after  %d seconds duration\n", b.tm_min, b.tm_sec, duration);
    SDK_TRACE_PRINT("RTC time diff: %d:%d (mm:ss)\n", min_diff, sec_diff);

    /* If minute is changed then we calculate sec_diff by adding a minute to it */
    if (min_diff)
        sec_diff = (sec_diff + 60);

    /* RTC needs to tick at least duration seconds. It can tick more times
     * if the card is running at slower freq e.g. Naples25 SWM, OCP cards
     * when they are running out of Aux power supply
     * */
    if (sec_diff < duration)
    {
        SDK_TRACE_ERR("RTC test Failed !!! (time diff is %d:%d (mm::ss): "
            "expected minimum time diff: %d second(s))\n", min_diff, sec_diff,
                duration);
        retval = -1;
    }

error_out:
    close(fd);

    return retval;
}

diag_ret_e rtc_test(test_mode_e mode, int argc, char* argv[])
{
    int test_all, arg_option;
    int index, retval;

    /* test flags */
    int rtc_sanity_test=0;

    /* If no args are give then run all the tests */
    if (argc < 2)
        test_all = 1;

    /* For RTC there are no differences between online/offline/post
     * so we will use a generic rtc_test_options in pasring options */
    while ((arg_option = getopt_long_only(argc, argv, "", rtc_test_options, &index)) >= 0) /* Parse command line args */
    {
        switch (arg_option)
        {
            case 'd':
                duration = atoi(optarg);
                break;
            case 'h':
                rtc_test_usage();
                return TEST_SHOW_HELP;
            case 'a':
                test_all = 1;
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

   LOG_TEST_BANNER("RTC Tests:");

   /* Enable when we have more than one test in RTC */
   if (test_all)
   {
       rtc_sanity_test = 1;

       /* Set additional test flags here if any */
   }

   if (rtc_sanity_test)
   {
       retval = do_rtc_test();
       LOG_TEST_RESULT("RTC Sanity test", retval);
   }

   return (retval ? TEST_PASS : TEST_FAIL);
}
} // namespace diag
} // namespace platform
} // namespace sdk
