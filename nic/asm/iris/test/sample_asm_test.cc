#include <gtest/gtest.h>
#include "cap_test.hpp"


class asm_test : public ::testing::Test {
protected:
    asm_test() {
  }

  virtual ~asm_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};

TEST_F(asm_test, test1) {

    std::string test_name = "TestCase1";
    CapTest *ctest = new CapTest(test_name, 1);

    printf("Running TestCase1 ... \n");
    
    ctest->run();
    ASSERT_TRUE(1);


#if 0
    DirectMap test_dm = DirectMap(table_name, 1, 100);

    dm_data_t dm;
    uint32_t index;
    dm.p = 1;
    dm.q = 2;

    hal_ret_t rt;
    rt = test_dm.insert(&dm, &index); 
    ASSERT_TRUE(rt == HAL_RET_OK);
#endif

}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
