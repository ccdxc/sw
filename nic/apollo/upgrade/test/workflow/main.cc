//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/sdk/upgrade/include/ev.hpp"

static sdk::event_thread::event_thread *g_svc_server_thread;
static std::string                      test_config_json;
static std::string                      svc_name;
static uint32_t                         svc_thread_id;


static inline upg_status_t
stage_to_upg_status (upg_stage_t evt)
{
#if 0
    std::string   test_id="TEST_ID_1";  // TODO: need to pass in mgs
    std::string   description;
    std::string   upg_stage = std::to_string(evt);
    upg_status_t  upg_status;


    boost::property_tree::ptree pt;
    std::ifstream json_cfg(test_config_json.c_str());
    read_json(json_cfg, pt);

    description   = pt.get<std::string>(test_id + "." + "description");
    upg_status    = (upg_status_t) pt.get<int>(test_id + "." + upg_stage);
    return upg_status;

   // TODO: need to do this timeout injection from test json
   if (UPG_STAGE_PREPARE == evt) {
       printf("\nSleeping ...\n");
       sleep (5);
       printf("\nSleep done ...\n");
   }
#endif
    return UPG_STATUS_OK;
}

static sdk_ret_t
test_upgrade (sdk::upg::upg_ev_params_t *params)
{
    params->response_cb(SDK_RET_OK, params->response_cookie);
    return SDK_RET_IN_PROGRESS;
}

static void
upg_ev_fill (sdk::upg::upg_ev_t *ev)
{
    ev->svc_id = svc_thread_id;
    strncpy(ev->svc_name, svc_name.c_str(), sizeof(ev->svc_name));
    ev->compat_check_hdlr = test_upgrade;
    ev->start_hdlr = test_upgrade;
    ev->backup_hdlr = test_upgrade;
    ev->prepare_hdlr = test_upgrade;
    ev->prepare_switchover_hdlr = test_upgrade;
    ev->switchover_hdlr = test_upgrade;
    ev->rollback_hdlr = test_upgrade;
    ev->ready_hdlr = test_upgrade;
    ev->sync_hdlr = test_upgrade;
    ev->repeal_hdlr = test_upgrade;
    ev->finish_hdlr = test_upgrade;
    ev->exit_hdlr = test_upgrade;
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
    fprintf(stdout, "Usage : %s -s|--startwith <start-up-config.json>"
            " -t|--testwith <test-config-data.json>\n", argv[0]);
}

static void
atexit_handler (void)
{
    printf("Exiting from svc ...");
}

int
main (int argc, char **argv)
{

    boost::property_tree::ptree pt;
    std::string                 startup_json;
    sdk_ret_t                   ret;
    int                         opt;

    struct option longopts[] = {
        { "startwith",  required_argument, NULL, 's' },
        { "testwith",   required_argument, NULL, 't' },
        { "help",       no_argument,       NULL, 'h'  }
    };
    atexit(atexit_handler);
    while ((opt = getopt_long(argc, argv, ":hs:t:W;", longopts, NULL)) != -1) {
        switch (opt) {
            case 's':
                if (optarg) {
                    startup_json = std::string(optarg);
                } else {
                    fprintf(stderr, "Service startup json "
                            " file is not specified\n");
                    print_usage(argv);
                        exit(1);
                }
                break;

            case 't':
                if (optarg) {
                    test_config_json = std::string(optarg);
                } else {
                    fprintf(stderr, "Test data json file is not specified\n");
                    print_usage(argv);
                    exit(1);
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


    if (access(startup_json.c_str(), R_OK) < 0) {
        fprintf(stderr, "Service start up json is not accessible\n");
        exit(1);
    }

    if (access(test_config_json.c_str(), R_OK) < 0) {
        fprintf(stderr, "Test configuration json is not accessible\n");
        exit(1);
    }


    try {
        std::ifstream json_cfg(startup_json.c_str());
        read_json(json_cfg, pt);
        svc_name        = pt.get<std::string>("svc_name");
        svc_thread_id   = pt.get<int>("svc_thread_id");
    } catch (...) {
        fprintf(stderr, " startup json doesn't have svc name and thread id.\n");
        exit(1);
    }

    if ((ret = init_svc ()) != SDK_RET_OK) {
        fprintf(stderr, "Service (%s) initialization failed, err %u",
                svc_name.c_str(), ret);
    }

    printf("\n Main thread waiting .......... \n");
    while (1) {
        sleep(10);
    }
}

