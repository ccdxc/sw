//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <string>
#include <map>
#include <set>
#include <exception>
#include "include/sdk/base.hpp"
#include "lib/ipc/ipc.hpp"
#include "lib/event_thread/event_thread.hpp"
#include "upgrade/include/ev.hpp"

static sdk::event_thread::event_thread *g_svc_server_thread;
static std::string svc_name;
static uint32_t svc_thread_id;
static uint32_t rsp_time_delay = 0;
static std::string fsm_stage;
static std::string error_code = "ok";
static std::map<std::string, sdk_ret_t> ret_code_map;
static std::set<std::string> upg_stages;
static bool dump_to_log_file = false;

static void
init_err_codes (void)
{
    ret_code_map["ok"] =  sdk_ret_t::SDK_RET_OK;
    ret_code_map["critical"] =  sdk_ret_t::SDK_RET_UPG_CRITICAL;
    ret_code_map["fail"] =  sdk_ret_t::SDK_RET_ERR;
    ret_code_map["noresponse"] =  sdk_ret_t::SDK_RET_ERR;
}

static void
init_stage_names (void)
{
    upg_stages.insert("UPG_STAGE_NONE");
    upg_stages.insert("UPG_STAGE_COMPAT_CHECK");
    upg_stages.insert("UPG_STAGE_START");
    upg_stages.insert("UPG_STAGE_BACKUP");
    upg_stages.insert("UPG_STAGE_PREPARE");
    upg_stages.insert("UPG_STAGE_SYNC");
    upg_stages.insert("UPG_STAGE_PREP_SWITCHOVER");
    upg_stages.insert("UPG_STAGE_SWITCHOVER");
    upg_stages.insert("UPG_STAGE_READY");
    upg_stages.insert("UPG_STAGE_RESPAWN");
    upg_stages.insert("UPG_STAGE_ROLLBACK");
    upg_stages.insert("UPG_STAGE_REPEAL");
    upg_stages.insert("UPG_STAGE_FINISH");
}

static int
msleep (long inject_delay)
{
    struct timespec time_spec;
    int ret;

    if (inject_delay < 0) {
        errno = EINVAL;
        return -1;
    }

    time_spec.tv_sec = inject_delay / 1000;
    time_spec.tv_nsec = (inject_delay % 1000) * 1000000;

    do {
        ret = nanosleep(&time_spec, &time_spec);
    } while (ret && errno == EINTR);

    return ret;
}

static sdk_ret_t
test_upgrade (sdk::upg::upg_ev_params_t *params)
{
    printf("\nSuccessfully to handled event\n");
    params->response_cb(SDK_RET_OK, params->response_cookie);
    return SDK_RET_IN_PROGRESS;
}

static sdk_ret_t
fault_injection (sdk::upg::upg_ev_params_t *params)
{
    if (rsp_time_delay > 0) {
        msleep(rsp_time_delay);
    }
    printf("\nFailed to handle event %s\n", fsm_stage.c_str());
    params->response_cb(ret_code_map[error_code], params->response_cookie);
    return SDK_RET_IN_PROGRESS;
}

