// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <algorithm>

#include <lmdb.h>

#include "delphi_server.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/delphi_objects.hpp"

namespace delphi {

using namespace std;
using namespace delphi::messenger;


// Checks if the service has mounted the (kind, key)
bool ServiceInfo::HasMounted(string kind, string key) {
    for (auto mount: this->Mounts) {
        if ((mount.MountPath == getPath(kind, "")) ||
            (mount.MountPath == getPath(kind, key))) {
            return true;
        }
    }
    return false;
}

void DelphiServer::initDb() {
    int rc;

    rc = mdb_env_create(&this->db);
    assert(rc == 0);

    LogInfo("DB location: {}", this->dbFilename);
    rc = mdb_env_open(this->db, this->dbFilename.c_str(), MDB_NOSUBDIR,
	S_IRUSR | S_IWUSR);
    assert(rc == 0);
}

void DelphiServer::persistObject(string kind, string key, ObjectDataPtr obj) {
    MDB_val db_key;
    MDB_val db_val;
    MDB_txn *db_txn;
    MDB_dbi db_dbi;
    string tmp;
    int rc;

    LogInfo("Persisting {}/{}", kind, key);
    
    tmp = kind + ":" + key;
    db_key.mv_size = tmp.size();
    db_key.mv_data = (void *)tmp.c_str();
    
    db_val.mv_size = obj->ByteSizeLong();
    db_val.mv_data = malloc(db_val.mv_size);

    obj->SerializeToArray(db_val.mv_data, db_val.mv_size);

    LogInfo("Serialized {} bytes", db_val.mv_size);

    rc = mdb_txn_begin(this->db, NULL, 0, &db_txn);
    assert(rc == 0);

    rc = mdb_dbi_open(db_txn, NULL, MDB_CREATE, &db_dbi);
    assert(rc == 0);

    rc = mdb_put(db_txn, db_dbi, &db_key, &db_val, 0);
    assert(rc == 0);

    rc = mdb_txn_commit(db_txn);
    assert(rc == 0);

    mdb_dbi_close(this->db, db_dbi);
    
    free(db_val.mv_data);
}

void DelphiServer::unpersistObject(string kind, string key) {
    MDB_val db_key;
    MDB_txn *db_txn;
    MDB_dbi db_dbi;
    string tmp;
    int rc;

    LogInfo("Un-persisting {}/{}", kind, key);
    tmp = kind + ":" + key;
    db_key.mv_size = tmp.size();
    db_key.mv_data = (void *)tmp.c_str();
    
    rc = mdb_txn_begin(this->db, NULL, 0, &db_txn);
    assert(rc == 0);

    rc = mdb_dbi_open(db_txn, NULL, MDB_CREATE, &db_dbi);
    assert(rc == 0);

    rc = mdb_del(db_txn, db_dbi, &db_key, NULL);
    assert(rc == 0  || rc == MDB_NOTFOUND);

    rc = mdb_txn_commit(db_txn);
    assert(rc == 0);

    mdb_dbi_close(this->db, db_dbi);
}

void DelphiServer::restorePersistentObjects() {
    MDB_val    db_key;
    MDB_val    db_val;
    MDB_cursor *db_cur;
    MDB_txn    *db_txn;
    MDB_dbi    db_dbi;
    int rc;
    

    LogInfo("Restoring Persistent Objects");

    rc = mdb_txn_begin(this->db, NULL, MDB_RDONLY, &db_txn);
    assert(rc == 0);

    rc = mdb_dbi_open(db_txn, NULL, MDB_CREATE, &db_dbi);
    assert(rc == 0);

    rc = mdb_cursor_open(db_txn, db_dbi, &db_cur);
    assert(rc == 0);


    rc = mdb_cursor_get(db_cur, &db_key, &db_val, MDB_FIRST);
    assert(rc == MDB_NOTFOUND || rc == 0);

    while (rc != MDB_NOTFOUND)
    {
	
	ObjectDataPtr obj = make_shared<ObjectData>();
	string tmp = string((const char *)db_key.mv_data, db_key.mv_size);
	string kind = tmp.substr(0, tmp.find(":"));
	string key = tmp.substr(tmp.find(":") + 1);

	LogInfo("Restoring db object {}/{} of size {}", kind, key,
	    db_val.mv_size);

	obj->ParseFromArray(db_val.mv_data, db_val.mv_size);
	this->addObject(kind, key, obj, true);
	
	rc = mdb_cursor_get(db_cur, &db_key, &db_val, MDB_NEXT);
	assert(rc == MDB_NOTFOUND || rc == 0);
	
    }

    mdb_cursor_close(db_cur);
    mdb_txn_commit(db_txn);
    mdb_dbi_close(this->db, db_dbi);
}

// DelphiServer constructor
DelphiServer::DelphiServer(string dbfilename) : dbFilename(dbfilename) {
    this->syncTimer_.set<DelphiServer, &DelphiServer::syncTimerHandler>(this);
    this->syncTimer_.start(SYNC_PERIOD, SYNC_PERIOD);
    this->currServiceId_ = 1;
    this->traceEnabled_ = false;

    this->initDb();
}

// Start starts the delphi server
error DelphiServer::Start() {
    // create a messager server
    this->msgServer_ = make_shared<MessangerServer>(shared_from_this());

    // init the delphi shared memory
    srv_shm_ = make_shared<delphi::shm::DelphiShm>();
    delphi::error err = srv_shm_->MemMap(DELPHI_SHM_NAME, DELPHI_SHM_SIZE, true);
    assert(err.IsOK());

    this->restorePersistentObjects();
    
    // start the server
    return this->msgServer_->Start();
}

// GetSubtree returns a subtree of objects for a kind
DbSubtreePtr DelphiServer::GetSubtree(string kind) {
    std::map<string, DbSubtreePtr>::iterator it;

    // find the subtree by kind
    it = this->subtrees_.find(kind);
    if (it != this->subtrees_.end()) {
        return it->second;
    }

    // create a new subtree
    DbSubtreePtr subtree = make_shared<DbSubtree>();
    this->subtrees_[kind] = subtree;

    return subtree;
}

// addObject adds an object into a subtree based on kind
error DelphiServer::addObject(string kind, string key, ObjectDataPtr obj,
	bool skipPersistence) {
    std::map<string, ObjectDataPtr>::iterator it;

    // some error checking on the objects
    if ((kind == "") || (key == "") || (obj->meta().path() == "" ) || (obj->meta().handle() == 0)) {
        LogError("Invalid object metadata {}/{}, {}:{}",
                kind, key, obj->meta().path(), obj->meta().handle());
        return error::New("Invalid object metadata");
    }

    // get the sub tree
    DbSubtreePtr subtree = this->GetSubtree(kind);

    // add the object into a subtree
    it = subtree->objects.find(key);
    if (it != subtree->objects.end()) {
        subtree->objects.erase(it);
    }
    subtree->objects[key] = obj;
    
    if (obj->persist() && !skipPersistence) {
	this->persistObject(kind, key, obj);
    }

    return error::OK();
}

// delObject deletes an object
error DelphiServer::delObject(string kind, string key, ObjectDataPtr obj) {
    std::map<string, ObjectDataPtr>::iterator it;

    // get the sub tree
    DbSubtreePtr subtree = this->GetSubtree(kind);

    // add the object into a subtree
    it = subtree->objects.find(key);
    if (it != subtree->objects.end()) {
        subtree->objects.erase (it);
    }

    if (obj->persist()) {
	this->unpersistObject(kind, key);
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
    this->services_[svcName] = svc;
    this->serviceIds_[svc->ServiceID] = svc;
    this->sockets_[sockCtx] = svc;

    return svc;
}

// findServiceName find service by name
ServiceInfoPtr DelphiServer::findServiceName(string svcName) {
    std::map<string, ServiceInfoPtr>::iterator it;

    // find in the map
    it = this->services_.find(svcName);
    if (it != this->services_.end()) {
        return it->second;
    }

    return nullptr;
}

// findServiceID find service by id
ServiceInfoPtr DelphiServer::findServiceID(uint16_t svcID) {
    std::map<uint16_t, ServiceInfoPtr>::iterator it;

    // find in the map
    it = this->serviceIds_.find(svcID);
    if (it != this->serviceIds_.end()) {
        return it->second;
    }

    return nullptr;
}

// getNewServiceID gets next available service id
uint16_t DelphiServer::getNewServiceID() {
    // keep trying to find the next available id
    for (int i = 0; i < 0xFFFF; i++) {
        ServiceInfoPtr svc = findServiceID(this->currServiceId_);
        if (svc == NULL) {
            return this->currServiceId_++;
        }
        this->currServiceId_++;
    }

    // we should never reach here
    assert(0);
    return 0;
}

// findSock finds the service by socket id
ServiceInfoPtr DelphiServer::findSock(int sockCtx) {
    std::map<int, ServiceInfoPtr>::iterator it;

    // find in the map
    it = this->sockets_.find(sockCtx);
    if (it != this->sockets_.end()) {
        return it->second;
    }

    return nullptr;
}

// delService deletes a service
error DelphiServer::delService(ServiceInfoPtr svc, int sockCtx) {
    // save the svc
    this->services_.erase(svc->ServiceName);
    this->serviceIds_.erase(svc->ServiceID);
    this->sockets_.erase(sockCtx);

    return error::OK();
}

// ListService lists all services in the delphi server
vector<ServiceInfoPtr> DelphiServer::ListService() {
    vector<ServiceInfoPtr> svcs;
    for (map<string, ServiceInfoPtr>::iterator iter=services_.begin(); iter!=services_.end(); ++iter) {
        svcs.push_back(iter->second);
    }

    return svcs;
}

// addMountPoint adds a mount point for a path
MountPointPtr DelphiServer::addMountPoint(string mountPath) {
    // check if the mount point already exists
    MountPointPtr mountPoint = this->findMountPoint(mountPath);
    if (mountPoint != NULL) {
        return mountPoint;
    }

    // add the mount point
    mountPoint = make_shared<MountPoint>();
    mountPoint->MountPath = mountPath;
    this->mountPoints_[mountPath] = mountPoint;

    return mountPoint;
}

// findMountPoint finds a mount point for a mount path
MountPointPtr DelphiServer::findMountPoint(string mountPath) {
    std::map<string, MountPointPtr>::iterator it;

    // find in the map
    it = this->mountPoints_.find(mountPath);
    if (it != this->mountPoints_.end()) {
        return it->second;
    }

    return nullptr;
}

// deleteMountPoint deletes a mount point
error DelphiServer::deleteMountPoint(string mountPath) {
    std::map<string, MountPointPtr>::iterator it;

    // delete the mount point
    this->mountPoints_.erase(mountPath);
    it = this->mountPoints_.find(mountPath);
    if (it != this->mountPoints_.end()) {
        assert(it->second->Services.size() == 0);
        this->mountPoints_.erase(it);
    }

    return error::OK();
}

// requestMount requests a kind to be mounted for a service
error DelphiServer::requestMount(string kind, string key, string svcName, MountMode mode) {
    // first add the mount point
    MountPointPtr mntPt = this->addMountPoint(getPath(kind, key));

    // find the service
    ServiceInfoPtr svc = this->findServiceName(svcName);
    if (svc == NULL) {
        return error::New("Service not found while mounting");
    }

    // check if two services are mounting this mount point in read-write mode
    if (mode == ReadWriteMode) {
        for (map<string, MountInfo>::iterator iter=mntPt->Services.begin(); iter!=mntPt->Services.end(); ++iter) {
            if ((iter->second.Mode == ReadWriteMode) && (iter->first != svcName)) {
                LogError("Service {} is mounting {} as RW while service {} has mounted it RW",
                        svcName, getPath(kind, key), iter->first);
                return error::New("Multiple services mounting in read-write mode");
            }
        }

        // if this is mounting a kind, key, verify no one has mounted the kind as RW
        if (key != "") {
            MountPointPtr parentMnt = this->addMountPoint(getPath(kind, ""));
            for (map<string, MountInfo>::iterator iter=parentMnt->Services.begin(); iter!=parentMnt->Services.end(); ++iter) {
                if ((iter->second.Mode == ReadWriteMode) && (iter->first != svcName)) {
                    LogError("Service {} is mounting {} as RW while service {} has mounted it RW",
                            svcName, getPath(kind, key), iter->first);
                    return error::New("Multiple services mounting in read-write mode");
                }
            }
        }
    }

    // add mount data
    MountInfo mnt;
    mnt.MountPath = getPath(kind, key);
    mnt.ServiceName = svcName;
    mnt.Mode = mode;
    mntPt->Services[svcName] = mnt;
    svc->Mounts.push_back(mnt);

    return error::OK();
}

// releaseMount release a service from mount point
error DelphiServer::releaseMount(string mountPath, string svcName) {
    std::map<string, MountInfo>::iterator it;

    // find the mount point
    MountPointPtr mntPt = this->findMountPoint(mountPath);
    if (mntPt == NULL) {
        LogError("Could not find the mount point for kind {}", mountPath);
        return error::New("Mount point not found");
    }

    // delete the service
    it = mntPt->Services.find(svcName);
    if (it != mntPt->Services.end()) {
        mntPt->Services.erase(it);
    }

    // if mount point is empty, delete it
    if (mntPt->Services.size() == 0) {
        this->deleteMountPoint(mountPath);
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
        string key = mnt.key();

        // request mount point
        error err = requestMount(kind, key, req->servicename(), mnt.mode());
        if (err.IsNotOK()) {
            return err;
        }

        LogInfo("Mounting {}/{} from service {} mode {}", kind, key, req->servicename(), mnt.mode());


        // get the subtree for the kind
        DbSubtreePtr subtree = this->GetSubtree(kind);

        // walk all objects for this kind and send them
        for (map<string, ObjectDataPtr>::iterator iter=subtree->objects.begin(); iter!=subtree->objects.end(); ++iter) {
            ObjectDataPtr obj = iter->second;
            if (svc->HasMounted(obj->meta().kind(), obj->meta().key())) {
                ObjectData *od = resp->add_objects();
                ObjectMeta *ometa = od->mutable_meta();
                ometa->CopyFrom(obj->meta());
                od->set_op(SetOp);
                od->set_data(obj->data());
            }
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
        error err = this->releaseMount(iter->MountPath, svc->ServiceName);
        if (err.IsNotOK()){
            LogError("Error releasing mount {} for service {}. Err: {}", iter->MountPath, svc->ServiceName, err);
        } else {
            LogInfo("Unmounting {} from service {}", iter->MountPath, svc->ServiceName);
        }
    }

    return this->delService(svc, sockCtx);
}

// HandleChangeReq handles change request from clients
error DelphiServer::HandleChangeReq(int sockCtx, vector<ObjectData> req, vector<ObjectData *> *resp) {
    // FIXME: verify this client has mounted these objects read-write

    // handle each object
    for (vector<ObjectData>::iterator iter=req.begin(); iter!=req.end(); ++iter) {
        string kind = iter->meta().kind();
        string key = iter->meta().key();

        // create a new object instance
        ObjectDataPtr newObj = make_shared<ObjectData>(*iter);

        // log the update
        if (traceEnabled_ && !isKindPeriodic(kind)) {
            BaseObjectPtr objinfo = BaseObject::Create(kind, newObj->data());
	    if (objinfo != nullptr) {
                LogInfo("{}: for {}/{} data: {}", ObjectOperation_Name(iter->op()),
                        kind, key, objinfo->GetMessage()->ShortDebugString());
	    } else {
                LogInfo("{}: for {}/{} ", ObjectOperation_Name(iter->op()), kind, key);
            }
        }

        // add or remove it from db
        switch (iter->op()) {
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
            error err = this->delObject(kind, key, newObj);
            if (err.IsNotOK()) {
                LogError("Error adding the object {}/{}. Err: {}", kind, key, err);
                return err;
            }
            break;
        }
        default:
        {
            LogError("Invalid operation on object {}", iter->DebugString());
            assert(0);
        }
        }

        // queue it for sync
        this->syncQueue_.push_back(newObj);
    }

    return error::OK();
}

error DelphiServer::Stop() {
    this->syncTimer_.stop();
    return msgServer_->Stop();
}

// syncTimerHandler is called when sync time fires
// FIXME: filtering is currently O(M*S*N), convert mounts to hash to make it O(M*S)
// where M is the message count, S is the service count and N is the mount count per server
void DelphiServer::syncTimerHandler(ev::timer &watcher, int revents) {

    // see if we have any objects to send
    if (syncQueue_.size() == 0) {
        return;
    }

    // walk all services and send the objects to them
    for (map<int, ServiceInfoPtr>::iterator iter=sockets_.begin(); iter!=sockets_.end(); ++iter) {
        ServiceInfoPtr svc = iter->second;
        vector<ObjectData *> objlist;

        // walk all objects in sync queue
        for (vector<ObjectDataPtr>::iterator i=syncQueue_.begin(); i!=syncQueue_.end(); ++i) {
            // dequeue from the sync queue
            ObjectDataPtr syncObj = *i;

             // check if the service is interested in this object before doin anything else
            if (svc->HasMounted(syncObj->meta().kind(), syncObj->meta().key()) == false) {
                continue;
            }

            ObjectData *newObj = new ObjectData();
            newObj->CopyFrom(*syncObj);

            // add it to object list to be sent to clients
            objlist.push_back(newObj);
        }

        // send the object list to the client
        if (objlist.size() > 0) {
            msgServer_->SendNotify(iter->first, objlist);
        }
    }

    // clear the queue
    syncQueue_.clear();

}

// isKindPeriodic checks if the kind is a periodic update kind
bool DelphiServer::isKindPeriodic(string kind) {
    if (kind == "DelphiClientStatus") return true;
    else if (kind == "AccelHwRingInfo") return true;

    return false;
}

} // namespace delphi
