// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <iostream>
#include "delphi_client.hpp"

namespace delphi {

using namespace std;
using namespace delphi::messanger;


// DelphiClient constructor
DelphiClient::DelphiClient() {
    // start the sync timer
    this->syncTimer.set<DelphiClient, &DelphiClient::syncTimerHandler>(this);
    this->syncTimer.start(CLIENT_SYNC_PERIOD, CLIENT_SYNC_PERIOD);
    this->eventTimer.set<DelphiClient, &DelphiClient::eventTimerHandler>(this);
    this->msgqTimer.set<DelphiClient, &DelphiClient::msgqTimerHandler>(this);
    pthread_mutex_init(&msgQlock, NULL);
    this->isConnected = false;
    this->isMountComplete = false;
    this->currObjectID = 1;
    this->service = nullptr;
}

// Connect connect to the server
error DelphiClient::Connect() {
    // if service is not registered, return error
    if (this->service == NULL) {
        return error::New("Service not registered");
    }

    // create a messanger client
    this->mclient = make_shared<MessangerClient>(shared_from_this());

    // connect to server
    error err = mclient->Connect();
    if (err.IsNotOK()) {
        LogError("Error connecting to messanger server. Err: {}", err);
        return err;
    }

    // mark ourselves as connected
    isConnected = true;

    // send mout request
    err = mclient->SendMountReq(this->service->Name(), this->mounts);
    if (err.IsNotOK()) {
        LogError("Error mounting. Err: {}", err);
        return err;
    }

    return error::OK();
}

// MountKind requests a kind to be mounted
error DelphiClient::MountKind(string kind, MountMode mode) {
    // all mounts have to be requested before we connect to the server
    assert(isConnected == false);

    // add to the list
    MountDataPtr mnt(make_shared<MountData>());
    mnt->set_kind(kind);
    mnt->set_mode(mode);
    mounts.push_back(mnt);

    return error::OK();
}

// WatchKind watches a kind of object
error DelphiClient::WatchKind(string kind, BaseReactorPtr rctr) {
    map<string, ReactorListPtr>::iterator it;

    // find the reactor list
    it = this->watchers.find(kind);
    if (it == this->watchers.end()) {
        this->watchers[kind] = make_shared<ReactorList>();
    }
    ReactorListPtr rl = this->watchers[kind];
    rl->reactors.push_back(rctr);

    return error::OK();
}

// GetReactorList gets a list of reactors for an object kind
ReactorListPtr DelphiClient::GetReactorList(string kind) {
    map<string, ReactorListPtr>::iterator it;

    it = this->watchers.find(kind);
    if (it != this->watchers.end()) {
        return it->second;
    }

    return nullptr;
}

// RegisterService registers a service
error DelphiClient::RegisterService(ServicePtr svc) {
    assert(this->service == NULL);
    this->service = svc;
    return error::OK();
}

// getSubtree gets the subtree from object kind
ObjSubtreePtr DelphiClient::getSubtree(string kind) {
    std::map<string, ObjSubtreePtr>::iterator it;

    // find the subtree by kind
    it = this->subtrees.find(kind);
    if (it != this->subtrees.end()) {
        return it->second;
    }

    // create a new subtree
    ObjSubtreePtr subtree = make_shared<ObjSubtree>();
    this->subtrees[kind] = subtree;

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
            this->handleDB[(*oi)->meta().handle()] = objinfo;
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
        this->handleDB[(*oi)->meta().handle()] = objinfo;
    }

    // save my service id and mark mount complete
    this->myServiceID = svcID;
    this->isMountComplete = true;

    // trigger  mount complete callback
    this->service->OnMountComplete();

    return error::OK();
}

// SetObject sets an object in database and queues it towards server
error DelphiClient::SetObject(BaseObjectPtr objinfo) {
    map<string, BaseObjectPtr>::iterator it;
    string key = objinfo->GetKey();

    // make sure mount is complete
    if (!this->isMountComplete) {
        LogError("Error creating object {}/{}. Mount is not complete", objinfo->GetMeta()->kind(), key);
        return error::New("Can not create objects before mount complete");
    }

    // key can not be empty
    if (key == "") {
        LogError("Object key can not be empty {}", objinfo->GetMessage()->ShortDebugString());
        return error::New("Object key is empty");
    }

    // set key in the meta
    objinfo->GetMeta()->set_key(key);

    // get the subtree
    ObjSubtreePtr subtree = this->getSubtree(objinfo->GetMeta()->kind());

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
    this->syncQueue.push_back(make_shared<ObjectMutation>(oldObj, dbObj, SetOp));
    this->eventQueue.push_back(make_shared<ObjectMutation>(oldObj, objinfo, SetOp));
    this->eventTimer.start(0, 0);

    return error::OK();
}

