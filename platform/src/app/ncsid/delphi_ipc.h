/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __DELPHI_IPC_SERVICE_H__
#define __DELPHI_IPC_SERVICE_H__

#include <memory>
#include <string>
#include <iostream>

#include "gen/proto/nicmgr/nicmgr.delphi.hpp"
#include "nic/sdk/platform/ncsi/ipc_service.h"

namespace dobj = delphi::objects;

typedef void (*delphi_cb)();

class DelphiIpcService : public delphi::Service,
                         public IpcService {
//                         public enable_shared_from_this<DelphiIpcService> {
private:
    delphi::SdkPtr  sdk_;
    delphi_cb callback;
public:
    delphi::SdkPtr sdk(void) const { return sdk_; }
    DelphiIpcService(delphi::SdkPtr sk, delphi_cb cb);
    int PostMsg(struct VlanFilterMsg& vlan_filter);
    int PostMsg(struct VlanModeMsg& vlan_mode);
    int PostMsg(struct MacFilterMsg& mac_filter);
    int PostMsg(struct SetLinkMsg& set_link_msg);
    int PostMsg(struct EnableChanMsg& enable_ch);
    int PostMsg(struct ResetChanMsg& reset_ch);
    int PostMsg(struct EnableChanTxMsg& enable_ch_tx);
    int PostMsg(struct EnableBcastFilterMsg& bcast_filter);
    int PostMsg(struct EnableGlobalMcastFilterMsg& mcast_filter);

    void OnMountComplete();
};

#endif // __DELPHI_IPC_SERVICE_H__

