// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_SDK_SERVICE_H_
#define _DELPHI_SDK_SERVICE_H_

#include <iostream>
#include <string>
#include <memory.h>

#include "nic/delphi/utils/utils.hpp"


namespace delphi {

using namespace std;

// Service is the abstract class all services have to implement
class Service {
public:
    // call back when objetc mounting is complete
    virtual void OnMountComplete() = 0;

    // callback when connection to delphi hub goes down
    virtual void SocketClosed() {};

    // callbacks for transactional commit
    virtual void TxnBegin() {};
    virtual void TxnCommit() {};
    virtual void TxnAbort() {};

    // Get the name of the service
    virtual string Name() {
        int     status;

        // get class name and demangle it
        return abi::__cxa_demangle(typeid(*this).name(), 0, 0, &status);
    };

    virtual std::pair<error, string> Heartbeat() {
        return std::make_pair(error::OK(), "");
    };

    virtual bool SkipHeartbeat() {
        return false;
    };
};
typedef std::shared_ptr<Service> ServicePtr;

} // namespace delphi

#endif // _DELPHI_SDK_SERVICE_H_
