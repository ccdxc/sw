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

#define NAT_CFG_MAX_ENTRIES 10

typedef struct nat_cfg_pol_test_s {
    int               pol_id;
    int               vrf_id;
    hal_handle_t      hal_hdl;
    ip_range_t        src_ip[NAT_CFG_MAX_ENTRIES];
    ip_range_t        dst_ip[NAT_CFG_MAX_ENTRIES];
    port_range_t      src_port[NAT_CFG_MAX_ENTRIES];
    port_range_t      dst_port[NAT_CFG_MAX_ENTRIES];
    nat::NatAction    src_nat_action;
    hal_handle_t      src_nat_pool;
    nat::NatAction    dst_nat_action;
    hal_handle_t      dst_nat_pool;
} nat_cfg_pol_test_t;

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
nat_cfg_pol_key_spec_build (nat::NatPolicySpec& spec, nat_cfg_pol_test_t *pol)
{
    if (pol->hal_hdl != HAL_HANDLE_INVALID)
        spec.mutable_key_or_handle()->set_policy_handle(pol->hal_hdl);

    if (pol->vrf_id != -1)
        spec.mutable_key_or_handle()->mutable_policy_key()->
            mutable_vrf_key_or_handle()->set_vrf_id(pol->vrf_id);

    if (pol->pol_id != -1)
        spec.mutable_key_or_handle()->mutable_policy_key()->
            set_nat_policy_id(pol->pol_id);
}

static inline void
nat_cfg_pol_spec_build (nat::NatPolicySpec& spec, nat_cfg_pol_test_t *pol)
{
    nat_cfg_pol_key_spec_build(spec, pol);
    nat_cfg_pol_data_spec_build(spec);
}

static inline hal_ret_t
nat_cfg_pol_create (nat::NatPolicySpec& spec, nat::NatPolicyResponse *rsp)
{
    hal_ret_t ret;

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::nat_policy_create(spec, rsp);
    hal::hal_cfg_db_close();
    return ret;
}

static inline void
nat_cfg_vrf_create ()
{
#define TEST_VRF_ID 1
    hal_ret_t ret;
    VrfSpec spec;
    VrfResponse rsp;

    spec.mutable_key_or_handle()->set_vrf_id(TEST_VRF_ID);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

TEST_F(nat_policy_test, create_pol_without_key_or_handle)
{
    hal_ret_t ret;
    NatPolicySpec spec;
    NatPolicyResponse rsp;

    ret = nat_cfg_pol_create(spec, &rsp);
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);
    ASSERT_TRUE(rsp.policy_status().nat_policy_handle() == HAL_HANDLE_INVALID);
}

TEST_F(nat_policy_test, create_pol_with_handle)
{
    hal_ret_t ret;
    NatPolicySpec spec;
    NatPolicyResponse rsp;
    nat_cfg_pol_test_t pol;

    memset(&pol, 0, sizeof(nat_cfg_pol_test_t));
    pol.hal_hdl = 0x10000; pol.vrf_id = -1; pol.pol_id = -1;

    nat_cfg_pol_key_spec_build(spec, &pol);
    ret = nat_cfg_pol_create(spec, &rsp);
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);
    ASSERT_TRUE(rsp.policy_status().nat_policy_handle() == HAL_HANDLE_INVALID);
}

TEST_F(nat_policy_test, create_pol_without_vrf)
{
    hal_ret_t ret;
    NatPolicySpec spec;
    NatPolicyResponse rsp;
    nat_cfg_pol_test_t pol;

    memset(&pol, 0, sizeof(nat_cfg_pol_test_t));
    pol.hal_hdl = HAL_HANDLE_INVALID; pol.vrf_id = -1; pol.pol_id = 10;

    nat_cfg_pol_key_spec_build(spec, &pol);
    ret = nat_cfg_pol_create(spec, &rsp);
    ASSERT_TRUE(ret == HAL_RET_VRF_NOT_FOUND);
    ASSERT_TRUE(rsp.policy_status().nat_policy_handle() == HAL_HANDLE_INVALID);
}

TEST_F(nat_policy_test, create_pol_with_no_rules)
{
    hal_ret_t ret;
    NatPolicySpec spec;
    NatPolicyResponse rsp;
    nat_cfg_pol_test_t pol;

    memset(&pol, 0, sizeof(nat_cfg_pol_test_t));
    pol.hal_hdl = HAL_HANDLE_INVALID; pol.vrf_id = TEST_VRF_ID; pol.pol_id = 10;

    nat_cfg_vrf_create();
    nat_cfg_pol_key_spec_build(spec, &pol);
    ret = nat_cfg_pol_create(spec, &rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(rsp.policy_status().nat_policy_handle() != HAL_HANDLE_INVALID);
}

int main (int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
