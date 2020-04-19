
/*
 * {C} Copyright 2020 Pensando Systems Inc. All rights reserved
 *
 *
 */

#include <pds_init.h>
#include <sched.h>
#include <getopt.h>
//#include <base.h>
#include "vnic.h"
#include "flow_cache.h"
#include "session_info.h"
#include "session_rewrite.h"
#include "base.h"

void (*test_function)(int index);
void (*cfunc)(int index);

ttime_t ttime[MAX_CURD_ENUM];

bool run_create_test = false;

int setup ( ) 
{
    pds_cinit_params_t init_params;
    pds_ret_t           ret = PDS_RET_OK;

    printf("Inside setup \n");
    memset(&init_params, 0, sizeof(init_params));
    init_params.init_mode = PDS_CINIT_MODE_COLD_START;
    init_params.trace_cb  = myprintf;

    ret = pds_global_init(&init_params);
    if (ret != PDS_RET_OK) {
        printf("PDS global init failed with ret %u\n", ret);
        exit(1);
    }

#ifdef __aarch64__ 
    printf("Waiting for 60 seconds...\n");
    sleep(60);
#endif

    memset(ttime, 0, sizeof(ttime));
    //process_options
    printf("Exit  setup \n");
    return ret;
}

void teardown (void)
{
    printf("Inside  teardown \n");
    pds_global_teardown();
    printf("Exit  teardown \n");
}

void
calc_time_delta(struct timeval *end,
                struct timeval *start)
{
    uint64_t end_us = ((uint64_t)end->tv_sec * USEC_PER_SEC) +
                      (uint64_t)end->tv_usec;
    uint64_t start_us = ((uint64_t)start->tv_sec * USEC_PER_SEC) +
                        (uint64_t)start->tv_usec;

    end->tv_sec = 0;
    end->tv_usec = 0;
    if (end_us > start_us) {
        end_us -= start_us;
        end->tv_sec = end_us / USEC_PER_SEC;
        end->tv_usec = end_us % USEC_PER_SEC;
    }
}

void run_test (int func_num, int count)
{
    struct timeval      stime = {0};
    struct timeval      etime = {0};
    int                 cfunc_num = 0;

    test_function = test_enum_to_func(func_num);

    if (run_create_test == true) {
        /* TODO: Fix this */
        if ((func_num-1) % 4) {
            cfunc_num = (func_num - ((func_num-1) % 4));
            printf("Starting pre setup test %s \n", test_enum_to_str(cfunc_num));
            cfunc = test_enum_to_func(cfunc_num);
            for (int i=1; i <= count ; i++) {
                cfunc(i);
            }
            printf("Completed pre setup test %s \n", test_enum_to_str(cfunc_num));
        }
    }

    printf("Starting test %s \n", test_enum_to_str(func_num));
    gettimeofday(&stime, NULL);
    for (int i = 1; i < count ; i++) {
        test_function(i);
    }
    gettimeofday(&etime, NULL);
    calc_time_delta(&etime, &stime);
    ttime[func_num].count = count;
    ttime[func_num].time_taken = etime;
    printf("Completed test %s \n", test_enum_to_str(func_num));
}

void print_total_time (void)
{

    printf("\n===========================================================\n\n");
    /*50 spaces for API Name
     *15 spaces for Count
     *Rest for Total time.
     */ 
    printf("%24s%23s%5s%5s%5s\n","API","","Count","","Total Time"); 
    printf("\n============================================================\n\n");
    for (int i = 0; i < MAX_CURD_ENUM; i++) {
        if (ttime[i].count != 0) {
            printf("%-50s%-15u%lu.%lu \n", test_enum_to_str(i), ttime[i].count,
                   ttime[i].time_taken.tv_sec, ttime[i].time_taken.tv_usec);
        }
    }
}

void
print_usage(char *argv[])
{
    printf("Usage : %s {[-d | --display] | [-h | --help] | "
           "[-c | --count  -a | --api [-s | --setup]]}\n",
            argv[0]);
    printf("-d | --display : display all the API's supported \n");
    printf("-h | --help    : help \n");
    printf("-c | --count   : Number of times to run the test \n");
    printf("-a | --api     : Single or Range (-) of API's to RUN. \n"
                             "\t\t API represented by number in -d \n");
    printf("-p | --pre-run : Run create testcase before a \n"
                             "\t\t update/read/delete API \n");
    printf("-s | --sched   : Set Schedule priority to FIFO \n");
}

void
set_scheduler_options (void)
{
    struct sched_param param = {0};
    int ret = 0;

    param.sched_priority = 99;
    if( (ret = sched_setscheduler( 0, SCHED_RR, &param )) == -1 ) {
        printf("error setting scheduler ...%d \n", ret);
    }

}


/*
 * Parse all the tests to RUN.
 * Tests could be seperated by - for range and ,.
 */
void
parse_and_exec_tests(char *tests, int count)
{
    char *end_ptr = NULL, delimit[2] = "-", *token = NULL;
    //int idx = strtol(optarg, &end_ptr, 10);
    int start = 0, end = 0;

    token = strtok(tests, delimit);

    while (token != NULL) {
         if (start == 0) {
             start = strtol(token, &end_ptr, 10);
         } else if (end == 0) {
             end = strtol(token, &end_ptr, 10);
         }
         token = strtok(NULL, delimit);
    }

    if (end == 0) {
        end = start;
    }
        
    if (count == 0) {
        /*
         * TODO: Set the count to Max value of a particular table.
         */
        count = 1;
    } 
    printf("Run tests from %d to %d \n", start, end);
    for (int idx = start ; idx <= end ; idx++) {
        run_test(idx, count);
    }
    print_total_time(); 
}

void
print_test_entries (void)
{
    printf("\n");
    for(int i = 1; i < MAX_CURD_ENUM; i++) {
        printf("%d. %s \n", i, test_enum_to_str(i));
    }

}

/*
 *  -d = show API list
 *  -c = API count
 *  TODO: Shoudl be able to take ", - seperated API numbers.
 */

int main (int argc, char *argv[])
{
    bool     run_tests = false;
    uint32_t count = 0;
    char     api_str[4096], *end_ptr = NULL;
    int      oc = 0;

    static struct option longopts[] = {
       { "count",     required_argument, NULL, 'c' },
       { "api",       required_argument, NULL, 'a' },
       { "display",   no_argument,       NULL, 'd' },
       { "help",      no_argument,       NULL, 'h' },
       { "pre-run",   no_argument,       NULL, 'p' },
       { "sched",     no_argument,       NULL, 's' },
       { 0,           0,                 0,    0   }
    };

    // parse CLI options
    while ((oc = getopt_long(argc, argv, "c:a:dhps", longopts, NULL)) != -1) {
        switch (oc) {
        case 0:
            break;
        case 'c':
            if (optarg) {
                count = strtol(optarg, &end_ptr, 10);
            } else {
                printf("Count is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;
        case 'd':
            print_test_entries();
            return 0;
            break;
        case 'h':
            print_usage(argv);
            return 0;
            break;
        case 's':
            set_scheduler_options();
            break;
        case 'p':
            run_create_test = true;
            break;
        case 'a':
            memset(api_str, 0, sizeof(api_str));
            run_tests = true;
            snprintf(api_str, 4096, "%s", optarg);
            break;
        default :
            printf("Unknown option encountered. Exit \n");
            break; 
        }
    }
    if (run_tests == true) {
        setup();
        parse_and_exec_tests(api_str, count);
        teardown();
    }

 return 0;
}
