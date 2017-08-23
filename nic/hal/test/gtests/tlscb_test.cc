#include <tlscb.hpp>
#include <tls_proxy_cb.pb.h>
#include <hal.hpp>
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>

using tlscb::TlsCbSpec;
using tlscb::TlsCbResponse;
using tlscb::TlsCbKeyHandle;
using tlscb::TlsCbGetRequest;
using tlscb::TlsCbGetResponse;


void
hal_initialize()
{
    char 			cfg_file[] = "hal.json";
	char 			*cfg_path;
    std::string     full_path;
    hal::hal_cfg_t  hal_cfg = { 0 };

    cfg_path = std::getenv("HAL_CONFIG_PATH");
    if (cfg_path) {
        full_path =  std::string(cfg_path) + "/" + std::string(cfg_file);
        std::cerr << "full path " << full_path << std::endl;
    } else {
        full_path = std::string(cfg_file);
    }

    // make sure cfg file exists
    if (access(full_path.c_str(), R_OK) < 0) {
        fprintf(stderr, "config file %s has no read permissions\n",
                full_path.c_str());
        exit(1);
    }

    printf("Json file: %s\n", full_path.c_str());

    if (hal::hal_parse_cfg(full_path.c_str(), &hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL config file parsing failed, quitting ...\n");
        ASSERT_TRUE(0);
    }
    printf("Parsed cfg json file \n");

    // initialize HAL
    if (hal::hal_init(&hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL initialization failed, quitting ...\n");
        exit(1);
    }
    printf("HAL Initialized \n");
}


class tlscb_test : public ::testing::Test {
protected:
  tlscb_test() {
  }

  virtual ~tlscb_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

  // Will be called at the beginning of all test cases in this class
  static void SetUpTestCase() {
    hal_initialize();
  }
  // Will be called at the end of all test cases in this class
  static void TearDownTestCase() {
  }
};

// ----------------------------------------------------------------------------
// Creating a TLS CB
// ----------------------------------------------------------------------------
TEST_F(tlscb_test, test1) 
{
    hal_ret_t            ret;
    TlsCbSpec spec;
    TlsCbResponse rsp;
    TlsCbGetRequest getReq;
    TlsCbGetResponse getRsp;


    spec.mutable_key_or_handle()->set_tlscb_id(0);

    ret = hal::tlscb_create(spec, &rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    
    spec.set_cipher_type(1);
    ret = hal::tlscb_update(spec, &rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);

    getReq.mutable_key_or_handle()->set_tlscb_id(0);
    ret = hal::tlscb_get(getReq, &getRsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    printf("cb_id: %d\n", getRsp.spec().key_or_handle().tlscb_id());
    printf("Done with test1\n");
}

// ----------------------------------------------------------------------------
// Creating muliple TlsCbs
// ----------------------------------------------------------------------------
TEST_F(tlscb_test, test2) 
{
    hal_ret_t           ret;
    TlsCbSpec           spec;
    TlsCbResponse       rsp;

    for (int i = 0; i < 10; i++) {
        spec.mutable_key_or_handle()->set_tlscb_id(i);

        ret = hal::tlscb_create(spec, &rsp);
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
