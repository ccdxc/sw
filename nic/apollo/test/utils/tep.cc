//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/test/utils/tep.hpp"
#include "nic/apollo/test/utils/utils.hpp"

namespace api_test {

constexpr pds_encap_t k_default_tep_encap = {PDS_ENCAP_TYPE_MPLSoUDP, 100};
constexpr pds_tep_type_t k_default_tep_type = PDS_TEP_TYPE_WORKLOAD;
constexpr pds_encap_t k_zero_encap = {PDS_ENCAP_TYPE_NONE, 0};

//----------------------------------------------------------------------------
// TEP feeder class routines
//----------------------------------------------------------------------------

void
tep_feeder::init(std::string ip_str, uint32_t num_tep, pds_encap_t encap,
                 bool nat, pds_tep_type_t type, std::string dipi_str,
                 uint64_t dmac) {
    this->type = type;
    extract_ip_addr(ip_str.c_str(), &this->ip);
    extract_ip_addr(dipi_str.c_str(), &this->dipi);
    this->dmac = dmac;
    this->encap = encap;
    this->nat = nat;
    this->num_obj = num_tep;
}

void
tep_feeder::iter_next(int width) {
    ip.addr.v4_addr += width;
    if (!ip_addr_is_zero(&dipi))
        dipi.addr.v6_addr.addr64[1] += width;
    if (dmac)
        dmac += width;
    cur_iter_pos++;
}

void
tep_feeder::key_build(pds_tep_key_t *key) const {
    memset(key, 0, sizeof(pds_tep_key_t));
    key->ip_addr = this->ip;
}

void
tep_feeder::spec_build(pds_tep_spec_t *spec) const {
    memset(spec, 0, sizeof(pds_tep_spec_t));
    this->key_build(&spec->key);
    spec->type = this->type;
    spec->encap = this->encap;
    spec->nat = this->nat;
    spec->key.ip_addr = this->ip;
    spec->ip_addr = this->dipi;
    MAC_UINT64_TO_ADDR(spec->mac, this->dmac);
}

bool
tep_feeder::key_compare(const pds_tep_key_t *key) const {
    ip_addr_t tep_ip = this->ip, key_ip = key->ip_addr;

    return (IPADDR_EQ(&key_ip, &tep_ip));
}

bool
tep_feeder::spec_compare(const pds_tep_spec_t *spec) const {
    ip_addr_t tep_dipi = this->dipi, spec_ip = spec->ip_addr;
    pds_encap_t tep_encap = ::apollo() ? this->encap : k_zero_encap;
    pds_encap_t spec_encap = spec->encap;

    if (!IPADDR_EQ(&tep_dipi, &spec_ip))
        return false;

    if (this->type != spec->type)
        return false;

    if (!api::pdsencap_isequal(&tep_encap, &spec_encap))
        return false;

    if (this->nat != spec->nat)
        return false;

    if (this->dmac != MAC_TO_UINT64(spec->mac))
        return false;

    return true;
}

//----------------------------------------------------------------------------
// Misc routines
//----------------------------------------------------------------------------

// do not modify these sample values as rest of system is sync with these
static tep_feeder k_tep_feeder;

void sample_tep_setup(std::string ip_str, uint32_t num_tep) {
    // setup and teardown parameters should be in sync
    k_tep_feeder.init(ip_str, num_tep);
    many_create(k_tep_feeder);
}

void sample_tep_validate(std::string ip_str, uint32_t num_tep) {
    k_tep_feeder.init(ip_str, num_tep);
    many_read(k_tep_feeder);
}

void sample_tep_teardown(std::string ip_str, uint32_t num_tep) {
    k_tep_feeder.init(ip_str, num_tep);
    many_delete(k_tep_feeder);
}

}    // namespace api_test
