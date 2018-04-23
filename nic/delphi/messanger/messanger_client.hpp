// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _MSG_MESSANGER_CLIENT_H_
#define _MSG_MESSANGER_CLIENT_H_

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

// abstract class that handles client messages
class ClientHandler {
public:
    virtual error HandleNotify(vector<ObjectData *> objlist) = 0;
    virtual error HandleMountResp(uint16_t svcID, string status, vector<ObjectData *> objlist) = 0;
};
typedef std::shared_ptr<ClientHandler> ClientHandlerPtr;

// messanger client
class MessangerClient : public TransportHandler, public enable_shared_from_this<MessangerClient> {
public:
    MessangerClient(ClientHandlerPtr handler);
    error Connect();
    bool IsConnected();
    error SendMountReq(string svcName, vector<MountDataPtr> mounts);
    error SendChangeReq(vector<ObjectData *> objlist);
    error Close();

    // needed by TransportHandler
    error HandleMsg(int sock, MessagePtr msg);
    error SocketClosed(int sock);
private:
    ClientHandlerPtr       handler;
    TransportClientPtr     client;
    int                    currMsgId;
    bool                   connected;
};
typedef std::shared_ptr<MessangerClient> MessangerClientPtr;

} // namespace messanger
} // namespace delphi

#endif // _MSG_MESSANGER_CLIENT_H_
