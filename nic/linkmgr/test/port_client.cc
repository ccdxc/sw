// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <getopt.h>
#include <grpc++/grpc++.h>
#include "nic/gen/proto/hal/types.grpc.pb.h"
#include "nic/gen/proto/hal/port.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using port::Port;
using port::PortSpec;
using port::PortRequestMsg;
using port::PortResponse;
using port::PortResponseMsg;
using port::PortGetRequest;
using port::PortGetRequestMsg;
using port::PortGetResponse;
using port::PortGetResponseMsg;
using port::PortDeleteRequest;
using port::PortDeleteRequestMsg;
using port::PortDeleteResponseMsg;
using port::PortDeleteResponse;

typedef enum port_op_e {
    PORT_OP_NONE,
    PORT_OP_CREATE,
    PORT_OP_GET,
    PORT_OP_UPDATE,
    PORT_OP_DELETE
} port_op_t;

typedef struct port_info_s {
    bool                 an_enable;
    uint8_t              num_lanes;
    uint32_t             port_id;
    uint32_t             db_time;
    port_op_t            op;
    port::PortSpeed      speed;
    port::PortFecType    fec_type;
    port::PortAdminState admin_state;
} port_info_t;

port_info_t port_info;

std::string  linkmgr_svc_endpoint_ = "localhost:50053";
uint64_t vrf_id = 1;

port::PortOperStatus port_oper_status = port::PORT_OPER_STATUS_NONE;
port::PortType       port_type        = port::PORT_TYPE_NONE;
port::PortAdminState port_admin_state = port::PORT_ADMIN_STATE_NONE;
port::PortSpeed      port_speed       = port::PORT_SPEED_NONE;

static void
print_port_info(void)
{
    std::cout << "Port: " << port_info.port_id
              << std::endl
              << "op: "  << port_info.op
              << std::endl
              << "speed: " << port_info.speed
              << std::endl
              << "admin_state: " << port_info.admin_state
              << std::endl
              << "num_lanes: " << static_cast<int>(port_info.num_lanes)
              << std::endl
              << "an_enable: " << port_info.an_enable
              << std::endl
              << "fec_type: " << port_info.fec_type
              << std::endl
              << "db_time: " << port_info.db_time
              << std::endl;
}

class port_client {
public:
    port_client(std::shared_ptr<Channel> channel) : port_stub_(Port::NewStub(channel)) {}

    bool port_handle_api_status(types::ApiStatus api_status,
                                uint32_t port_id) {
        switch(api_status) {
            case types::API_STATUS_OK:
                return true;

            case types::API_STATUS_NOT_FOUND:
                std::cout << "Port "
                          << port_id
                          << " not found"
                          << std::endl;
                return false;

            case types::API_STATUS_EXISTS_ALREADY:
                std::cout << "Port "
                          << port_id
                          << " exists already"
                          << std::endl;
                return false;

            default:
                assert(0);
        }

        return true;
    }

    int port_create(port_info_t *port_info) {
        PortSpec            *spec;
        PortRequestMsg      req_msg;
        PortResponseMsg     rsp_msg;
        ClientContext       context;
        Status               status;

        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->set_port_id(port_info->port_id);
        spec->mutable_meta()->set_vrf_id(vrf_id);
        spec->set_port_speed(port_info->speed);
        spec->set_num_lanes(port_info->num_lanes);
        spec->set_port_type(::port::PORT_TYPE_ETH);
        spec->set_admin_state(port_info->admin_state);

        status = port_stub_->PortCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            if (port_handle_api_status(
                rsp_msg.response(0).api_status(), port_info->port_id) == true) {
                std::cout << "Port create succeeded for port "
                          << port_info->port_id
                          << std::endl;
            } else {
                return -1;
            }

            return 0;
        }

