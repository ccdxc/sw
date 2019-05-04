//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the mapping test cases
///
//----------------------------------------------------------------------------

#include <stdio.h>
#include <getopt.h>
#include <gtest/gtest.h>
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/model_sim/include/lib_model_client.h"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/vnic.hpp"
#include "nic/apollo/test/utils/device.hpp"
#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/tep.hpp"
#include "nic/apollo/test/utils/route.hpp"
#include "nic/apollo/test/utils/subnet.hpp"
#include "nic/apollo/test/utils/local_mapping.hpp"
#include "nic/apollo/test/utils/remote_mapping.hpp"

namespace api_test {

// Globals
char *g_cfg_file = NULL;
static pds_epoch_t g_batch_epoch = PDS_EPOCH_INVALID;

// Config for VPC 1
uint32_t g_vpc_id = 1;
uint32_t g_subnet_id = 1;
uint16_t g_vnic_id = 1;
uint32_t g_encap_val = 1;
uint64_t g_base_vnic_mac = 0x000000030b020a01;
std::string g_vpc_cidr_v4("5.5.0.0/16");
std::string g_vnic_cidr_v4("5.5.0.2/16");
std::string g_subnet_cidr_v4("5.5.0.0/16");
std::string g_public_ip_v4("192.168.0.2/16");
std::string g_tep_cidr_v4("1.0.0.1");
std::string g_rt_cidr_v4("1.0.0.1");
std::string vnic_test_ip = "5.5.0.0";
std::string tep_test_ip = "1.0.0.1";
std::string remote_test_ip = "5.5.0.10";
uint64_t g_remote_vnic_mac = 0x000000030b020a02;

// Constants
const std::string g_device_ip("91.0.0.1");
const std::string g_gateway_ip("90.0.0.2");
const std::string g_device_macaddr("00:00:01:02:0a:0b");

//----------------------------------------------------------------------------
// Mapping test class
//----------------------------------------------------------------------------

static inline void vnic_stepper_seed_init(int seed_base, uint64_t seed_mac,
                                          vnic_stepper_seed_t *seed);

class mapping_test : public pds_test_base {
protected:
    mapping_test() {}
    virtual ~mapping_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        test_case_params_t params;
        params.cfg_file = api_test::g_cfg_file;
        params.enable_fte = false;
        pds_test_base::SetUpTestCase(params);
        uint16_t vpc_id = api_test::g_vpc_id;
        uint16_t vnic_stepper = api_test::g_vpc_id;
        uint32_t num_vnics = PDS_MAX_VNIC;
        uint32_t rt_id_v4 = api_test::g_subnet_id;
        uint32_t rt_id_v6 = api_test::g_subnet_id + 1024;
        uint32_t num_teps = PDS_MAX_TEP - 1;
        uint64_t vnic_stepper_mac = api_test::g_base_vnic_mac;
        pds_vpc_key_t vpc_key = {0};
        pds_subnet_key_t subnet_key = {0};
        pds_vnic_info_t vnic_info = {0};
        pds_device_info_t dev_info = {0};
        pds_vpc_info_t vpc_info = {0};
        pds_subnet_info_t sub_info = {0};
        pds_tep_info_t tep_info = {0};
        vnic_stepper_seed_t seed;
        pds_batch_params_t batch_params = {0};
        pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};
        std::string subnet_cidr = api_test::g_subnet_cidr_v4;
        std::string rt_ip = api_test::g_rt_cidr_v4;
        std::string nr_cidr = "100.0.0.1/16";
        ip_prefix_t ip_pfx, rt_pfx, nr_pfx;
        ip_addr_t ipaddr, rt_addr, nr_addr;

        vpc_key.id = api_test::g_vpc_id;
        subnet_key.id = api_test::g_subnet_id;
        extract_ip_pfx((char *)subnet_cidr.c_str(), &ip_pfx);
        extract_ip_pfx((char *)api_test::g_rt_cidr_v4.c_str(), &rt_pfx);
        extract_ip_pfx((char *)nr_cidr.c_str(), &nr_pfx);

        device_util device_obj(g_device_ip, g_device_macaddr, g_gateway_ip);
        tep_util tep_obj(api_test::g_device_ip, PDS_TEP_TYPE_WORKLOAD, encap);

        batch_params.epoch = ++api_test::g_batch_epoch;
        ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
        ASSERT_TRUE(device_obj.create() == sdk::SDK_RET_OK);
        ASSERT_TRUE(vpc_util::many_create(vpc_key, api_test::g_vpc_cidr_v4,
                                          PDS_MAX_VPC, PDS_VPC_TYPE_TENANT) ==
                    sdk::SDK_RET_OK);
        ASSERT_TRUE(tep_obj.create() == sdk::SDK_RET_OK);
        ASSERT_TRUE(tep_util::many_create(num_teps, api_test::g_tep_cidr_v4,
                                          PDS_TEP_TYPE_WORKLOAD,
                                          encap) == sdk::SDK_RET_OK);
        for (uint16_t idx = 0; idx < num_teps; idx++) {
            route_table_util rt_obj(rt_id_v4 + idx, nr_cidr, rt_ip, IP_AF_IPV4,
                                    1);
            ip_prefix_ip_next(&rt_pfx, &rt_addr);
            rt_pfx.addr = rt_addr;
            rt_ip = ipv4addr2str(rt_addr.addr.v4_addr);

            ip_prefix_ip_next(&nr_pfx, &nr_addr);
            nr_pfx.addr = nr_addr;
            nr_cidr = ippfx2str(&nr_pfx);

            ASSERT_TRUE(rt_obj.create() == sdk::SDK_RET_OK);
        }

        subnet_key.id = api_test::g_subnet_id;
        vpc_key.id = api_test::g_vpc_id;
        for (uint16_t idx = 0; idx < PDS_MAX_VPC; idx++) {
            ASSERT_TRUE(subnet_util::many_create(subnet_key, vpc_key,
                                                 subnet_cidr,
                                                 1) == sdk ::SDK_RET_OK);
            subnet_key.id += 1;
            vpc_key.id += 1;
            ip_prefix_ip_next(&ip_pfx, &ipaddr);
            ip_pfx.addr = ipaddr;
            subnet_cidr = ippfx2str(&ip_pfx);
        }
        vnic_stepper_seed_init(vnic_stepper, vnic_stepper_mac, &seed);
        ASSERT_TRUE(vnic_util::many_create(&seed, num_vnics) ==
                    sdk::SDK_RET_OK);
        ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

