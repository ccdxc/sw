// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <unistd.h>
#include "gtest/gtest.h"

#include "tcp_server.hpp"
#include "tcp_client.hpp"

namespace {

using namespace std;
using namespace delphi;
using namespace delphi::messanger;

// number of clients to test
#define NUM_CLIENTS 3

// port where test tcp server runs
#define TEST_TCP_PORT 7701

// number of message types we'll use
#define MAX_MSG_TYPES 10

// event loop thread
void * startEventLoop(void* arg) {
    ev::default_loop *loop = (ev::default_loop *)arg;
    loop->run(0);

    return NULL;
}

class testHandler : public TransportHandler {
public:
    string name;
    int msgStats[MAX_MSG_TYPES];
    testHandler() {
        for (int i = 0; i < MAX_MSG_TYPES; i++) {
            this->msgStats[i] = 0;
        }
    }
    error HandleMsg(int sock, MessagePtr msg) {
        LogDebug("{} Handler got called for msg type {}", name.c_str(), msg->type());
        this->msgStats[msg->type()]++;
        return error::OK();
    }
    error SocketClosed(int sock) {
        return error::OK();
    }
};

class TcpTransportTest : public testing::Test {
protected:
    ev::default_loop     loop;
    pthread_t            ev_thread_id = 0;
    TransportServerPtr   server;
    TransportClientPtr   clients[NUM_CLIENTS];
    shared_ptr<testHandler>          serverHandler;
    shared_ptr<testHandler>          clientHandler[NUM_CLIENTS];

public:

    void SetUp() {
        string localHost = "127.0.0.1";

        LogInfo("Starting server\n");

        // create server and start listening on the port
        serverHandler = make_shared<testHandler>();
        serverHandler->name = "server";
        server = make_shared<TcpServer>(serverHandler);
        server->Listen(TEST_TCP_PORT);

        LogInfo("TCP server is listening on {}", TEST_TCP_PORT);

        usleep(1000);

        // start each of the clients
        for (int i = 0; i < NUM_CLIENTS; i++) {
            clientHandler[i] = make_shared<testHandler>();
            clientHandler[i]->name = "client";
            clients[i] = make_shared<TcpClient>(clientHandler[i]);
            clients[i]->Connect((char *)localHost.c_str(), TEST_TCP_PORT);
        }

        pthread_create(&ev_thread_id, 0, &startEventLoop, (void*)&loop);
        usleep(1000);
    }

    void TearDown() {
        for (int i = 0; i < NUM_CLIENTS; i++) {
            clients[i]->Close();
        }
        server->Stop();
        usleep(1000);

        // kill the event loop thread
        LogDebug("Stopping event loop\n");
        loop.break_loop(ev::ALL);
        pthread_cancel(ev_thread_id);
        pthread_join(ev_thread_id, NULL);
    }
};

TEST_F(TcpTransportTest, BasicMsgTest) {
    int i, j;

    // send a message from each client
    for (i = 0; i < NUM_CLIENTS; i++) {
        MessagePtr msg(make_shared<Message>());

        // build the message
        msg->set_type(ChangeReq);

        for (j = 0; j < 2; j++) {
            ObjectData *obj = msg->add_objects();
            obj->set_data("Test Data");
            obj->set_op(SetOp);
            ObjectMeta *meta = obj->mutable_meta();
            meta->set_kind("Endpoint");
        }

        // send the message
        clients[i]->Send(msg);
    }

    usleep(5000);

    // verify server received the messages
    ASSERT_EQ(serverHandler->msgStats[ChangeReq], NUM_CLIENTS) << "Server did not receive all the messages";
    LogDebug("Server got {} messages\n", serverHandler->msgStats[ChangeReq]);

    usleep(1000);

    // get all sockets
    vector<int> sockList = server->ListSockets();

    for(vector<int>::iterator iter=sockList.begin(); iter!=sockList.end(); ++iter){
        MessagePtr msg(make_shared<Message>());

        // build the message
        msg->set_type(ChangeReq);

        for (j = 0; j < 2; j++) {
            ObjectData *obj = msg->add_objects();
            obj->set_data("Test Data");
            obj->set_op(SetOp);
            ObjectMeta *meta = obj->mutable_meta();
            meta->set_kind("Endpoint");
        }

        server->Send(*iter, msg);
    }

    usleep(1000);

    // verify client received the messages
    for (j = 0; j < NUM_CLIENTS; j++) {
        ASSERT_EQ(clientHandler[j]->msgStats[ChangeReq], 1) << "Client did not receive all the messages";
        LogDebug("Client {} got {} messages\n", j, clientHandler[j]->msgStats[ChangeReq]);
    }
}

} // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