        std::cout << "Port create failed for port "
                  << port_info->port_id
                  << " , error = " << rsp_msg.response(0).api_status()
                  << std::endl;
        return -1;
    }

    int port_update(port_info_t *port_info) {
        PortSpec            *spec;
        PortRequestMsg      req_msg;
        PortResponseMsg     rsp_msg;
        ClientContext       context;
        Status               status;

        spec = req_msg.add_request();

        spec->mutable_key_or_handle()->set_port_id(port_info->port_id);
        spec->mutable_meta()->set_vrf_id(vrf_id);
        spec->set_port_speed(port_info->speed);
        spec->set_admin_state(port_info->admin_state);
        spec->set_fec_type(port_info->fec_type);
        spec->set_debounce_time(port_info->db_time);
        spec->set_auto_neg_enable(port_info->an_enable);

        status = port_stub_->PortUpdate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            if (port_handle_api_status(
                    rsp_msg.response(0).api_status(), port_info->port_id) == true) {
                std::cout << "Port update succeeded for port "
                          << port_info->port_id
                          << std::endl;
            } else {
                return -1;
            }

            return 0;
        }

        std::cout << "Port update failed for port "
                  << port_info->port_id
                  << " , error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return -1;
    }

    int port_get(port_info_t *port_info) {
        PortGetRequest      *req;
        PortGetRequestMsg   req_msg;
        PortGetResponseMsg  rsp_msg;
        ClientContext       context;
        Status              status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_port_id(port_info->port_id);
        req->mutable_meta()->set_vrf_id(vrf_id);

        // port get
        status = port_stub_->PortGet(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            if (port_handle_api_status(
                    rsp_msg.response(0).api_status(), port_info->port_id) == true) {
                std::cout << "Port Get succeeded for port "
                          << port_info->port_id << std::endl
                          << " Port oper status: "
                          << rsp_msg.response(0).status().oper_status() << std::endl
                          << " Port type: "
                          << rsp_msg.response(0).spec().port_type() << std::endl
                          << " Admin state: "
                          << rsp_msg.response(0).spec().admin_state() << std::endl
                          << " Port speed: "
                          << rsp_msg.response(0).spec().port_speed() << std::endl
                          << " MAC ID: "
                          << rsp_msg.response(0).spec().mac_id() << std::endl
                          << " MAC channel: "
                          << rsp_msg.response(0).spec().mac_ch() << std::endl
                          << " Num lanes: "
                          << rsp_msg.response(0).spec().num_lanes() << std::endl;
            } else {
                return -1;
            }

            return 0;
        }

        std::cout << "Port Get failed for port "
                  << port_info->port_id
                  << " , error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return -1;
    }

    int port_delete(port_info_t *port_info) {
        PortDeleteRequest      *req;
        PortDeleteRequestMsg   req_msg;
        PortDeleteResponseMsg     rsp_msg;
        ClientContext          context;
        Status                 status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_port_id(port_info->port_id);
        req->mutable_meta()->set_vrf_id(vrf_id);

        // port get
        status = port_stub_->PortDelete(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            if (port_handle_api_status(
                    rsp_msg.response(0).api_status(), port_info->port_id) == true) {
                std::cout << "Port Delete succeeded for port "
                          << port_info->port_id << std::endl;
            } else {
                return -1;
            }

            return 0;
        }

        std::cout << "Port Delete failed for port"
                  << port_info->port_id
                  << " , error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return -1;
    }

private:
    std::unique_ptr<Port::Stub> port_stub_;
};

static port::PortSpeed
parse_port_speed(int speed)
{
    switch(speed) {
        case 100:
            return port::PORT_SPEED_100G;

        case 50:
            return port::PORT_SPEED_50G;

        case 40:
            return port::PORT_SPEED_40G;

        case 25:
            return port::PORT_SPEED_25G;

        case 10:
            return port::PORT_SPEED_10G;

        default:
            return port::PORT_SPEED_100G;

    }

    return port::PORT_SPEED_100G;
}

