// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_SDK_H_
#define _DELPHI_SDK_H_

#include "delphi_client.hpp"
#include "base_object.hpp"
#include "nic/delphi/utils/utils.hpp"

namespace delphi {

// main SDK class
class Sdk  {
public:
    // Delphi Sdk constructor
    Sdk();

    // RegisterService Registers a service object with the SDK
    error RegisterService(ServicePtr service);

    // MountKind mounts all objects of a specific kind
    error MountKind(string kind, MountMode mode);

    // WatchKind watches all objects of a kind
    error WatchKind(string kind, BaseReactorPtr rctr);

    // SetObject updates an object in db (needs to be called inside the event loop)
    error SetObject(BaseObjectPtr objinfo);

    // DeleteObject deletes an object from DB (needs to be called from inside event loop)
    error DeleteObject(BaseObjectPtr objinfo);

    // FindObject finds an object by key
    BaseObjectPtr FindObject(BaseObjectPtr objinfo);

    // ListKind gets a list of object of specific kind
    vector<BaseObjectPtr> ListKind(string kind);

    // GetSubtree returns a map of objects of a specific kind
    map<string, BaseObjectPtr> GetSubtree(string kind);

    // QueueUpdate Queues object updates to DB (thread safe and can be called from any thread)
    error QueueUpdate(BaseObjectPtr objinfo);

    // QueueDelete queues object deletes to DB (thread safe and can be called from any thread)
    error QueueDelete(BaseObjectPtr objinfo);

    // MainLoop runs the main event loop
    int MainLoop();

    // TestLoop runs tha fake event loop for unit tests
    void TestLoop();

    // Stop the sdk  (mainly for test purposes)
    error Stop();

private:
    DelphiClientPtr client_;
    ev::default_loop loop_;

};
typedef std::shared_ptr<Sdk> SdkPtr;

} // namespace delphi

#endif // _DELPHI_SDK_H_
