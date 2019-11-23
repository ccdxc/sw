// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <iostream>
#include "delphi_client.hpp"
#include "gen/proto/client.delphi.hpp"

namespace delphi {

using namespace std;
using namespace delphi::messenger;

// DelphiClient constructor
DelphiClient::DelphiClient() {
    // start the sync timer
    this->syncTimer_.set<DelphiClient, &DelphiClient::syncTimerHandler>(this);
    this->syncTimer_.start(CLIENT_SYNC_PERIOD, CLIENT_SYNC_PERIOD);
    this->eventTimer_.set<DelphiClient, &DelphiClient::eventTimerHandler>(this);
    this->msgqAsync_.set<DelphiClient, &DelphiClient::msgqAsyncHandler>(this);
    this->msgqAsync_.start();
    this->heartbeatTimer_.set<DelphiClient, &DelphiClient::heartbeatTimerHandler>(this);
    this->reconnectTimer_.set<DelphiClient, &DelphiClient::reconnectTimerHandler>(this);
    pthread_mutex_init(&msgQlock_, NULL);
    this->isConnected_     = false;
    this->reconnectPeriod_ = CLIENT_INITIAL_RECONNECT_PERIOD;
    this->isMountComplete_ = false;
    this->adminMode_       = false;
    this->currObjectID_    = 1;
    this->service_         = nullptr;
}

// Connect connect to the server
error DelphiClient::Connect() {
    // if service is not registered, return error
    if (this->service_ == NULL) {
        return error::New("Service not registered");
    }

    // create a messenger client
    this->mclient_ = make_shared<MessangerClient>(shared_from_this());

    // connect to server
    error err = this->mclient_->Connect();
    if (err.IsNotOK()) {
        // LogInfo("Error({}) connecting to hub. Will try again", err);

        // reconnect in background
        this->reconnectTimer_.start(0, this->reconnectPeriod_);
        if (this->reconnectPeriod_ < CLIENT_MAX_RECONNECT_PERIOD) {
            this->reconnectPeriod_ *= 2;
        } else {
            this->reconnectPeriod_ = CLIENT_MAX_RECONNECT_PERIOD;
        }

        return error::OK();
    }

    // mark ourselves as connected
    this->isConnected_ = true;

    // send mout request
    err = this->mclient_->SendMountReq(this->service_->Name(), this->mounts_);
    if (err.IsNotOK()) {
        LogError("Error mounting. Err: {}", err);
        return err;
    }

    return error::OK();
}

// MountKind requests a kind to be mounted
error DelphiClient::MountKind(string kind, MountMode mode) {
    // all mounts have to be requested before we connect to the server
    assert(this->isConnected_ == false);

    // add to the list
    MountDataPtr mnt(make_shared<MountData>());
    mnt->set_kind(kind);
    mnt->set_mode(mode);
    this->mounts_.push_back(mnt);

    return error::OK();
}

// MountKey requests a specific key to be mounted
error DelphiClient::MountKey(string kind, string key, MountMode mode) {
    // all mounts have to be requested before we connect to the server
    assert(this->isConnected_ == false);

    // add to the list
    MountDataPtr mnt(make_shared<MountData>());
    mnt->set_kind(kind);
    mnt->set_key(key);
    mnt->set_mode(mode);
    this->mounts_.push_back(mnt);

    return error::OK();
}

// canWrite Checks if the service has mounted the (kind, key) as read/write
bool DelphiClient::canWrite(string kind, string key) {
    // if we are in admin mode, we can read/write any object
    if (this->adminMode_) {
        return true;
    }

    // walk all mount points
    for (auto mount: this->mounts_) {
        if ((getPath(mount->kind(), mount->key()) == getPath(kind, "")) ||
            (getPath(mount->kind(), mount->key()) == getPath(kind, key))) {
            if (mount->mode() == ReadWriteMode) {
                return true;
            }
        }
    }

    return false;
}

// enterAdminMode is used by delphictl to enter a special admin mode
// where service can read/write any object
void DelphiClient::enterAdminMode() {
    this->adminMode_ = true;
}

// WatchKind watches a kind of object
error DelphiClient::WatchKind(string kind, BaseReactorPtr rctr) {
    map<string, ReactorListPtr>::iterator it;

    // find the reactor list
    it = this->watchers_.find(kind);
    if (it == this->watchers_.end()) {
        this->watchers_[kind] = make_shared<ReactorList>();
    }
    ReactorListPtr rl = this->watchers_[kind];
    rl->reactors.push_back(rctr);

    return error::OK();
}

// GetReactorList gets a list of reactors for an object kind
ReactorListPtr DelphiClient::GetReactorList(string kind) {
    map<string, ReactorListPtr>::iterator it;

    it = this->watchers_.find(kind);
    if (it != this->watchers_.end()) {
        return it->second;
    }

    return nullptr;
}

void DelphiClient::MountClientStatus() {
    auto obj = make_shared<delphi::objects::DelphiClientStatus>();
    obj->set_key(this->service_->Name());
    this->MountKey(obj->GetDescriptor()->name(), obj->GetKey(), delphi::ReadWriteMode);
}

// RegisterService registers a service
error DelphiClient::RegisterService(ServicePtr svc) {
    assert(this->service_ == NULL);
    this->service_ = svc;

    this->MountClientStatus();

    return error::OK();
}

// WatchMountComplete registers a reactor for a mount complete callback
error DelphiClient::WatchMountComplete(BaseReactorPtr rctr) {
    this->mountWatchers_.push_back(rctr);
    return error::OK();
}

// getSubtree gets the subtree from object kind
ObjSubtreePtr DelphiClient::getSubtree(string kind) {
    std::map<string, ObjSubtreePtr>::iterator it;

    // find the subtree by kind
    it = this->subtrees_.find(kind);
    if (it != this->subtrees_.end()) {
        return it->second;
    }

    // create a new subtree
    ObjSubtreePtr subtree = make_shared<ObjSubtree>();
    this->subtrees_[kind] = subtree;

    return subtree;
}

// HandleNotify handles object updates from delphi hub
error DelphiClient::HandleNotify(vector<ObjectData *> objlist) {
    // de-serialize all objects and trigger events on them
    for (vector<ObjectData *>::iterator oi=objlist.begin(); oi!=objlist.end(); ++oi) {
        map<string, BaseObjectPtr>::iterator it;
        string kind = (*oi)->meta().kind();

        // create the object
        BaseObjectPtr objinfo = BaseObject::Create(kind, (*oi)->data());

        // get the subtree
        ObjSubtreePtr subtree = this->getSubtree(kind);

        // find the old object
        string key = (*oi)->meta().key();
        BaseObjectPtr oldObj;
        it = subtree->objects.find(key);
        if (it != subtree->objects.end()) {
            oldObj = it->second;
            subtree->objects.erase(it);
        }

        // add or remove it from db
        switch ((*oi)->op()) {
        case SetOp:
        {
            // save it to the DB
            subtree->objects[key] = objinfo->Clone();
            assert((*oi)->meta().handle() != 0);
            this->handleDB_[(*oi)->meta().handle()] = objinfo;
            break;
        }
        case DeleteOp:
        {
            // old object was already removed from DB
            break;
        }
        default:
        {
            LogError("Delphi client received Invalid operation on object {}", (*oi)->DebugString());
            assert(0);
        }
        }


        // find the watcher for the kind
        ReactorListPtr rl = this->GetReactorList(kind);
        if (rl != NULL) {
            LogDebug("Triggering event {} for object {}", (*oi)->op(), (*oi)->DebugString());

            // trigger events on the object
            error err = objinfo->TriggerEvent(oldObj, (*oi)->op(), rl);
            if (err.IsNotOK()) {
                LogError("Error triggering event {} on object {}", (*oi)->op(), key);
                return err;
            }
        }
    }

    return error::OK();
}

// HandleMountResp handles mount response
error DelphiClient::HandleMountResp(uint16_t svcID, string status, vector<ObjectData *> objlist) {
    // if mount failed, just exit for now
    if (status != "") {
        LogError("Mount failed. Error: {}", status);
        exit(1);
    }

    LogDebug("Received mount resp with service id {} num objects {}", svcID, objlist.size());

    // de-serialize and create all the objects
    for (vector<ObjectData *>::iterator oi=objlist.begin(); oi!=objlist.end(); ++oi) {
        map<string, BaseObjectPtr>::iterator it;

        // create the object from factory
        BaseObjectPtr objinfo = BaseObject::Create((*oi)->meta().kind(), (*oi)->data());

        // get the subtree
        ObjSubtreePtr subtree = this->getSubtree((*oi)->meta().kind());

        // save the object
        string key = (*oi)->meta().key();
        subtree->objects[key] = objinfo;
        assert((*oi)->meta().handle() != 0);
        this->handleDB_[(*oi)->meta().handle()] = objinfo;
    }

    // save my service id and mark mount complete
    this->myServiceID_ = svcID;
    this->isMountComplete_ = true;
    this->my_thread_ = pthread_self();

    // trigger  mount complete callback
    this->service_->OnMountComplete();

    // walk all the reactors and make On mount complete callbacks
    for (vector<BaseReactorPtr>::iterator rc = mountWatchers_.begin(); rc != mountWatchers_.end(); ++rc) {
        (*rc)->OnMountComplete();
    }

    // process any pending async messages
    this->msgqAsync_.send();

    // finally start the heartbeat
    this->heartbeatTimer_.start(0, CLIENT_HEARTBEAT_PERIOD);

    return error::OK();
}

// SetObject sets an object in database and queues it towards server
error DelphiClient::SetObject(BaseObjectPtr objinfo) {
    map<string, BaseObjectPtr>::iterator it;
    string key = objinfo->GetKey();
    string kind = objinfo->GetMeta()->kind();

    // make sure mount is complete
    if (!this->isMountComplete_) {
        LogError("Error creating object {}/{}. Mount is not complete", kind, key);
        return error::New("Can not create objects before mount complete");
    }

    // key can not be empty
    if (key == "") {
        LogError("Object key can not be empty {}", objinfo->GetMessage()->ShortDebugString());
        return error::New("Object key is empty");
    }

    // make sure SetObject is not called from other threads
    pthread_t thread_id = pthread_self();
    if (! pthread_equal(thread_id, this->my_thread_)) {
        LogError("Error creating object {}/{}. SetObject needs to be called from delphi thread", kind, key);
        return error::New("Called from invalid thread");
    }

    // make sure this service can write this object
    if (!canWrite(kind, key)) {
        LogError("Error creating object {}/{}. Object is not mounted read/write", kind, key);
        return error::New("object not mounted read-write");
    }

    // set key in the meta
    objinfo->GetMeta()->set_key(key);
    objinfo->GetMeta()->set_path(getPath(kind, key));

    // get the subtree
    ObjSubtreePtr subtree = this->getSubtree(kind);

    // find the old object
    BaseObjectPtr oldObj;
    it = subtree->objects.find(key);
    if (it != subtree->objects.end()) {
        oldObj = it->second;
        subtree->objects.erase(it);

        // copy the handle from old object
        objinfo->GetMeta()->set_handle(oldObj->GetMeta()->handle());
    } else {
        RETURN_IF_FAILED(this->allocHandle(objinfo));
    }

    // save the new object
    // FIXME: should we save it to db after we go back to mainloop or inline here?
    BaseObjectPtr dbObj = objinfo->Clone();
    subtree->objects[key] = dbObj;

    // add it to sync and event queue
    this->syncQueue_.push_back(make_shared<ObjectMutation>(oldObj, dbObj, SetOp));
    this->eventQueue_.push_back(make_shared<ObjectMutation>(oldObj, objinfo, SetOp));
    this->eventTimer_.start(0, 0);

    return error::OK();
}

// SyncObject sets and syncs the object
error DelphiClient::SyncObject(BaseObjectPtr objinfo) {
    // set object
    error err = SetObject(objinfo);
    if (err.IsNotOK()) {
        return err;
    }

    // immediately send the object to hub
    this->syncTimerHandler(this->syncTimer_, 0);

    return error::OK();
}

// QueueUpdate queues object updates in a thread safe way to delphi client
error DelphiClient::QueueUpdate(BaseObjectPtr objinfo) {
    // kind and key can not be empty
    if ((objinfo->GetKey() == "") || (objinfo->GetMeta()->kind()  == "")) {
        LogError("Object kind/key can not be empty {}", objinfo->GetMessage()->ShortDebugString());
        return error::New("Object key is empty");
    }

    // make sure this service can write this object
    if (!canWrite(objinfo->GetMeta()->kind(), objinfo->GetKey())) {
        LogError("Error updating object {}/{}. Object is not mounted read/write", 
                 objinfo->GetMeta()->kind(), objinfo->GetKey());
        return error::New("object not mounted read-write");
    }

    // lock message queue
    pthread_mutex_lock(&msgQlock_);

    // add it to the queue
    this->msgQueue_.push_back(make_shared<ObjectMutation>(nullptr, objinfo, SetOp));
    this->msgqAsync_.send();

    // unlock message queue
    pthread_mutex_unlock(&msgQlock_);

    return error::OK();
}

// QueueDelete queues object deletes in a thread safe way to delphi client
error DelphiClient::QueueDelete(BaseObjectPtr objinfo) {
    // kind and key can not be empty
    if ((objinfo->GetKey() == "") || (objinfo->GetMeta()->kind()  == "")) {
        LogError("Object kind/key can not be empty {}", objinfo->GetMessage()->ShortDebugString());
        return error::New("Object key is empty");
    }

    // make sure this service can write this object
    if (!canWrite(objinfo->GetMeta()->kind(), objinfo->GetKey())) {
        LogError("Error deleting object {}/{}. Object is not mounted read/write", 
                 objinfo->GetMeta()->kind(), objinfo->GetKey());
        return error::New("object not mounted read-write");
    }

    // lock message queue
    pthread_mutex_lock(&msgQlock_);

    // add it to the queue
    this->msgQueue_.push_back(make_shared<ObjectMutation>(nullptr, objinfo, DeleteOp));
    this->msgqAsync_.send();

    // unlock message queue
    pthread_mutex_unlock(&msgQlock_);

    return error::OK();
}

// allocHandle allocates a unique object handle
error DelphiClient::allocHandle(BaseObjectPtr objinfo) {
    map<uint64_t, BaseObjectPtr>::iterator it;

    for (int i = 0; i < MAX_OBJECTS; i++) {
        uint64_t handle = OBJECT_HANDLE(this->myServiceID_, this->currObjectID_);

        // see if the current id is taken
        it = this->handleDB_.find(handle);
        if (it == this->handleDB_.end()) {
            objinfo->GetMeta()->set_handle(handle);
            this->handleDB_[handle] = objinfo;
            this->currObjectID_++;

            return error::OK();
        }

        // try next object id
        this->currObjectID_++;
    }

    return error::New("Could not allocate an object id");
}

// freeHandle frees the object handle
error DelphiClient::freeHandle(BaseObjectPtr objinfo) {
    map<uint64_t, BaseObjectPtr>::iterator it;
    uint64_t handle = objinfo->GetMeta()->handle();

    // find the object in handle db
    it = this->handleDB_.find(handle);
    if (it == this->handleDB_.end()) {
        LogError("Could not find the object for handle {}", handle);
        return error::New("Object handle not found");
    }

    // remove from the DB
    this->handleDB_.erase(it);
    return error::OK();
}

// FindObject checks if an object exists in delphi cache
BaseObjectPtr DelphiClient::FindObject(BaseObjectPtr objinfo) {
    map<string, BaseObjectPtr>::iterator it;
    string key = objinfo->GetKey();

    // get the subtree
    ObjSubtreePtr subtree = this->getSubtree(objinfo->GetMeta()->kind());

    // find it in the subtree
    it = subtree->objects.find(key);
    if (it != subtree->objects.end()) {
        BaseObjectPtr obj = it->second;
        return obj->Clone();
    }
    return nullptr;
}

// DeleteObject deletes an object
error DelphiClient::DeleteObject(BaseObjectPtr objinfo) {
    map<string, BaseObjectPtr>::iterator it;
    string key = objinfo->GetKey();
    string kind = objinfo->GetMeta()->kind();

    // make sure mount is complete
    if (!this->isMountComplete_) {
        LogError("Error deleting object {}/{}. Mount is not complete", kind, key);
        return error::New("Can not create objects before mount complete");
    }

    // key can not be empty
    if (key == "") {
        LogError("Object key can not be empty {}", objinfo->GetMessage()->ShortDebugString());
        return error::New("Object key is empty");
    }

    // make sure this service can write this object
    if (!canWrite(kind, key)) {
        LogError("Error deleting object {}/{}. Object is not mounted read/write", kind, key);
        return error::New("object not mounted read-write");
    }

    // make sure DeleteObject is not called from other threads
    pthread_t thread_id = pthread_self();
    if (! pthread_equal(thread_id, this->my_thread_)) {
        LogError("Error deleting object {}/{}. DeleteObject needs to be called from delphi thread", kind, key);
        return error::New("Called from invalid thread");
    }

    // set key in the meta
    objinfo->GetMeta()->set_key(key);
    objinfo->GetMeta()->set_path(getPath(kind, key));

    // get the subtree
    ObjSubtreePtr subtree = this->getSubtree(kind);

    // find the old object
    it = subtree->objects.find(key);
    if (it == subtree->objects.end()) {
        LogError("Failed deleting Object. {}/{} not found", kind, key);
        return error::New("Object not found");
    }

    subtree->objects.erase(it);
    RETURN_IF_FAILED(this->freeHandle(objinfo));

    // add it to sync and event queue
    this->syncQueue_.push_back(make_shared<ObjectMutation>(nullptr, objinfo, DeleteOp));
    this->eventQueue_.push_back(make_shared<ObjectMutation>(nullptr, objinfo, DeleteOp));
    this->eventTimer_.start(0, 0);

    return error::OK();
}

// ListKind returns all objects of a kind
vector<BaseObjectPtr> DelphiClient::ListKind(string kind) {
    vector<BaseObjectPtr> objlist;
    std::map<string, ObjSubtreePtr>::iterator it;

    // find the subtree by kind
    it = this->subtrees_.find(kind);
    if (it != this->subtrees_.end()) {
        ObjSubtreePtr subtree = it->second;

        // walk all objects in the DB
        for (map<string, BaseObjectPtr>::iterator i = subtree->objects.begin(); i != subtree->objects.end(); ++i) {
            BaseObjectPtr objinfo = i->second;
            objlist.push_back(objinfo->Clone());
        }
    }

    return objlist;
}

// GetSubtree returns a map of objects of a kind
map<string, BaseObjectPtr> DelphiClient::GetSubtree(string kind) {
    map<string, BaseObjectPtr> objmap;

    // get the subtree
    ObjSubtreePtr subtree = this->getSubtree(kind);

    // walk all objects in the DB
    for (map<string, BaseObjectPtr>::iterator i = subtree->objects.begin(); i != subtree->objects.end(); ++i) {
        objmap[i->first] = i->second->Clone();
    }

    return objmap;
}

// Close stops the client and closes connection to delphi hub
error DelphiClient::Close() {
    this->syncTimer_.stop();
    this->eventTimer_.stop();
    this->heartbeatTimer_.stop();
    this->reconnectTimer_.stop();
    this->msgqAsync_.stop();
    this->isConnected_ = false;
    this->myServiceID_ = 0;
    this->isMountComplete_ = false;

    // close messenger
    if (this->mclient_ != NULL) {
        this->mclient_->Close();
    }

    return error::OK();
}

// syncTimerHandler is called when sync time fires
void DelphiClient::syncTimerHandler(ev::timer &watcher, int revents) {
    vector<ObjectData *> objlist;
    string out_str;

    // see if we have any objects to send
    if (syncQueue_.size() == 0) {
        return;
    }

    // see if we are connected
    if ((this->mclient_ == NULL) || (!this->mclient_->IsConnected())) {
        return;
    }

    // walk all objects in sync queue
    for (vector<ObjectMutationPtr>::iterator i=syncQueue_.begin(); i!=syncQueue_.end(); ++i) {
        // dequeue from the sync queue
        BaseObjectPtr objinfo = (*i)->newObj;
        ObjectOperation op = (*i)->op;

        // serialize the object
        ObjectData *od = new ObjectData;
        ObjectMeta *meta = od->mutable_meta();
        meta->CopyFrom(*objinfo->GetMeta());
        od->set_op(op);
	od->set_persist(objinfo->IsPersistent());
        objinfo->GetMessage()->SerializeToString(&out_str);
        od->set_data(out_str);

        LogDebug("Sending object to hub: {}", objinfo->GetMessage()->ShortDebugString());

        // add it to object list to be sent to clients
        objlist.push_back(od);
    }
    syncQueue_.clear();

    // send it to the server
    error err = this->mclient_->SendChangeReq(objlist);
    if (err.IsNotOK()) {
        LogError("Error sending object change req to server. Err: {}", err);
    }
}

// eventTimerHandler handles local object events
void DelphiClient::eventTimerHandler(ev::timer &watcher, int revents) {
    // see if we have any objects to send
    if (eventQueue_.size() == 0) {
        return;
    }

    while (eventQueue_.size() > 0) {
        // handle one object at a time
        vector<ObjectMutationPtr>::iterator iter=eventQueue_.begin();
        if (iter !=eventQueue_.end()) {
            // dequeue from the event queue
            BaseObjectPtr     oldObj = (*iter)->oldObj;
            BaseObjectPtr     newObj = (*iter)->newObj;
            ObjectOperation   op     = (*iter)->op;
            eventQueue_.erase(iter);

            // find the watcher for the kind
            ReactorListPtr rl = this->GetReactorList(newObj->GetMeta()->kind());
            if (rl != NULL) {

                LogDebug("Triggering event {} for object {}", op, newObj->GetMeta()->DebugString());

                // trigger events on the object
                error err = newObj->TriggerEvent(oldObj, op, rl);
                if (err.IsNotOK()) {
                    LogError("Error triggering event {} on object {}", op, newObj->GetKey());
                }
            }
        }
    }

    // restart the timer if we still have objects
    if (eventQueue_.size() > 0) {
        this->eventTimer_.start(0, 0);
    }
}

// msgqAsyncHandler handles msg queue events
void DelphiClient::msgqAsyncHandler(ev::async &watcher, int revents) {
    // dont process any messages till mount complete is done
    if (!this->isMountComplete_) {
        return;
    }

    // lock message queue
    pthread_mutex_lock(&msgQlock_);

    // walk all objects in the message queue
    for (vector<ObjectMutationPtr>::iterator iter=msgQueue_.begin(); iter!=msgQueue_.end(); ++iter) {
        map<string, BaseObjectPtr>::iterator it;
        BaseObjectPtr objinfo = (*iter)->newObj;
        BaseObjectPtr oldObj = nullptr;
        string kind = objinfo->GetMeta()->kind();
        string key = objinfo->GetKey();

        // set key in the meta
        objinfo->GetMeta()->set_key(key);
        objinfo->GetMeta()->set_path(getPath(kind, key));

        // get the subtree
        ObjSubtreePtr subtree = this->getSubtree(kind);

        // add or remove it from db
        switch ((*iter)->op) {
        case SetOp:
        {
            // find the old object
            it = subtree->objects.find(key);
            if (it != subtree->objects.end()) {
                oldObj = it->second;
                subtree->objects.erase(it);

                // copy the handle from old object
                objinfo->GetMeta()->set_handle(oldObj->GetMeta()->handle());
            } else {
                error err = this->allocHandle(objinfo);
                if (err.IsNotOK()) {
                    LogError("Error allocating object handle {}", err);
                    pthread_mutex_unlock(&msgQlock_);
                    return;
                }
            }

            // save it to the DB
            subtree->objects[key] = objinfo->Clone();
            this->handleDB_[objinfo->GetMeta()->handle()] = objinfo;
            break;
        }
        case DeleteOp:
        {
            // delete the object
            it = subtree->objects.find(key);
            if (it != subtree->objects.end()) {
                oldObj = it->second;
                subtree->objects.erase(it);
            }

            break;
        }
        default:
        {
            LogError("Delphi client received Invalid operation on object {}", objinfo->GetMeta()->ShortDebugString());
            assert(0);
        }
        }


        this->syncQueue_.push_back(make_shared<ObjectMutation>(oldObj, objinfo, (*iter)->op));

        // find the watcher for the kind
        ReactorListPtr rl = this->GetReactorList(kind);
        if (rl != NULL) {
            LogDebug("Triggering event {} for object {}", (*iter)->op, objinfo->GetMeta()->DebugString());

            // trigger events on the object
            error err = objinfo->TriggerEvent(oldObj, (*iter)->op, rl);
            if (err.IsNotOK()) {
                LogError("Error triggering event {} on object {}/{}", (*iter)->op, kind, key);
            }
        }
    }
    msgQueue_.clear();

    // unlock message queue
    pthread_mutex_unlock(&msgQlock_);
}

// heartbeatTimerHandler handles msg queue events
void DelphiClient::heartbeatTimerHandler(ev::timer &watcher, int revents) {
    if (this->service_->SkipHeartbeat()) {
        return;
    }
    // Check with the service first if it wants to override the status
    auto service_status = this->service_->Heartbeat();
    // Set the status object in Delphi
    auto status = make_shared<delphi::objects::DelphiClientStatus>();
    status->set_key(this->service_->Name());
    status->set_serviceid(this->myServiceID_);
    status->set_pid(getpid());
    status->set_lastseen(time(NULL));
    status->set_isok(service_status.first.IsOK());
    status->set_errorstring(service_status.second);
    this->SetObject(status);
}

// reconnectTimerHandler tries to reconnect in background
// FIXME: do we need to force a sync of local objects to hub??
void DelphiClient::reconnectTimerHandler(ev::timer &watcher, int revents) {
    // LogInfo("Trying to reconnect to delphi hub");
    this->isConnected_ = false;

    // connect to server
    error err = this->mclient_->Connect();
    if (err.IsNotOK()) {
        // LogInfo("Error({}) connecting to hub. Will try again", err);
        return;
    }

    // mark ourselves as connected
    this->isConnected_ = true;

    // send mout request
    err = this->mclient_->SendMountReq(this->service_->Name(), this->mounts_);
    if (err.IsNotOK()) {
        LogError("Error mounting. Err: {}", err);
        return;
    }

    LogInfo("Connected to delphi hub..");

    // stop the reconnect timer
    this->reconnectTimer_.stop();
}

// MockConnect simulates a connection to delphi hub
error DelphiClient::MockConnect(uint16_t mySvcId) {
    // if service is not registered, return error
    if (this->service_ == NULL) {
        return error::New("Service not registered");
    }

    // save my service id and mark mount complete
    this->myServiceID_     = mySvcId;
    this->isMountComplete_ = true;
    this->isConnected_     = true;
    this->my_thread_       = pthread_self();

    // fake a mount complete callback
    this->service_->OnMountComplete();

    return error::OK();
}
// SocketClosed handles socket close event
error DelphiClient::SocketClosed() {
    LogWarn("Connection to delphi hub went down");

    // remember that socket is not connected
    this->isConnected_ = false;

    // let the service know that delphi hub went down
    this->service_->SocketClosed();

    // clear our local cache of the database
    this->subtrees_.clear();
    this->syncQueue_.clear();
    this->eventQueue_.clear();
    
    // reconnect in background
    this->reconnectPeriod_ = CLIENT_INITIAL_RECONNECT_PERIOD;
    this->reconnectTimer_.start(0, this->reconnectPeriod_);
    this->reconnectPeriod_ *= 2;

    return error::OK();
}
} // namespace delphi
