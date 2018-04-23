// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_MSG_TCP_CLIENT_H_
#define _DELPHI_MSG_TCP_CLIENT_H_

// TCP client implementation

#include <map>
#include <ev++.h>

#include "transport.hpp"
#include "sock_utils.hpp"
#include "sock_receiver.hpp"

namespace delphi {
namespace messanger {

class TcpClient : public TransportClient {
public:
    TcpClient(TransportHandlerPtr handler);
    error Connect(char *hostName, int hostPort);
    error Send(MessagePtr msg);
    error Close();
private:
    ev::io                           evio;
    int                              hsock;
    TransportHandlerPtr              handler;
    SockReceiverUptr                  receiver;
};
typedef std::shared_ptr<TcpClient> TcpClientPtr;

} // namespace messanger
} // namespace delphi

#endif // _DELPHI_MSG_TCP_TRANSPORT_H_
