//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains testcases to test the in_mem_fsm_logger utility.
///
//----------------------------------------------------------------------------
#include <stdio.h>
#include <time.h>
#include "gtest/gtest.h"
#include "lib/utils/in_mem_fsm_logger.hpp"
#include "linkmgr/linkmgr.hpp"

using sdk::utils::in_mem_fsm_logger;
using sdk::utils::record_t;

typedef struct dump_test_state_record_args_s {
    uint32_t   index;
    timespec_t prev_ts;
} __PACK__ dump_test_state_record_args_t;

enum class test_state_t {
    BEGIN          = 0,
    PREPRPCESSING  = 1,
    CONFIGURATION  = 2,
    POSTPROCESSING = 3,
    FINISH         = 4,
};

static inline std::string
test_state_to_str (test_state_t t)
{
    switch (t) {
    case test_state_t::BEGIN:
        return "Begin";
    case test_state_t::PREPRPCESSING:
        return "Preprocessing";
    case test_state_t::CONFIGURATION:
        return "Configuration";
    case test_state_t::POSTPROCESSING:
        return "Postprocessing";
    case test_state_t::FINISH:
        return "Finish";
    default:
        return "Unknown";
    }
}

//------------------------------------------------------------------------------
// in_mem_fsm_logger test Class
//------------------------------------------------------------------------------
class in_mem_fsm_logger_test : public ::testing::Test {
protected:

    void append_log(in_mem_fsm_logger *logger, test_state_t s) {
        int tmp = static_cast<int>(s);

        logger->append((char *)&tmp);
    }

    virtual ~in_mem_fsm_logger_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
  }
};

static inline bool
dump_test_state_log (record_t *record, void *ctxt) {
    uint32_t written = 0;
    char ts_buff[TIME_STR_SIZE];
    char td_buff[TIME_STR_SIZE];
    test_state_t s = *(test_state_t *)record->data;
    dump_test_state_record_args_t *args = (dump_test_state_record_args_t *)ctxt;

    written = strftime(ts_buff, TIME_STR_SIZE, "%Y-%m-%d %H:%M:%S",
                       localtime(&record->real_ts.tv_sec));
    snprintf(ts_buff+written, TIME_STR_SIZE-written, ".%.lu",
             record->real_ts.tv_nsec/TIME_NSECS_PER_MSEC);
    sdk::timestamp_diff_to_str(&record->mon_ts, &args->prev_ts, td_buff,
                               TIME_STR_SIZE);
    printf(" %-30s  %-20s  %-s \n", ts_buff, test_state_to_str(s).c_str(),
           (args->index == 0) ? "-" : td_buff);

    SDK_ASSERT(s == static_cast<test_state_t>(args->index));
    args->index++;
    args->prev_ts = record->mon_ts;
    return false;
};

TEST_F(in_mem_fsm_logger_test, test_append) {
    uint32_t capacity = 5;
    dump_test_state_record_args_t args = { 0 };
    in_mem_fsm_logger *logger =
        in_mem_fsm_logger::factory(capacity, sizeof(test_state_t));

    append_log(logger, test_state_t::PREPRPCESSING);
    append_log(logger, test_state_t::CONFIGURATION);
    append_log(logger, test_state_t::POSTPROCESSING);
    logger->reset();
    append_log(logger, test_state_t::BEGIN);
    append_log(logger, test_state_t::PREPRPCESSING);
    append_log(logger, test_state_t::CONFIGURATION);
    append_log(logger, test_state_t::POSTPROCESSING);
    append_log(logger, test_state_t::FINISH);

    ASSERT_EQ(logger->capacity(), capacity);
    ASSERT_EQ(logger->num_records(), capacity);

    printf(" %-30s  %-20s  %-s\n", "Timestamp", "test_state_t", "Duration");
    printf("%s\n", std::string(88, '-').c_str());

    logger->walk(dump_test_state_log, (void *)&args);
    in_mem_fsm_logger::destroy(logger);
}

int
main (int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
