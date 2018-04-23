// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _MSG_MESSANGER_SERVER_H_
#define _MSG_MESSANGER_SERVER_H_

// messanger server and client class definitions

#include "nic/delphi/messanger/proto/types.pb.h"
#include "nic/delphi/utils/utils.hpp"
#include "nic/delphi/messanger/transport.hpp"

namespace delphi {
namespace messanger {


// shared pointer types
typedef std::shared_ptr<ObjectMeta>   ObjectMetaPtr;
typedef std::shared_ptr<ObjectData>   ObjectDataPtr;
typedef std::shared_ptr<MountData>    MountDataPtr;
typedef std::shared_ptr<MountReqMsg>  MountReqMsgPtr;
typedef std::shared_ptr<MountRespMsg> MountRespMsgPtr;


// abstract class definition that handles server messages
class ServerHandler {
public:
    virtual error HandleChangeReq(int sockCtx, vector<ObjectData *> req, vector<ObjectData *> *resp) = 0;
    virtual error HandleMountReq(int sockCtx, MountReqMsgPtr req, MountRespMsgPtr resp) = 0;
    virtual error HandleSocketClosed(int sockCtx) = 0;
};
typedef std::shared_ptr<ServerHandler> ServerHandlerPtr;

// messanger server
class MessangerServer : public TransportHandler,  public enable_shared_from_this<MessangerServer> {
public:
    MessangerServer(ServerHandlerPtr handler);
    error Start();
    error SendNotify(int sockCtx, vector<ObjectData *> objlist);
    error Stop();

    // needed by TransportHandler
    error HandleMsg(int sock, MessagePtr msg);
    error SocketClosed(int sock);
private:
    // state
    ServerHandlerPtr       handler;
    TransportServerPtr     srv;
    int                    currMsgId;

    // methods
    error       handleMountReq(int sockCtx, MessagePtr req, MessagePtr resp);
    error       handleChangeReq(int sockCtx, MessagePtr req, MessagePtr resp);
};
typedef std::shared_ptr<MessangerServer> MessangerServerPtr;

} // namespace messanger
} // namespace delphi

#endif // _MSG_MESSANGER_H_
