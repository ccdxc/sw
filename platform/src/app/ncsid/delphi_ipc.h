/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __DELPHI_IPC_SERVICE_H__
#define __DELPHI_IPC_SERVICE_H__

#include <memory>
#include <string>
#include <iostream>

#include "gen/proto/nicmgr/nicmgr.delphi.hpp"
#include "gen/proto/port.delphi.hpp"
#include "nic/sdk/platform/ncsi/ipc_service.h"

namespace dobj = delphi::objects;
using dobj::PortStatusPtr;
using delphi::error;

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
    void Init(std::shared_ptr<DelphiIpcService> ncsid_svc);
    int PostMsg(struct VlanFilterMsg& vlan_filter);
    int PostMsg(struct VlanModeMsg& vlan_mode);
    int PostMsg(struct MacFilterMsg& mac_filter);
    int PostMsg(struct SetLinkMsg& set_link_msg);
    int PostMsg(struct EnableChanMsg& enable_ch);
    int PostMsg(struct ResetChanMsg& reset_ch);
    int PostMsg(struct EnableChanTxMsg& enable_ch_tx);
    int PostMsg(struct EnableBcastFilterMsg& bcast_filter);
    int PostMsg(struct EnableGlobalMcastFilterMsg& mcast_filter);
    int GetLinkStatus();

    void OnMountComplete();
};

// link_event_handler is the reactor for the Port object
class link_event_handler : public dobj::PortStatusReactor {
public:
    link_event_handler(delphi::SdkPtr sdk) {
        this->sdk_ = sdk;
    }

    // OnPortUpdate gets called when PortStatus object is updated
    virtual error OnPortStatusUpdate(PortStatusPtr port);
    virtual error OnPortStatusCreate(PortStatusPtr portStatus);

    // update_link_status updates port status in ncsid app
    error update_link_status(PortStatusPtr port);
    void get_link_status();
private:
    delphi::SdkPtr    sdk_;
};

typedef std::shared_ptr<link_event_handler> link_event_handler_ptr_t;

#endif // __DELPHI_IPC_SERVICE_H__

