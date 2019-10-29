// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_SDK_CLIENT_H_
#define _DELPHI_SDK_CLIENT_H_

#include <map>
#include <memory.h>
#include <ev++.h>
#include <cxxabi.h>
#include <pthread.h>

#include "base_object.hpp"
#include "delphi_service.hpp"
#include "nic/delphi/messenger/messenger_client.hpp"

namespace delphi {

using namespace std;
using namespace delphi::messenger;

// sync interval for syncing to delphi hub
#define CLIENT_SYNC_PERIOD 0.005

// heartbeat interval in seconds for updating the client status
#define CLIENT_HEARTBEAT_PERIOD 5

// reconnect interval
#define CLIENT_INITIAL_RECONNECT_PERIOD 0.005
#define CLIENT_MAX_RECONNECT_PERIOD     0.250

// max service id
#define MAX_SERVICE_ID (0xFFFF)

// Max object ID
#define MAX_OBJECT_HANDLE    (0xFFFFFFFFFFFFll)

// max objects in delphi (for datastructure sizing purposes)
#define MAX_OBJECTS (10 * 1024 * 1024)

// get a object handle from service id and object id
#define OBJECT_HANDLE(svcID, objID) ((((uint64_t)svcID & MAX_SERVICE_ID) << 48) | (objID & MAX_OBJECT_HANDLE))

// the number of tries we will try connecting to the hub before giving up
#define CONNECT_TRIES 10

// milliseconds that we will wait before trying to connect again
#define CONNECT_SLEEP_MS 1000

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
class DelphiClient : public messenger::ClientHandler,  public enable_shared_from_this<DelphiClient> {
public:
    DelphiClient();                                           // constructor
    error Connect();                                          // Connect to hub
    error Close();                                            // disconnect from delphi hub
    error MockConnect(uint16_t mySvcId);                      // to be used for testing purposes to fake a delphi hub connection
    error MountKind(string kind, MountMode mode);             // Mount a kind of objects
    error MountKey(string kind, string key, MountMode mode);  // Mount a specific key
    error WatchKind(string kind, BaseReactorPtr rctr);        // watch a kind of objects
    error RegisterService(ServicePtr svc);                    // register a service
    error SetObject(BaseObjectPtr objinfo);                   // update an object
    error SyncObject(BaseObjectPtr objinfo);                  // update and sync object
    error QueueUpdate(BaseObjectPtr objinfo);                 // queue update to object (thread safe)
    error QueueDelete(BaseObjectPtr objinfo);                 // queue delete object (thread safe)
    BaseObjectPtr FindObject(BaseObjectPtr objinfo);          // find an object
    error DeleteObject(BaseObjectPtr objinfo);                // delete an object
    vector<BaseObjectPtr> ListKind(string kind);              // list all objects of a kind
    map<string, BaseObjectPtr> GetSubtree(string kind);       // reyurn a map of objects of a kind
    ReactorListPtr GetReactorList(string kind);               // get reactors for a kind
    error WatchMountComplete(BaseReactorPtr rctr);            // register a reactor for mount complete callback
    void  enterAdminMode();                                   // special admin mode for delphictl

    // required by messenger::ClientHandler
    virtual error HandleNotify(vector<ObjectData *> objlist);
    virtual error HandleMountResp(uint16_t svcID, string status, vector<ObjectData *> objlist);
    virtual error SocketClosed();

protected:
    void syncTimerHandler(ev::timer &watcher, int revents);      // sync object updates to delphi hub
    void eventTimerHandler(ev::timer &watcher, int revents);     // handle pending events
    void msgqAsyncHandler(ev::async &watcher, int revents);      // handle msq events
    void heartbeatTimerHandler(ev::timer &watcher, int revents); // publish client status
    void reconnectTimerHandler(ev::timer &watcher, int revents); // reconnect to delphi hub
    error allocHandle(BaseObjectPtr objinfo);                    // allocate a object handle
    error freeHandle(BaseObjectPtr objinfo);                     // free object handle
    ObjSubtreePtr getSubtree(string kind);                       // get subtree of objects (use for testing only)
    void MountClientStatus();                                    // Mounts the DelphiClientStatus message as ReadWrite for self
    bool canWrite(string kind, string key);                      // checks if this service can write the object

private:
    MessangerClientPtr             mclient_;         // messenger
    bool                           isConnected_;     // are we connected to hub?
    double                         reconnectPeriod_; // Current reconnect period
    bool                           isMountComplete_; // mount completed
    bool                           adminMode_;       // special admin mode for debug only
    uint16_t                       myServiceID_;     // my service id assigned by hub
    vector<MountDataPtr>           mounts_;          // mount points we requested
    ServicePtr                     service_;         // currently registered service
    map<string, ObjSubtreePtr>     subtrees_;        // map kind -> subtree
    map<uint64_t, BaseObjectPtr>   handleDB_;        // map obj handle -> object
    vector<ObjectMutationPtr>      syncQueue_;       // sync queue to send updates to hub
    vector<ObjectMutationPtr>      eventQueue_;      // pending events to be triggered
    map<string, ReactorListPtr>    watchers_;        // map object kind -> reactor list
    vector<ObjectMutationPtr>      msgQueue_;        // msg queue is thread safe way to update object state
    pthread_mutex_t                msgQlock_;        // lock for the message queue
    ev::timer                      syncTimer_;       // timer to sync to hub
    ev::timer                      eventTimer_;      // timer to trigger pending events
    ev::async                      msgqAsync_;       // async handler for message queue updates
    ev::timer                      heartbeatTimer_;  // timer to handle message queue updates
    ev::timer                      reconnectTimer_;  // timer to reconnect to hub
    uint64_t                       currObjectID_;    // running counter of object handle
    vector<BaseReactorPtr>         mountWatchers_;   // reactors watching mount complete
    pthread_t                      my_thread_;       // delphi thread id
};
typedef std::shared_ptr<DelphiClient> DelphiClientPtr;

} // namespace delphi

#endif // _DELPHI_SDK_CLIENT_H_
