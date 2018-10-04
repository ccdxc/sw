// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <getopt.h>
#include <grpc++/grpc++.h>
#include "gen/proto/types.grpc.pb.h"
#include "gen/proto/port.grpc.pb.h"
#include "gen/proto/debug.grpc.pb.h"

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
using port::PortInfoGetRequest;
using port::PortInfoGetRequestMsg;
using port::PortInfoGetResponse;
using port::PortInfoGetResponseMsg;
using port::PortDeleteRequest;
using port::PortDeleteRequestMsg;
using port::PortDeleteResponseMsg;
using port::PortDeleteResponse;

using debug::Debug;
using debug::GenericOpnRequestMsg;
using debug::GenericOpnRequest;
using debug::GenericOpnResponseMsg;
using debug::GenericOpnResponse;

typedef enum port_op_e {
    PORT_OP_NONE,
    PORT_OP_CREATE,
    PORT_OP_GET,
    PORT_OP_UPDATE,
    PORT_OP_DELETE
} port_op_t;

typedef struct port_info_s {
    uint32_t             port_id;
    port_op_t            op;
    port::PortSpeed      speed;
    uint32_t             mac_id;
    uint32_t             mac_ch;
    uint8_t              num_lanes;
    port::PortType       port_type;
    port::PortAdminState admin_state;
    port::PortFecType    fec_type;
    uint32_t             db_time;
    uint32_t             mtu;
    bool                 an_enable;

    // bool to track updates
    bool                 update_an;
    bool                 update_mtu;
    bool                 update_db_time;
    bool                 port_id_set;
} port_info_t;

typedef struct debug_info_s {
    uint32_t opn;
    uint32_t val1;
    uint32_t val2;
    uint32_t val3;
    uint32_t val4;
} debug_info_t;

port_info_t  port_info;
debug_info_t debug_info;
uint64_t     vrf_id = 1;
bool         invoke_debug  = false;
std::string  svc_endpoint_ = "localhost:50054";

#define MAX_MAC_STATS 89
#define PRINT_LOG(format, ...) \
    printf(format"\n", __VA_ARGS__);

std::string mac_stats[MAX_MAC_STATS] = {
    "Frames Received OK",
    "Frames Received All (Good/Bad Frames)",
    "Frames Received with Bad FCS",
    "Frames with any bad (CRC, Length, Align)",
    "Octets Received in Good Frames",
    "Octets Received (Good/Bad Frames)",
    "Frames Received with Unicast Address",
    "Frames Received with Multicast Address",
    "Frames Received with Broadcast Address",
    "Frames Received of type PAUSE",
    "Frames Received with Bad Length",
    "Frames Received Undersized",
    "Frames Received Oversized",
    "Fragments Received",
    "Jabber Received",
    "Priority Pause Frames",
    "Stomped CRC",
    "Frame Too Long",
    "Rx VLAN Frames (Good)",
    "Frames Dropped (Buffer Full)",
    "Frames Received Length<64",
    "Frames Received Length=64",
    "Frames Received Length=65~127",
    "Frames Received Length=128~255",
    "Frames Received Length=256~511",
    "Frames Received Length=512~1023",
    "Frames Received Length=1024~1518",
    "Frames Received Length=1519~2047",
    "Frames Received Length=2048~4095",
    "Frames Received Length=4096~8191",
    "Frames Received Length=8192~9215",
    "Frames Received Length>=9216",
    "Frames Transmitted OK",
    "Frames Transmitted All (Good/Bad Frames)",
    "Frames Transmitted Bad",
    "Octets Transmitted Good",
    "Octets Transmitted Total (Good/Bad)",
    "Frames Transmitted with Unicast Address",
    "Frames Transmitted with Multicast Address",
    "Frames Transmitted with Broadcast Address",
    "Frames Transmitted of type PAUSE",
    "Frames Transmitted of type PriPAUSE",
    "Frames Transmitted VLAN",
    "Frames Transmitted Length<64",
    "Frames Transmitted Length=64",
    "Frames Transmitted Length=65~127",
    "Frames Transmitted Length=128~255",
    "Frames Transmitted Length=256~511",
    "Frames Transmitted Length=512~1023",
    "Frames Transmitted Length=1024~1518",
    "Frames Transmitted Length=1519~2047",
    "Frames Transmitted Length=2048~4095",
    "Frames Transmitted Length=4096~8191",
    "Frames Transmitted Length=8192~9215",
    "Frames Transmitted Length>=9216",
    "Pri-0 Frames Transmitted",
    "Pri-1 Frames Transmitted",
    "Pri-2 Frames Transmitted",
    "Pri-3 Frames Transmitted",
    "Pri-4 Frames Transmitted",
    "Pri-5 Frames Transmitted",
    "Pri-6 Frames Transmitted",
    "Pri-7 Frames Transmitted",
    "Pri-0 Frames Received",
    "Pri-1 Frames Received",
    "Pri-2 Frames Received",
    "Pri-3 Frames Received",
    "Pri-4 Frames Received",
    "Pri-5 Frames Received",
    "Pri-6 Frames Received",
    "Pri-7 Frames Received",
    "Transmit Pri-0 Pause 1US Count",
    "Transmit Pri-1 Pause 1US Count",
    "Transmit Pri-2 Pause 1US Count",
    "Transmit Pri-3 Pause 1US Count",
    "Transmit Pri-4 Pause 1US Count",
    "Transmit Pri-5 Pause 1US Count",
    "Transmit Pri-6 Pause 1US Count",
    "Transmit Pri-7 Pause 1US Count",
    "Receive Pri-0 Pause 1US Count",
    "Receive Pri-1 Pause 1US Count",
    "Receive Pri-2 Pause 1US Count",
    "Receive Pri-3 Pause 1US Count",
    "Receive Pri-4 Pause 1US Count",
    "Receive Pri-5 Pause 1US Count",
    "Receive Pri-6 Pause 1US Count",
    "Receive Pri-7 Pause 1US Count",
    "Receive Standard Pause 1US Count",
    "Frames Truncated",
};

