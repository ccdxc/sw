//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "ftltest_base.hpp"
#include <thread>
#include <chrono>

using namespace std;

class multi_thread: public FtlGtestBase {

};

TEST_F(multi_thread, insert_iterate_remove) {
    sdk_ret_t rs;

    // Insert 16k entries
    rs = Insert(16*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    // Iterate entries in the table
    thread insert_th(&multi_thread::Iterate, this);

    // Sleep for 1 microsec, just to make sure iteratation is
    // started before we start kicking entries out.
    std::this_thread::sleep_for(std::chrono::microseconds(1));

    // Start removing entries
    thread remove_th(&multi_thread::Remove, this, 16*1024,
                     sdk::SDK_RET_OK, WITHOUT_HASH, 0);

    insert_th.join();
    remove_th.join();
}
