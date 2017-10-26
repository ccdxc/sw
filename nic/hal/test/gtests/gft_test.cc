#include <stdio.h>
#include <gtest/gtest.h>
#include "nic/model_sim/include/lib_model_client.h"
#include "nic/hal/pd/capri/capri_config.hpp"

class gft_test : public ::testing::Test {
  protected:
    gft_test() {}
    virtual ~gft_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
};


TEST_F(gft_test, test1)
{
    int ret = 0;
#if 0
    printf("Connecting to ASIC SIM\n");
    ret = lib_model_connect();
    ASSERT_NE(ret, -1);
    ret = capri_load_config((char *)"obj/gft/pgm_bin");
#endif
    ASSERT_NE(ret, -1);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
