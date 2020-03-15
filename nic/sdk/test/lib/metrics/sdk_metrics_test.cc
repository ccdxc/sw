#include <gtest/gtest.h>
#include <inttypes.h>
#include <memory>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lib/pal/pal.hpp"
#include "lib/metrics/metrics.hpp"

static sdk::lib::pal_ret_t
reg_read (uint64_t addr, uint32_t *dst, uint32_t count)
{
    memcpy((uint64_t *)addr, dst, sizeof(uint32_t) * count);

    return sdk::lib::PAL_RET_OK;
}

class sdk_metrics_test : public ::testing::Test {
};

static
sdk::metrics::schema_t schema = {
    "utTable",
    sdk::metrics::SW,
    {
        "counter1",
        "counter2",
        NULL,
    }
};

static
sdk::metrics::schema_t schema_hbm = {
    "utTableHBM",
    sdk::metrics::HBM,
    {
        "counter1",
        "_reserved",
        "counter2",
        NULL,
    }
};

TEST_F (sdk_metrics_test, basic_test) {
    void *handler;

    // Write
    handler = sdk::metrics::create(&schema);
    ASSERT_TRUE(handler != NULL);

    sdk::metrics::metrics_update(handler, 1, 0, 12);
    sdk::metrics::metrics_update(handler, 1, 1, 13);

    // Read
    handler = sdk::metrics::metrics_open("utTable");
    sdk::metrics::metrics_counters_t values;
    
    ASSERT_TRUE(handler != NULL);
    values = sdk::metrics::metrics_read(handler, 1);

    printf("%lu\n", values.size());
    ASSERT_TRUE(values.size() == 2);
    ASSERT_TRUE(values[0].first == "counter1");
    ASSERT_TRUE(values[0].second = 12);
    ASSERT_TRUE(values[1].first == "counter2");
    ASSERT_TRUE(values[1].second = 13);
    
};


TEST_F (sdk_metrics_test, pointer_test) {
    sdk::lib::pal_rwvectors_t vecs = {0};

    vecs.reg_read = reg_read;

    sdk::lib::gl_pal_info.rwvecs = vecs;

    void *handler;
    uint64_t counters[] = {
        12,
        99,
        500,
    };

    // Write
    handler = sdk::metrics::create(&schema_hbm);
    ASSERT_TRUE(handler != NULL);

    sdk::metrics::row_address(handler, 1, counters);

    // Read
    handler = sdk::metrics::metrics_open("utTableHBM");
    ASSERT_TRUE(handler != NULL);

    sdk::metrics::metrics_counters_t values;
    values = sdk::metrics::metrics_read(handler, 1);

    printf("%lu\n", values.size());
    ASSERT_TRUE(values.size() == 2);
    ASSERT_TRUE(values[0].first == "counter1");
    ASSERT_TRUE(values[0].second = 12);
    ASSERT_TRUE(values[1].first == "counter2");
    ASSERT_TRUE(values[1].second = 500);
}

int
main (int argc, char **argv)
{
    unlink("/dev/shm/metrics_shm");

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
