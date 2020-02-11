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
    evutil_timer_start(EV_A_ &ncsi_rx_timer, NcsiMgr::RecvNcsiCtrlPkts, this, 0.0, 0.1);

    return 0;
}

void
NcsiMgr::RecvNcsiCtrlPkts(void *obj)
{
    int pkt_sz = 0;
    NcsiMgr *mgr = (NcsiMgr *)obj;

    //printf("Waiting for ncsi packet from transport\n");
    pkt_sz = mgr->xport->RecvPkt(&PktData, 1500);
    if (pkt_sz < 0)
        return;
#if 0
    printf("Received NCSI Packet:\n");

    for (int i=0; i < pkt_sz; i++) {
        if (!(i % 16))
            printf("\n 0x%04x: ", i);
        printf("0x%02x ", PktData[i]);
    }
    printf("\n");

    printf("Handling command now\n");
#endif

    mgr->ncsi_cmd_hndlr->HandleCmd(PktData, pkt_sz);
}

} // namespace ncsi
} // namespace platform
} // namespace sdk

