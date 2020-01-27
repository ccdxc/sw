//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/test/api/utils/tep.hpp"

namespace test {
namespace api {

// globals
constexpr pds_encap_t k_default_tep_encap = {PDS_ENCAP_TYPE_MPLSoUDP, 100};
constexpr pds_encap_t k_zero_encap = {PDS_ENCAP_TYPE_NONE, 0};
constexpr uint64_t k_tep_mac = 0x0E0D0A0B0200;
const pds_obj_key_t k_base_nh_key = int2pdsobjkey(1);
const pds_obj_key_t k_base_nh_group_key = int2pdsobjkey(1);
const uint16_t k_max_tep = apulu() ? 2048 : PDS_MAX_TEP;
static constexpr bool k_nat = FALSE;
static const std::string k_base_dipi = "50::50:1:1";

//----------------------------------------------------------------------------
// TEP feeder class routines
//----------------------------------------------------------------------------


void
tep_feeder::init(uint32_t id, std::string ip_str, uint32_t num_tep,
                 pds_encap_t encap, bool nat, pds_tep_type_t type,
                 std::string dipi_str, uint64_t dmac) {
    memset(&this->spec, 0, sizeof(pds_tep_spec_t));
    this->spec.key = int2pdsobjkey(id);
    this->spec.type = type;
    extract_ip_addr(ip_str.c_str(), &this->spec.remote_ip);
    extract_ip_addr(dipi_str.c_str(), &this->spec.ip_addr);
    MAC_UINT64_TO_ADDR(this->spec.mac, dmac);
    this->spec.encap = encap;
    this->spec.nat = nat;
    this->num_obj = num_tep;
}

void
tep_feeder::init(uint32_t id, uint64_t dmac, std::string ip_str,
                 uint32_t num_tep, pds_nh_type_t nh_type,
                 pds_obj_key_t base_nh,
                 pds_obj_key_t base_nh_group) {
    memset(&this->spec, 0, sizeof(pds_tep_spec_t));
    this->spec.key = int2pdsobjkey(id);
    extract_ip_addr(ip_str.c_str(), &this->spec.remote_ip);
    MAC_UINT64_TO_ADDR(this->spec.mac, dmac);
    this->spec.nh_type = nh_type;
    switch (nh_type) {
    case PDS_NH_TYPE_UNDERLAY_ECMP:
        this->spec.nh_group = base_nh_group;
        break;
    case PDS_NH_TYPE_UNDERLAY:
        this->spec.nh = base_nh;
        break;
    default:
        break;
    }
    this->num_obj = num_tep;
}

void
tep_feeder::iter_next(int width) {
    this->spec.key = int2pdsobjkey(pdsobjkey2int(this->spec.key) + width);
    if (!ip_addr_is_zero(&this->spec.ip_addr))
        test::increment_ip_addr(&this->spec.ip_addr, width);
    increment_ip_addr(&this->spec.remote_ip, width);
    if (is_mac_set(this->spec.mac))
        test::increment_mac_addr(this->spec.mac, width);
    if (!ip_addr_is_zero(&this->spec.remote_svc_public_ip))
        test::increment_ip_addr(&this->spec.remote_svc_public_ip, width);
    switch (this->spec.nh_type) {
    case PDS_NH_TYPE_UNDERLAY_ECMP:
        this->spec.nh_group = int2pdsobjkey(pdsobjkey2int(this->spec.nh_group) + width);
        if (pdsobjkey2int(this->spec.nh_group) >= PDS_MAX_NEXTHOP_GROUP) {
            this->spec.nh_group = int2pdsobjkey(1);
        }
        break;
    case PDS_NH_TYPE_UNDERLAY:
        this->spec.nh = int2pdsobjkey(pdsobjkey2int(this->spec.nh) + width);
        if (pdsobjkey2int(this->spec.nh) >= PDS_MAX_NEXTHOP) {
            this->spec.nh = int2pdsobjkey(1);
        }
        break;
    default:
        break;
    }
    cur_iter_pos += width;
}

void
tep_feeder::key_build(pds_obj_key_t *key) const {
    memcpy(key, &this->spec.key, sizeof(pds_obj_key_t));
}

void
tep_feeder::spec_build(pds_tep_spec_t *spec) const {
    memcpy(spec, &this->spec, sizeof(pds_tep_spec_t));
}

bool
tep_feeder::key_compare(const pds_obj_key_t *key) const {
    return *key == this->spec.key;
}

bool
tep_feeder::spec_compare(const pds_tep_spec_t *spec) const {
    ip_addr_t tep_dipi = this->spec.ip_addr, spec_ip = spec->ip_addr;
    ip_addr_t tep_ip = this->spec.remote_ip, spec_remote_ip = spec->remote_ip;
    ip_addr_t remote_svc_public_ip = this->spec.remote_svc_public_ip;
    ip_addr_t spec_remote_svc_public_ip = spec->remote_svc_public_ip;
    pds_encap_t tep_encap = this->spec.encap;
    pds_encap_t spec_encap = spec->encap;
    pds_encap_t spec_remote_svc_encap = spec->remote_svc_encap;
    pds_encap_t tep_remote_svc_encap = k_zero_encap;

    // TODO: move to memcmp

    if (this->spec.vpc != spec->vpc)
        return false;

    if (!IPADDR_EQ(&tep_dipi, &spec_ip))
        return false;

    if (!IPADDR_EQ(&tep_ip, &spec_remote_ip))
        return false;

    if (MAC_TO_UINT64(this->spec.mac) != MAC_TO_UINT64(spec->mac))
        return false;

    if (this->spec.type != spec->type)
        return false;

    if (!test::pdsencap_isequal(&tep_encap, &spec_encap))
        return false;

    if (this->spec.nat != spec->nat)
        return false;

    if (this->spec.remote_svc != spec->remote_svc)
        return false;

    if (!test::pdsencap_isequal(&tep_remote_svc_encap, &spec_remote_svc_encap))
        return false;

    if (!IPADDR_EQ(&remote_svc_public_ip, &spec_remote_svc_public_ip))
        return false;

    if (this->spec.nh_type != spec->nh_type)
        return false;

#if 0
    switch (spec->nh_type) {
    case PDS_NH_TYPE_UNDERLAY_ECMP:
        if (this->spec.nh_group != spec->nh_group)
            return false;
        break;
    case PDS_NH_TYPE_UNDERLAY:
        if (this->spec.nh != spec->nh)
            return false;
        break;
    default:
        break;
    }
#endif

    return true;
}

//----------------------------------------------------------------------------
// Misc routines
//----------------------------------------------------------------------------

// do not modify these sample values as rest of system is sync with these
static tep_feeder k_tep_feeder;

static inline void tep_feeder_init(uint32_t tep_id, std::string ip_str,
                                   uint32_t num_tep)
{
    if (artemis()) {
        k_tep_feeder.init(tep_id, ip_str, k_max_tep, k_zero_encap,
                          k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, k_tep_mac);
    } else if (apulu()) {
        k_tep_feeder.init(tep_id, k_tep_mac, ip_str, num_tep);
    } else if (apollo()) {
        k_tep_feeder.init(tep_id, ip_str, num_tep);
    }
}

void sample_tep_setup(pds_batch_ctxt_t bctxt, uint32_t tep_id,
                      std::string ip_str, uint32_t num_tep) {
    // setup and teardown parameters should be in sync
    tep_feeder_init(tep_id, ip_str, num_tep);
    many_create(bctxt, k_tep_feeder);
}

void sample_tep_validate(uint32_t tep_id, std::string ip_str,
                         uint32_t num_tep) {
    tep_feeder_init(tep_id, ip_str, num_tep);
    many_read(k_tep_feeder);
}

void sample_tep_teardown(pds_batch_ctxt_t bctxt, uint32_t tep_id,
                         std::string ip_str, uint32_t num_tep) {
    tep_feeder_init(tep_id, ip_str, num_tep);
    many_delete(bctxt, k_tep_feeder);
}

}    // namespace api
}    // namespace test
