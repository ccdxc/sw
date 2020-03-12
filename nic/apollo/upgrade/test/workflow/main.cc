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
#include "nic/apollo/upgrade/svc/upgrade.hpp"
#include "nic/apollo/upgrade/include/upgrade.hpp"

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
#endif
    return UPG_STATUS_OK;
}

static void
upg_event_handler (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    upg_event_msg_t *event = (upg_event_msg_t *)msg->data();
    upg_event_msg_t resp;

    printf("Received UPG IPC event stage %s \n",
                    upg_stage2str(event->stage));

    resp.stage          = event->stage;
    resp.rsp_status     = stage_to_upg_status(event->stage);
    resp.rsp_thread_id  = svc_thread_id;
    strncpy(resp.rsp_thread_name,svc_name.c_str(),sizeof(resp.rsp_thread_name));
    sdk::ipc::respond(msg, &resp, sizeof(resp));
}

void
init (void *ctxt)
{
    sdk::ipc::subscribe(PDS_IPC_MSG_ID_UPGRADE, upg_event_handler, NULL);
    sdk::ipc::reg_request_handler(PDS_IPC_MSG_ID_UPGRADE,
                                  upg_event_handler, NULL);
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
    while (1);
}

