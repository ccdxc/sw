// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

// TCP server implementation

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

#include "tcp_server.hpp"

namespace delphi {
namespace messanger {

using namespace std;
using namespace google::protobuf::io;

// ------------- class methods

// TcpServer is the constructor
TcpServer::TcpServer(TransportHandlerPtr hndlr) {
    this->handler = hndlr;
}

// Accept accepts incoming connections and instantiates a receiver for each connection
void TcpServer::Accept(ev::io &watcher, int revents) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    // check for error event
    if (EV_ERROR & revents) {
        LogError("TC server Got error event {}", revents);
        return;
    }

    // accept incoming connection
    int client_sd = accept(watcher.fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_sd < 0) {
        LogError("accept error {}, errno {}", client_sd, errno);
        return;
    }

    LogDebug("--------------------- Received connection from {}", inet_ntoa(client_addr.sin_addr));

    // create new TCP handler for the socket
    this->receivers[client_sd] = unique_ptr<SockReceiver>(new SockReceiver(client_sd, this->handler));
}

// Listen starts listening on a port
error TcpServer::Listen(int lis_port) {
    int p_int = 1;
    struct sockaddr_in my_addr;

    // create the socket
    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if (hsock == -1){
            LogError("Error initializing socket {}", errno);
            exit(1);
    }

    // set some socket options
    if((setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)&p_int, sizeof(int)) == -1 )||
       (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)&p_int, sizeof(int)) == -1 ) ){
        LogError("Error setting options {}", errno);
        exit(1);
    }

    my_addr.sin_family = AF_INET ;
    my_addr.sin_port = htons(lis_port);
    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = INADDR_ANY ;

    // bind and listen to the port
    if (bind(hsock, (sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
            LogError("Error binding to socket, make sure nothing else is listening on this port {}", errno);
            exit(1);
    }
    // FIXME: put the socket in non blocking mode
    // fcntl(hsock, F_SETFL, fcntl(hsock, F_GETFL, 0) | O_NONBLOCK);

    LogDebug("waiting for a connection on port {}", lis_port);

    if (listen(hsock, 10) == -1 ){
            LogError("Error listening {}", errno);
            exit(1);
    }

    this->evio.set<TcpServer, &TcpServer::Accept>(this);
    this->evio.start(hsock, ev::READ);

    return error::OK();
}

error TcpServer::Send(int sock, MessagePtr msg) {
    return sendMsg(sock, msg);
}

vector<int> TcpServer::ListSockets() {
    vector<int> list;

    // collect all sockets
    for(map<int, SockReceiverUptr>::iterator i=receivers.begin(); i!=receivers.end(); ++i) {
        list.push_back(i->first);
    }

    return list;
}

error TcpServer::Stop() {
    // close all connected sockets
    for(map<int, SockReceiverUptr>::iterator i=receivers.begin(); i!=receivers.end(); ++i) {
        i->second->Stop();
        close(i->first);
    }

    // close the listening socket
    this->evio.stop();
    close(this->hsock);

    return error::OK();
}
} // messanger namespace
} // namespace delphi