static void
upg_ev_fill (sdk::upg::upg_ev_t *ev)
{
    ev->svc_ipc_id = svc_thread_id;
    strncpy(ev->svc_name, svc_name.c_str(), sizeof(ev->svc_name));

    if (fsm_stage.compare("UPG_STAGE_COMPAT_CHECK") == 0) {
        printf("\nSetting fault injection in stage UPG_STAGE_COMPAT_CHECK\n");
        ev->compat_check_hdlr = fault_injection;
    } else {
        ev->compat_check_hdlr = test_upgrade;
    }

    if (fsm_stage.compare("UPG_STAGE_START") == 0) {
        printf("\nSetting fault injection in stage UPG_STAGE_START\n");
        ev->start_hdlr = fault_injection;
    } else {
        ev->start_hdlr = test_upgrade;
    }

    if (fsm_stage.compare("UPG_STAGE_BACKUP") == 0) {
        printf("\nSetting fault injection in stage UPG_STAGE_BACKUP\n");
        ev->backup_hdlr = fault_injection;
    } else {
        ev->backup_hdlr = test_upgrade;
    }

    if (fsm_stage.compare("UPG_STAGE_PREPARE") == 0) {
        printf("\nSetting fault injection in stage UPG_STAGE_PREPARE\n");
        ev->prepare_hdlr = fault_injection;
    } else {
        ev->prepare_hdlr = test_upgrade;
    }

    if (fsm_stage.compare("UPG_STAGE_PREP_SWITCHOVER") == 0) {
        printf("\nSetting fault injection in stage UPG_STAGE_PREP_SWITCHOVER\n");
        ev->prepare_switchover_hdlr = fault_injection;
    } else {
        ev->prepare_switchover_hdlr = test_upgrade;
    }

    if (fsm_stage.compare("UPG_STAGE_SWITCHOVER") == 0) {
        printf("\nSetting fault injection in stage UPG_STAGE_SWITCHOVER\n");
        ev->switchover_hdlr = fault_injection;
    } else {
        ev->switchover_hdlr = test_upgrade;
    }

    if (fsm_stage.compare("UPG_STAGE_ROLLBACK") == 0) {
        printf("\nSetting fault injection in stage UPG_STAGE_ROLLBACK\n");
        ev->rollback_hdlr = fault_injection;
    } else {
        ev->rollback_hdlr = test_upgrade;
    }

    if (fsm_stage.compare("UPG_STAGE_READY") == 0) {
        printf("\nSetting fault injection in stage UPG_STAGE_READY\n");
        ev->ready_hdlr = fault_injection;
    } else {
        ev->ready_hdlr = test_upgrade;
    }

    if (fsm_stage.compare("UPG_STAGE_SYNC") == 0) {
        printf("\nSetting fault injection in stage UPG_STAGE_SYNC\n");
        ev->sync_hdlr = fault_injection;
    } else {
        ev->sync_hdlr = test_upgrade;
    }

    if (fsm_stage.compare("UPG_STAGE_REPEAL") == 0) {
        printf("\nSetting fault injection in stage UPG_STAGE_REPEAL\n");
        ev->repeal_hdlr = fault_injection;
    } else {
        ev->repeal_hdlr = test_upgrade;
    }

    if (fsm_stage.compare("UPG_STAGE_FINISH") == 0) {
        printf("\nSetting fault injection in stage UPG_STAGE_FINISH\n");
        ev->finish_hdlr = fault_injection;
    } else {
        ev->finish_hdlr = test_upgrade;
    }
}

void
init (void *ctxt)
{
    sdk::upg::upg_ev_t ev;

    // register for upgrade events
    upg_ev_fill(&ev);
    sdk::upg::upg_ev_hdlr_register(ev);
}

void
exit (void *ctxt)
{
    printf("Exiting from svc event thread...");
}

sdk_ret_t
spawn_svc_thread (void)
{
    g_svc_server_thread =
        sdk::event_thread::event_thread::factory(
            svc_name.c_str(), svc_thread_id,
            sdk::lib::THREAD_ROLE_CONTROL, 0x0, init,
            exit, NULL,
            sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            true);

    g_svc_server_thread->start(g_svc_server_thread);

    return SDK_RET_OK;
}

static void
open_logger(std::string log_file)
{
    try {
        std::string log_path;
        if(const char* env_p = std::getenv("PDSPKG_TOPDIR")) {
            printf("\nLog PATH is: %s\n",env_p);
            log_path = *env_p;
        } else {
            log_path = "/sw/nic/";
        }
        log_file = log_path + "/" + log_file;

        printf ("\n Log file path : %s\n", log_file.c_str());
        int file_desc = open(log_file.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 644);

        if (file_desc < 0) {
            fprintf(stderr, "\nFailed to open log file %s\n", log_file.c_str());
            exit(1);
        }
        if (dup2(file_desc,1) < 0) {
            close(file_desc);
            fprintf(stderr, "\nFailed to redirect to %s\n", log_file.c_str());
            exit(1);
        }
    } catch (std::exception& e) {
        printf("\nexception caught: %s\n", e.what());
    }

}

sdk_ret_t
init_svc(void)
{
    if (dump_to_log_file) {
        std::string log("fsm_test_");
        log = log + svc_name + ".log";
        open_logger(log);
    }
    spawn_svc_thread();
    printf("\n server thread response handler is running \n");
    return SDK_RET_OK;
}

