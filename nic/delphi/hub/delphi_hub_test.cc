// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <unistd.h>
#include <ev++.h>
#include "gtest/gtest.h"

#include "nic/delphi/utils/log.hpp"
#include "nic/delphi/hub/delphi_server.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"

namespace {
using namespace std;
using namespace delphi;

//---------------------------- To be generated code ------------------
class TestReactor;
typedef std::shared_ptr<TestReactor> TestReactorPtr;

class TestObject : public BaseObject, public messanger::TestObject, public enable_shared_from_this<TestObject> {
private:
    TestReactorPtr reactor;
public:
    TestObject(const string &data) {
        this->ParseFromString(data);
    }
    TestObject() {}
    TestObject(const TestObject& arg) {
        this->CopyFrom(arg);
        this->reactor = arg.reactor;
    }
    virtual BaseObjectPtr Clone() { return make_shared<TestObject>(*this); }

    virtual delphi::ObjectMeta *GetMeta() {
        return this->mutable_meta();
    };
    virtual string GetKey() {
        return this->key().ShortDebugString();
    };
    virtual ::google::protobuf::Message *GetMessage() {
        return this;
    }
    static error Mount(DelphiClientPtr client, MountMode mode) {
        return client->MountKind("TestObject", mode);
    }
    static error Watch(DelphiClientPtr client, TestReactorPtr reactor);
    virtual error TriggerEvent(BaseObjectPtr oldObj, ObjectOperation op, ReactorListPtr rl);
};
typedef std::shared_ptr<TestObject> TestObjectPtr;

REGISTER_KIND(TestObject);

class TestReactor : public BaseReactor {
public:
    virtual error OnTestObjectCreate(TestObjectPtr obj) {
        return error::OK();
    }
    virtual error OnTestObjectDelete(TestObjectPtr obj) {
        return error::OK();
    }
    virtual error OnTestData1(TestObjectPtr obj) {
        return error::OK();
    }
};

error TestObject::Watch(DelphiClientPtr client, TestReactorPtr reactor) {
    return client->WatchKind("TestObject", reactor);
}

error TestObject::TriggerEvent(BaseObjectPtr oldObj, ObjectOperation op, ReactorListPtr rl) {
    TestObjectPtr exObj = static_pointer_cast<TestObject>(oldObj);
    for (vector<BaseReactorPtr>::iterator iter=rl->reactors.begin(); iter!=rl->reactors.end(); ++iter) {
        TestReactorPtr rctr = static_pointer_cast<TestReactor>(*iter);

        if (op == delphi::SetOp) {
            if (oldObj == NULL) {
                RETURN_IF_FAILED(rctr->OnTestObjectCreate(shared_from_this()));
            } else if (this->testdata1() != exObj->testdata1()) {
                RETURN_IF_FAILED(rctr->OnTestData1(shared_from_this()));
            }
        } else if (op == delphi::DeleteOp) {
            RETURN_IF_FAILED(rctr->OnTestObjectDelete(shared_from_this()));
        }
    }

    return error::OK();
}

//--------------------------- end of to be generated code --------------------
// event loop thread
void * startEventLoop(void* arg) {
    ev::default_loop *loop = (ev::default_loop *)arg;
    loop->run(0);

    return NULL;
}

class testObjMgr : public TestReactor {
public:
    int  numCreateCallbacks;
    testObjMgr() {
        numCreateCallbacks = 0;
    }
    virtual error OnTestObjectCreate(TestObjectPtr obj) {
        numCreateCallbacks++;
        return error::OK();
    }
};

class TestService : public Service {
private:
    int                      service_id = 0;
    DelphiClientPtr          client;
    ev::timer     createTimer;
    int           batchSize;
    int           numBatches;
    int           currBatch = 0;
public:
    TestService(int clid, DelphiClientPtr cl) {
        this->service_id = clid;
        this->client = cl;

        // mount and watch test objects
        objMgr = make_shared<testObjMgr>();
        TestObject::Mount(cl, ReadWriteMode);
        TestObject::Watch(cl, objMgr);

    }
    virtual string Name() {
        return "TestService-" + to_string(this->service_id);
    }
    void OnMountComplete() {
        LogInfo("TestHandler {} OnMountComplete got called\n", service_id);
        inited = true;
    }
    void SendObject(int unique_id) {
        // create an object
        TestObjectPtr tobj = make_shared<TestObject>("");
        delphi::ObjectMeta *meta = tobj->mutable_meta();
        meta->set_kind(tobj->GetDescriptor()->name());
        tobj->set_testdata1("Test Data");
        tobj->mutable_key()->set_idx(unique_id);

        // add it to database
        client->SetObject(tobj);
    }
    void createTimerHandler(ev::timer &watcher, int revents) {
        if (this->currBatch < this->numBatches) {
            this->currBatch++;
            for (int i = 0; i < this->batchSize; i++) {
                int uid = (this->currBatch * this->batchSize) + i + 1;
                this->SendObject(uid);
            }
            LogInfo("Batch {}, created {} objects", this->currBatch, this->batchSize);
        } else {
            LogInfo("service {} Stopping object create handler", service_id);
            this->createTimer.stop();
        }
    }
    void StartBatchSend(int bsize, int nbatch) {
        this->createTimer.set<TestService, &TestService::createTimerHandler>(this);
        this->createTimer.start(SYNC_PERIOD, SYNC_PERIOD);
        this->batchSize = bsize;
        this->numBatches = nbatch;
        this->currBatch = 0;
    }
    void stop() {
        LogInfo("service {} Stopping", service_id);
        this->createTimer.stop();
    }
    // public state
    bool inited = false;
    shared_ptr<testObjMgr>   objMgr;
};
typedef std::shared_ptr<TestService> TestServicePtr;

#define NUM_CLIENTS 5

class DelphiHubTest : public testing::Test {
protected:
    pthread_t ev_thread_id = 0;
    ev::default_loop   loop;
    DelphiServerPtr    server;
    DelphiClientPtr    clients[NUM_CLIENTS];
    TestServicePtr     services[NUM_CLIENTS];
public:
    virtual void SetUp() {

        // instantiate the delphi server
        server = make_shared<DelphiServer>();
        server->Start();
        usleep(1000);

        // start the clients
        for (int i = 0; i < NUM_CLIENTS; i++) {
            clients[i] = make_shared<DelphiClient>();
            services[i] = make_shared<TestService>(i, clients[i]);
            clients[i]->RegisterService(services[i]);
            clients[i]->Connect();
        }
        usleep(1000);

        // start event loop
        pthread_create(&ev_thread_id, 0, &startEventLoop, (void*)&loop);
        LogInfo("Started thread {}", ev_thread_id);
        usleep(1000);
    }
    virtual void TearDown() {
        for (int i = 0; i < NUM_CLIENTS; i++) {
            clients[i]->Close();
            services[i]->stop();
        }
        server->Stop();

        // kill the event loop thread
        LogInfo("Stopping event loop");
        loop.break_loop(ev::ALL);
        pthread_cancel(ev_thread_id);
        pthread_join(ev_thread_id, NULL);
        usleep(1000);
    }
};

TEST_F(DelphiHubTest, BasicServerTest) {
    usleep(1000);

    // verify all the clients are inited
    for (int i = 0; i < NUM_CLIENTS; i++) {
        ASSERT_EQ(services[i]->inited, true) << "client was not inited";
    }

    // send one object from each client
    for (int i = 0; i < NUM_CLIENTS; i++) {
        services[i]->SendObject(i+1);
    }
    usleep(1000 * 100);

    for (int i = 0; i < NUM_CLIENTS; i++) {
        LogInfo("Client {} has {} objects", i, clients[i]->ListKind("TestObject").size());
        ASSERT_EQ(clients[i]->ListKind("TestObject").size(), NUM_CLIENTS) << "client did not have all the objects";
        ASSERT_EQ(services[i]->objMgr->numCreateCallbacks, NUM_CLIENTS) << "reactor did not receive create callbacks";

        vector<BaseObjectPtr> db = clients[i]->ListKind("TestObject");
        for (vector<BaseObjectPtr>::iterator iter=db.begin(); iter!=db.end(); ++iter) {
            TestObjectPtr tobj = static_pointer_cast<TestObject>(*iter);
            ASSERT_EQ(tobj->testdata1(), "Test Data") << "client has invalid objects";
        }
    }

    // delete the object and make sure it goes away
    for (int i = 0; i < NUM_CLIENTS; i++) {
        vector<BaseObjectPtr> objs = clients[i]->ListKind("TestObject");
        for (vector<BaseObjectPtr>::iterator iter=objs.begin(); iter!=objs.end(); ++iter) {
            ASSERT_EQ(clients[i]->DeleteObject(*iter), error::OK()) << "Error deleting objects";
        }
    }
    usleep(1000 * 100);

    for (int i = 0; i < NUM_CLIENTS; i++) {
        LogInfo("Client {} has {} objects after deletion", i, clients[i]->ListKind("TestObject").size());
        ASSERT_EQ(clients[i]->ListKind("TestObject").size(), 0) << "client did not have all the objects";
    }
}

TEST_F(DelphiHubTest, ServerBenchmaskTest) {
    usleep(1000 * 100);

    int bsize = 1000;
    int nbatch = 20;

    // verify all the clients are inited
    for (int i = 0; i < NUM_CLIENTS; i++) {
        ASSERT_EQ(services[i]->inited, true) << "client was not inited";
    }

    // send batched object create from first client
    services[0]->StartBatchSend(bsize, nbatch);

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    LogInfo("Benchmark test started at {}", asctime(&tm));

    usleep(1000 * 100);

    // wait till all the clients have objects
    bool notFound = false;
    for (int iter = 0; iter < 1000; iter++) {
        notFound = false;

        if (server->GetSubtree("TestObject")->objects.size() < uint(bsize * nbatch)) {
            notFound = true;
            LogInfo("Server has {} objects", server->GetSubtree("TestObject")->objects.size());
        }

        for (int i = 0; i < NUM_CLIENTS; i++) {
            if (services[i]->objMgr->numCreateCallbacks < (bsize * nbatch)){
                notFound = true;
                LogInfo("Service {} has {} Create callbacks", i, services[i]->objMgr->numCreateCallbacks);
                break;
            }
        }

        if (!notFound) {
            break;
        }
        usleep(1000 * 100);
    }

    t = std::time(nullptr);
    tm = *std::localtime(&t);
    LogInfo("Benchmark test ended at {}. notFound: {}", asctime(&tm), notFound);

    ASSERT_EQ(notFound, false) << "Not all objects were found in clients";
}

} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
