// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <unistd.h>
#include <ev++.h>
#include "gtest/gtest.h"
#include "shm.hpp"

namespace {
using namespace std;
using namespace sdk;
using namespace sdk::metrics;

const int32_t kshmTestMemSize = (1024 * 1024);
const string kshmName = "test_shm";

class ShmTest : public testing::Test {
protected:
    ShmPtr srv_shm_;
    ShmPtr client_shm_;
public:
    virtual void SetUp() {
        // create the server
        srv_shm_ = make_shared<Shm>();
        error err = srv_shm_->MemMap(kshmName, kshmTestMemSize);
        assert(err.IsOK());

        // create the client
        client_shm_ = make_shared<Shm>();
        err = client_shm_->MemMap(kshmName, kshmTestMemSize);
        assert(err.IsOK());
    }
    virtual void TearDown() {
        error err = srv_shm_->MemUnmap();
        assert(err.IsOK());
    }
};

TEST_F(ShmTest, BasicShmTest) {
    // allocate some memory
    void *sptr = srv_shm_->Alloc(1);
    ASSERT_TRUE(sptr != NULL) << "shared memory alloc failed on server";
    void *cptr = client_shm_->Alloc(1);
    ASSERT_TRUE(cptr != NULL) << "shared memory alloc failed on client";

    // free the memory
    error err = srv_shm_->Free(sptr);
    ASSERT_TRUE(err.IsOK()) << "shared memory free failed on server";
    err = client_shm_->Free(cptr);
    ASSERT_TRUE(err.IsOK()) << "shared memory free failed on client";

    // try to free random pointers and verify it fails
    err = client_shm_->Free(0);
    err = client_shm_->Free(((uint8_t *)cptr) + kshmTestMemSize);

    // dump shm info
    client_shm_->DumpMeta();

    // try to create a new client to uninitialized shared memory and verify it fails
    ShmPtr client_inv = make_shared<Shm>();
    err = client_inv->MemMap("invalid", kshmTestMemSize);
    ASSERT_NE(err, error::OK()) << "creating client to uninitialized shared memory suceeded";
}

} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
