// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_RESP_H__
#define __UPGRADE_RESP_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/proto/upgrade.delphi.hpp"

namespace upgrade {

using namespace std;

class UpgMgrResp {
    delphi::SdkPtr     sdk_;
public:
    UpgMgrResp(delphi::SdkPtr sk) {
        sdk_ = sk;
    }

    // findUpgMgrRespObj used to find UpgMgrResp object in delphi db
    delphi::objects::UpgRespPtr findUpgMgrRespObj (uint32_t id);

    // createUpgMgrResp will create the upgResp API and set the right value
    delphi::error createUpgMgrResp(uint32_t id, UpgRespType val, vector<string> &str);

    // DeleteUpgMgrResp will delete the UpgResp object
    delphi::error DeleteUpgMgrResp(void);

    //UpgradeFinish is the API that will respond to the agent with success/fail
    delphi::error UpgradeFinish(UpgRespType respType, vector<string> &str);
};
typedef std::shared_ptr<UpgMgrResp> UpgMgrRespPtr;

} //namespace upgrade

#endif //__UPGRADE_RESP_H__
