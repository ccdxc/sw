// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

// TCP client implementation

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

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

#include "tcp_client.hpp"

namespace delphi {
namespace messenger {

using namespace std;
using namespace google::protobuf::io;

TcpClient::TcpClient(TransportHandlerPtr hndlr) {
    this->handler = hndlr;
    this->isConnected = false;
}

error TcpClient::Connect(char *hostName, int hostPort) {
    struct sockaddr_in my_addr;

    // create the socket
    hsock = socket(AF_INET, SOCK_STREAM  | SOCK_CLOEXEC, 0);
    if(hsock == -1){
            LogError("Error initializing socket {}",errno);
            exit(1);
    }

    // where to connect to
    my_addr.sin_family = AF_INET ;
    my_addr.sin_port = htons(hostPort);
    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = inet_addr(hostName);

    // connect to the server
    if (connect(hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
        if(errno != EINPROGRESS){
            // LogError("Error connecting socket {}", errno);
            close(hsock);
            return error::New("Connection failed");
        }
    }

    // create new TCP receiver for the socket
    this->receiver = unique_ptr<SockReceiver>(new SockReceiver(hsock, this->handler));

    return 0;
}

error TcpClient::Send(MessagePtr msg) {
    return sendMsg(this->hsock, msg);
}

error TcpClient::Close() {
    this->receiver->Stop();
    close(this->hsock);

    return error::OK();
}

} // namespace messenger
} // namespace delphi
