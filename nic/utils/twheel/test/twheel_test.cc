#include "nic/utils/twheel/twheel.hpp"
#include <gtest/gtest.h>

using hal::utils::twheel;

class twheel_test : public ::testing::Test {
protected:
  twheel_test() {
  }

  virtual ~twheel_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};

uint32_t g_ctxt = 100;

void
timeout_handler(uint32_t timer_id, void *ctxt)
{
    uint32_t    *i = (uint32_t *)ctxt;
    ASSERT_EQ(*i, 100);
}

TEST_F(twheel_test, fire_periodic_timer) {
    twheel   *test_twheel;
    void     *timer;

    test_twheel = twheel::factory(10, 100, true);
    ASSERT_TRUE(test_twheel != NULL);

    timer = test_twheel->add_timer(1, 500, &g_ctxt, timeout_handler, true);
    ASSERT_TRUE(timer != NULL);
    sleep(1);
    test_twheel->tick(1000);
    ASSERT_EQ(test_twheel->num_entries(), 1);
    sleep(1);
    test_twheel->tick(1000);
    ASSERT_EQ(test_twheel->num_entries(), 1);

    twheel::destroy(test_twheel);
}

TEST_F(twheel_test, fire_timer) {
    twheel   *test_twheel;
    void     *timer;

    test_twheel = twheel::factory(10, 100, true);
    ASSERT_TRUE(test_twheel != NULL);

    timer = test_twheel->add_timer(1, 500, &g_ctxt, timeout_handler, false);
    ASSERT_TRUE(timer != NULL);
    sleep(1);
    test_twheel->tick(1000);
    ASSERT_EQ(test_twheel->num_entries(), 0);

    twheel::destroy(test_twheel);
}

// test creation and deletion of timerwheel
TEST_F(twheel_test, create_delete) {
    twheel   *test_twheel;

    test_twheel = twheel::factory(10, 100, true);
    ASSERT_TRUE(test_twheel != NULL);

    twheel::destroy(test_twheel);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
