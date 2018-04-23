// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <algorithm>

#include "delphi_server.hpp"

namespace delphi {

using namespace std;
using namespace delphi::messanger;


// DelphiServer constructor
DelphiServer::DelphiServer() {
    this->syncTimer.set<DelphiServer, &DelphiServer::syncTimerHandler>(this);
    this->syncTimer.start(SYNC_PERIOD, SYNC_PERIOD);
    this->currServiceId = 1;
}

// Start starts the delphi server
error DelphiServer::Start() {
    // create a messager server
    this->msgServer = make_shared<MessangerServer>(shared_from_this());

    // start the server
    return this->msgServer->Start();
}

// GetSubtree returns a subtree of objects for a kind
DbSubtreePtr DelphiServer::GetSubtree(string kind) {
    std::map<string, DbSubtreePtr>::iterator it;

    // find the subtree by kind
    it = this->subtrees.find(kind);
    if (it != this->subtrees.end()) {
        return it->second;
    }

    // create a new subtree
    DbSubtreePtr subtree = make_shared<DbSubtree>();
    this->subtrees[kind] = subtree;

    return subtree;
}

// addObject adds an object into a subtree based on kind
error DelphiServer::addObject(string kind, string key, ObjectData *obj) {
    std::map<string, ObjectData *>::iterator it;

    // some error checking on the objects
    if ((kind == "") || (key == "") || (obj->meta().handle() == 0)) {
        LogError("Invalid object metadata {}/{}:{}", kind, key, obj->meta().handle());
        return error::New("Invalid object metadata");
    }

    // get the sub tree
    DbSubtreePtr subtree = this->GetSubtree(kind);

    // add the object into a subtree
    it = subtree->objects.find(key);
    if (it != subtree->objects.end()) {
        subtree->objects.erase (it);
    }
    subtree->objects[key] = obj;

    return error::OK();
}

// delObject deletes an object
error DelphiServer::delObject(string kind, string key) {
    std::map<string, ObjectData *>::iterator it;

    // get the sub tree
    DbSubtreePtr subtree = this->GetSubtree(kind);

    // add the object into a subtree
    it = subtree->objects.find(key);
    if (it != subtree->objects.end()) {
        subtree->objects.erase (it);
    }

    return error::OK();
}

// addService adds a service to delphi hub
ServiceInfoPtr DelphiServer::addService(string svcName, int sockCtx) {
    // see if the service already exists
    ServiceInfoPtr svc = findServiceName(svcName);
    if (svc == NULL) {
        // add the service
        svc = make_shared<ServiceInfo>();
        svc->ServiceName = svcName;
        svc->ServiceID = this->getNewServiceID();
        svc->SockCtx = sockCtx;
    }

    // save the svc
    this->services[svcName] = svc;
    this->serviceIds[svc->ServiceID] = svc;
    this->sockets[sockCtx] = svc;

    return svc;
}

// findServiceName find service by name
ServiceInfoPtr DelphiServer::findServiceName(string svcName) {
    std::map<string, ServiceInfoPtr>::iterator it;

    // find in the map
    it = this->services.find(svcName);
    if (it != this->services.end()) {
        return it->second;
    }

    return nullptr;
}

// findServiceID find service by id
ServiceInfoPtr DelphiServer::findServiceID(uint16_t svcID) {
    std::map<uint16_t, ServiceInfoPtr>::iterator it;

    // find in the map
    it = this->serviceIds.find(svcID);
    if (it != this->serviceIds.end()) {
        return it->second;
    }

    return nullptr;
}

// getNewServiceID gets next available service id
uint16_t DelphiServer::getNewServiceID() {
    // keep trying to find the next available id
    for (int i = 0; i < 0xFFFF; i++) {
        ServiceInfoPtr svc = findServiceID(this->currServiceId);
        if (svc == NULL) {
            return this->currServiceId++;
        }
        this->currServiceId++;
    }

    // we should never reach here
    assert(0);
    return 0;
}

// findSock finds the service by socket id
ServiceInfoPtr DelphiServer::findSock(int sockCtx) {
    std::map<int, ServiceInfoPtr>::iterator it;

    // find in the map
    it = this->sockets.find(sockCtx);
    if (it != this->sockets.end()) {
        return it->second;
    }

    return nullptr;
}

// delService deletes a service
error DelphiServer::delService(ServiceInfoPtr svc, int sockCtx) {
    // save the svc
    this->services.erase(svc->ServiceName);
    this->serviceIds.erase(svc->ServiceID);
    this->sockets.erase(sockCtx);

    return error::OK();
}

// ListService lists all services in the delphi server
vector<ServiceInfoPtr> DelphiServer::ListService() {
    vector<ServiceInfoPtr> svcs;
    for (map<string, ServiceInfoPtr>::iterator iter=services.begin(); iter!=services.end(); ++iter) {
        svcs.push_back(iter->second);
    }

    return svcs;
}

// addMountPoint adds a point point for a kind
MountPointPtr DelphiServer::addMountPoint(string kind) {
    // check if the mount point already exists
    MountPointPtr mountPoint = this->findMountPoint(kind);
    if (mountPoint != NULL) {
        return mountPoint;
    }

    // add the mount point
    mountPoint = make_shared<MountPoint>();
    mountPoint->Kind = kind;
    this->mountPoints[kind] = mountPoint;

    return mountPoint;
}

// findMountPoint finds a mount point for an object kind
MountPointPtr DelphiServer::findMountPoint(string kind) {
    std::map<string, MountPointPtr>::iterator it;

    // find in the map
    it = this->mountPoints.find(kind);
    if (it != this->mountPoints.end()) {
        return it->second;
    }

    return nullptr;
}

// deleteMountPoint deletes a mount point
error DelphiServer::deleteMountPoint(string kind) {
    std::map<string, MountPointPtr>::iterator it;

    // delete the mount point
    this->mountPoints.erase(kind);
    it = this->mountPoints.find(kind);
    if (it != this->mountPoints.end()) {
        assert(it->second->Services.size() == 0);
        this->mountPoints.erase(it);
    }

    return error::OK();
}

// requestMount requests a kind to be mounted for a service
error DelphiServer::requestMount(string kind, string svcName, MountMode mode) {
    // first add the mount point
    MountPointPtr mntPt = this->addMountPoint(kind);

    // find the service
    ServiceInfoPtr svc = this->findServiceName(svcName);
    if (svc == NULL) {
        return error::New("Service not found while mounting");
    }

    // add mount data
    // FIXME: check if two services are mounting this mount point in read-write mode
    MountInfo mnt;
    mnt.Kind = kind;
    mnt.ServiceName = svcName;
    mnt.Mode = mode;
    mntPt->Services[svcName] = mnt;
    svc->Mounts.push_back(mnt);

    return error::OK();
}

// releaseMount release a service from mount point
error DelphiServer::releaseMount(string kind, string svcName) {
    std::map<string, MountInfo>::iterator it;

    // find the mount point
    MountPointPtr mntPt = this->findMountPoint(kind);
    if (mntPt == NULL) {
        LogError("Could not find the mount point for kind {}", kind);
        return error::New("Mount point not found");
    }

    // delete the service
    it = mntPt->Services.find(kind);
    if (it != mntPt->Services.end()) {
        mntPt->Services.erase(it);
    }

    // if mount point is empty, delete it
    if (mntPt->Services.size() == 0) {
        this->deleteMountPoint(kind);
    }

    return error::OK();
}

// HandleMountReq handles mount request from the client
error DelphiServer::HandleMountReq(int sockCtx, MountReqMsgPtr req, MountRespMsgPtr resp) {
    // check if we know about this service and add it if we dont
    ServiceInfoPtr svc = this->findServiceName(req->servicename());
    ServiceInfoPtr sockSvc = this->findSock(sockCtx);
    if ((svc == NULL) && (sockSvc == NULL)) {
        svc = this->addService(req->servicename(), sockCtx);
    } else if (((svc == NULL) && (sockSvc != NULL)) || ((svc != NULL) && (sockSvc == NULL))) {
        return error::New("Service and socket mismatch");
    } else if (svc->SockCtx != sockCtx) {
        return error::New("Multiple sockets for same service");
    } else if (svc != sockSvc) {
        return error::New("Multiple services on same socket");
    }

    // set resp
    resp->set_servicename(req->servicename());
    resp->set_serviceid(svc->ServiceID);

    LogInfo("Service connected {}/{} on socket {}", svc->ServiceName, svc->ServiceID, sockCtx);

    // walk each kind to be mounted
    for (int i = 0; i < req->mounts().size(); i++) {
        const MountData &mnt = req->mounts(i);
        string kind = mnt.kind();

        // request mount point
        error err = requestMount(kind, req->servicename(), mnt.mode());
        if (err.IsNotOK()) {
            return err;
        }

        LogInfo("Mounting {} from service {} mode {}", kind, req->servicename(), mnt.mode());


        // get the subtree for the kind
        DbSubtreePtr subtree = this->GetSubtree(kind);

        // walk all objects for this kind and send them
        for (map<string, ObjectData *>::iterator iter=subtree->objects.begin(); iter!=subtree->objects.end(); ++iter) {
            string out_str;

            ObjectData *obj = iter->second;
            ObjectData *od = resp->add_objects();
            obj->SerializeToString(&out_str);
            ObjectMeta *ometa = od->mutable_meta();
            ometa->CopyFrom(obj->meta());
            od->set_op(SetOp);
            od->set_data(out_str);
        }
    }

    return error::OK();
}

// HandleSocketClosed handles client socket getting closed
error DelphiServer::HandleSocketClosed(int sockCtx) {
    // remove service and all the mount points
    ServiceInfoPtr svc = this->findSock(sockCtx);
    if (svc == NULL) {
        return error::New("Could not find the service for socket");
    }

    LogInfo("Closing socket {} for service {}/{}", sockCtx, svc->ServiceName, svc->ServiceID);

    // release all mount points
    for (vector<MountInfo>::iterator iter=svc->Mounts.begin(); iter!=svc->Mounts.end(); ++iter) {
        error err = this->releaseMount(iter->Kind, svc->ServiceName);
        if (err.IsNotOK()){
            LogError("Error releasing mount {} for service {}. Err: {}", iter->Kind, svc->ServiceName, err);
        } else {
            LogInfo("Unmounting {} from service {}", iter->Kind, svc->ServiceName);
        }
    }

    return this->delService(svc, sockCtx);
}

// HandleChangeReq handles change request from clients
error DelphiServer::HandleChangeReq(int sockCtx, vector<ObjectData *> req, vector<ObjectData *> *resp) {
    // FIXME: verify this client has mounted these objects read-write

    // handle each object
    for (vector<ObjectData *>::iterator iter=req.begin(); iter!=req.end(); ++iter) {
        string kind = (*iter)->meta().kind();
        string key = (*iter)->meta().key();
        std::map<string, ObjectData *>::iterator it;

        // create a new object instance
        ObjectData *newObj = new ObjectData();
        newObj->CopyFrom(*(*iter));

        // add or remove it from db
        switch ((*iter)->op()) {
        case SetOp:
        {
            error err = this->addObject(kind, key, newObj);
            if (err.IsNotOK()) {
                LogError("Error adding the object {}/{}. Err: {}", kind, key, err);
                return err;
            }
            break;
        }
        case DeleteOp:
        {
            error err = this->delObject(kind, key);
            if (err.IsNotOK()) {
                LogError("Error adding the object {}/{}. Err: {}", kind, key, err);
                return err;
            }
            break;
        }
        default:
        {
            LogError("Invalid operation on object {}", (*iter)->DebugString());
            assert(0);
        }
        }

        // queue it for sync
        this->syncQueue[key] = newObj;
    }

    return error::OK();
}

error DelphiServer::Stop() {
    this->syncTimer.stop();
    return msgServer->Stop();
}

// syncTimerHandler is called when sync time fires
// FIXME: implement filtering
void DelphiServer::syncTimerHandler(ev::timer &watcher, int revents) {

    // see if we have any objects to send
    if (syncQueue.size() == 0) {
        return;
    }

    // walk all services and send the objects to them
    for (map<int, ServiceInfoPtr>::iterator iter=sockets.begin(); iter!=sockets.end(); ++iter) {
        vector<ObjectData *> objlist;

        // walk all objects in sync queue
        for (map<string, ObjectData *>::iterator i=syncQueue.begin(); i!=syncQueue.end(); ++i) {
            // dequeue from the sync queue
            ObjectData *syncObj = i->second;
            ObjectData *newObj = new ObjectData();
            newObj->CopyFrom(*syncObj);

            // add it to object list to be sent to clients
            objlist.push_back(newObj);
        }


        // send the object list to the client
        msgServer->SendNotify(iter->first, objlist);
    }
    syncQueue.clear();

}


} // namespace delphi
