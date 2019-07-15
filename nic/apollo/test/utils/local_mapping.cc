//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/local_mapping.hpp"
#include "nic/apollo/test/utils/utils.hpp"

namespace api_test {

local_mapping_util::local_mapping_util() {
    __init();
}

local_mapping_util::local_mapping_util(
    uint32_t vpc_id_, uint32_t sub_id_, std::string vnic_ip_,
    pds_vnic_id_t vnic_id_, uint64_t mac_u64, pds_encap_type_t encap_type_,
    uint32_t encap_val_, bool public_ip_valid, std::string pub_ip) {

    __init();
    mac_addr_t mac;
    this->vpc_id = vpc_id_;
    this->sub_id = sub_id_;
    this->vnic_id = vnic_id_;
    this->vnic_ip = vnic_ip_;
    this->is_public_ip_valid = public_ip_valid;
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
local_mapping_util::create(void) const {
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
local_mapping_util::update(void) const {
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

    return pds_local_mapping_update(&local_spec);
}

sdk::sdk_ret_t
local_mapping_util::read(pds_local_mapping_info_t *info) const {
    sdk_ret_t rv = sdk::SDK_RET_OK;
    pds_mapping_key_t key = {0};


    memset(info, 0, sizeof(pds_local_mapping_info_t));
    key.vpc.id = this->vpc_id;
    extract_ip_addr(this->vnic_ip.c_str(), &key.ip_addr);
    rv = pds_local_mapping_read(&key, info);
    if (rv != SDK_RET_OK) {
        return rv;
    }

    if (::capri_mock_mode() == true)
        return SDK_RET_OK;

    std::cout << "Mapping info:" << "\n";
    std::cout << "HW: vpc id: " << info->spec.key.vpc.id
              << " SW: vpc id: " << this->vpc_id << "\n";
    std::cout << "HW: vnic ip: " << ipaddr2str(&info->spec.key.ip_addr)
              << " SW: vnic ip: " << this->vnic_ip.c_str() << "\n";
    if (this->is_public_ip_valid)
        std::cout << "HW: public ip: " << ipaddr2str(&info->spec.public_ip)
                  << " SW: public ip: " << this->public_ip.c_str() << "\n";
    std::cout << "HW: encap_type: " << info->spec.fabric_encap.type
              << " HW: encap_val: " << info->spec.fabric_encap.val.value << "\n";

    SDK_ASSERT(this->vpc_id == info->spec.key.vpc.id);
    SDK_ASSERT(strcmp(this->vnic_ip.c_str(),
                      ipaddr2str(&info->spec.key.ip_addr)) == 0);
    if (this->encap_type == PDS_ENCAP_TYPE_MPLSoUDP) {
        SDK_ASSERT(PDS_ENCAP_TYPE_MPLSoUDP == info->spec.fabric_encap.type);
        SDK_ASSERT(this->mpls_slot == info->spec.fabric_encap.val.mpls_tag);
    } else if (this->encap_type == PDS_ENCAP_TYPE_VXLAN) {
        SDK_ASSERT(PDS_ENCAP_TYPE_VXLAN == info->spec.fabric_encap.type);
        SDK_ASSERT(this->vxlan_id == info->spec.fabric_encap.val.vnid);
    }
    if (this->is_public_ip_valid) {
        SDK_ASSERT(true == info->spec.public_ip_valid);
        SDK_ASSERT(strcmp(this->public_ip.c_str(),
                          ipaddr2str(&info->spec.public_ip)) == 0);
    } else {
        SDK_ASSERT(false == info->spec.public_ip_valid);
    }

    return rv;
}

sdk::sdk_ret_t
local_mapping_util::del(void) const {
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
mapping_util_object_stepper (utils_op_t op, local_mapping_stepper_seed_t *seed,
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
    if (seed->is_public_ip_valid) {
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
    SDK_ASSERT(seed->num_vnics * seed->num_ip_per_vnic <= (PDS_MAX_VNIC * PDS_MAX_VNIC_IP));
    if (seed->vnic_id_stepper == 0)
        seed->vnic_id_stepper = 1;
    for (uint16_t vnic_indx = seed->vnic_id_stepper;
         vnic_indx < (seed->vnic_id_stepper + seed->num_vnics); vnic_indx++) {
        for (uint16_t vip_indx = 0; vip_indx < seed->num_ip_per_vnic; vip_indx++) {
            local_mapping_util mapping_obj(seed->vpc_id, seed->subnet_id,
                                           vnic_ip, vnic_indx, curr_vnic_mac,
                                           seed->encap_type, curr_encap_val,
                                           seed->is_public_ip_valid, public_ip);

            switch (op) {
            case OP_MANY_CREATE:
                rv = mapping_obj.create();
                break;
            case OP_MANY_UPDATE:
                rv = mapping_obj.update();
                break;
            case OP_MANY_DELETE:
                rv = mapping_obj.del();
                break;
            case OP_MANY_READ:
                rv = mapping_obj.read(&info);
                break;
            default:
                return sdk::SDK_RET_INVALID_OP;
            }

            if (rv != expected_result) {
                return sdk::SDK_RET_ERR;
            } else {
                rv = sdk::SDK_RET_OK;
            }

            // Increment to next mapping i.e. next vnic ip
            if (ip_version(vnic_ip.c_str()) == IP_AF_IPV6) {
                // ippfx.addr.addr.v6_addr += 1;
            } else {
                ipaddr.addr.v4_addr += 1;
                vnic_ip = ipv4addr2str(ipaddr.addr.v4_addr);
            }

            if (seed->is_public_ip_valid) {
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
local_mapping_util::many_create(local_mapping_stepper_seed_t *seed) {
    return (mapping_util_object_stepper(OP_MANY_CREATE, seed));
}

sdk::sdk_ret_t
local_mapping_util::many_update(local_mapping_stepper_seed_t *seed) {
    return (mapping_util_object_stepper(OP_MANY_UPDATE, seed));
}

sdk::sdk_ret_t
local_mapping_util::many_delete(local_mapping_stepper_seed_t *seed) {
    return (mapping_util_object_stepper(OP_MANY_DELETE, seed));
}

sdk::sdk_ret_t
local_mapping_util::many_read(local_mapping_stepper_seed_t *seed,
                              sdk::sdk_ret_t expected_result) {
    return (mapping_util_object_stepper(OP_MANY_READ, seed, expected_result));
}

sdk::sdk_ret_t
local_mapping_util::local_mapping_stepper_seed_init(local_mapping_stepper_seed_t *seed,
                                      uint32_t vpc_id, uint32_t subnet_id,
                                      uint16_t base_vnic_id, pds_encap_type_t encap_type,
                                      uint32_t base_encap_val, uint64_t base_mac_64,
                                      std::string vnic_ip_cidr, bool public_ip_valid,
                                      std::string public_ip_cidr) {

    seed->vpc_id = vpc_id;
    seed->subnet_id = subnet_id;
    seed->vnic_id_stepper = base_vnic_id;
    seed->vnic_ip_stepper = vnic_ip_cidr;
    seed->is_public_ip_valid = public_ip_valid;
    seed->public_ip_stepper = public_ip_cidr;
    seed->encap_type = encap_type;
    seed->encap_val_stepper = base_encap_val;
    seed->vnic_mac_stepper = base_mac_64;
    return sdk::SDK_RET_OK;
}

void
local_mapping_util::__init(void)
{
    this->vpc_id = 0;
    this->sub_id = 0;
    this->vnic_id = 0;
    this->vnic_ip = "0";
    this->is_public_ip_valid = false;
    this->public_ip = "0";
    this->encap_type = PDS_ENCAP_TYPE_MPLSoUDP;
    this->mpls_slot = 0;
    this->vnic_mac = "0";
}

}    // namespace api_test
