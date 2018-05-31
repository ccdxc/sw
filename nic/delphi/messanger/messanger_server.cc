// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <assert.h>
#include <stdio.h>

#include "messanger_server.hpp"
#include "tcp_server.hpp"

namespace delphi {
namespace messanger {

using namespace std;

// MessangerServer constructor
MessangerServer::MessangerServer(ServerHandlerPtr handler) {
    this->handler = handler;
    this->currMsgId = 1;
}

// Start starts the transport server listening on a port
error MessangerServer::Start() {
    this->srv = make_shared<TcpServer>(shared_from_this());

    LogInfo("Server is listening on localhost: {}", TCP_LISTEN_PORT);

    // start TCP server
    return srv->Listen(TCP_LISTEN_PORT);
}

// HandleMsg handles an incoming message (called from transport)
error MessangerServer::HandleMsg(int sock, MessagePtr msg) {
    // handle message by type
    switch (msg->type()) {
    case ChangeReq:
    {
        MessagePtr resp(make_shared<Message>());

        // call the handler
        error err = this->handleChangeReq(sock, msg, resp);
        if (err.IsNotOK()) {
            LogError("Error during change req handling. Err: {}", err);
            resp->set_status("Error handling change request");
            // just send an error response
        }

        // send a response
        err = this->srv->Send(sock, resp);
        if (err.IsNotOK()) {
            LogError("Error sending response to sock {}. Err: {}", sock, err);
            return err;
        }
        break;
    }
    case MountReq:
    {
        MessagePtr resp(make_shared<Message>());

        // call the handler
        error err = this->handleMountReq(sock, msg, resp);
        if (err.IsNotOK()) {
            LogError("Error during mount req handling. Err: {}", err);
            resp->set_status("Error handling mount request");
            // send the error response
        }

        // send a response
        err = this->srv->Send(sock, resp);
        if (err.IsNotOK()) {
            LogError("Error sending response to sock {}. Err: {}", sock, err);
            return err;
        }
        break;
    }
    default:
    {
        LogError("Received unknown message type {}, msg id {}, num objs: {}", msg->type(), msg->messageid(), msg->objects_size());
        assert(0);
        return error::New("Server received unknown message");
    }
    };

    return error::OK();
}

// SocketClosed handles socket closed callback
error MessangerServer::SocketClosed(int sock) {
    return this->handler->HandleSocketClosed(sock);
}

// handleMountReq handles a mount request message
error MessangerServer::handleMountReq(int sockCtx, MessagePtr req, MessagePtr resp) {
    MountReqMsgPtr mountReq = make_shared<MountReqMsg>();
    MountRespMsgPtr mountResp = make_shared<MountRespMsg>();

    // parse the incoming request
    assert(req->objects_size() == 1);
    const ObjectData& obj = req->objects(0);
    mountReq->ParseFromString(obj.data());
    LogDebug("Got mount Req: {}", mountReq->DebugString().c_str());

    // prepare response
    resp->set_type(MountResp);
    resp->set_messageid(this->currMsgId++);
    resp->set_responseto(req->messageid());

    // call the handler
    error err = this->handler->HandleMountReq(sockCtx, mountReq, mountResp);
    if (err.IsNotOK()) {
        LogError("Error during mount req handling. Err: {}", err);
        return err;
    }

    LogDebug("Sent Mount Resp: {}", mountResp->DebugString().c_str());

    // encode the response
    string out_str;
    mountResp->SerializeToString(&out_str);
    ObjectData *objd = resp->add_objects();
    objd->set_data(out_str);

    return error::OK();
}

error MessangerServer::handleChangeReq(int sockCtx, MessagePtr req, MessagePtr resp) {
    vector<ObjectData *> objReq;
    vector<ObjectData *> objResp;

    LogDebug("Got change Req: {}", req->DebugString().c_str());

    // setup the response
    resp->set_type(StatusResp);
    resp->set_messageid(this->currMsgId++);
    resp->set_responseto(req->messageid());

    // parse the incoming request
    for (int i = 0; i < req->objects_size(); i++) {
        ObjectData *obj = req->mutable_objects(i);
        objReq.push_back(obj);
    }

    // call the handler
    error err = this->handler->HandleChangeReq(sockCtx, objReq, &objResp);
    if (err.IsNotOK()) {
        LogError("Error during change req handling. Err: {}", err);
        return err;
    }

    // encode the response
    for(vector<ObjectData *>::iterator iter=objResp.begin(); iter!=objResp.end(); ++iter) {
        resp->mutable_objects()->AddAllocated(*iter);

    }

    return error::OK();
}

// SendNotify sends a notify message to a client
error MessangerServer::SendNotify(int sockCtx, vector<ObjectData *> objlist) {
    MessagePtr msg(make_shared<Message>());

    // set message params
    msg->set_type(Notify);
    msg->set_messageid(this->currMsgId++);

    // encode the objects
    for(vector<ObjectData *>::iterator iter=objlist.begin(); iter!=objlist.end(); ++iter) {
        msg->mutable_objects()->AddAllocated(*iter);
    }

    // send the message
    error err = this->srv->Send(sockCtx, msg);
    if (err.IsNotOK()) {
        LogError("Error sending message to sock {}. Err: {}", sockCtx, err);
        return err;
    }

    LogDebug("Sent notify to sock {}: {}", sockCtx, msg->DebugString().c_str());

    return error::OK();
}

// Stop stops the messanger server
error MessangerServer::Stop() {
    // stop the transport server
    return this->srv->Stop();
}

}  // namespace messanger
} // namespace delphi
