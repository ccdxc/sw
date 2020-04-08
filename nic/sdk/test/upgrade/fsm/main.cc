//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string>
#include <map>
#include <set>
#include "include/sdk/base.hpp"
#include "lib/ipc/ipc.hpp"
#include "lib/event_thread/event_thread.hpp"
#include "upgrade/include/ev.hpp"

static sdk::event_thread::event_thread *g_svc_server_thread;
static std::string svc_name;
static uint32_t svc_thread_id;
static std::string fsm_stage;
static std::string error_code = "ok";
static std::map<std::string, sdk_ret_t> ret_code_map;
static std::set<std::string> upg_stages;

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
    upg_stages.insert("none");
    upg_stages.insert("compat_check");
    upg_stages.insert("start");
    upg_stages.insert("backup");
    upg_stages.insert("prepare");
    upg_stages.insert("sync");
    upg_stages.insert("prep_switchover");
    upg_stages.insert("switchover");
    upg_stages.insert("ready");
    upg_stages.insert("respawn");
    upg_stages.insert("rollback");
    upg_stages.insert("repeal");
    upg_stages.insert("finish");
}

static sdk_ret_t
test_upgrade (sdk::upg::upg_ev_params_t *params)
{
    params->response_cb(SDK_RET_OK, params->response_cookie);
    return SDK_RET_IN_PROGRESS;
}

static sdk_ret_t
fault_injection (sdk::upg::upg_ev_params_t *params)
{
    params->response_cb(ret_code_map[error_code], params->response_cookie);
    return SDK_RET_IN_PROGRESS;
}

static void
upg_ev_fill (sdk::upg::upg_ev_t *ev)
{
    ev->svc_ipc_id = svc_thread_id;
    strncpy(ev->svc_name, svc_name.c_str(), sizeof(ev->svc_name));

    if (fsm_stage.compare("compat_check") == 0) {
        ev->compat_check_hdlr = fault_injection;
    } else {
        ev->compat_check_hdlr = test_upgrade;
    }

    if (fsm_stage.compare("start") == 0) {
        ev->start_hdlr = fault_injection;
    } else {
        ev->start_hdlr = test_upgrade;
    }

    if (fsm_stage.compare("backup") == 0) {
        printf("\nsettign fault injection\n");
        ev->backup_hdlr = fault_injection;
    } else {
        ev->backup_hdlr = test_upgrade;
    }

    if (fsm_stage.compare("prepare") == 0) {
        ev->prepare_hdlr = fault_injection;
    } else {
        ev->prepare_hdlr = test_upgrade;
    }

    if (fsm_stage.compare("prep_switchover") == 0) {
        ev->prepare_switchover_hdlr = fault_injection;
    } else {
        ev->prepare_switchover_hdlr = test_upgrade;
    }

    if (fsm_stage.compare("switchover") == 0) {
        ev->switchover_hdlr = fault_injection;
    } else {
        ev->switchover_hdlr = test_upgrade;
    }

    if (fsm_stage.compare("rollback") == 0) {
        ev->rollback_hdlr = fault_injection;
    } else {
        ev->rollback_hdlr = test_upgrade;
    }

    if (fsm_stage.compare("ready") == 0) {
        ev->ready_hdlr = fault_injection;
    } else {
        ev->ready_hdlr = test_upgrade;
    }

    if (fsm_stage.compare("sync") == 0) {
        ev->sync_hdlr = fault_injection;
    } else {
        ev->sync_hdlr = test_upgrade;
    }

    if (fsm_stage.compare("repeal") == 0) {
        ev->repeal_hdlr = fault_injection;
    } else {
        ev->repeal_hdlr = test_upgrade;
    }

    if (fsm_stage.compare("finish") == 0) {
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

sdk_ret_t
init_svc(void)
{
    spawn_svc_thread();
    printf("\n server thread response handler is running \n");
    return SDK_RET_OK;
}

static void
print_usage (char **argv)
{
    fprintf(stdout, "Usage : %s \n\t-s|--svcname <name of the service>"
            " \n\t-i|--svcid <service id> "
            "\n\t[ -e|--err < error code >] "
            "\n\t[ -f|--fsmstage <name of the stage where error code needs "
            "to be injected > ]\n", argv[0]);
#if 0
    fprintf(stdout, "Possible values for stages are : compat_check, start,"
            " backup, prepare, sync, prep_switchover, switchover, ready, "
            "respawn, rollback, repeal, finish \n");

    fprintf(stdout, "Possible values are error code :"
            "ok, critical, fail, noresponse\n");
#endif
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
        { "help", no_argument, NULL, 'h'}
    };

    atexit(atexit_handler);

    init_err_codes();
    init_stage_names();
    try {

        while ((opt = getopt_long(argc, argv, "s:i:e:f:h",
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

    if ((ret = init_svc ()) != SDK_RET_OK) {
        fprintf(stderr, "Service (%s) initialization failed, err %u",
                svc_name.c_str(), ret);
    }

    printf("\n Main thread waiting .......... \n");
    while (1) {
        sleep(10);
    }
}

