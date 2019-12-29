/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __NcsiParamDb_H__
#define __NcsiParamDb_H__ 

#include <string>
#include <memory>
#include "pkt-defs.h"
#include "ipc_service.h"

namespace sdk {
namespace platform {
namespace ncsi {

class NcsiParamDb {
public:
    void UpdateNcsiParam(struct VlanFilterMsg);
    void UpdateNcsiParam(struct MacFilterMsg);
    void UpdateNcsiParam(struct VlanModeMsg& msg);
    void UpdateNcsiParam(struct EnableChanMsg& msg);
    void UpdateNcsiParam(struct ResetChanMsg& msg);
    void UpdateNcsiParam(struct EnableChanTxMsg& msg);
    void UpdateNcsiParam(struct SetLinkMsg& msg);
    void UpdateNcsiParam(struct EnableBcastFilterMsg& msg);
    void UpdateNcsiParam(struct EnableGlobalMcastFilterMsg& msg);
private:
    struct GetParamRespPkt NcsiParamResp;
};

} // namespace ncsi
} // namespace platform
} // namespace sdk

#endif //__NcsiParamDb_H__
