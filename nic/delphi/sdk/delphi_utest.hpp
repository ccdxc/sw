// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_SDK_UTEST_H_
#define _DELPHI_SDK_UTEST_H_

#include <unistd.h>
#include "gtest/gtest.h"

// DELPHI_SERVICE_TEST instantiates a gtest fixture for testing a service
#define DELPHI_SERVICE_TEST(test_name, svc_name) \
class test_name : public testing::Test { \
protected: \
    pthread_t ev_thread_id = 0; \
    delphi::SdkPtr sdk_; \
    shared_ptr<svc_name> exsvc_; \
public: \
    virtual void SetUp() { \
        sdk_ = make_shared<delphi::Sdk>(); \
        exsvc_ = make_shared<svc_name>(sdk_); \
        assert(exsvc_ != NULL); \
        sdk_->RegisterService(exsvc_); \
        pthread_create(&ev_thread_id, 0, &StartTestLoop, (void*)&sdk_); \
        usleep(1000); \
    } \
    virtual void TearDown() { \
        sdk_->Stop(); \
        pthread_cancel(ev_thread_id); \
        pthread_join(ev_thread_id, NULL); \
        usleep(1000); \
    } \
};

// -------------- wrappers for asynchronous assertions -------------------

// ASSERT_EQ_EVENTUALLY asserts (left == right) eventually
// it retries the check every 10ms for up to 1sec
#define ASSERT_EQ_EVENTUALLY(left, right) \
for (int _tidx = 0; _tidx < 100; _tidx++) { \
    if ((left) == (right)) { \
        break; \
    } \
    usleep(1000 * 10); \
} \
ASSERT_EQ(left, right)

// ASSERT_NE_EVENTUALLY asserts (left != right) eventually
// it retries the check every 10ms for up to 1sec
#define ASSERT_NE_EVENTUALLY(left, right) \
for (int _tidx = 0; _tidx < 100; _tidx++) { \
    if ((left) != (right)) { \
        break; \
    } \
    usleep(1000 * 10); \
} \
ASSERT_NE(left, right)


// StartTestLoop starts the test loop in a pthread
void * StartTestLoop(void* arg);

#endif // _DELPHI_SDK_UTEST_H_
