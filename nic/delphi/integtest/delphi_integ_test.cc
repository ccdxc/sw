// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ev++.h>
#include "gtest/gtest.h"

#include "nic/delphi/hub/delphi_server.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/delphi/sdk/delphi_utest.hpp"
#include "nic/delphi/shm/delphi_metrics.hpp"
#include "gen/proto/example.delphi.hpp"
#include "gen/proto/example_stats.delphi.hpp"

namespace {
using namespace std;
using namespace delphi;
using delphi::objects::ExampleSpec;
using delphi::objects::ExampleStatus;
using delphi::objects::ExampleSpecPtr;
using delphi::objects::ExampleStatusPtr;

// event loop thread
void * startEventLoop(void* arg) {
    delphi::SdkPtr sdk = *(delphi::SdkPtr *)arg;
    sdk->MainLoop();

    return NULL;
}

class testReactor : public delphi::objects::ExampleSpecReactor,
                    public delphi::objects::ExampleStatusReactor {
public:
    int  numCreateCallbacks;
    int  numUpdateCallbacks;
    int  numDeleteCallbacks;
    testReactor() {
        numCreateCallbacks = 0;
        numUpdateCallbacks = 0;
        numDeleteCallbacks = 0;
    }
    virtual error OnExampleSpecCreate(ExampleSpecPtr obj) {
        numCreateCallbacks++;
        return error::OK();
    }
    virtual error OnExampleSpecUpdate(ExampleSpecPtr obj) {
        numUpdateCallbacks++;
        return error::OK();
    }
    virtual error OnExampleSpecDelete(ExampleSpecPtr obj) {
        numDeleteCallbacks++;
        return error::OK();
    }
    virtual error OnExampleStatusCreate(ExampleStatusPtr obj) {
        numCreateCallbacks++;
        return error::OK();
    }
    virtual error OnExampleStatusUpdate(ExampleStatusPtr obj) {
        numUpdateCallbacks++;
        return error::OK();
    }
    virtual error OnExampleStatusDelete(ExampleStatusPtr obj) {
        numDeleteCallbacks++;
        return error::OK();
    }
};

class TestService : public Service {
private:
    int               service_id_ = 0;
    delphi::SdkPtr    sdk_;
public:
    // public state
    bool inited = false;
    shared_ptr<testReactor>   objMgr;
    ev::async   msgqAsync;
    vector<int> msgQueue;
    TestService(int clid, delphi::SdkPtr sk) {
        this->service_id_ = clid;
        this->sdk_ = sk;
        sk->enterAdminMode();

        this->msgqAsync.set<TestService, &TestService::msgqAsyncHandler>(this);
        this->msgqAsync.start();

        // mount and watch test objects
        objMgr = make_shared<testReactor>();
        if (clid == 0) {
            delphi::objects::ExampleSpec::Mount(sk, ReadWriteMode);
            delphi::objects::ExampleStatus::Mount(sk, ReadWriteMode);
        } else {
            delphi::objects::ExampleSpec::Mount(sk, ReadMode);
            delphi::objects::ExampleStatus::Mount(sk, ReadMode);
        }
        delphi::objects::ExampleSpec::Watch(sk, objMgr);
        delphi::objects::ExampleStatus::Watch(sk, objMgr);

    }
    virtual string Name() {
        return "TestService-" + to_string(this->service_id_);
    }
    virtual void OnMountComplete() {
        LogInfo("TestHandler {} OnMountComplete got called\n", service_id_);
        inited = true;
    }
    virtual void SocketClosed() {
        LogInfo("TestHandler {} SocketClosed got called\n", service_id_);
        inited = false;
    }
    delphi::error QueueObject(int obj_id, int data_id) {
        static char buffer [33];
        // create an object
        ExampleSpecPtr tobj = make_shared<ExampleSpec>();
        sprintf(buffer, "-%d", data_id);
        tobj->set_macaddress("Test Data" + string(buffer));
        tobj->mutable_key()->set_ifidx(obj_id);

        // add it to database
        return sdk_->QueueUpdate(tobj);
    }
    delphi::error SendObject(int obj_id) {
        // create an object
        ExampleSpecPtr tobj = make_shared<ExampleSpec>();
        tobj->set_macaddress("Test Data");
        tobj->mutable_key()->set_ifidx(obj_id);

        // add it to database
        return sdk_->SetObject(tobj);
    }
    void msgqAsyncHandler(ev::async &watcher, int revents) {
        static int counter = 1;
        static char buffer [33];
        for (vector<int>::iterator iter=msgQueue.begin(); iter!=msgQueue.end(); ++iter) {
            int obj_id = *iter;
            ExampleSpecPtr tobj = make_shared<ExampleSpec>("");
            sprintf(buffer, "-%d", counter++);
            tobj->set_macaddress("Test Data" + string(buffer));
            tobj->mutable_key()->set_ifidx(obj_id);

            // update and sync
            sdk_->SyncObject(tobj);
        }
    }
};
typedef std::shared_ptr<TestService> TestServicePtr;

#define NUM_CLIENTS 5

class DelphiIntegTest : public testing::Test {
protected:
    pthread_t          ev_thread_id;
    DelphiServerPtr    server;
    delphi::SdkPtr     sdks[NUM_CLIENTS];
    TestServicePtr     services[NUM_CLIENTS];
public:
    virtual void SetUp() {

        // instantiate the delphi server
        server = make_shared<DelphiServer>();
        server->Start();
        usleep(1000);

        // start the clients
        for (int i = 0; i < NUM_CLIENTS; i++) {
            sdks[i] = make_shared<delphi::Sdk>();
            services[i] = make_shared<TestService>(i, sdks[i]);
            sdks[i]->RegisterService(services[i]);
            // firt client run the main loop other just connect
            if (i != 0) {
                sdks[i]->Connect();
            }
        }
        usleep(1000);

        // start event loop
        pthread_create(&ev_thread_id, 0, &startEventLoop, (void*)&sdks[0]);
        LogInfo("Started thread {}", ev_thread_id);
        usleep(1000);
    }
    virtual void TearDown() {
        for (int i = 0; i < NUM_CLIENTS; i++) {
            sdks[i]->Stop();
        }
        usleep(1000 * 10);
        // stop the thread
        pthread_cancel(ev_thread_id);
        pthread_join(ev_thread_id, NULL);
        usleep(1000);

        server->Stop();
        usleep(1000 * 10);
    }
};

TEST_F(DelphiIntegTest, BasicIntegTest) {
    usleep(1000);

    // verify all the clients are inited
    for (int i = 0; i < NUM_CLIENTS; i++) {
        ASSERT_EQ_EVENTUALLY(services[i]->inited, true) << "client was not inited";
    }

    // send one object from each client
    for (int i = 0; i < NUM_CLIENTS; i++) {
        auto err = services[i]->QueueObject(i+1, 0);
        ASSERT_TRUE(err.IsOK()) << "Error queuing object";
    }

    for (int i = 0; i < NUM_CLIENTS; i++) {
        LogInfo("Client {} has {} objects", i, sdks[i]->ListKind("ExampleSpec").size());
        ASSERT_EQ_EVENTUALLY(sdks[i]->ListKind("ExampleSpec").size(), NUM_CLIENTS) << "client did not have all the objects";
        ASSERT_EQ_EVENTUALLY(services[i]->objMgr->numCreateCallbacks, NUM_CLIENTS) << "reactor did not receive create callbacks";
    }

}

TEST_F(DelphiIntegTest, MultipleUpdateTest) {
    int num_updates = 10;
    usleep(1000);

    // verify all the clients are inited
    for (int i = 0; i < NUM_CLIENTS; i++) {
        ASSERT_EQ_EVENTUALLY(services[i]->inited, true) << "client was not inited";
    }

    // send one object from each client
    for (int i = 0; i < NUM_CLIENTS; i++) {
        auto err = services[i]->QueueObject(i+1, 0);
        ASSERT_TRUE(err.IsOK()) << "Error queuing object";
    }

    for (int i = 0; i < NUM_CLIENTS; i++) {
        ASSERT_EQ_EVENTUALLY(services[i]->objMgr->numCreateCallbacks, NUM_CLIENTS) << "reactor did not receive create callbacks";
    }

    // verify duplicate updates dont trigger update callback
    for (int i = 0; i < NUM_CLIENTS; i++) {
        auto err = services[i]->QueueObject(i+1, 0);
        ASSERT_TRUE(err.IsOK()) << "Error queuing object";
    }
    usleep(1000 * 100);

    for (int i = 0; i < NUM_CLIENTS; i++) {
        ASSERT_EQ_EVENTUALLY(services[i]->objMgr->numCreateCallbacks, NUM_CLIENTS) << "reactor did not receive create callbacks";
        ASSERT_EQ_EVENTUALLY(services[i]->objMgr->numUpdateCallbacks, 0) << "reactor got invalid update callbacks";
    }

    // repeatedly send updates and verify we get only one update each time
    for (int iter = 0; iter < num_updates; iter++) {
        // send one object from each client
        for (int i = 0; i < NUM_CLIENTS; i++) {
            auto err = services[i]->QueueObject(i+1, iter+1);
            ASSERT_TRUE(err.IsOK()) << "Error queuing object";
        }

        for (int i = 0; i < NUM_CLIENTS; i++) {
            ASSERT_EQ(services[i]->objMgr->numCreateCallbacks, NUM_CLIENTS) << "reactor did not receive create callbacks";
            ASSERT_EQ_EVENTUALLY(services[i]->objMgr->numUpdateCallbacks, (NUM_CLIENTS * (iter + 1))) << "reactor did not receive create callbacks";
        }
    }

}

TEST_F(DelphiIntegTest, HubDisconnectTest) {
    usleep(1000);

    // verify all the clients are inited
    for (int i = 0; i < NUM_CLIENTS; i++) {
        ASSERT_EQ_EVENTUALLY(services[i]->inited, true) << "client was not inited";
    }

    // send one object from each client
    for (int i = 0; i < NUM_CLIENTS; i++) {
        auto err = services[i]->QueueObject(i+1, 0);
        ASSERT_TRUE(err.IsOK()) << "Error queuing object";
    }

    for (int i = 0; i < NUM_CLIENTS; i++) {
        ASSERT_EQ_EVENTUALLY(services[i]->objMgr->numCreateCallbacks, NUM_CLIENTS) << "reactor did not receive create callbacks";
    }

    // stop delphi hub
    this->server->Stop();
    usleep(1000 * 10);

    // verify socket close callback got called
    for (int i = 0; i < NUM_CLIENTS; i++) {
        ASSERT_EQ_EVENTUALLY(services[i]->inited, false) << "socket closed was not called";
    }

    // restart the server
    this->server = make_shared<DelphiServer>();
    this->server->Start();
    usleep(1000);

    // verify all the clients got on mount callback
    for (int i = 0; i < NUM_CLIENTS; i++) {
        ASSERT_EQ_EVENTUALLY(services[i]->inited, true) << "client was not inited";
        ASSERT_EQ_EVENTUALLY(sdks[i]->ListKind("ExampleSpec").size(), NUM_CLIENTS) << "client did not have all the objects";
    }
}
} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