static void
print_port_info(void)
{
    std::cout << "PORT INFO" << std::endl;
    std::cout << "Port: " << port_info.port_id
              << std::endl
              << "op: "  << port_info.op
              << std::endl
              << "speed: " << port_info.speed
              << std::endl
              << "mac_id: " << port_info.mac_id
              << std::endl
              << "mac_ch: " << port_info.mac_ch
              << std::endl
              << "num_lanes: " << static_cast<int>(port_info.num_lanes)
              << std::endl
              << "port_type: " << port_info.port_type
              << std::endl
              << "admin_state: " << port_info.admin_state
              << std::endl
              << "fec_type: " << port_info.fec_type
              << std::endl
              << "db_time: " << port_info.db_time
              << std::endl
              << "mtu: " << port_info.mtu
              << std::endl
              << "an_enable: " << port_info.an_enable
              << std::endl;
}

static void
print_debug_info(void)
{
    std::cout << "DEBUG INFO" << std::endl;
    std::cout << "Opn: " << debug_info.opn
              << std::endl
              << "val1: " << debug_info.val1
              << std::endl
              << "val2: " << debug_info.val2
              << std::endl
              << "val3: " << debug_info.val3
              << std::endl
              << "val4: " << debug_info.val4
              << std::endl;
}

class port_client {
public:
    port_client(std::shared_ptr<Channel> channel):
                    port_stub_(Port::NewStub(channel)),
                    debug_stub_(Debug::NewStub(channel))
                    {}