// QueueUpdate queues object updates in a thread safe way to delphi client
error DelphiClient::QueueUpdate(BaseObjectPtr objinfo) {
    // make sure mount is complete
    if (!this->isMountComplete) {
        LogError("Error creating object {}/{}. Mount is not complete", objinfo->GetMeta()->kind(), objinfo->GetKey());
        return error::New("Can not create objects before mount complete");
    }

    // kind and key can not be empty
    if ((objinfo->GetKey() == "") || (objinfo->GetMeta()->kind()  == "")) {
        LogError("Object kind/key can not be empty {}", objinfo->GetMessage()->ShortDebugString());
        return error::New("Object key is empty");
    }

    // lock message queue
    pthread_mutex_lock(&msgQlock);

    // add it to the queue
    this->msgQueue.push_back(make_shared<ObjectMutation>(nullptr, objinfo, SetOp));
    this->msgqTimer.start(0, 0);

    // unlock message queue
    pthread_mutex_unlock(&msgQlock);

    return error::OK();
}

// QueueDelete queues object deletes in a thread safe way to delphi client
error DelphiClient::QueueDelete(BaseObjectPtr objinfo) {
    // make sure mount is complete
    if (!this->isMountComplete) {
        LogError("Error creating object {}/{}. Mount is not complete", objinfo->GetMeta()->kind(), objinfo->GetKey());
        return error::New("Can not create objects before mount complete");
    }

    // kind and key can not be empty
    if ((objinfo->GetKey() == "") || (objinfo->GetMeta()->kind()  == "")) {
        LogError("Object kind/key can not be empty {}", objinfo->GetMessage()->ShortDebugString());
        return error::New("Object key is empty");
    }

    // lock message queue
    pthread_mutex_lock(&msgQlock);

    // add it to the queue
    this->msgQueue.push_back(make_shared<ObjectMutation>(nullptr, objinfo, DeleteOp));
    this->msgqTimer.start(0, 0);

    // unlock message queue
    pthread_mutex_unlock(&msgQlock);

    return error::OK();
}

// allocHandle allocates a unique object handle
error DelphiClient::allocHandle(BaseObjectPtr objinfo) {
    map<uint64_t, BaseObjectPtr>::iterator it;

    for (int i = 0; i < MAX_OBJECTS; i++) {
        uint64_t handle = OBJECT_HANDLE(this->myServiceID, this->currObjectID);

        // see if the current id is taken
        it = this->handleDB.find(handle);
        if (it == this->handleDB.end()) {
            objinfo->GetMeta()->set_handle(handle);
            this->handleDB[handle] = objinfo;
            this->currObjectID++;

            return error::OK();
        }

        // try next object id
        this->currObjectID++;
    }

    return error::New("Could not allocate an object id");
}

// freeHandle frees the object handle
error DelphiClient::freeHandle(BaseObjectPtr objinfo) {
    map<uint64_t, BaseObjectPtr>::iterator it;
    uint64_t handle = objinfo->GetMeta()->handle();

    // find the object in handle db
    it = this->handleDB.find(handle);
    if (it == this->handleDB.end()) {
        LogError("Could not find the object for handle {}", handle);
        return error::New("Object handle not found");
    }

    // remove from the DB
    this->handleDB.erase(it);
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

    // make sure mount is complete
    if (!this->isMountComplete) {
        LogError("Error deleting object {}/{}. Mount is not complete", objinfo->GetMeta()->kind(), key);
        return error::New("Can not create objects before mount complete");
    }

    // set key in the meta
    objinfo->GetMeta()->set_key(key);

    // get the subtree
    ObjSubtreePtr subtree = this->getSubtree(objinfo->GetMeta()->kind());

    // find the old object
    it = subtree->objects.find(key);
    if (it == subtree->objects.end()) {
        LogError("Failed deleting Object. {}/{} not found", objinfo->GetMeta()->kind(), key);
        return error::New("Object not found");
    }

    subtree->objects.erase(it);
    RETURN_IF_FAILED(this->freeHandle(objinfo));

    // add it to sync and event queue
    this->syncQueue.push_back(make_shared<ObjectMutation>(nullptr, objinfo, DeleteOp));
    this->eventQueue.push_back(make_shared<ObjectMutation>(nullptr, objinfo, DeleteOp));
    this->eventTimer.start(0, 0);

    return error::OK();
}

