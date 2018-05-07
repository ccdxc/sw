#include "nic/hal/src/nat/nat.hpp"
#include "nic/hal/hal.hpp"
#include "sdk/list.hpp"
#include "nic/gen/proto/hal/vrf.pb.h"
#include "nic/gen/proto/hal/nat.pb.h"
#include "nic/hal/hal.hpp"
#include "nic/gen/proto/hal/types.pb.h"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "nic/fte/fte_ctx.hpp"
#include "nic/fte/acl/acl_ctx.hpp"
#include "nic/fte/acl/list.hpp"

using vrf::VrfSpec;
using vrf::VrfResponse;
using nat::NatPolicySpec;
using types::IPProtocol;
using namespace hal;
using namespace fte;
using namespace nwsec;

class nat_policy_test : public hal_base_test {
protected:
    nat_policy_test() { }
    virtual ~nat_policy_test() { }

    // will be called immediately after the constructor before each test
    virtual void SetUp() { }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
#if 0
        //Route acl is holding one - so increasing it to 1 till we fix that
        EXPECT_EQ(acl_ctx_t::num_ctx_in_use(), 1);
        EXPECT_EQ(list_t::num_lists_in_use(), 1);
        EXPECT_EQ(list_t::num_items_in_use(), 0);
        EXPECT_EQ(g_hal_state->nwsec_policy_ht()->num_entries(), 0);
        EXPECT_EQ(g_hal_state->nwsec_rule_slab()->num_in_use(), 0);
        EXPECT_EQ(g_hal_state->ipv4_rule_slab()->num_in_use(), 0);
#endif
    }

    // Will be called at the beginning of all test cases in this class
    static void SetUpTestCase() {
        hal_base_test::SetUpTestCase();
        hal_test_utils_slab_disable_delete();
    }
};


static inline void
nat_cfg_pol_rule_spec_build (nat::NatPolicySpec& spec)
{
}

static inline void
nat_cfg_pol_data_spec_build (nat::NatPolicySpec& spec)
{
    nat_cfg_pol_rule_spec_build(spec);
}

static inline void
nat_cfg_pol_key_spec_build (nat::NatPolicySpec& spec)
{
    spec.mutable_key_or_handle()->mutable_policy_key()->
        set_nat_policy_id(10);
    spec.mutable_key_or_handle()->mutable_policy_key()->
        mutable_vrf_key_or_handle()->set_vrf_id(0);
}

static inline void
nat_cfg_pol_spec_build (nat::NatPolicySpec& spec)
{
    nat_cfg_pol_key_spec_build(spec);
    nat_cfg_pol_data_spec_build(spec);
}

TEST_F(nat_policy_test, create_pol_without_key_or_handle)
{
    hal_ret_t ret;
    NatPolicySpec pol_spec;
    NatPolicyResponse pol_rsp;

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::nat_policy_create(pol_spec, &pol_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);
}

TEST_F(nat_policy_test, create_pol_without_vrf)
{
    hal_ret_t ret;
    NatPolicySpec pol_spec;
    NatPolicyResponse pol_rsp;

    nat_cfg_pol_spec_build(pol_spec);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::nat_policy_create(pol_spec, &pol_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_VRF_NOT_FOUND);
}

int main (int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
