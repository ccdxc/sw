/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#include <cstdio>
#include "ncsi_mgr.h"

uint8_t PktData[1500];
evutil_timer ncsi_rx_timer;

namespace sdk {
namespace platform {
namespace ncsi {

int
NcsiMgr::Init(transport* xport_obj, shared_ptr<IpcService> ipc)
{
    xport = xport_obj;


    //Initialize the transport interface
    xport->Init();

    ncsi_cmd_hndlr = new CmdHndler(ipc, xport);

    memset(PktData, 0, sizeof(PktData));

    //Start timer thread to recv packets from transport interface
    this->loop = EV_DEFAULT;

    xport_fd = xport->GetFd();

    if (xport_fd < 0) {
        SDK_TRACE_INFO("Cannot find valid socket to receive NCSI packets");
        return -1;
    }

    evutil_add_fd(EV_A_ xport_fd, NcsiMgr::RecvNcsiCtrlPkts, NULL, this);
    return 0;
}

void
NcsiMgr::RecvNcsiCtrlPkts(void *obj)
{
    int pkt_sz = 0;
    NcsiMgr *mgr = (NcsiMgr *)obj;

    pkt_sz = mgr->xport->RecvPkt(&PktData, 1500);
    if (pkt_sz < 0)
        return;

    mgr->ncsi_cmd_hndlr->HandleCmd(PktData, pkt_sz);
}

} // namespace ncsi
} // namespace platform
} // namespace sdk

