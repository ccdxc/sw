//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __FTLITE_GTEST_BASE_HPP__
#define __FTLITE_GTEST_BASE_HPP__

#include <stdio.h>
#include <time.h>

#include "include/sdk/base.hpp"
#include "include/sdk/timestamp.hpp"
#include "include/sdk/table.hpp"
#include "nic/utils/ftlite/ftlite.hpp"
#include "nic/utils/ftlite/test/p4pd_mock/ftlite_p4pd_mock.hpp"

#define MHTEST_CHECK_RETURN(_exp, _ret) if (!(_exp)) return (_ret)

#define RUN_TEST(_func) \
{ \
    timespec_t ts1, ts2;\
    uint64_t nsecs;\
    clock_gettime(CLOCK_MONOTONIC, &ts1);\
    (_func)(&testobj);\
    clock_gettime(CLOCK_MONOTONIC, &ts2);\
    auto diff = sdk::timestamp_diff(&ts2, &ts1);\
    sdk::timestamp_to_nsecs(&diff, &nsecs);\
    printf("%s : %ld\n", #_func, nsecs);\
}

class FtliteTestObject {
protected:
    uint32_t num_insert;
    uint32_t table_count;
    ftlite::key_params_t key;
    ftlite::data_params_t data;

public:
    FtliteTestObject() {}
    virtual ~FtliteTestObject() {}
    
    virtual void SetUp(const char *tsname, const char *tcname ) {
        SDK_TRACE_VERBOSE("=== SETUP : %s.%s ===", tsname, tcname);
        ftlite::init_params_t params = {0};
        params.ipv4_main_table_id = FTLITE_TBLID_IPV4;
        params.ipv4_num_hints = 2;
        params.ipv6_main_table_id = FTLITE_TBLID_IPV6;
        params.ipv6_num_hints = 4;

        ftlite_mock_init();
        ftlite::init(&params);

        num_insert = 0;
        table_count = 0;
        memset(&key, 0, sizeof(key));
        memset(&data, 0, sizeof(data));
    }

    virtual void TearDown(const char *tsname, const char *tcname) {
        ftlite_mock_cleanup();
        SDK_TRACE_VERBOSE("=== TEARDOWN : %s.%s ===", tsname, tcname);
    }

    sdk_ret_t Insert(uint32_t count, sdk_ret_t expret,
                     uint32_t ipaf, bool collision) {
        memset(&data, 0, sizeof(data));

        for (uint32_t i = 0; i < count; i++) {
            memset(&key, 0, sizeof(key));
            key.ipaf = ipaf;
            if (collision) {
                key.parent_index = i;
                key.parent_is_hint = 0;
                key.parent_hint_slot = 1;
                if (ipaf == 1) {
                    key.parent.v4entry.entry_valid = 1;
                } else {
                    key.parent.v6entry.entry_valid = 1;
                }
            }
            if (ipaf == 1) {
                key.leaf.v4entry.entry_valid = 0;
                key.leaf.v4entry.src = i;
            } else {
                key.leaf.v6entry.entry_valid = 0;
                key.leaf.v6entry.src[15] = i & 0xFF;
            }
            key.flow_hash = i;
            auto rs = ftlite::insert(&key, &data);
            MHTEST_CHECK_RETURN(rs == expret, rs);
        }
        return SDK_RET_OK;
    }
};

#endif
