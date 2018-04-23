// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "sock_receiver.hpp"


namespace delphi {
namespace messanger {

using namespace std;
using namespace google::protobuf::io;

SockReceiver::SockReceiver(int sock, TransportHandlerPtr hndlr) {
    this->sock = sock;
    this->handler = hndlr;
    // FIXME: put the socket in non-block mode
    // fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);
    this->evio.set<SockReceiver, &SockReceiver::ReadSocket>(this);
    this->evio.start(sock, ev::READ);
}

void SockReceiver::ReadSocket(ev::io &watcher, int revents) {
    // check for error event
    if (EV_ERROR & revents) {
        LogError("Socket Got error event {}", revents);
        assert(0);
    }

    char buffer[4];
    int bytecount = 0;
    memset(buffer, '\0', 4);

    //Peek into the socket and get the packet size
    if((bytecount = recv(watcher.fd, buffer, 4, MSG_PEEK))== -1){
        LogError("Error receiving data {}. Closing socket connection\n");
        this->evio.stop();
        if (this->handler != NULL) {
            this->handler->SocketClosed(this->sock);
        }
        return;
    } else if (bytecount == 0) {
        LogError("Remote side Closed socket connection. errno {}", errno);
        this->evio.stop();
        if (this->handler != NULL) {
            this->handler->SocketClosed(this->sock);
        }
        return;
    }

    // read the body
    MessagePtr msg = readBody(watcher.fd, readHdr(buffer));
    assert(msg != NULL);

    // handle the message
    this->handler->HandleMsg(watcher.fd, msg);
}

void SockReceiver::Stop() {
    this->evio.stop();
}

} // messanger namespace
} // namespace delphi
