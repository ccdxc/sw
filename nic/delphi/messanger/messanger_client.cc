// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <sstream>

#include "messanger_client.hpp"
#include "tcp_client.hpp"

namespace delphi {
namespace messanger {

// MessangerClient constructor
MessangerClient::MessangerClient(ClientHandlerPtr handler) {
    this->handler = handler;
    this->currMsgId = 1;
    this->connected = false;
}

// Connect connects the client to server
error MessangerClient::Connect() {
    this->client = make_shared<TcpClient>(shared_from_this());
    string localHost = "127.0.0.1";

    // connect to the server
    error err = this->client->Connect((char *)localHost.c_str(), TCP_LISTEN_PORT);
    if (err.IsNotOK()) {
        return err;
    }

    // remember we are connected
    this->connected = true;

    return err;
}

bool MessangerClient::IsConnected() {
    return this->connected;
}

// Mount mounts specific part of the object tree
error MessangerClient::SendMountReq(string svcName, vector<MountDataPtr> mounts) {
    MessagePtr msg(make_shared<Message>());
    MountReqMsgPtr mntReq = make_shared<MountReqMsg>();
    assert(msg != NULL);

    // set message params
    msg->set_type(MessageType::MountReq);
    msg->set_messageid(this->currMsgId++);
    mntReq->set_servicename(svcName);

    // add each mount point
    for(vector<MountDataPtr>::iterator iter=mounts.begin(); iter!=mounts.end(); ++iter) {
        MountData *mnt = mntReq->add_mounts();
        *mnt = *(*iter);
    }

    // serialize the mount request
    string out_str;
    mntReq->SerializeToString(&out_str);
    ObjectData *obj = msg->add_objects();
    ObjectMeta *meta = obj->mutable_meta();
    meta->set_kind(mntReq->GetDescriptor()->name());
    obj->set_data(out_str);

    // send the message to server
    error err = this->client->Send(msg);
    if (err.IsNotOK()) {
        LogError("Error sending the mount request to server. Err: {}", err);
        return err;
    }

    return error::OK();
}

// SendChangeReq sends a change request message to server
error MessangerClient::SendChangeReq(vector<ObjectData *> objlist) {
    MessagePtr msg(make_shared<Message>());
    assert(msg != NULL);

    // set message params
    msg->set_type(ChangeReq);
    msg->set_messageid(this->currMsgId++);

    // encode the objects
    for(vector<ObjectData *>::iterator iter=objlist.begin(); iter!=objlist.end(); ++iter) {
        msg->mutable_objects()->AddAllocated(*iter);
    }

    // send the message
    error err = this->client->Send(msg);
    if (err.IsNotOK()) {
        LogError("Error sending message to server. Err: {}", err);
        return err;
    }

    return error::OK();
}

// HandleMsg handles incoming messages from transport
error MessangerClient::HandleMsg(int sock, MessagePtr msg) {
    // handle based on message type
    switch (msg->type()) {
    case Notify:
    {
        vector<ObjectData *> objlist;

        // parse the incoming request
        for (int i = 0; i < msg->objects_size(); i++) {
            ObjectData *obj = msg->mutable_objects(i);
            objlist.push_back(obj);
        }

        // call the handler
        error err = this->handler->HandleNotify(objlist);
        if (err.IsNotOK()) {
            LogError("Error during notify handling. Err: {}", err);
            return err;
        }
        break;
    }
    case MountResp:
    {
        MountRespMsgPtr mountResp = make_shared<MountRespMsg>();
        vector<ObjectData *> objlist;

        // check for errors
        if (msg->status() != "") {
            LogError("Mount failed. response error: {}", msg->status());

            // let handler know about it
            this->handler->HandleMountResp(mountResp->serviceid(), msg->status(), objlist);

            return error::New("Mount Failed");
        }

        // decode mount response
        assert(msg->objects_size() == 1);
        const ObjectData& obj = msg->objects(0);
        mountResp->ParseFromString(obj.data());

        // parse the incoming request
        for (int i = 0; i < mountResp->objects_size(); i++) {
            ObjectData *obj = mountResp->mutable_objects(i);
            objlist.push_back(obj);
        }

        // call the handler
        error err = this->handler->HandleMountResp(mountResp->serviceid(), msg->status(), objlist);
        if (err.IsNotOK()) {
            LogError("Error during mount responce handling. Err: {}", err);
            return err;
        }
        break;
    }
    case StatusResp:
        // FIXME: nothing to do for now.
        break;
    default:
    {
        LogError("Received unknown message type {}", msg->type());
        assert(0);
        return error::New("Received unknown message type");
    }
    }

    return error::OK();
}

// SocketClosed handle socket closed event
error MessangerClient::SocketClosed(int sock) {
    // FIXME: handle socket closing
    LogError("Connection to server closed. Exiting...");
    exit(1);
    return error::OK();
}

error MessangerClient::Close() {
    // close the transport socket
    return this->client->Close();
}

}  // namespace messanger
} // namespace delphi
