//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __FTLITE_GTEST_BASE_HPP__
#define __FTLITE_GTEST_BASE_HPP__

#include <stdio.h>
#include <time.h>
#include <string.h>

#include <gtest/gtest.h>

#include "include/sdk/base.hpp"
#include "include/sdk/timestamp.hpp"
#include "include/sdk/table.hpp"
#include "nic/utils/ftlite/ftlite.hpp"
#include "nic/utils/ftlite/p4psim/ftlite_p4psim.hpp"
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
    printf("%s : Time-taken(nsecs) = %ld\n", #_func, nsecs);\
}

class FtliteTestObject: public ::testing::Test {
protected:
    uint32_t num_insert;
    uint32_t table_count;
    ftlite::insert_params_t ips;
    timespec_t start_ts, end_ts;

protected:
    FtliteTestObject() {}
    virtual ~FtliteTestObject() {}
    
    virtual void SetUp() {
        SDK_TRACE_VERBOSE("=== SETUP : %s.%s ===",
                          ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                          ::testing::UnitTest::GetInstance()->current_test_info()->name());
        ftlite::init_params_t params = {FTLITE_TBLID_IPV4, FTLITE_TBLID_IPV6};
        ftlite_mock_init();
        ftlite::init(&params);
        ftlite::p4psim::init(&params);

        num_insert = 0;
        table_count = 0;
        clock_gettime(CLOCK_MONOTONIC, &start_ts);
    }

    virtual void TearDown() {
        clock_gettime(CLOCK_MONOTONIC, &end_ts);
        uint64_t nsecs;
        auto diff = sdk::timestamp_diff(&start_ts, &end_ts);
        sdk::timestamp_to_nsecs(&diff, &nsecs);
        printf("Time-taken(nsecs) = %ld\n", nsecs);
        ftlite_mock_cleanup();
        SDK_TRACE_VERBOSE("=== TEARDOWN : %s.%s ===",
                          ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                          ::testing::UnitTest::GetInstance()->current_test_info()->name());
    }
    
    template<class T>
    void fill_flows(T *iflow, T *rflow, uint32_t index) {
        iflow->lentry.flow_role = 0;
        rflow->lentry.flow_role = 1;
        iflow->lentry.epoch = rflow->lentry.epoch = 1;
        iflow->lentry.vpc_id = rflow->lentry.vpc_id = index;
        iflow->lentry.proto = rflow->lentry.proto = 17;
        iflow->lentry.sport = rflow->lentry.dport = index & 0xFFFF;
        iflow->lentry.dport = rflow->lentry.sport = (index>>16) & 0xFFFF;
        uint32_t sip = index + 0x22000000;
        memcpy(&iflow->lentry.src, &sip, sizeof(index));
        memcpy(&rflow->lentry.dst, &sip, sizeof(index));
        uint32_t dip = index + 0x33000000;
        memcpy(&iflow->lentry.dst, &dip, sizeof(index));
        memcpy(&rflow->lentry.src, &dip, sizeof(index));
        return;
    }

    void fill(ftlite::meta_t *meta, ftlite::info_t *iflow,
              ftlite::info_t *rflow, uint32_t index) {
        if (meta->ipv6) {
            fill_flows<ftlite::v6info_t>(&iflow->v6, &rflow->v6, index);
        } else {
            fill_flows<ftlite::v4info_t>(&iflow->v4, &rflow->v4, index);
        }
    }

    sdk_ret_t Insert(uint32_t count, sdk_ret_t expret, uint32_t ipv6 = 0) {
        for (uint32_t i = 0; i < count; i++) {
            memset(&ips, 0, sizeof(ips));
            ips.imeta.ipv6 = ips.rmeta.ipv6 = ipv6;
            fill(&ips.imeta, &ips.iflow, &ips.rflow, i);
            auto rs = ftlite::p4psim::insert(&ips);
            MHTEST_CHECK_RETURN(rs == expret, rs);
        }
        return SDK_RET_OK;
    }

    sdk_ret_t InsertCollision(uint32_t count, sdk_ret_t expret,
                              uint32_t ipv6 = 0, uint8_t maxlevels = 8) {
        uint32_t nextdepth = maxlevels;
        uint32_t chainid = 0;
        for (uint32_t i = 0; i < count; i++) {
            memset(&ips, 0, sizeof(ips));
            ips.imeta.ipv6 = ips.rmeta.ipv6 = ipv6;
            fill(&ips.imeta, &ips.iflow, &ips.rflow, i);
            
            if (i == nextdepth) {
                chainid++;
                nextdepth += (maxlevels - 1);
                maxlevels--;
            }
            SDK_TRACE_DEBUG("session:%d chainid:%d nextdepth:%d",
                            i, chainid, nextdepth);
            ips.cmeta.action = 0xFF;
            ips.imeta.hash.msb = chainid;
            ips.imeta.hash.index = 100;
            ips.rmeta.hash.msb = chainid;
            ips.rmeta.hash.index = 200;

            auto rs = ftlite::p4psim::insert(&ips);
            MHTEST_CHECK_RETURN(rs == expret, rs);
        }
        return SDK_RET_OK;
    }

    sdk_ret_t InsertV4(uint32_t count, sdk_ret_t expret) {
        return Insert(count, expret, 0);
    }
    sdk_ret_t InsertCollisionV4(uint32_t count, sdk_ret_t expret) {
        return InsertCollision(count, expret, 0);
    }
    sdk_ret_t InsertV6(uint32_t count, sdk_ret_t expret) {
        return Insert(count, expret, 1);
    }
    sdk_ret_t InsertCollisionV6(uint32_t count, sdk_ret_t expret) {
        return InsertCollision(count, expret, 1);
    }

};

#endif
