//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <unistd.h>
#include <grpc++/grpc++.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "gen/proto/upgrade.grpc.pb.h"
#include "nic/apollo/include/globals.hpp"

using std::string;
using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

static std::string g_upg_svc_endpoint_;
static std::unique_ptr<pds::UpgSvc::Stub>  g_upg_svc_stub_;


static sdk_ret_t
send_upg_grpc (void)
{
    ClientContext           context;
    pds::UpgradeRequest     req;
    pds::UpgradeResponse    rsp;
    Status                  ret_status;
    pds::UpgradeSpec        *spec = req.mutable_request();

    spec->set_requesttype(pds::UpgradeRequestType::UPGRADE_REQUEST_START);
    spec->set_packagename("naples_fw.tar"); // unused in simulation
    spec->set_mode(pds::UpgradeMode::UPGRADE_MODE_GRACEFUL);

    ret_status = g_upg_svc_stub_->UpgRequest(&context, req, &rsp);
    printf("Upgrade response, grpc status ok %u, upgmgr rsp %u, upgmgr rspmsg %s\n",
           ret_status.ok(), rsp.status(), rsp.mutable_statusmsg()->c_str());
    if (!ret_status.ok() ||
        rsp.status() != pds::UpgradeStatus::UPGRADE_STATUS_OK) {
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

static void
svc_init (void)
{
    grpc_init();
    if (g_upg_svc_endpoint_.empty()) {
        g_upg_svc_endpoint_ = std::string("localhost:") +
            std::to_string(PDS_GRPC_PORT_UPGMGR);
    }
    std::shared_ptr<Channel> channel =
        grpc::CreateChannel(g_upg_svc_endpoint_,
                            grpc::InsecureChannelCredentials());
    g_upg_svc_stub_ = pds::UpgSvc::NewStub(channel);

}

int
main (int argc, char **argv)
{
    svc_init();
    if (send_upg_grpc() == SDK_RET_OK) {
        exit(0);
    } else {
        exit(1);
    }
}