        subnet_key.id = api_test::g_subnet_id;
        vpc_key.id = api_test::g_vpc_id;
        ASSERT_TRUE(vpc_util::many_read(vpc_key, PDS_MAX_VPC,
                                        sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
        ASSERT_TRUE(subnet_util::many_read(subnet_key, PDS_MAX_VPC,
                                           sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
        ASSERT_TRUE(device_obj.read(&dev_info, false) == sdk::SDK_RET_OK);
        ASSERT_TRUE(tep_obj.read(&tep_info) == sdk::SDK_RET_OK);
        ASSERT_TRUE(tep_util::many_read(num_teps, api_test::g_tep_cidr_v4,
                                        PDS_TEP_TYPE_WORKLOAD,
                                        encap) == sdk::SDK_RET_OK);
        ASSERT_TRUE(vnic_util::many_read(&seed, num_vnics, sdk::SDK_RET_OK) ==
                    sdk::SDK_RET_OK);
    }
    static void TearDownTestCase() {
#if 0
        uint16_t vpc_id = api_test::g_vpc_id;
        uint16_t vnic_stepper = api_test::g_vpc_id;
        uint32_t num_vnics = PDS_MAX_VNIC;
        uint32_t rt_id_v4 = api_test::g_subnet_id;
        uint32_t rt_id_v6 = api_test::g_subnet_id + 1024;
        uint32_t num_teps = PDS_MAX_TEP - 1;
        uint64_t vnic_stepper_mac = api_test::g_base_vnic_mac;
        std::string subnet_cidr = api_test::g_subnet_cidr_v4;
        std::string rt_ip = api_test::g_rt_cidr_v4;
        std::string nr_cidr = "100.0.0.1/16";
        ip_prefix_t ip_pfx, rt_pfx, nr_pfx;
        ip_addr_t ipaddr, rt_addr, nr_addr;
        pds_vpc_key_t vpc_key = {0};
        pds_subnet_key_t subnet_key = {0};
        vnic_stepper_seed_t seed;
        pds_batch_params_t batch_params = {0};
        pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};

        vpc_key.id = api_test::g_vpc_id;
        subnet_key.id = api_test::g_subnet_id;
        extract_ip_pfx((char *)subnet_cidr.c_str(), &ip_pfx);
        extract_ip_pfx((char *)api_test::g_rt_cidr_v4.c_str(), &rt_pfx);
        extract_ip_pfx((char *)nr_cidr.c_str(), &nr_pfx);

        device_util device_obj(g_device_ip, g_device_macaddr, g_gateway_ip);
        tep_util tep_obj(api_test::g_device_ip, PDS_TEP_TYPE_WORKLOAD, encap);

        batch_params.epoch = ++api_test::g_batch_epoch;
        ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
        ASSERT_TRUE(vnic_util::many_delete(&seed, num_vnics) ==
                    sdk::SDK_RET_OK);
        for (uint16_t idx = 0; idx < num_teps; idx++) {
            route_table_util rt_obj(rt_id_v4 + idx, nr_cidr, rt_ip, IP_AF_IPV4,
                                    1);
            ip_prefix_ip_next(&rt_pfx, &rt_addr);
            rt_pfx.addr = rt_addr;
            rt_ip = ipv4addr2str(rt_addr.addr.v4_addr);

            ip_prefix_ip_next(&nr_pfx, &nr_addr);
            nr_pfx.addr = nr_addr;
            nr_cidr = ippfx2str(&nr_pfx);

            ASSERT_TRUE(rt_obj.del() == sdk::SDK_RET_OK);
        }
        ASSERT_TRUE(tep_util::many_delete(num_teps, api_test::g_tep_cidr_v4,
                                          PDS_TEP_TYPE_WORKLOAD,
                                          encap) == sdk::SDK_RET_OK);
        ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
        subnet_key.id = api_test::g_subnet_id;
        vpc_key.id = api_test::g_vpc_id;
        for (uint16_t idx = 0; idx < PDS_MAX_VPC; idx++) {
            subnet_util sub_obj(vpc_key.id, subnet_key.id, subnet_cidr,
                                rt_id_v4++, rt_id_v6++);
            ASSERT_TRUE(sub_obj.del() == sdk::SDK_RET_OK);
            subnet_key.id += 1;
            vpc_key.id += 1;
            ip_prefix_ip_next(&ip_pfx, &ipaddr);
            ip_pfx.addr = ipaddr;
            subnet_cidr = ippfx2str(&ip_pfx);
        }
        ASSERT_TRUE(vpc_util::many_delete(vpc_key, PDS_MAX_VPC) ==
                    sdk::SDK_RET_OK);
        ASSERT_TRUE(device_obj.del() == sdk::SDK_RET_OK);
        ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
#endif
        pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// Mapping test cases implementation
//----------------------------------------------------------------------------

/// \defgroup Mapping
/// @{

/// --------------------------- IPv4 MAPPINGS -----------------------

static inline void
vnic_stepper_seed_init (int seed_base, uint64_t seed_mac,
                        vnic_stepper_seed_t *seed)
{
    seed->id = seed_base;
    seed->vnic_encap.type = PDS_ENCAP_TYPE_DOT1Q;
    seed->vnic_encap.val.vlan_tag = seed_base;
    seed->fabric_encap.type = PDS_ENCAP_TYPE_MPLSoUDP;
    seed->fabric_encap.val.mpls_tag = seed_base;
    seed->mac_u64 = seed_mac;
}

static void
local_mapping_stepper_seed_init (local_mapping_stepper_seed_t *seed,
                                 uint16_t base_vnic_id, uint32_t base_encap_val,
                                 uint64_t base_mac_64, std::string vnic_ip_cidr,
                                 std::string public_ip_cidr)
{
    seed->vnic_id_stepper = base_vnic_id;
    seed->encap_val_stepper = base_encap_val;
    seed->vnic_mac_stepper = base_mac_64;
    seed->vnic_ip_stepper = vnic_ip_cidr;
    seed->public_ip_stepper = public_ip_cidr;
}

static void
remote_mapping_stepper_seed_init (remote_mapping_stepper_seed_t *seed,
                                  uint32_t base_encap_val, uint64_t base_mac_64,
                                  std::string vnic_ip_cidr,
                                  std::string tep_ip_cidr)
{
    seed->encap_val_stepper = base_encap_val;
    seed->vnic_mac_stepper = base_mac_64;
    seed->vnic_ip_stepper = vnic_ip_cidr;
    seed->tep_ip_stepper = tep_ip_cidr;
}

static void
step_up_ip (ip_prefix_t *pfx, uint16_t num_steps)
{
    if (pfx->addr.af == IP_AF_IPV4) {
        pfx->addr.addr.v4_addr += num_steps;
    }
}

static void
step_up_ip_next (ip_prefix_t *pfx, uint16_t num_steps)
{
    ip_addr_t ipaddr_next;
    for (uint8_t i = 0; i < num_steps; i++) {
        ip_prefix_ip_next(pfx, &ipaddr_next);
        pfx->addr = ipaddr_next;
    }
}

/// --------------------------- LOCAL MAPPINGS -----------------------

/// \brief Dummy test case to check
/// create/read/delete operations on
/// one mapping using one vnic and one IPv4
TEST_F(mapping_test, DISABLED_v4_dummy_mapping_test) {
    pds_batch_params_t batch_params = {0};
    pds_local_mapping_info_t l_info = {0};
    pds_remote_mapping_info_t r_info = {0};

    // setup
    local_mapping_util local_obj(api_test::g_vpc_id, api_test::g_subnet_id,
                                 api_test::vnic_test_ip, api_test::g_vnic_id,
                                 api_test::g_base_vnic_mac);
    remote_mapping_util rem_obj(api_test::g_vpc_id, api_test::g_subnet_id,
                                api_test::remote_test_ip, api_test::tep_test_ip,
                                api_test::g_remote_vnic_mac);

    // trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(local_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(rem_obj.create() == sdk::SDK_RET_OK);
    // ASSERT_TRUE(rem_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // ASSERT_TRUE(local_obj.del() == sdk::SDK_RET_OK);
    // ASSERT_TRUE(rem_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Verify using read
    ASSERT_TRUE(local_obj.read(&l_info, false) == sdk::SDK_RET_OK);
    ASSERT_TRUE(rem_obj.read(&r_info, false) == sdk::SDK_RET_OK);
}

/// \brief Create and delete max local mappings in the same batch
/// The operation should be de-duped by framework and is
/// a NO-OP from hardware perspective
/// [ Create SetMax, Delete SetMax ] - Read
TEST_F(mapping_test, v4_local_mapping_workflow_1) {
    uint32_t num_vnics = PDS_MAX_VNIC;
    uint32_t num_ip_per_vnic = PDS_MAX_VNIC_IP;
    pds_batch_params_t batch_params = {0};
    local_mapping_stepper_seed_t map_seed = {0};

    local_mapping_stepper_seed_init(
        &map_seed, api_test::g_vnic_id, api_test::g_encap_val,
        api_test::g_base_vnic_mac, api_test::g_vnic_cidr_v4,
        api_test::g_public_ip_v4);

    // trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(local_mapping_util::many_create(
                    num_ip_per_vnic, num_vnics, api_test::g_vpc_id,
                    api_test::g_subnet_id, &map_seed) == sdk::SDK_RET_OK);
    // ASSERT_TRUE(local_mapping_util::many_delete(num_ip_per_vnic, num_vnics,
    //                                              api_test::g_vpc_id,
    //                                              &map_seed) ==
    //                                              sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(local_mapping_util::many_read(
                    num_ip_per_vnic, num_vnics, api_test::g_vpc_id,
                    api_test::g_subnet_id, &map_seed, PDS_ENCAP_TYPE_MPLSoUDP,
                    false, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
}

///\brief Create, delete and create max local mappings in the same batch
/// Create and delete should be de-deduped by framework and subsequent create
/// should result in successful creation
/// [ Create SetMax - Delete SetMax - Create SetMax ] - Read
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_2) {
    uint32_t num_vnics = PDS_MAX_VNIC;
    uint32_t num_ip_per_vnic = PDS_MAX_VNIC_IP;
    pds_batch_params_t batch_params = {0};
    local_mapping_stepper_seed_t map_seed;

    local_mapping_stepper_seed_init(
        &map_seed, api_test::g_vnic_id, api_test::g_encap_val,
        api_test::g_base_vnic_mac, api_test::g_vnic_cidr_v4,
        api_test::g_public_ip_v4);

    // trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(local_mapping_util::many_create(
                    num_ip_per_vnic, num_vnics, api_test::g_vpc_id,
                    api_test::g_subnet_id, &map_seed) == sdk::SDK_RET_OK);
    // ASSERT_TRUE(local_mapping_util::many_delete(num_ip_per_vnic, num_vnics,
    // api_test::g_vpc_id,
    //                                          &map_seed) == sdk::SDK_RET_OK);
    // ASSERT_TRUE(local_mapping_util::many_create(num_ip_per_vnic, num_vnics,
    // api_test::g_vpc_id,
    //                                            api_test::g_subnet_id,
    //                                            &map_seed) ==
    //                                            sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Verify using read
    // ASSERT_TRUE(local_mapping_util::many_read(num_ip_per_vnic, num_vnics,
    // api_test::g_vpc_id,
    //                                          api_test::g_subnet_id,
    //                                          &map_seed) == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // ASSERT_TRUE(local_mapping_util::many_delete(num_ip_per_vnic, num_vnics,
    // api_test::g_vpc_id,
    //                                            &map_seed) ==
    //                                            sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create two sets, followed by delete one of the sets and creating one
