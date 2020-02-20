/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __NCSI_MGR_H__
#define __NCSI_MGR_H_

#include <string>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <arpa/inet.h>

#include "platform/evutils/include/evutils.h"

#include "transport.h"
#include "rbt_transport.h"
#include "mctp_transport.h"
#include "cmd_hndlr.h"

using namespace std;

namespace sdk {
namespace platform {
namespace ncsi {

class NcsiMgr {
public:

    int Init(transport* xport_obj, shared_ptr<IpcService> ipc);
private:
    transport *xport;
    CmdHndler *ncsi_cmd_hndlr;
    EV_P;
    static void RecvNcsiCtrlPkts(void *obj);
};

} // namespace ncsi
} // namespace platform
} // namespace sdk

#endif //__NCSI_MGR_H__
