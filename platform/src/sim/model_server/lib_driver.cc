#include <iostream>
#include <deque>
#include <thread>
#include <cmath>
#include <map>
#include <pthread.h>
#include <condition_variable>
#include <grpc++/grpc++.h>
#include <zmq.h>

#include "gen/proto/interface.grpc.pb.h"
#include "gen/proto/internal.grpc.pb.h"
#include "gen/proto/rdma.grpc.pb.h"

#include "nic/model_sim/include/buf_hdr.h"
#include "nic/model_sim/include/lib_model_client.h"
#include "nic/utils/host_mem/host_mem.hpp"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using namespace internal;
using namespace grpc;
using namespace std;


static shared_ptr<Channel> hal_channel = 0;
static void hal_channel_create(void)
{
    char *grpc_port_env;
    std::string grpc_ep = "localhost:";

    if (!hal_channel) {
        if ((grpc_port_env = getenv("HAL_GRPC_PORT")) != NULL) {
            grpc_ep.append(grpc_port_env);
        } else {
            grpc_ep.append("50054");
        }
        hal_channel = CreateChannel(grpc_ep, InsecureChannelCredentials());
    }
}

static shared_ptr<Internal::Stub> internal_svc = 0;
shared_ptr<Internal::Stub> GetInternalStub (void)
{
    if (internal_svc)
        return internal_svc;
    
    hal_channel_create();
    StubOptions options;

    internal_svc = Internal::NewStub(hal_channel, options);
    return internal_svc;
}

extern "C" int hal_alloc_hbm_address(const char *handle, 
                                     u_int64_t *addr,
                                     u_int32_t *size)
{
    ClientContext context;
    AllocHbmAddressRequestMsg req_msg;
    AllocHbmAddressResponseMsg resp_msg;

    shared_ptr<Internal::Stub> internal_svc = GetInternalStub();

    auto req = req_msg.add_request();
    req->set_handle(handle);

    auto status = internal_svc->AllocHbmAddress(&context, req_msg, &resp_msg);
    if (!status.ok()) {
        cout << "[ERROR] " << __FUNCTION__
             << ": handle = " << handle
             << ", Status = " << status.error_code() << ":" << status.error_message()
             << endl;
        return -1;
    }

    *addr = resp_msg.response(0).addr();
    *size = resp_msg.response(0).size();
    return 0;
}