/// more new set Read operation shoud successfully read last two sets and return
/// entries not found for deleted set [ Create Set1, Set2 - Delete Set1 - Create
/// Set3 ] - Read
TEST_F(mapping_test, DISABLED_local_mapping_workflow_3) {
    uint32_t num_ip_per_vnic = PDS_MAX_VNIC_IP;
    uint32_t num_vnics_per_set = 341;
    uint32_t ip_stepper = num_ip_per_vnic * num_vnics_per_set;
    std::string vnic_ip_cidr = "0.0.0.0/8";
    std::string pub_ip_cidr = "0.0.0.0/8";
    pds_batch_params_t batch_params = {0};
    ip_prefix_t vip_pfx;
    ip_prefix_t pip_pfx;
    local_mapping_stepper_seed_t s1_seed;
    local_mapping_stepper_seed_t s2_seed;
    local_mapping_stepper_seed_t s3_seed;

    local_mapping_stepper_seed_init(
        &s1_seed, api_test::g_vnic_id, api_test::g_encap_val,
        api_test::g_base_vnic_mac, api_test::g_vnic_cidr_v4,
        api_test::g_public_ip_v4);

    str2ipv4pfx((char *)api_test::g_vnic_cidr_v4.c_str(), &vip_pfx);
    step_up_ip(&vip_pfx, ip_stepper);
    vnic_ip_cidr = ippfx2str(&vip_pfx);
    str2ipv4pfx((char *)api_test::g_public_ip_v4.c_str(), &pip_pfx);
    step_up_ip(&pip_pfx, ip_stepper);
    pub_ip_cidr = ippfx2str(&pip_pfx);
    local_mapping_stepper_seed_init(
        &s2_seed, api_test::g_vnic_id + num_vnics_per_set,
        api_test::g_encap_val + num_vnics_per_set,
        api_test::g_base_vnic_mac + num_vnics_per_set, vnic_ip_cidr.c_str(),
        pub_ip_cidr.c_str());

    step_up_ip(&vip_pfx, ip_stepper);
    step_up_ip(&pip_pfx, ip_stepper);
    local_mapping_stepper_seed_init(&s3_seed, api_test::g_vnic_id + 682,
                                    api_test::g_encap_val + 682,
                                    api_test::g_base_vnic_mac + 682,
                                    ippfx2str(&vip_pfx), ippfx2str(&pip_pfx));

    // trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(local_mapping_util::many_create(
                    num_ip_per_vnic, num_vnics_per_set, api_test::g_vpc_id,
                    api_test::g_subnet_id, &s1_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(local_mapping_util::many_create(
                    num_ip_per_vnic, num_vnics_per_set, api_test::g_vpc_id,
                    api_test::g_subnet_id, &s2_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(local_mapping_util::many_delete(
                    num_ip_per_vnic, num_vnics_per_set, api_test::g_vpc_id,
                    &s1_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(local_mapping_util::many_create(
                    num_ip_per_vnic, 342, api_test::g_vpc_id,
                    api_test::g_subnet_id, &s3_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // set 1 : expected = entry not found
    ASSERT_TRUE(local_mapping_util::many_read(
                    num_ip_per_vnic, num_vnics_per_set, api_test::g_vpc_id,
                    api_test::g_subnet_id, &s1_seed, PDS_ENCAP_TYPE_MPLSoUDP,
                    false, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);

    // set 2, 3: expected = entries present
    ASSERT_TRUE(local_mapping_util::many_read(
                    num_ip_per_vnic, num_vnics_per_set, api_test::g_vpc_id,
                    api_test::g_subnet_id, &s2_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(local_mapping_util::many_read(
                    num_ip_per_vnic, 342, api_test::g_vpc_id,
                    api_test::g_subnet_id, &s3_seed) == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // ASSERT_TRUE(local_mapping_util::many_delete(num_ip_per_vnic,
    // num_vnics_per_set,
    //                                              api_test::g_vpc_id,
    //                                              &s2_seed) ==
    //                                              sdk::SDK_RET_OK);
    // ASSERT_TRUE(local_mapping_util::many_delete(num_ip_per_vnic, 342,
    //                                              api_test::g_vpc_id,
    //                                              &s3_seed) ==
    //                                              sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create and delete local mappings in two batches
/// The hardware should create and delete mappings correctly.
/// Validate using reads at each batch end
/// [ Create SetMax ] - Read - [ Delete SetMax ] - Read
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_4) {
    uint32_t num_vnics = PDS_MAX_VNIC;
    uint32_t num_ip_per_vnic = PDS_MAX_VNIC_IP;
    pds_batch_params_t batch_params = {0};
    local_mapping_stepper_seed_t map_seed;

    local_mapping_stepper_seed_init(
        &map_seed, api_test::g_vnic_id, api_test::g_encap_val,
        api_test::g_base_vnic_mac, api_test::g_vnic_cidr_v4,
        api_test::g_public_ip_v4);

    // trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(local_mapping_util::many_create(
                    num_ip_per_vnic, num_vnics, api_test::g_vpc_id,
                    api_test::g_subnet_id, &map_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // ASSERT_TRUE(local_mapping_util::many_read(num_ip_per_vnic, num_vnics,
    //                                          api_test::g_vpc_id,
    //                                          api_test::g_subnet_id,
    //                                          &map_seed) == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // ASSERT_TRUE(local_mapping_util::many_delete(num_ip_per_vnic, num_vnics,
    //                                              api_test::g_vpc_id,
    //                                              api_test::g_subnet_id,
    //                                              &map_seed) ==
    //                                              sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // ASSERT_TRUE(local_mapping_util::many_read(num_ip_per_vnic, num_vnics,
    // api_test::g_vpc_id,
    //                                          api_test::g_subnet_id,
    //                                          &map_seed,
    //                                          PDS_ENCAP_TYPE_MPLSoUDP, false,
    //                                           sdk::SDK_RET_ENTRY_NOT_FOUND)
    //                                           == sdk::SDK_RET_OK);
}

/// \brief Create and delete mix and match of mappings in two batches
/// [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_5) {
    uint32_t num_ip_per_vnic = PDS_MAX_VNIC_IP;
    uint32_t num_vnics_per_set = 341;
    uint32_t ip_stepper = num_ip_per_vnic * num_vnics_per_set;
    std::string vnic_ip_cidr = "0.0.0.0/8";
    std::string pub_ip_cidr = "0.0.0.0/8";
    pds_batch_params_t batch_params = {0};
    ip_prefix_t vip_pfx;
    ip_prefix_t pip_pfx;
    local_mapping_stepper_seed_t s1_seed;
    local_mapping_stepper_seed_t s2_seed;
    local_mapping_stepper_seed_t s3_seed;

    local_mapping_stepper_seed_init(
        &s1_seed, api_test::g_vnic_id, api_test::g_encap_val,
        api_test::g_base_vnic_mac, api_test::g_vnic_cidr_v4,
        api_test::g_public_ip_v4);

    str2ipv4pfx((char *)api_test::g_vnic_cidr_v4.c_str(), &vip_pfx);
    step_up_ip(&vip_pfx, ip_stepper);
    vnic_ip_cidr = ippfx2str(&vip_pfx);
    str2ipv4pfx((char *)api_test::g_public_ip_v4.c_str(), &pip_pfx);
    step_up_ip(&pip_pfx, ip_stepper);
    pub_ip_cidr = ippfx2str(&pip_pfx);
    local_mapping_stepper_seed_init(
        &s2_seed, api_test::g_vnic_id + num_vnics_per_set,
        api_test::g_encap_val + num_vnics_per_set,
        api_test::g_base_vnic_mac + num_vnics_per_set, vnic_ip_cidr.c_str(),
        pub_ip_cidr.c_str());

    step_up_ip(&vip_pfx, ip_stepper * 2);
    step_up_ip(&pip_pfx, ip_stepper * 2);
    local_mapping_stepper_seed_init(&s3_seed, api_test::g_vnic_id + 682,
                                    api_test::g_encap_val + 682,
                                    api_test::g_base_vnic_mac + 682,
                                    ippfx2str(&vip_pfx), ippfx2str(&pip_pfx));

    // trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(local_mapping_util::many_create(
                    num_ip_per_vnic, num_vnics_per_set, api_test::g_vpc_id,
                    api_test::g_subnet_id, &s1_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(local_mapping_util::many_create(
                    num_ip_per_vnic, num_vnics_per_set, api_test::g_vpc_id,
                    api_test::g_subnet_id, &s2_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // ASSERT_TRUE(local_mapping_util::many_delete(num_ip_per_vnic,
    // num_vnics_per_set,
    //                                              api_test::g_vpc_id,
    //                                              &s1_seed) ==
    //                                              sdk::SDK_RET_OK);
    ASSERT_TRUE(local_mapping_util::many_create(
                    num_ip_per_vnic, 342, api_test::g_vpc_id,
                    api_test::g_subnet_id, &s3_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // set 1 : expected = entry not found
    // ASSERT_TRUE(local_mapping_util::many_read(num_ip_per_vnic,
    // num_vnics_per_set, api_test::g_vpc_id,
    //                                          api_test::g_subnet_id, &s1_seed,
    //                                          PDS_ENCAP_TYPE_MPLSoUDP, false,
    //                                           sdk::SDK_RET_ENTRY_NOT_FOUND)
    //                                           == sdk::SDK_RET_OK);

    // set 2, 3: expected = entries present
    // ASSERT_TRUE(local_mapping_util::many_read(num_ip_per_vnic,
    // num_vnics_per_set,
    //                                          api_test::g_vpc_id,
    //                                          api_test::g_subnet_id, &s2_seed)
    //                                          == sdk::SDK_RET_OK);
    // ASSERT_TRUE(local_mapping_util::many_read(num_ip_per_vnic, 342,
    //                                          api_test::g_vpc_id,
    //                                          api_test::g_subnet_id, &s3_seed)
    //                                          == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // ASSERT_TRUE(local_mapping_util::many_delete(num_ip_per_vnic, 342,
    //                                              api_test::g_vpc_id,
    //                                              &s3_seed) ==
    //                                              sdk::SDK_RET_OK);
    // ASSERT_TRUE(local_mapping_util::many_delete(num_ip_per_vnic,
    // num_vnics_per_set,
    //                                              api_test::g_vpc_id,
    //                                              &s2_seed) ==
    //                                              sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create maximum number of mappings in two batches
/// [ Create SetMax ] - [ Create SetMax ] - Read
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_neg_1) {
    uint32_t num_vnics = PDS_MAX_VNIC;
    uint32_t num_ip_per_vnic = PDS_MAX_VNIC_IP;
    pds_batch_params_t batch_params = {0};
    local_mapping_stepper_seed_t map_seed;

    local_mapping_stepper_seed_init(
        &map_seed, api_test::g_vnic_id, api_test::g_encap_val,
        api_test::g_base_vnic_mac, api_test::g_vnic_cidr_v4,
        api_test::g_public_ip_v4);

    // trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(local_mapping_util::many_create(
                    num_ip_per_vnic, num_vnics, api_test::g_vpc_id,
                    api_test::g_subnet_id, &map_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // ASSERT_TRUE(local_mapping_util::many_read(num_ip_per_vnic, num_vnics,
    // api_test::g_vpc_id,
    //                                          api_test::g_subnet_id,
    //                                          &map_seed) == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(local_mapping_util::many_create(
                    num_ip_per_vnic, num_vnics, api_test::g_vpc_id,
                    api_test::g_subnet_id, &map_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // ASSERT_TRUE(local_mapping_util::many_delete(num_ip_per_vnic, num_vnics,
    //                                              api_test::g_vpc_id,
    //                                              &map_seed) ==
    //                                              sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create more than maximum number of mappings supported.
/// [ Create SetMax+1] - Read
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_neg_2) {
    uint32_t num_vnics = PDS_MAX_VNIC;
    uint32_t num_ip_per_vnic = PDS_MAX_VNIC_IP;
    uint32_t vnic_id = 1024;
    uint64_t vnic_mac =
        api_test::g_base_vnic_mac + (num_ip_per_vnic * num_vnics);
    std::string vnic_cidr = "10.10.0.3";
    pds_batch_params_t batch_params = {0};
    local_mapping_stepper_seed_t map_seed;

    local_mapping_stepper_seed_init(
        &map_seed, api_test::g_vnic_id, api_test::g_encap_val,
        api_test::g_base_vnic_mac, api_test::g_vnic_cidr_v4,
        api_test::g_public_ip_v4);

    local_mapping_util mapping_obj(api_test::g_vpc_id, api_test::g_subnet_id,
                                   vnic_cidr, vnic_id, vnic_mac);

    // trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(local_mapping_util::many_create(
                    num_ip_per_vnic, num_vnics, api_test::g_vpc_id,
                    api_test::g_subnet_id, &map_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mapping_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_NO_RESOURCE);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // ASSERT_TRUE(local_mapping_util::many_read(num_ip_per_vnic, num_vnics,
    // api_test::g_vpc_id,
    //                                          api_test::g_subnet_id,
    //                                          &map_seed,
    //                                          PDS_ENCAP_TYPE_MPLSoUDP, false,
    //                                          sdk::SDK_RET_ENTRY_NOT_FOUND) ==
    //                                          sdk::SDK_RET_OK);
}

/// \brief Read of a non-existing VNIC should return entry not found.
/// Read NonEx
TEST_F(mapping_test, DISABLED_local_mapping_workflow_neg_3a) {
    uint32_t vnic_id = 9999;
    uint64_t vnic_mac = 0x0000000040302010;
    std::string vnic_cidr = "100.99.98.97";
    pds_batch_params_t batch_params = {0};
    pds_local_mapping_info_t info = {0};

    // setup
    local_mapping_util mapping_obj(api_test::g_vpc_id, api_test::g_subnet_id,
                                   vnic_cidr, vnic_id, vnic_mac);

    // trigger
    ASSERT_TRUE(mapping_obj.read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Deletion of a non-existing VNICs should fail.
/// [Delete NonEx]
TEST_F(mapping_test, DISABLED_local_mapping_workflow_neg_3b) {
    uint32_t vnic_id = 9999;
    uint64_t vnic_mac = 0x00000000000abcde;
    std::string vnic_cidr = "97.98.99.100";
    pds_batch_params_t batch_params = {0};
    pds_local_mapping_info_t info = {0};

    // setup
    local_mapping_util mapping_obj(api_test::g_vpc_id, api_test::g_subnet_id,
                                   vnic_cidr, vnic_id, vnic_mac);

    // trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mapping_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// --------------------------- END LOCAL MAPPINGS -----------------------

/// --------------------------- REMOTE MAPPINGS -----------------------

/// \brief Create and delete max remote mappings in the same batch
/// The operation should be de-duped by framework and is
/// a NO-OP from hardware perspective
/// [ Create SetMax, Delete SetMax ] - Read
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_1) {
    uint32_t num_vnics = PDS_MAX_VNIC;
    uint32_t num_teps = 1022;
    remote_mapping_stepper_seed_t seed;
    pds_batch_params_t batch_params = {0};

    remote_mapping_stepper_seed_init(
        &seed, api_test::g_encap_val, api_test::g_base_vnic_mac,
        api_test::g_vnic_cidr_v4, api_test::g_tep_cidr_v4);
    // trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_mapping_util::many_create(
                    num_vnics, num_teps, api_test::g_vpc_id,
                    api_test::g_subnet_id, &seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_mapping_util::many_delete(num_vnics, num_teps,
                                                 api_test::g_vpc_id,
                                                 &seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(remote_mapping_util::many_read(
                    num_vnics, num_teps, api_test::g_vpc_id,
                    api_test::g_subnet_id, &seed, PDS_ENCAP_TYPE_MPLSoUDP,
                    sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

///\brief Create, delete and create max remote mappings in the same batch
/// Create and delete should be de-deduped by framework and subsequent create
/// should result in successful creation
/// [ Create SetMax - Delete SetMax - Create SetMax ] - Read
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_2) {
    uint32_t num_vnics = PDS_MAX_VNIC;
    uint32_t num_teps = 8;
    remote_mapping_stepper_seed_t seed;
    pds_batch_params_t batch_params = {0};

    remote_mapping_stepper_seed_init(
        &seed, api_test::g_encap_val, api_test::g_base_vnic_mac,
        api_test::g_vnic_cidr_v4, api_test::g_tep_cidr_v4);
    // trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_mapping_util::many_create(
                    num_vnics, num_teps, api_test::g_vpc_id,
                    api_test::g_subnet_id, &seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_mapping_util::many_delete(num_vnics, num_teps,
                                                 api_test::g_vpc_id,
                                                 &seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_mapping_util::many_create(
                    num_vnics, num_teps, api_test::g_vpc_id,
                    api_test::g_subnet_id, &seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(remote_mapping_util::many_read(
                    num_vnics, num_teps, api_test::g_vpc_id,
                    api_test::g_subnet_id, &seed) == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_mapping_util::many_delete(num_vnics, num_teps,
                                                 api_test::g_vpc_id,
                                                 &seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create two sets, followed by delete one of the sets and creating one
/// more new set Read operation shoud successfully read last two sets and return
/// entries not found for deleted set [ Create Set1, Set2 - Delete Set1 - Create
/// Set3 ] - Read
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_3) {
    uint32_t num_vnics_per_set = PDS_MAX_VNIC;
    uint32_t num_teps_per_set = 50;
    uint32_t stepper = num_teps_per_set * num_vnics_per_set;
    ip_prefix_t vip_pfx;
    ip_prefix_t tip_pfx;
    remote_mapping_stepper_seed_t s1_seed = {0};
    remote_mapping_stepper_seed_t s2_seed = {0};
    remote_mapping_stepper_seed_t s3_seed = {0};

    pds_batch_params_t batch_params = {0};

    remote_mapping_stepper_seed_init(
        &s1_seed, api_test::g_encap_val, api_test::g_base_vnic_mac,
        api_test::g_vnic_cidr_v4, api_test::g_tep_cidr_v4);

    str2ipv4pfx((char *)api_test::g_vnic_cidr_v4.c_str(), &vip_pfx);
    step_up_ip_next(&vip_pfx, num_teps_per_set);
    str2ipv4pfx((char *)api_test::g_tep_cidr_v4.c_str(), &tip_pfx);
    step_up_ip_next(&tip_pfx, num_teps_per_set);
    remote_mapping_stepper_seed_init(&s2_seed, api_test::g_encap_val + stepper,
                                     api_test::g_base_vnic_mac + stepper,
                                     ippfx2str(&vip_pfx), ippfx2str(&tip_pfx));

    step_up_ip_next(&vip_pfx, num_teps_per_set);
    step_up_ip_next(&tip_pfx, num_teps_per_set);
    remote_mapping_stepper_seed_init(&s3_seed,
                                     api_test::g_encap_val + (stepper * 2),
                                     api_test::g_base_vnic_mac + (stepper * 2),
                                     ippfx2str(&vip_pfx), ippfx2str(&tip_pfx));

    // trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_mapping_util::many_create(
                    num_vnics_per_set, num_teps_per_set, api_test::g_vpc_id,
                    api_test::g_subnet_id, &s1_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_mapping_util::many_create(
                    num_vnics_per_set, num_teps_per_set,
                    api_test::g_vpc_id + num_teps_per_set,
                    api_test::g_subnet_id + num_teps_per_set,
                    &s2_seed) == sdk::SDK_RET_OK);
#if 0
    ASSERT_TRUE(remote_mapping_util::many_delete(num_vnics_per_set, num_teps_per_set, api_test::g_vpc_id,
                                                 &s1_seed) == sdk::SDK_RET_OK);
#endif
    ASSERT_TRUE(remote_mapping_util::many_create(
                    num_vnics_per_set, num_teps_per_set,
                    api_test::g_vpc_id + (num_teps_per_set * 2),
                    api_test::g_subnet_id + (num_teps_per_set * 2),
                    &s3_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(remote_mapping_util::many_read(
                    num_vnics_per_set, num_teps_per_set, api_test::g_vpc_id,
                    api_test::g_subnet_id, &s1_seed, PDS_ENCAP_TYPE_MPLSoUDP,
                    sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);

    ASSERT_TRUE(
        remote_mapping_util::many_read(num_vnics_per_set, num_teps_per_set,
                                       api_test::g_vpc_id + num_teps_per_set,
                                       api_test::g_subnet_id + num_teps_per_set,
                                       &s2_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_mapping_util::many_read(
                    num_vnics_per_set, num_teps_per_set,
                    api_test::g_vpc_id + (num_teps_per_set * 2),
                    api_test::g_subnet_id + (num_teps_per_set * 2),
                    &s3_seed) == sdk::SDK_RET_OK);

#if 0
    // cleanup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_mapping_util::many_delete(num_vnics_per_set, num_teps_per_set,
                                                 api_test::g_vpc_id + (num_teps_per_set * 2),
                                                 &s3_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_mapping_util::many_delete(num_vnics_per_set, num_teps_per_set,
                                                 api_test::g_vpc_id + num_teps_per_set,
                                                 &s2_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
#endif
}

/// \brief Create and delete remote mappings in two batches
/// The hardware should create and delete mappings correctly.
/// Validate using reads at each batch end
/// [ Create SetMax ] - Read - [ Delete SetMax ] - Read
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_4) {
    uint32_t num_vnics = PDS_MAX_VNIC;
    uint32_t num_teps = 700;
    remote_mapping_stepper_seed_t seed;
    pds_batch_params_t batch_params = {0};

    remote_mapping_stepper_seed_init(
        &seed, api_test::g_encap_val, api_test::g_base_vnic_mac,
        api_test::g_vnic_cidr_v4, api_test::g_tep_cidr_v4);
    // trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_mapping_util::many_create(
                    num_vnics, num_teps, api_test::g_vpc_id,
                    api_test::g_subnet_id, &seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(remote_mapping_util::many_read(
                    num_vnics, num_teps, api_test::g_vpc_id,
                    api_test::g_subnet_id, &seed) == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_mapping_util::many_delete(num_vnics, num_teps,
                                                 api_test::g_vpc_id,
                                                 &seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(remote_mapping_util::many_read(
                    num_vnics, num_teps, api_test::g_vpc_id,
                    api_test::g_subnet_id, &seed, PDS_ENCAP_TYPE_MPLSoUDP,
                    sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create and delete mix and match of mappings in two batches
/// [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_5) {
    uint32_t num_vnics_per_set = PDS_MAX_VNIC;
    uint32_t num_teps = 2;
    uint32_t stepper = num_teps * num_vnics_per_set;
    ip_prefix_t vip_pfx;
    ip_prefix_t tip_pfx;
    remote_mapping_stepper_seed_t s1_seed = {0};
    remote_mapping_stepper_seed_t s2_seed = {0};
    remote_mapping_stepper_seed_t s3_seed = {0};

    pds_batch_params_t batch_params = {0};

    remote_mapping_stepper_seed_init(
        &s1_seed, api_test::g_encap_val, api_test::g_base_vnic_mac,
        api_test::g_vnic_cidr_v4, api_test::g_tep_cidr_v4);

    str2ipv4pfx((char *)api_test::g_vnic_cidr_v4.c_str(), &vip_pfx);
    step_up_ip_next(&vip_pfx, num_teps);
    str2ipv4pfx((char *)api_test::g_tep_cidr_v4.c_str(), &tip_pfx);
    step_up_ip_next(&tip_pfx, num_teps);
    remote_mapping_stepper_seed_init(&s2_seed, api_test::g_encap_val + stepper,
                                     api_test::g_base_vnic_mac + stepper,
                                     ippfx2str(&vip_pfx), ippfx2str(&tip_pfx));

    step_up_ip_next(&vip_pfx, (num_teps * 2));
    step_up_ip_next(&tip_pfx, (num_teps * 2));
    remote_mapping_stepper_seed_init(&s3_seed,
                                     api_test::g_encap_val + (stepper * 2),
                                     api_test::g_base_vnic_mac + (stepper * 2),
                                     ippfx2str(&vip_pfx), ippfx2str(&tip_pfx));

    // trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_mapping_util::many_create(
                    num_vnics_per_set, num_teps, api_test::g_vpc_id,
                    api_test::g_subnet_id, &s1_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_mapping_util::many_create(
                    num_vnics_per_set, num_teps, api_test::g_vpc_id + num_teps,
                    api_test::g_subnet_id, &s2_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // ASSERT_TRUE(remote_mapping_util::many_read(num_vnics_per_set, num_teps,
    // api_test::g_vpc_id + num_teps,
    //                                          api_test::g_subnet_id, &s2_seed)
    //                                          == sdk::SDK_RET_OK);
    // ASSERT_TRUE(remote_mapping_util::many_read(num_vnics_per_set, num_teps,
    // api_test::g_vpc_id,
    //                                          api_test::g_subnet_id, &s1_seed)
    //                                          == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // ASSERT_TRUE(remote_mapping_util::many_delete(num_vnics_per_set, num_teps,
    // api_test::g_vpc_id,
    //                                            &s1_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_mapping_util::many_create(
                    num_vnics_per_set, num_teps,
                    api_test::g_vpc_id + (num_teps * 2), api_test::g_subnet_id,
                    &s3_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // ASSERT_TRUE(remote_mapping_util::many_read(342, num_teps,
    // api_test::g_vpc_id + (num_teps * 2),
    //                                          api_test::g_subnet_id, &s3_seed)
    //                                          == sdk::SDK_RET_OK);
    // ASSERT_TRUE(remote_mapping_util::many_read(num_vnics_per_set, num_teps,
    // api_test::g_vpc_id,
    //                                          api_test::g_subnet_id, &s1_seed,
    //                                          sdk::SDK_RET_ENTRY_FOUND) ==
    //                                          sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // ASSERT_TRUE(remote_mapping_util::many_delete(342, num_teps,
    // api_test::g_vpc_id + (num_teps * 2),
    //                                            &s3_seed) == sdk::SDK_RET_OK);
    // ASSERT_TRUE(remote_mapping_util::many_delete(num_vnics_per_set, num_teps,
    // api_test::g_vpc_id + num_teps,
    //                                            &s2_seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create maximum number of mappings in two batches
/// [ Create SetMax ] - [ Create SetMax ] - Read
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_neg_1) {
    uint32_t num_vnics = PDS_MAX_VNIC;
    uint32_t num_teps = 2;
    remote_mapping_stepper_seed_t seed;
    pds_batch_params_t batch_params = {0};

    remote_mapping_stepper_seed_init(
        &seed, api_test::g_encap_val, api_test::g_base_vnic_mac,
        api_test::g_vnic_cidr_v4, api_test::g_tep_cidr_v4);
    // trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_mapping_util::many_create(
                    num_vnics, num_teps, api_test::g_vpc_id,
                    api_test::g_subnet_id, &seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_mapping_util::many_create(
                    num_vnics, num_teps, api_test::g_vpc_id,
                    api_test::g_subnet_id, &seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // ASSERT_TRUE(remote_mapping_util::many_read(num_vnics, num_teps,
    // api_test::g_vpc_id,
    //                                          api_test::g_subnet_id, &seed) ==
    //                                          sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // ASSERT_TRUE(remote_mapping_util::many_delete(num_vnics, num_teps,
    // api_test::g_vpc_id,
    //                                            &seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create more than maximum number of mappings supported
/// [ Create SetMax+1] - Read
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_neg_2) {
    uint32_t num_vnics = PDS_MAX_VNIC;
    uint32_t num_teps = PDS_MAX_TEP;
    remote_mapping_stepper_seed_t seed;
    pds_batch_params_t batch_params = {0};

    remote_mapping_stepper_seed_init(
        &seed, api_test::g_encap_val, api_test::g_base_vnic_mac,
        api_test::g_vnic_cidr_v4, api_test::g_tep_cidr_v4);
    // trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_mapping_util::many_create(
                    num_vnics, num_teps, api_test::g_vpc_id,
                    api_test::g_subnet_id, &seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // ASSERT_TRUE(remote_mapping_util::many_read(num_vnics, num_teps,
    // api_test::g_vpc_id,
    //                                          api_test::g_subnet_id, &seed,
    //                                          sdk::SDK_RET_ENTRY_NOT_FOUND) ==
    //                                          sdk::SDK_RET_OK);
}

/// \brief Read and delete of a non-existing mappings should return entry not
/// found [Delete NonEx] - Read NonEx
TEST_F(mapping_test, DISABLED_remote_mapping_workflow_neg_3) {
    remote_mapping_stepper_seed_t seed;
    pds_batch_params_t batch_params = {0};
    pds_remote_mapping_info_t info = {0};

    remote_mapping_stepper_seed_init(
        &seed, api_test::g_encap_val, api_test::g_base_vnic_mac,
        api_test::g_vnic_cidr_v4, api_test::g_tep_cidr_v4);

    remote_mapping_util remote_obj(1, 1, seed.vnic_ip_stepper,
                                   seed.tep_ip_stepper, seed.vnic_mac_stepper);
    // trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // ASSERT_TRUE(remote_obj.read(&info, false) ==
    // sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Invalid batch shouldn't affect entries of previous batch
/// [ Create Set1 ] - [Delete Set1, Set2 ] - Read
TEST_F(mapping_test, DISABLED_remote_mapping_workflow_neg_4) {
    uint32_t num_vnics = 10;
    uint32_t num_teps = 1;
    remote_mapping_stepper_seed_t seed;
    pds_batch_params_t batch_params = {0};
    pds_remote_mapping_info_t info = {0};

    remote_mapping_stepper_seed_init(
        &seed, api_test::g_encap_val, api_test::g_base_vnic_mac,
        api_test::g_vnic_cidr_v4, api_test::g_tep_cidr_v4);

    // trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_mapping_util::many_create(
                    num_vnics, num_teps, api_test::g_vpc_id,
                    api_test::g_subnet_id, &seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    remote_mapping_util remote_obj(1, 1, seed.vnic_ip_stepper,
                                   seed.tep_ip_stepper, seed.vnic_mac_stepper);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // ASSERT_TRUE(remote_mapping_util::many_delete(num_vnics, num_teps,
    // api_test::g_vpc_id,
    //                                            &seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(remote_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // ASSERT_TRUE(remote_mapping_util::many_read(num_vnics, num_teps,
    // api_test::g_vpc_id,
    //                                          api_test::g_subnet_id, &seed) ==
    //                                          sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // ASSERT_TRUE(remote_mapping_util::many_delete(num_vnics, num_teps,
    // api_test::g_vpc_id,
    //                                            &seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// --------------------------END REMOTE MAPPINGS -----------------------

/// --------------------------END IPv4 MAPPINGS -----------------------
// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

static inline void
mapping_test_usage_print (char **argv)
{
    cout << "Usage : " << argv[0] << " -c <hal.json>" << endl;
    return;
}

static inline sdk_ret_t
mapping_test_options_validate (void)
{
    if (!api_test::g_cfg_file) {
        cerr << "HAL config file is not specified" << endl;
        return sdk::SDK_RET_ERR;
    }
    return sdk::SDK_RET_OK;
}

static void
mapping_test_options_parse (int argc, char **argv)
{
    int oc = -1;
    struct option longopts[] = {{"config", required_argument, NULL, 'c'},
                                {"help", no_argument, NULL, 'h'},
                                {0, 0, 0, 0}};

    while ((oc = getopt_long(argc, argv, ":hc:", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            api_test::g_cfg_file = optarg;
            break;
        default:    // ignore all other options
            break;
        }
    }
    return;
}

int
main (int argc, char **argv)
{
    mapping_test_options_parse(argc, argv);
    if (mapping_test_options_validate() != sdk::SDK_RET_OK) {
        mapping_test_usage_print(argv);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
