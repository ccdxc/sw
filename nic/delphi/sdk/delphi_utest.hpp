// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_SDK_UTEST_H_
#define _DELPHI_SDK_UTEST_H_

#include <unistd.h>
#include "gtest/gtest.h"
#include "nic/delphi/utils/utest.hpp"

// DELPHI_SERVICE_TEST instantiates a gtest fixture for testing a service
#define DELPHI_SERVICE_TEST(test_name, svc_name) \
class test_name : public testing::Test { \
protected: \
    pthread_t ev_thread_id = 0; \
    delphi::SdkPtr sdk_; \
    delphi::shm::DelphiShmPtr srv_shm_; \
    shared_ptr<svc_name> exsvc_; \
public: \
    virtual void SetUp() { \
        sdk_ = make_shared<delphi::Sdk>(); \
        srv_shm_ = make_shared<delphi::shm::DelphiShm>(); \
        delphi::error err = srv_shm_->MemMap(DELPHI_SHM_NAME, DELPHI_SHM_SIZE, true); \
        assert(err.IsOK()); \
        exsvc_ = make_shared<svc_name>(sdk_); \
        assert(exsvc_ != NULL); \
        sdk_->RegisterService(exsvc_); \
        pthread_create(&ev_thread_id, 0, &StartTestLoop, (void*)&sdk_); \
        usleep(10 * 1000); \
    } \
    virtual void TearDown() { \
        sdk_->Stop(); \
        srv_shm_->MemUnmap(); \
        usleep(1000); \
        pthread_cancel(ev_thread_id); \
        pthread_join(ev_thread_id, NULL); \
        usleep(1000); \
    } \
};

// StartTestLoop starts the test loop in a pthread
void * StartTestLoop(void* arg);

#endif // _DELPHI_SDK_UTEST_H_
