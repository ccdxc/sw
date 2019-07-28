//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/gen/proto/hal/debug.pb.h"
#include "nic/gen/proto/hal/debug.grpc.pb.h"
#include <grpc++/grpc++.h>

using grpc::Channel;
using ::debug::Debug;

std::shared_ptr<Channel>     channel;
std::unique_ptr<Debug::Stub> stub;

int
cli_init(char *grpc_server_port)
{
    grpc_init();

    channel =
        grpc::CreateChannel(grpc_server_port, grpc::InsecureChannelCredentials());

    stub    = ::debug::Debug::NewStub(channel);

    return 0;
}
