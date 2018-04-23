// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <unistd.h>
#include <ev++.h>
#include "gtest/gtest.h"

#include "nic/delphi/utils/log.hpp"
#include "nic/delphi/sdk/delphi_client.hpp"
#include "nic/delphi/messanger/proto/types.pb.h"
#include "nic/delphi/sdk/delphi_utest.hpp"

namespace {
using namespace std;
using namespace delphi;

class TestReactor;
typedef std::shared_ptr<TestReactor> TestReactorPtr;

class TestObject : public BaseObject, public messanger::TestObject, public enable_shared_from_this<TestObject> {
private:
public:
    TestObject(const string &data) {
        this->ParseFromString(data);
    }
    TestObject() {
        delphi::ObjectMeta *meta = this->mutable_meta();
        meta->set_kind(this->GetDescriptor()->name());
    }
    TestObject(const TestObject& arg) {this->CopyFrom(arg);}
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
    error CreateObject(int unique_id) {
        // create an object
        TestObjectPtr tobj = make_shared<TestObject>();
        tobj->set_testdata1("Test Data");
        tobj->mutable_key()->set_idx(unique_id);

        // add it to database
        return client->QueueUpdate(tobj);
    }

    // public state
    bool inited = false;
    shared_ptr<testObjMgr>   objMgr;
};
typedef std::shared_ptr<TestService> TestServicePtr;

class DelphiClientTest : public testing::Test {
protected:
    pthread_t ev_thread_id = 0;
    ev::default_loop   loop;
    DelphiClientPtr    client;
    TestServicePtr     service;
public:
    virtual void SetUp() {
        // start the client
        client = make_shared<DelphiClient>();
        service = make_shared<TestService>(1, client);
        client->RegisterService(service);
        client->MockConnect(1);
        usleep(1000);

        // start event loop
        pthread_create(&ev_thread_id, 0, &startEventLoop, (void*)&loop);
        LogInfo("Started thread {}", ev_thread_id);
        usleep(1000);
    }
    virtual void TearDown() {
        client->Close();

        // kill the event loop thread
        LogInfo("Stopping event loop");
        loop.break_loop(ev::ALL);
        pthread_cancel(ev_thread_id);
        pthread_join(ev_thread_id, NULL);
        usleep(1000);
    }
};

TEST_F(DelphiClientTest, BasicClientTest) {
    usleep(1000);

    // verify  the service is inited
    ASSERT_EQ(service->inited, true) << "client was not inited";

    // create one object from client
    service->CreateObject(1);

    ASSERT_EQ_EVENTUALLY(client->ListKind("TestObject").size(), 1) << "client did not have all the objects";
    ASSERT_EQ_EVENTUALLY(service->objMgr->numCreateCallbacks, 1) << "reactor did not receive create callbacks";

    vector<BaseObjectPtr> db = client->ListKind("TestObject");
    for (vector<BaseObjectPtr>::iterator iter=db.begin(); iter!=db.end(); ++iter) {
        TestObjectPtr tobj = static_pointer_cast<TestObject>(*iter);
        ASSERT_EQ(tobj->testdata1(), "Test Data") << "client has invalid objects";
        ASSERT_EQ(tobj->GetMeta()->key(), tobj->key().ShortDebugString()) << "client meta is invalid";
        ASSERT_NE(tobj->GetMeta()->handle(), 0) << "client meta is invalid";

    }

    // try creating an object without key and verify it fails
    TestObjectPtr tobj2 = make_shared<TestObject>();
    error err = client->QueueUpdate(tobj2);
    ASSERT_NE(err, error::OK()) << "creating object without key suceeded";

    // delete the object
    tobj2->mutable_key()->set_idx(1);
    err = client->QueueDelete(tobj2);
    ASSERT_EQ(err, error::OK()) << "Queueing delete failed";
    ASSERT_EQ_EVENTUALLY(client->ListKind("TestObject").size(), 0) << "objects were not deleted";
}

class DelphiMountTest : public testing::Test {
protected:
    DelphiClientPtr    client_;
    TestServicePtr     service_;
public:
    virtual void SetUp() {
        // start the client
        client_ = make_shared<DelphiClient>();
        service_ = make_shared<TestService>(1, client_);
    }
    virtual void TearDown() {
    }
};

TEST_F(DelphiMountTest, MountErrorTests) {
    // try to connect without registering a service
    error err = client_->Connect();
    ASSERT_NE(err, error::OK());
    err = client_->MockConnect(1);
    ASSERT_NE(err, error::OK());

    // try creating object without connecting
    TestObjectPtr tobj = make_shared<TestObject>();
    tobj->set_testdata1("Test Data");
    tobj->mutable_key()->set_idx(1);
    err = client_->QueueUpdate(tobj);
    ASSERT_NE(err, error::OK()) << "Creating object should have failed";
    err = client_->SetObject(tobj);
    ASSERT_NE(err, error::OK()) << "Creating object should have failed";

    // try deleting object before connecting
    err = client_->DeleteObject(tobj);
    ASSERT_NE(err, error::OK()) << "Deleting object should have failed";
    err = client_->QueueDelete(tobj);
    ASSERT_NE(err, error::OK()) << "Deleting object should have failed";

    // close the client
    err = client_->Close();
    ASSERT_EQ(err, error::OK());
}

} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
