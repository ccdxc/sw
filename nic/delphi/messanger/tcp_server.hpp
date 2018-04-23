// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_MSG_TCP_SERVER_H_
#define _DELPHI_MSG_TCP_SERVER_H_

// TCP server implementation

#include <map>
#include <ev++.h>

#include "transport.hpp"
#include "sock_utils.hpp"
#include "sock_receiver.hpp"

namespace delphi {
namespace messanger {

// tcp server implementation
class TcpServer : public TransportServer {
public:
    TcpServer(TransportHandlerPtr handler);
    error       Listen(int lis_port);
    void        Accept(ev::io &watcher, int revents);
    error       Send(int sock, MessagePtr msg);
    vector<int> ListSockets();
    error       Stop();
private:
    ev::io                       evio;
    int                          hsock;
    TransportHandlerPtr          handler;
    map<int, SockReceiverUptr>   receivers;
};
typedef std::shared_ptr<TcpServer> TcpServerPtr;


} // namespace messanger
} // namespace delphi

#endif // _DELPHI_MSG_TCP_TRANSPORT_H_