    bool port_handle_api_status(types::ApiStatus api_status,
                                uint32_t port_id) {
        switch(api_status) {
            case types::API_STATUS_OK:
                return true;

            case types::API_STATUS_NOT_FOUND:
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
        spec->set_port_speed(port_info->speed);
        spec->set_num_lanes(port_info->num_lanes);
        spec->set_port_type(::port::PORT_TYPE_ETH);
        spec->set_admin_state(port_info->admin_state);
        spec->set_fec_type(port_info->fec_type);
        spec->set_mac_id(port_info->mac_id);
        spec->set_mac_ch(port_info->mac_ch);

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
        ClientContext       get_context;
        PortGetRequest      *get_req;
        PortGetRequestMsg   get_req_msg;
        PortGetResponseMsg  get_rsp_msg;
        PortSpec            *spec;
        PortRequestMsg      req_msg;
        PortResponseMsg     rsp_msg;
        ClientContext       context;
        Status               status;

        if (port_info->port_id_set == false) {
            std::cout << "port not specified" << std::endl;
            return -1;
        }

        // get the port details
        get_req = get_req_msg.add_request();
        get_req->mutable_key_or_handle()->set_port_id(port_info->port_id);

        status = port_stub_->PortGet(&context, get_req_msg, &get_rsp_msg);
        if (status.ok()) {
            PortGetResponse get_response = get_rsp_msg.response(0);

            // params which cannot be updated
            port_info->mac_id = get_response.spec().mac_id();
            port_info->mac_ch = get_response.spec().mac_ch();
            port_info->num_lanes = get_response.spec().num_lanes();
            port_info->port_type = get_response.spec().port_type();

            // params which can be updated

            if (port_info->admin_state == port::PORT_ADMIN_STATE_NONE) {
                port_info->admin_state = get_response.spec().admin_state();
            } else {
                PRINT_LOG("port: %d, udpating admin_state: %d",
                          port_info->port_id, port_info->admin_state);
            }

            if (port_info->speed == port::PORT_SPEED_NONE) {
                port_info->speed = get_response.spec().port_speed();
            } else {
                PRINT_LOG("port: %d, udpating speed: %d",
                          port_info->port_id, port_info->speed);
            }

            if (port_info->fec_type == port::PORT_FEC_TYPE_NONE) {
                port_info->fec_type = get_response.spec().fec_type();
            } else {
                PRINT_LOG("port: %d, udpating fec_type: %d",
                          port_info->port_id, port_info->fec_type);
            }

            if (port_info->update_db_time == false) {
                port_info->db_time = get_response.spec().debounce_time();
            } else {
                PRINT_LOG("port: %d, udpating db_time: %d",
                          port_info->port_id, port_info->db_time);
            }

            if (port_info->update_an == false) {
                port_info->an_enable = get_response.spec().auto_neg_enable();
            } else {
                PRINT_LOG("port: %d, udpating an_enable: %d",
                          port_info->port_id, port_info->an_enable);
            }

            if (port_info->update_mtu == false) {
                port_info->mtu = get_response.spec().mtu();
            } else {
                PRINT_LOG("port: %d, udpating mtu: %d",
                          port_info->port_id, port_info->mtu);
            }
        } else {
            std::cout << "Failed to get port: " << port_info->port_id
                      << std::endl;
            return -1;
        }

        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->set_port_id(port_info->port_id);
        spec->set_port_speed(port_info->speed);
        spec->set_admin_state(port_info->admin_state);
        spec->set_fec_type(port_info->fec_type);
        spec->set_debounce_time(port_info->db_time);
        spec->set_mtu(port_info->mtu);
        spec->set_auto_neg_enable(port_info->an_enable);

        status = port_stub_->PortUpdate(&get_context, req_msg, &rsp_msg);
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

        if (port_info->port_id_set == true) {
            req->mutable_key_or_handle()->set_port_id(port_info->port_id);
        }

        // port get
        status = port_stub_->PortGet(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            for (int i = 0; i < rsp_msg.response_size(); ++i) {
                uint32_t port_id =
                        rsp_msg.response(i).spec().key_or_handle().port_id();

                if (port_handle_api_status(
                        rsp_msg.response(i).api_status(), port_id) == true) {
                    std::cout
                      << "Port Get succeeded for port "
                      << port_id << std::endl
                      << " Port oper status: "
                      << rsp_msg.response(i).status().oper_status() << std::endl
                      << " Port type: "
                      << rsp_msg.response(i).spec().port_type() << std::endl
                      << " Admin state: "
                      << rsp_msg.response(i).spec().admin_state() << std::endl
                      << " Port speed: "
                      << rsp_msg.response(i).spec().port_speed() << std::endl
                      << " MAC ID: "
                      << rsp_msg.response(i).spec().mac_id() << std::endl
                      << " MAC channel: "
                      << rsp_msg.response(i).spec().mac_ch() << std::endl
                      << " Num lanes: "
                      << rsp_msg.response(i).spec().num_lanes() << std::endl
                      << " Fec Type: "
                      << rsp_msg.response(i).spec().fec_type() << std::endl
                      << " Debounce Time: "
                      << rsp_msg.response(i).spec().debounce_time() << std::endl
                      << " MTU: "
                      << rsp_msg.response(i).spec().mtu() << std::endl
                      << " AutoNeg: "
                      << rsp_msg.response(i).spec().auto_neg_enable() << std::endl;

                    for (int j = 0; j < MAX_MAC_STATS; ++j) {
                        printf("%-41s: %lu\n",
                               mac_stats[j].c_str(),
                               rsp_msg.response(i).
                                       stats().mac_stats(j).count());
                    }
                } else {
                    std::cout << "Port Get failed for port "
                              << port_info->port_id
                              << " , status = "
                              << rsp_msg.response(0).api_status()
                              << std::endl;
                }
            }   // for

            return 0;
        }   // status ok

        std::cout << "Port Get failed for port "
                  << port_info->port_id
                  << " , error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return -1;
    }

    int port_info_get(port_info_t *port_info) {
        PortInfoGetRequest      *req;
        PortInfoGetRequestMsg   req_msg;
        PortInfoGetResponseMsg  rsp_msg;
        ClientContext           context;
        Status                  status;

        req = req_msg.add_request();
        (void)req;

        // port info get
        status = port_stub_->PortInfoGet(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            for (int i = 0; i < rsp_msg.response_size(); ++i) {
                uint32_t port_id =
                        rsp_msg.response(i).spec().key_or_handle().port_id();

                if (port_handle_api_status(
                        rsp_msg.response(i).api_status(), port_id) == true) {
                    std::cout
                      << "Port Info Get succeeded for port "
                      << port_id << std::endl
                      << " Port Type: "
                      << rsp_msg.response(i).spec().port_type() << std::endl
                      << " Num Lanes: "
                      << rsp_msg.response(i).spec().num_lanes() << std::endl
                      << " Breakout Modes: " << std::endl;
                     for (int j = 0;
                          j < rsp_msg.response(i).spec().breakout_modes_size();
                          ++j) {
                         std::cout << rsp_msg.response(i).spec().breakout_modes(j)
                                   << std::endl;
                     }
                }
                else {
                    break;
                }
            }   // for

            return 0;
        }   // status ok

        std::cout << "Port Info Get failed for port "
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

    int generic_opn(debug_info_t *debug_info) {
        GenericOpnRequestMsg  req_msg;
        GenericOpnRequest     *req;
        GenericOpnResponseMsg rsp_msg;
        ClientContext         context;
        Status                status;

        req = req_msg.add_request();
        req->set_opn(debug_info->opn);
        req->set_val1(debug_info->val1);
        req->set_val2(debug_info->val2);
        req->set_val3(debug_info->val3);
        req->set_val4(debug_info->val4);

        status = debug_stub_->GenericOpn(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            if (rsp_msg.response(0).api_status() == types::API_STATUS_OK) {
                std::cout << "Generic Operation succeeded"
                          << std::endl;
            } else {
                return -1;
            }
            return 0;
        }

        std::cout << "Generic Operation failed"
                  << " , error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return -1;
    }

private:
    std::unique_ptr<Port::Stub>  port_stub_;
    std::unique_ptr<Debug::Stub> debug_stub_;
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
            return port::PORT_SPEED_NONE;

    }

    return port::PORT_SPEED_NONE;
}

static port::PortType
parse_port_type(char *port_type)
{
    if (!(strcmp(port_type, "eth"))) {
        return port::PORT_TYPE_ETH;
    } else if (!(strcmp(port_type, "mgmt"))) {
        return port::PORT_TYPE_MGMT;
    }

    return port::PORT_TYPE_NONE;
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
    int option_index = 0;
	struct option longopts[] = {
	   { "create",       no_argument,       NULL, 'c' },
	   { "get",          no_argument,       NULL, 'r' },
	   { "update",       no_argument,       NULL, 'u' },
	   { "delete",       no_argument,       NULL, 'd' },
	   { "dry_run",      no_argument,       NULL, 't' },
	   { "help",         no_argument,       NULL, 'h' },
	   { "port",         required_argument, NULL, 'p' },
	   { "speed",        required_argument, NULL,  0  },
	   { "admin_state",  required_argument, NULL,  0  },
	   { "mac_id",       required_argument, NULL,  0  },
	   { "mac_ch",       required_argument, NULL,  0  },
	   { "num_lanes",    required_argument, NULL,  0  },
	   { "port_type",    required_argument, NULL,  0  },
	   { "fec_type",     required_argument, NULL,  0  },
	   { "db_time",      required_argument, NULL,  0  },
	   { "an_enable",    required_argument, NULL,  0  },
	   { "mtu",          required_argument, NULL,  0  },
	   { "debug_op",     required_argument, NULL, 'o' },
	   { "debug_val1",   required_argument, NULL, 'w' },
	   { "debug_val2",   required_argument, NULL, 'x' },
	   { "debug_val3",   required_argument, NULL, 'y' },
	   { "debug_val4",   required_argument, NULL, 'z' },
	   { "svc_endpoint", required_argument, NULL, 'g' },
	   { 0,              0,                 0,     0 }
	};

    // parse CLI options
    while ((oc = getopt_long(argc, argv,
                             ":crudthp:0:o:w:x:y:z:g:W;",
                             longopts, &option_index)) != -1) {
        switch (oc) {
        case 'p':
            if (optarg) {
                port_info.port_id = atoi(optarg);
            } else {
                fprintf(stderr, "port not specified\n");
                print_usage(argv);
                exit(1);
            }
            port_info.port_id_set = true;
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

        case 0:
            if (optarg) {
                if (!strcmp("speed", longopts[option_index].name)) {
                    port_info.speed = parse_port_speed(atoi(optarg));
                } else if (!strcmp("admin_state",
                                   longopts[option_index].name)) {
                    port_info.admin_state = parse_admin_state(optarg);
                } else if (!strcmp("mac_id",    longopts[option_index].name)) {
                    port_info.mac_id      = atoi(optarg);
                } else if (!strcmp("mac_ch",    longopts[option_index].name)) {
                    port_info.mac_ch      = atoi(optarg);
                } else if (!strcmp("num_lanes", longopts[option_index].name)) {
                    port_info.num_lanes   = atoi(optarg);
                } else if (!strcmp("port_type",  longopts[option_index].name)) {
                    port_info.port_type   = parse_port_type(optarg);
                } else if (!strcmp("fec_type",  longopts[option_index].name)) {
                    port_info.fec_type    = parse_fec_type(optarg);
                } else if (!strcmp("db_time",   longopts[option_index].name)) {
                    port_info.update_db_time = true;
                    port_info.db_time     = atoi(optarg);
                } else if (!strcmp("an_enable", longopts[option_index].name)) {
                    port_info.update_an = true;
                    port_info.an_enable   = parse_an(atoi(optarg));
                } else if (!strcmp("mtu", longopts[option_index].name)) {
                    port_info.update_mtu = true;
                    port_info.mtu   = parse_an(atoi(optarg));
                }
            } else {
                fprintf(stderr, "%s not specified\n",
                        longopts[option_index].name);
                print_usage(argv);
                exit(1);
            }
            break;

        case 't':
            print_port_info();
            print_debug_info();
            exit(0);
            break;

        case 'o':
            if (optarg) {
                debug_info.opn = atoi(optarg);
                invoke_debug = true;
            } else {
                fprintf(stderr, "opn not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'w':
            if (optarg) {
                debug_info.val1 = atoi(optarg);
            } else {
                fprintf(stderr, "val1 not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'x':
            if (optarg) {
                debug_info.val2 = atoi(optarg);
            } else {
                fprintf(stderr, "val2 not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'y':
            if (optarg) {
                debug_info.val3 = atoi(optarg);
            } else {
                fprintf(stderr, "val2 not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'z':
            if (optarg) {
                debug_info.val4 = atoi(optarg);
            } else {
                fprintf(stderr, "val2 not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'g':
            if (optarg) {
                svc_endpoint_ = optarg;
            } else {
                fprintf(stderr, "svc endpoint not specified\n");
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

    return 0;
}

// main test driver
int
main (int argc, char** argv)
{
    parse_options(argc, argv);

    std::string svc_endpoint = svc_endpoint_;

    port_client pClient(
        grpc::CreateChannel(svc_endpoint, grpc::InsecureChannelCredentials()));

    switch (port_info.op) {
        case PORT_OP_CREATE:
            pClient.port_create(&port_info);
            break;

        case PORT_OP_GET:
            pClient.port_get(&port_info);
            pClient.port_info_get(&port_info);
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

    if (invoke_debug == true) {
        pClient.generic_opn(&debug_info);
    }

    return 0;
}