static void
print_usage (char **argv)
{
    fprintf(stdout, "\n\nUsage : %s "
            "\n\t\t -s | --svcname   <name of the service>"
            "\n\t\t -i | --svcid     <service id> "
            "\n\t\t -e | --err       < SVC_RSP_OK | SVC_RSP_FAIL | "
            "SVC_RSP_CRIT | SVC_RSP_NONE>"
            "\n\t\t -f | --fsmstage  <name of the stage where error code needs"
            " to be injected>\n\n", argv[0]);


    fprintf(stdout, "Example : %s -s test_service -i 51 -e fail -f "
            "backup \n\n", argv[0]);


    fprintf(stdout, "Possible values for -f | --fsmstage : "
            "UPG_STAGE_NONE, "
            "UPG_STAGE_COMPAT_CHECK, "
            "UPG_STAGE_START, "
            "UPG_STAGE_BACKUP, "
            "UPG_STAGE_PREPARE, "
            "UPG_STAGE_SYNC, "
            "UPG_STAGE_PREP_SWITCHOVER, "
            "UPG_STAGE_SWITCHOVER, "
            "UPG_STAGE_READY, "
            "UPG_STAGE_RESPAWN, "
            "UPG_STAGE_ROLLBACK, "
            "UPG_STAGE_REPEAL, "
            "UPG_STAGE_FINISH \n\n");

}

static void
atexit_handler (void)
{
    printf("Exiting from svc ...");
}

int
main (int argc, char **argv)
{

    sdk_ret_t                   ret;
    int                         opt;

    struct option longopts[] = {
        { "svcname", required_argument, NULL, 's'},
        { "svcid", required_argument, NULL, 'i'},
        { "err", required_argument, NULL, 'e'},
        { "fsmstage", required_argument, NULL, 'f'},
        { "rsp-time-delay", required_argument, NULL, 't'},
        { "dumplog", no_argument, NULL, 'd'},
        { "help", no_argument, NULL, 'h'}
    };

    atexit(atexit_handler);

    init_err_codes();
    init_stage_names();
    try {

        while ((opt = getopt_long(argc, argv, "s:t:i:e:f:hd",
                                  longopts, NULL)) != -1) {
            switch (opt) {
            case 's':
                if (optarg) {
                    svc_name = std::string(optarg);
                } else {
                    fprintf(stderr, "Service name is not specified\n");
                    print_usage(argv);
                    exit(1);
                }
                break;

            case 't':
                if (optarg) {
                    rsp_time_delay = std::stoi(std::string(optarg));
                } else {
                    fprintf(stderr, "rsp time delay is not specified\n");
                    print_usage(argv);
                    exit(1);
                }
                break;

            case 'i':
                if (optarg) {
                    svc_thread_id = std::stoi(std::string(optarg));
                } else {
                    fprintf(stderr, "Service id is not specified\n");
                    print_usage(argv);
                    exit(1);
                }
                break;

            case 'e':
                if (optarg) {
                    error_code = std::string(optarg);
                } else {
                    fprintf(stdout, "Error code is %s\n", error_code.c_str());
                }
                break;

            case 'f':
                if (optarg) {
                    fsm_stage = std::string(optarg);
                } else {
                    fprintf(stdout, "FSM stage is %s\n", fsm_stage.c_str());
                    fsm_stage = "none";
                }
                break;

            case 'd':
                dump_to_log_file = true;
                break;

            case 'h':
                print_usage(argv);
                exit(0);
                break;

            case ':':
                fprintf(stderr, "%s: option -%c requires an argument\n",
                        argv[0], optopt);
                print_usage(argv);
                exit(1);
                break;

            case '?':
            default:
                fprintf(stderr, "%s: option -%c is invalid, quitting ...\n",
                        argv[0], optopt);
                print_usage(argv);
                exit(1);
                break;
            }
        }
    } catch (...) {
        fprintf(stderr, "Invalid input ! \n");
        print_usage(argv);
        exit(1);
    }

    printf("\nsvc : %s, id %d, err %s, stage: %s\n", svc_name.c_str(),
           svc_thread_id, error_code.c_str(), fsm_stage.c_str());

    try {
        if ((ret = init_svc ()) != SDK_RET_OK) {
            fprintf(stderr, "Service (%s) initialization failed, err %u",
                    svc_name.c_str(), ret);
        }
    } catch (std::exception& e) {
        printf("\nFailed ti start service, exception caught: %s\n", e.what());
    }

    printf("\n Main thread waiting .......... \n");
    while (1) {
        sleep(10);
    }
}

