//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the local IP map test utility routines implementation
///
//----------------------------------------------------------------------------

#include "iostream"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/test/utils/local_mapping.hpp"

namespace api_test {

local_mapping_util::local_mapping_util() {}

local_mapping_util::local_mapping_util(
    pds_vpc_id_t vpc_id_, pds_subnet_id_t sub_id_, std::string vnic_ip_,
    pds_vnic_id_t vnic_id_, uint64_t mac_u64, pds_encap_type_t encap_type_,
    uint32_t encap_val_, bool public_ip_valid, std::string pub_ip) {
    mac_addr_t mac;
    this->vpc_id = vpc_id_;
    this->vnic_ip = vnic_ip_;
    this->sub_id = sub_id_;
    this->is_public_ip_valid = public_ip_valid;
    this->vnic_id = vnic_id_;
    this->public_ip = pub_ip;
    this->encap_type = encap_type_;
    switch (encap_type_) {
    case PDS_ENCAP_TYPE_MPLSoUDP:
        this->mpls_slot = encap_val_;
        break;
    case PDS_ENCAP_TYPE_VXLAN:
        this->vxlan_id = encap_val_;
        break;
    default:
        break;
    }
    MAC_UINT64_TO_ADDR(mac, mac_u64);
    this->vnic_mac = macaddr2str(mac);
}

local_mapping_util::~local_mapping_util() {}

sdk::sdk_ret_t
local_mapping_util::create(void) {
    pds_local_mapping_spec_t local_spec = {0};

    local_spec.key.vpc.id = this->vpc_id;
    extract_ip_addr(this->vnic_ip.c_str(), &local_spec.key.ip_addr);
    local_spec.subnet.id = this->sub_id;

    switch (this->encap_type) {
    case PDS_ENCAP_TYPE_MPLSoUDP:
        local_spec.fabric_encap.type = PDS_ENCAP_TYPE_MPLSoUDP;
        local_spec.fabric_encap.val.mpls_tag = this->mpls_slot;
        break;
    case PDS_ENCAP_TYPE_VXLAN:
        local_spec.fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
        local_spec.fabric_encap.val.vnid = this->vxlan_id;
        break;
    default:
        break;
    }

    mac_str_to_addr((char *)this->vnic_mac.c_str(), local_spec.vnic_mac);
    local_spec.vnic.id = this->vnic_id;
    local_spec.public_ip_valid = false;
    if (this->is_public_ip_valid) {
        extract_ip_addr(this->public_ip.c_str(), &local_spec.public_ip);
        local_spec.public_ip_valid = true;
    }

    return pds_local_mapping_create(&local_spec);
}

sdk::sdk_ret_t
local_mapping_util::read(pds_local_mapping_info_t *info, bool compare_spec) {
    // uint64_t mac1, mac2;
    sdk_ret_t rv = sdk::SDK_RET_OK;
    pds_mapping_key_t key = {0};
    // mac_addr_t mac;

    // mac_str_to_addr((char *)this->vnic_mac.c_str(), mac);
    key.vpc.id = this->vpc_id;
    extract_ip_addr(this->vnic_ip.c_str(), &key.ip_addr);
    rv = pds_local_mapping_read(&key, info);
    if (rv != SDK_RET_OK) {
        return rv;
    }

    std::cout << "Mapping info:" << "\n";
    std::cout << "HW: vpc id: " << info->spec.key.vpc.id
              << " SW: vpc_id: " << this->vpc_id << "\n";
    std::cout << "HW: vnic ip: " << ipaddr2str(&info->spec.key.ip_addr)
              << " SW: vpc_id: " << this->vnic_ip.c_str() << "\n";
    if (this->is_public_ip_valid)
        std::cout << "HW: public ip: " << ipaddr2str(&info->spec.public_ip)
                  << " SW: public ip: " << this->public_ip.c_str() << "\n";
    std::cout << "HW: mpls_type: " << info->spec.fabric_encap.type
              << " SW: mpls_tag: " << this->mpls_slot << "\n";

    if (compare_spec) {
        // mac1 = MAC_TO_UINT64(mac);
        // mac2 = MAC_TO_UINT64(info->spec.vnic_mac);
        SDK_ASSERT(this->vpc_id == info->spec.key.vpc.id);
        // SDK_ASSERT(this->sub_id == info->spec.subnet.id); // not stored in hw
        // table
        SDK_ASSERT(strcmp(this->vnic_ip.c_str(),
                          ipaddr2str(&info->spec.key.ip_addr)) == 0);
        if (this->encap_type == PDS_ENCAP_TYPE_MPLSoUDP) {
            SDK_ASSERT(PDS_ENCAP_TYPE_MPLSoUDP == info->spec.fabric_encap.type);
            SDK_ASSERT(this->mpls_slot == info->spec.fabric_encap.val.mpls_tag);
        } else if (this->encap_type == PDS_ENCAP_TYPE_VXLAN) {
            SDK_ASSERT(PDS_ENCAP_TYPE_VXLAN == info->spec.fabric_encap.type);
            SDK_ASSERT(this->vxlan_id == info->spec.fabric_encap.val.vnid);
        }
        // SDK_ASSERT((mac1 == mac2) == 0); // not stored in hw table
        if (this->is_public_ip_valid) {
            SDK_ASSERT(true == info->spec.public_ip_valid);
            SDK_ASSERT(strcmp(this->public_ip.c_str(),
                              ipaddr2str(&info->spec.public_ip)) == 0);
            // SDK_ASSERT(this->vnic_id == info->spec.vnic.id); // Read returns
            // hw_id
        } else {
            SDK_ASSERT(false == info->spec.public_ip_valid);
        }
    }

    return rv;
}

sdk::sdk_ret_t
local_mapping_util::del(void) {
    pds_mapping_key_t key;

    key.vpc.id = vpc_id;
    extract_ip_addr(vnic_ip.c_str(), &key.ip_addr);
    return pds_local_mapping_delete(&key);
}

/// \brief get <num_vnics> per subnet and <num_ip_per_vnic>
/// and create/delete/read maximum mappings for
/// given <vpc_id, sub_id>
/// vnic_ip derived from vpc_ip
/// vnic_mac calculated and incremented automatically
static inline sdk::sdk_ret_t
mapping_util_object_stepper (
    utils_op_t op, uint32_t num_ip_per_vnic, uint32_t num_vnics,
    pds_vpc_id_t vpc_id, pds_subnet_id_t sub_id,
    local_mapping_stepper_seed_t *seed,
    pds_encap_type_t encap_type = PDS_ENCAP_TYPE_MPLSoUDP,
    bool is_public_ip_valid = false,
    sdk::sdk_ret_t expected_result = sdk::SDK_RET_OK) {
    uint32_t curr_encap_val;
    uint64_t curr_vnic_mac;
    std::string vnic_ip = "0.0.0.0";
    std::string public_ip = "0.0.0.0";
    sdk::sdk_ret_t rv = sdk::SDK_RET_OK;
    pds_local_mapping_info_t info = {0};
    ip_prefix_t ippfx;
    ip_addr_t ipaddr;
    ip_addr_t ipaddr_next;
    ip_prefix_t pub_ippfx;
    ip_addr_t pub_ipaddr;

    // Calculate vnic ip from vnic ip string
    extract_ip_pfx((char *)seed->vnic_ip_stepper.c_str(), &ippfx);
    ipaddr = ippfx.addr;
    if (ip_version(seed->vnic_ip_stepper.c_str()) == IP_AF_IPV6) {
        vnic_ip = ipv6addr2str(ipaddr.addr.v6_addr);
    } else {
        vnic_ip = ipv4addr2str(ipaddr.addr.v4_addr);
    }

    // Max mapping limit is reached when reaches
    // this ip address i.e. <ipaddr_next>
    ip_prefix_ip_high(&ippfx, &ipaddr_next);

    // Store public ip if <is_public_ip_valid> true
    if (is_public_ip_valid) {
        extract_ip_pfx((char *)seed->public_ip_stepper.c_str(), &pub_ippfx);
        pub_ipaddr = pub_ippfx.addr;
        if (ip_version(seed->public_ip_stepper.c_str()) == IP_AF_IPV6) {
            public_ip = ipv6addr2str(pub_ipaddr.addr.v6_addr);
        } else {
            public_ip = ipv4addr2str(pub_ipaddr.addr.v4_addr);
        }
    }

    curr_vnic_mac = seed->vnic_mac_stepper;
    curr_encap_val = seed->encap_val_stepper;

    // Max local mappings limit
    SDK_ASSERT(num_vnics * num_ip_per_vnic <= (PDS_MAX_VNIC * PDS_MAX_VNIC_IP));
    if (seed->vnic_id_stepper == 0)
        seed->vnic_id_stepper = 1;
    for (uint16_t vnic_indx = seed->vnic_id_stepper;
         vnic_indx < (seed->vnic_id_stepper + num_vnics); vnic_indx++) {
        for (uint16_t vip_indx = 0; vip_indx < num_ip_per_vnic; vip_indx++) {
            local_mapping_util mapping_obj(
                vpc_id, sub_id, vnic_ip, vnic_indx, curr_vnic_mac, encap_type,
                curr_encap_val, is_public_ip_valid, public_ip);
            std::cout << "vpc_id: " << vpc_id << " & vnic_indx:" << vnic_indx
                      << "\n";
            std::cout << "vnic_ip:" << vnic_ip << "\n";
            // std::cout << "encap_val:" << curr_encap_val << "and" <<
            // "vnic_mac:" << curr_vnic_mac << "\n";
            switch (op) {
            case OP_MANY_CREATE:
                rv = mapping_obj.create();
                break;
            case OP_MANY_DELETE:
                rv = mapping_obj.del();
                break;
            case OP_MANY_READ:
                // TODO: read vnic id from remote table in impl
                memset(&info, 0, sizeof(info));
                info.spec.vnic.id = vnic_indx;
                rv = mapping_obj.read(&info);
                break;
            default:
                return sdk::SDK_RET_INVALID_OP;
            }

            if (rv != expected_result) {
                return rv;
            }

            // Increment to next mapping i.e. next vnic ip
            if (ip_version(vnic_ip.c_str()) == IP_AF_IPV6) {
                // ippfx.addr.addr.v6_addr += 1;
            } else {
                ipaddr.addr.v4_addr += 1;
                vnic_ip = ipv4addr2str(ipaddr.addr.v4_addr);
            }

            if (is_public_ip_valid) {
                if (ip_version(public_ip.c_str()) == IP_AF_IPV6) {
                    public_ip = ipv6addr2str(pub_ipaddr.addr.v6_addr);
                } else {
                    pub_ipaddr.addr.v4_addr += 1;
                    public_ip = ipv4addr2str(pub_ipaddr.addr.v4_addr);
                }
            }

            // Reached max mapping limit as per ip_prefix len
            if (ipaddr.addr.v4_addr == ipaddr_next.addr.v4_addr) {
                SDK_ASSERT(0);
                return rv;
            }
        }
        curr_vnic_mac++;
        curr_encap_val++;
    }
    return rv;
}

sdk::sdk_ret_t
local_mapping_util::many_create(uint16_t num_ip_per_vnic, uint16_t num_vnics,
                                pds_vpc_id_t vpc_id, pds_subnet_id_t sub_id,
                                local_mapping_stepper_seed_t *seed,
                                pds_encap_type_t encap_type,
                                bool is_public_ip_valid,
                                std::string public_ip_cidr_str) {
    return (mapping_util_object_stepper(OP_MANY_CREATE, num_ip_per_vnic,
                                        num_vnics, vpc_id, sub_id, seed,
                                        encap_type, is_public_ip_valid));
}

sdk::sdk_ret_t
local_mapping_util::many_delete(uint16_t num_ip_per_vnic, uint16_t num_vnics,
                                pds_vpc_id_t vpc_id,
                                local_mapping_stepper_seed_t *seed) {
    return (mapping_util_object_stepper(OP_MANY_DELETE, num_ip_per_vnic,
                                        num_vnics, vpc_id, 0, seed));
}

sdk::sdk_ret_t
local_mapping_util::many_read(uint16_t num_ip_per_vnic, uint16_t num_vnics,
                              pds_vpc_id_t vpc_id, pds_subnet_id_t sub_id,
                              local_mapping_stepper_seed_t *seed,
                              pds_encap_type_t encap_type,
                              bool is_public_ip_valid,
                              sdk::sdk_ret_t expected_result) {
    return (mapping_util_object_stepper(
        OP_MANY_READ, num_ip_per_vnic, num_vnics, vpc_id, sub_id, seed,
        encap_type, is_public_ip_valid, expected_result));
}

}    // namespace api_test
