#include <gtest/gtest.h>
#include <inttypes.h>
#include <memory>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lib/metrics/metrics.hpp"

class sdk_metrics_test : public ::testing::Test {
};

static
sdk::metrics::metrics_schema_t schema = {
    "utTable",
    {
        {
            "counter1",
            sdk::metrics::METRICS_COUNTER_VALUE64
        },
        {
            "counter2",
            sdk::metrics::METRICS_COUNTER_VALUE64
        },
        {
            NULL
        },
    }
};


TEST_F (sdk_metrics_test, basic_test) {
    void *handler;

    // Write
    handler = sdk::metrics::metrics_register(&schema);
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

int
main (int argc, char **argv)
{
    unlink("/dev/shm/metrics_shm");

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
