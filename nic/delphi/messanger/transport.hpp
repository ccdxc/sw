// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_MSG_TRANSPORT_H_
#define _DELPHI_MSG_TRANSPORT_H_

// abstract class definitions for transport server and client
#include <memory.h>

#include "nic/delphi/messanger/proto/types.pb.h"
#include "nic/delphi/utils/utils.hpp"

namespace delphi {
namespace messanger {

// TCP server port
#define TCP_LISTEN_PORT 7001

// Message pointers
typedef std::shared_ptr<Message> MessagePtr;

// TransportServer : interface that all transport plugins implement
class TransportServer {
public:
    virtual error Listen(int lis_port) = 0;
    virtual error Send(int sock, MessagePtr msg) = 0;
    virtual vector<int> ListSockets() = 0;
    virtual error Stop() = 0;
};
typedef std::shared_ptr<TransportServer> TransportServerPtr;

// TransportClient : interface that all transport plugins implement
class TransportClient {
public:
    virtual error Connect(char *hostName, int hostPort) = 0;
    virtual error Send(MessagePtr msg) = 0;
    virtual error Close() = 0;
};
typedef std::shared_ptr<TransportClient> TransportClientPtr;

// TransportHandler : interface implemented by message handlers
class TransportHandler {
public:
    virtual error HandleMsg(int sock, MessagePtr msg) = 0;
    virtual error SocketClosed(int sock) = 0;
};

typedef std::shared_ptr<TransportHandler> TransportHandlerPtr;

} // namespace messanger
} // namespace delphi

#endif // _DELPHI_MSG_TRANSPORT_H_