static port::PortFecType
parse_fec_type(char *fec_type)
{
    if (!(strcmp(fec_type, "fc"))) {
        return port::PORT_FEC_TYPE_FC;
    } else if (!(strcmp(fec_type, "rs"))) {
        return port::PORT_FEC_TYPE_RS;
    }

    return port::PORT_FEC_TYPE_NONE;
}

static port::PortAdminState
parse_admin_state(char *admin_state)
{
    if (!(strcmp(admin_state, "up"))) {
        return port::PORT_ADMIN_STATE_UP;
    } else if (!(strcmp(admin_state, "down"))) {
        return port::PORT_ADMIN_STATE_DOWN;
    }

    return port::PORT_ADMIN_STATE_NONE;
}

static bool
parse_an(int an)
{
    if (an == 1) {
        return true;
    }

    return false;
}

static void inline
print_usage (char **argv)
{
}

static int
parse_options(int argc, char **argv)
{
    int oc = 0;
	struct option longopts[] = {
	   { "port",        required_argument, NULL, 'p' },
	   { "create",      optional_argument, NULL, 'c' },
	   { "get",         optional_argument, NULL, 'r' },
	   { "update",      optional_argument, NULL, 'u' },
	   { "delete",      optional_argument, NULL, 'd' },
	   { "speed",       optional_argument, NULL, 's' },
	   { "admin_state", optional_argument, NULL, 'e' },
	   { "fec_type",    optional_argument, NULL, 'f' },
	   { "db_time",     optional_argument, NULL, 'b' },
	   { "an_enable",   optional_argument, NULL, 'a' },
	   { "num_lanes",   optional_argument, NULL, 'n' },
	   { "dry_run",     optional_argument, NULL, 't' },
	   { "help",        optional_argument, NULL, 'h' },
	   { 0,             0,                 0,     0 }
	};

    // parse CLI options
    while ((oc = getopt_long(argc, argv, ":p:cruds:e:f:b:a:n:thW;", longopts, NULL)) != -1) {
        switch (oc) {
        case 'p':
            if (optarg) {
                port_info.port_id = atoi(optarg);
            } else {
                fprintf(stderr, "port not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'c':
            port_info.op = PORT_OP_CREATE;
            break;

        case 'r':
            port_info.op = PORT_OP_GET;
            break;

        case 'u':
            port_info.op = PORT_OP_UPDATE;
            break;

        case 'd':
            port_info.op = PORT_OP_DELETE;
            break;

        case 's':
            if (optarg) {
                port_info.speed = parse_port_speed(atoi(optarg));
            } else {
                fprintf(stderr, "speed not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'e':
            if (optarg) {
                port_info.admin_state = parse_admin_state(optarg);
            } else {
                fprintf(stderr, "admin_state not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'f':
            if (optarg) {
                port_info.fec_type = parse_fec_type(optarg);
            } else {
                fprintf(stderr, "fec_type not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'b':
            if (optarg) {
                port_info.db_time = atoi(optarg);
            } else {
                fprintf(stderr, "db_time not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'a':
            if (optarg) {
                port_info.an_enable = parse_an(atoi(optarg));
            } else {
                fprintf(stderr, "an_enable not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'n':
            if (optarg) {
                port_info.num_lanes = atoi(optarg);
            } else {
                fprintf(stderr, "num_lanes not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 't':
            print_port_info();
            exit(0);
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

    return 0;
}

// main test driver
int
main (int argc, char** argv)
{
    parse_options(argc, argv);

    std::string svc_endpoint = linkmgr_svc_endpoint_;

    port_client pClient(
        grpc::CreateChannel(svc_endpoint, grpc::InsecureChannelCredentials()));

    switch (port_info.op) {
        case PORT_OP_CREATE:
            pClient.port_create(&port_info);
            break;

        case PORT_OP_GET:
            pClient.port_get(&port_info);
            break;

        case PORT_OP_UPDATE:
            pClient.port_update(&port_info);
            break;

        case PORT_OP_DELETE:
            pClient.port_delete(&port_info);
            break;

        default:
            break;
    }

    return 0;
}