// ListKind returns all objects of a kind
vector<BaseObjectPtr> DelphiClient::ListKind(string kind) {
    vector<BaseObjectPtr> objlist;

    // get the subtree
    ObjSubtreePtr subtree = this->getSubtree(kind);

    // walk all objects in the DB
    for (map<string, BaseObjectPtr>::iterator i = subtree->objects.begin(); i != subtree->objects.end(); ++i) {
        BaseObjectPtr objinfo = i->second;
        objlist.push_back(objinfo->Clone());
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
    this->syncTimer.stop();
    this->isConnected = true;
    this->myServiceID = 0;
    this->isMountComplete = false;

    // close messanger
    if (mclient != NULL) {
        mclient->Close();
    }

    return error::OK();
}

// syncTimerHandler is called when sync time fires
void DelphiClient::syncTimerHandler(ev::timer &watcher, int revents) {
    vector<ObjectData *> objlist;
    string out_str;

    // see if we have any objects to send
    if (syncQueue.size() == 0) {
        return;
    }

    // see if we are connected
    if ((mclient == NULL) || (!mclient->IsConnected())) {
        return;
    }

    // walk all objects in sync queue
    for (vector<ObjectMutationPtr>::iterator i=syncQueue.begin(); i!=syncQueue.end(); ++i) {
        // dequeue from the sync queue
        BaseObjectPtr objinfo = (*i)->newObj;
        ObjectOperation op = (*i)->op;

        // serialize the object
        ObjectData *od = new ObjectData;
        ObjectMeta *meta = od->mutable_meta();
        meta->CopyFrom(*objinfo->GetMeta());
        od->set_op(op);
        objinfo->GetMessage()->SerializeToString(&out_str);
        od->set_data(out_str);

        LogDebug("Sending object to hub: {}", objinfo->GetMessage()->ShortDebugString());

        // add it to object list to be sent to clients
        objlist.push_back(od);
    }
    syncQueue.clear();

    // send it to the server
    error err = mclient->SendChangeReq(objlist);
    if (err.IsNotOK()) {
        LogError("Error sending object change req to server. Err: {}", err);
    }
}

// eventTimerHandler handles local object events
void DelphiClient::eventTimerHandler(ev::timer &watcher, int revents) {
    // see if we have any objects to send
    if (eventQueue.size() == 0) {
        return;
    }

    while (eventQueue.size() > 0) {
        // handle one object at a time
        vector<ObjectMutationPtr>::iterator iter=eventQueue.begin();
        if (iter !=eventQueue.end()) {
            // dequeue from the event queue
            BaseObjectPtr     oldObj = (*iter)->oldObj;
            BaseObjectPtr     newObj = (*iter)->newObj;
            ObjectOperation   op     = (*iter)->op;
            eventQueue.erase(iter);

            // find the watcher for the kind
            ReactorListPtr rl = this->GetReactorList(newObj->GetMeta()->kind());
            if (rl != NULL) {
                // trigger events on the object
                error err = newObj->TriggerEvent(oldObj, op, rl);
                if (err.IsNotOK()) {
                    LogError("Error triggering event {} on object {}", op, newObj->GetKey());
                }
            }
        }
    }

    // restart the timer if we still have objects
    if (eventQueue.size() > 0) {
        this->eventTimer.start(0, 0);
    }
}

// msgqTimerHandler handles msg queue events
void DelphiClient::msgqTimerHandler(ev::timer &watcher, int revents) {
    // lock message queue
    pthread_mutex_lock(&msgQlock);

    // walk all objects in the message queue
    for (vector<ObjectMutationPtr>::iterator iter=msgQueue.begin(); iter!=msgQueue.end(); ++iter) {
        map<string, BaseObjectPtr>::iterator it;
        BaseObjectPtr objinfo = (*iter)->newObj;
        BaseObjectPtr oldObj = nullptr;
        string kind = objinfo->GetMeta()->kind();
        string key = objinfo->GetKey();

        // set key in the meta
        objinfo->GetMeta()->set_key(key);

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
                    pthread_mutex_unlock(&msgQlock);
                    return;
                }
            }

            // save it to the DB
            subtree->objects[key] = objinfo->Clone();
            this->handleDB[objinfo->GetMeta()->handle()] = objinfo;
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


        // find the watcher for the kind
        ReactorListPtr rl = this->GetReactorList(kind);
        if (rl != NULL) {
            // trigger events on the object
            error err = objinfo->TriggerEvent(oldObj, (*iter)->op, rl);
            if (err.IsNotOK()) {
                LogError("Error triggering event {} on object {}/{}", (*iter)->op, kind, key);
            }
        }
    }

    // unlock message queue
    pthread_mutex_unlock(&msgQlock);
}

// MockConnect simulates a connection to delphi hub
error DelphiClient::MockConnect(uint16_t mySvcId) {
    // if service is not registered, return error
    if (this->service == NULL) {
        return error::New("Service not registered");
    }

    // save my service id and mark mount complete
    this->myServiceID = mySvcId;
    this->isMountComplete = true;

    // fake a mount complete callback
    this->service->OnMountComplete();

    return error::OK();
}
} // namespace delphi
