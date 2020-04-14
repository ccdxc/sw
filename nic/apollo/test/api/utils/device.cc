//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/device.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// Device feeder class routines
//----------------------------------------------------------------------------

void
device_feeder::init(std::string device_ip_str, std::string mac_addr_str,
                    std::string gw_ip_str, int num_device) {
    this->device_ip_str = device_ip_str;
    this->mac_addr_str = mac_addr_str;
    this->gw_ip_str = gw_ip_str;
    num_obj = num_device;
}

void
device_feeder::spec_build(pds_device_spec_t *spec) const {
    ip_prefix_t device_ip_pfx, gw_ip_pfx;

    memset(spec, 0, sizeof(pds_device_spec_t));

    test::extract_ip_pfx(device_ip_str.c_str(), &device_ip_pfx);
    if (device_ip_pfx.addr.af == IP_AF_IPV6) {
        spec->device_ip_addr.af = IP_AF_IPV6;
        memcpy(spec->device_ip_addr.addr.v6_addr.addr8,
               device_ip_pfx.addr.addr.v6_addr.addr8, IP6_ADDR8_LEN);
    } else {
        spec->device_ip_addr.af = IP_AF_IPV4;
        spec->device_ip_addr.addr.v4_addr = device_ip_pfx.addr.addr.v4_addr;
    }

    extract_ip_pfx(gw_ip_str.c_str(), &gw_ip_pfx);
    if (gw_ip_pfx.addr.af == IP_AF_IPV6) {
        spec->gateway_ip_addr.af = IP_AF_IPV6;
        memcpy(spec->gateway_ip_addr.addr.v6_addr.addr8,
               gw_ip_pfx.addr.addr.v6_addr.addr8, IP6_ADDR8_LEN);
    } else {
        spec->gateway_ip_addr.af = IP_AF_IPV4;
        spec->gateway_ip_addr.addr.v4_addr = gw_ip_pfx.addr.addr.v4_addr;
    }

    if (!apulu()) {
        mac_str_to_addr((char *)mac_addr_str.c_str(), spec->device_mac_addr);
    }

    if (apulu()) {
        spec->dev_oper_mode = PDS_DEV_OPER_MODE_HOST;
    } else {
        spec->dev_oper_mode = PDS_DEV_OPER_MODE_BITW;
    }
}

bool
device_feeder::spec_compare(const pds_device_spec_t *spec) const {
    ip_prefix_t device_ip_pfx, gw_ip_pfx;
    mac_addr_t  device_mac_addr;

    test::extract_ip_pfx(device_ip_str.c_str(), &device_ip_pfx);
    extract_ip_pfx(gw_ip_str.c_str(), &gw_ip_pfx);

    if (!IPADDR_EQ(&device_ip_pfx.addr, &spec->device_ip_addr)) {
        return false;
    }
    mac_str_to_addr((char *)mac_addr_str.c_str(), device_mac_addr);
    if (!apulu()) {
        if (memcmp(device_mac_addr, spec->device_mac_addr, ETH_ADDR_LEN)) {
            return false;
        }
    }
    if (!IPADDR_EQ(&gw_ip_pfx.addr, &spec->gateway_ip_addr)) {
        return false;
    }
    return true;
}

bool
device_feeder::status_compare(const pds_device_status_t *status1,
                              const pds_device_status_t *status2) const {
    return true;
}

//----------------------------------------------------------------------------
// DEVICE CRUD helper routines
//----------------------------------------------------------------------------

void
device_create (device_feeder& feeder)
{
    pds_batch_ctxt_t bctxt = batch_start();

    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_create<device_feeder>(bctxt, feeder)));
    batch_commit(bctxt);
}

void
device_read (device_feeder& feeder, sdk_ret_t exp_result)
{
    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_read<device_feeder>(feeder, exp_result)));
}

static void
device_attr_update (device_feeder& feeder, pds_device_spec_t *spec,
                    uint64_t chg_bmap)
{
    if (bit_isset(chg_bmap, DEVICE_ATTR_DEVICE_IP)) {
        feeder.spec.device_ip_addr = spec->device_ip_addr;
    }
    if (bit_isset(chg_bmap, DEVICE_ATTR_DEVICE_MAC)) {
        memcpy(&feeder.spec.device_mac_addr, &spec->device_mac_addr,
               sizeof(spec->device_mac_addr));
    }
    if (bit_isset(chg_bmap, DEVICE_ATTR_GATEWAY_IP)) {
        feeder.spec.gateway_ip_addr = spec->gateway_ip_addr;
    }
    if (bit_isset(chg_bmap, DEVICE_ATTR_BRIDGING_EN)) {
        feeder.spec.bridging_en = spec->bridging_en;
    }
    if (bit_isset(chg_bmap, DEVICE_ATTR_LEARNING_EN)) {
        feeder.spec.learning_en = spec->learning_en;
    }
    if (bit_isset(chg_bmap, DEVICE_ATTR_LEARN_AGE_TIME_OUT)) {
        feeder.spec.learn_age_timeout = spec->learn_age_timeout;
    }
    if (bit_isset(chg_bmap, DEVICE_ATTR_OVERLAY_ROUTING_EN)) {
        feeder.spec.overlay_routing_en = spec->overlay_routing_en;
    }
    if (bit_isset(chg_bmap, DEVICE_ATTR_DEVICE_PROFILE)) {
        feeder.spec.device_profile = spec->device_profile;
    }
    if (bit_isset(chg_bmap, DEVICE_ATTR_MEMORY_PROFILE)) {
        feeder.spec.memory_profile = spec->memory_profile;
    }
    if (bit_isset(chg_bmap, DEVICE_ATTR_DEV_OPER_MODE)) {
        feeder.spec.dev_oper_mode = spec->dev_oper_mode;
    }
}

void
device_update (device_feeder& feeder, pds_device_spec_t *spec,
               uint64_t chg_bmap, sdk_ret_t exp_result)
{
    pds_batch_ctxt_t bctxt = batch_start();

    device_attr_update(feeder, spec, chg_bmap);
    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_update<device_feeder>(bctxt, feeder)));

    // if expected result is err, batch commit should fail
    if (exp_result == SDK_RET_ERR)
        batch_commit_fail(bctxt);
    else
        batch_commit(bctxt);
}

void
device_delete (device_feeder& feeder)
{
    pds_batch_ctxt_t bctxt = batch_start();

    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_delete<device_feeder>(bctxt, feeder)));
    batch_commit(bctxt);
}

//----------------------------------------------------------------------------
// Misc routines
//----------------------------------------------------------------------------

// do not modify these sample values as rest of system is sync with these
std::string k_device_ip("91.0.0.1");
static device_feeder k_device_feeder;

void sample_device_setup(pds_batch_ctxt_t bctxt) {
    // setup and teardown parameters should be in sync
    k_device_feeder.init(k_device_ip, "00:00:01:02:0a:0b", "90.0.0.2");
    many_create(bctxt, k_device_feeder);
}

void sample_device_setup_validate() {
    k_device_feeder.init(k_device_ip, "00:00:01:02:0a:0b", "90.0.0.2");
    many_read(k_device_feeder);
}

void sample_device_teardown(pds_batch_ctxt_t bctxt) {
    // this feeder base values doesn't matter in case of deletes
    k_device_feeder.init(k_device_ip, "00:00:01:02:0a:0b", "90.0.0.2");
    many_delete(bctxt, k_device_feeder);
}

}    // namespace api
}    // namespace test
