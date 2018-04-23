// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_SDK_CLIENT_H_
#define _DELPHI_SDK_CLIENT_H_

#include <map>
#include <memory.h>
#include <ev++.h>
#include <cxxabi.h>
#include<pthread.h>

#include "base_object.hpp"
#include "delphi_service.hpp"
#include "nic/delphi/messanger/messanger_client.hpp"

namespace delphi {

using namespace std;
using namespace delphi::messanger;

// sync interval for syncing to delphi hub
#define CLIENT_SYNC_PERIOD 0.005

// max service id
#define MAX_SERVICE_ID (0xFFFF)

// Max object ID
#define MAX_OBJECT_HANDLE    (0xFFFFFFFFFFFFll)

// max objects in delphi (for datastructure sizing purposes)
#define MAX_OBJECTS (10 * 1024 * 1024)

// get a object handle from service id and object id
#define OBJECT_HANDLE(svcID, objID) ((((uint64_t)svcID & MAX_SERVICE_ID) << 48) | (objID & MAX_OBJECT_HANDLE))

// ObjectMutation keeps of an object that changed(keeps both old and new objects)
class ObjectMutation {
public:
    BaseObjectPtr oldObj;
    BaseObjectPtr newObj;
    ObjectOperation op;
    ObjectMutation(BaseObjectPtr o, BaseObjectPtr n, ObjectOperation p) {
        oldObj = o;
        newObj = n;
        op     = p;
    }
};
typedef std::shared_ptr<ObjectMutation> ObjectMutationPtr;


// ObjSubtree is a sub tree of delphi objects
class ObjSubtree {
public:
    map<string, BaseObjectPtr>   objects; // map of key->object
};
typedef std::shared_ptr<ObjSubtree> ObjSubtreePtr;

// DelphiClient is the delphi database client thats responsible for object sync and event callbacks
class DelphiClient : public messanger::ClientHandler,  public enable_shared_from_this<DelphiClient> {
public:
    DelphiClient();                                           // constructor
    error Connect();                                          // Connect to hub
    error Close();                                            // disconnect from delphi hub
    error MockConnect(uint16_t mySvcId);                      // to be used for testing purposes to fake a delphi hub connection
    error MountKind(string kind, MountMode mode);  // Mount a kind of objects
    error WatchKind(string kind, BaseReactorPtr rctr);        // watch a kind of objects
    error RegisterService(ServicePtr svc);                    // register a service
    error SetObject(BaseObjectPtr objinfo);                   // update an object
    error QueueUpdate(BaseObjectPtr objinfo);                 // queue update to object (thread safe)
    error QueueDelete(BaseObjectPtr objinfo);                 // queue delete object (thread safe)
    BaseObjectPtr FindObject(BaseObjectPtr objinfo);          // find an object
    error DeleteObject(BaseObjectPtr objinfo);                // delete an object
    vector<BaseObjectPtr> ListKind(string kind);              // list all objects of a kind
    map<string, BaseObjectPtr> GetSubtree(string kind);       // reyurn a map of objects of a kind
    ReactorListPtr GetReactorList(string kind);               // get reactors for a kind

    // required by messanger::ClientHandler
    error HandleNotify(vector<ObjectData *> objlist);
    error HandleMountResp(uint16_t svcID, string status, vector<ObjectData *> objlist);

protected:
    void syncTimerHandler(ev::timer &watcher, int revents);   // sync object updates to delphi hub
    void eventTimerHandler(ev::timer &watcher, int revents);  // handle pending events
    void msgqTimerHandler(ev::timer &watcher, int revents);   // handle updates from other threads
    error allocHandle(BaseObjectPtr objinfo);                 // allocate a object handle
    error freeHandle(BaseObjectPtr objinfo);                  // free object handle
    ObjSubtreePtr getSubtree(string kind);                    // get subtree of objects (use for testing only)

private:
    MessangerClientPtr             mclient;        // messanger
    bool                           isConnected;    // are we connected to hub?
    bool                           isMountComplete;
    uint16_t                       myServiceID;    // my service id assigned by hub
    vector<MountDataPtr>           mounts;         // mount points we requested
    ServicePtr                     service;        // currently registered service
    map<string, ObjSubtreePtr>     subtrees;       // map kind -> subtree
    map<uint64_t, BaseObjectPtr>   handleDB;       // map obj handle -> object
    vector<ObjectMutationPtr>      syncQueue;      // sync queue to send updates to hub
    vector<ObjectMutationPtr>      eventQueue;     // pending events to be triggered
    map<string, ReactorListPtr>    watchers;       // map object kind -> reactor list
    vector<ObjectMutationPtr>      msgQueue;       // msg queue is thread safe way to update object state
    pthread_mutex_t                msgQlock;       // lock for the message queue
    ev::timer                      syncTimer;      // timer to sync to hub
    ev::timer                      eventTimer;     // timer to trigger pending events
    ev::timer                      msgqTimer;      // timer to handle message queue updates
    uint64_t                       currObjectID;   // running counter of object handle

};
typedef std::shared_ptr<DelphiClient> DelphiClientPtr;

} // namespace delphi

#endif // _DELPHI_SDK_CLIENT_H_
