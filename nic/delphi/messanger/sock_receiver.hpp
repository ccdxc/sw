// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_MSG_SOCK_RECEIVER_H_
#define _DELPHI_MSG_SOCK_RECEIVER_H_

// socket receiver implementation

#include <map>
#include <ev++.h>

#include "transport.hpp"
#include "sock_utils.hpp"

namespace delphi {
namespace messanger {

class SockReceiver  {
public:
    SockReceiver(int sock, TransportHandlerPtr handler);
    void ReadSocket(ev::io &watcher, int revents);
    void Stop();
private:
    TransportHandlerPtr handler;
    ev::io              evio;
    int                 sock;
};

typedef std::unique_ptr<SockReceiver>  SockReceiverUptr;

} // namespace messanger
} // namespace delphi

#endif // _DELPHI_MSG_SOCK_RECEIVER_H_
