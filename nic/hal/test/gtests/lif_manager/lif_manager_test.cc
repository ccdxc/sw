#include "platform/utils/lif_mgr/lif_mgr.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include "lib/indexer/indexer.hpp"
#include "nic/utils/trace/trace.hpp"

using sdk::lib::indexer;
using sdk::platform::utils::lif_mgr;
const static char *kLif2QstateHBMLabel = "nicmgrqstate_map";

class pciemgr *pciemgr;

//------------------------------------------------------------------------------
// lif_mgr Test Class
//------------------------------------------------------------------------------
class lif_mgr_test : public ::testing::Test {
protected:
  lif_mgr_test() {
  }

  virtual ~lif_mgr_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};

//----------------------------------------------------------------------------
// Test 1:
//
// Summary:
// --------
//  - Tests the basic case of allocation and free of an index
//----------------------------------------------------------------------------
TEST_F(lif_mgr_test, test1) {
    sdk_ret_t ret = SDK_RET_OK;
    uint32_t idx = 0;

    lif_mgr *lm = lif_mgr::factory(2048, NULL, kLif2QstateHBMLabel);

    ret = lm->alloc_id(&idx, 1);
    ASSERT_TRUE(ret == SDK_RET_OK);

    ret = lm->alloc_id(&idx, 1);
    ASSERT_TRUE(ret == SDK_RET_OK);

    ret = lm->alloc_id(&idx, 10);
    ASSERT_TRUE(ret == SDK_RET_OK);

    ret = lm->reserve_id(1, 1);
    ASSERT_TRUE(ret == sdk::SDK_RET_ENTRY_EXISTS);

    ret = lm->alloc_id(&idx, 1);
    ASSERT_TRUE(ret == SDK_RET_OK);

}

static int
hal_sdk_logger (uint32_t mod_id, sdk_trace_level_e trace_level,
                const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    // HAL_TRACE_ERR_NO_META("{}", logbuf);
    printf("%s\n", logbuf);
    va_end(args);

    return 0;
}


//-----------------------------------------------------------------------------
// SDK initiaization
//-----------------------------------------------------------------------------
void
hal_sdk_init (void)
{
    sdk::lib::logger::init(hal_sdk_logger);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

#if 0
     std::string logfile;
     logfile = std::string("./lif_mgr.log");
     hal::utils::trace_init("lif_mgr", 0x3, true,
                            logfile.c_str(),
                            TRACE_FILE_SIZE_DEFAULT,
                            TRACE_NUM_FILES_DEFAULT,
                            ::utils::trace_debug);

     hal_sdk_init();
#endif

     // HAL_TRACE_DEBUG("Starting Main ... ");

  return RUN_ALL_TESTS();
}
