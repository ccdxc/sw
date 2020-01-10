//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_VNIC_HPP__
#define __TEST_API_UTILS_VNIC_HPP__

#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/feeder.hpp"

namespace test {
namespace api {

// Export variables
extern const uint64_t k_feeder_mac;
extern const uint32_t k_max_vnic;

// VNIC test feeder class
class vnic_feeder : public feeder {
public:
    // TODO: move to vnic_spec_t instead of below variables
    pds_vnic_id_t id;
    pds_vpc_key_t vpc;
    pds_subnet_id_t subnet_id;
    pds_encap_t vnic_encap;
    pds_encap_t fabric_encap;
    uint64_t mac_u64;
    bool src_dst_check;
    uint8_t tx_mirror_session_bmap;
    uint8_t rx_mirror_session_bmap;
    bool configure_policy;

    // Constructor
    vnic_feeder() {
        vpc.reset();
    }
    vnic_feeder(const vnic_feeder& feeder) {
        init(feeder.id, feeder.num_obj, feeder.mac_u64, feeder.vnic_encap.type,
             feeder.fabric_encap.type, feeder.src_dst_check,
             feeder.configure_policy);
    }

    // Initialize feeder with the base set of values
    void init(uint32_t id, uint32_t num_vnic = k_max_vnic,
              uint64_t mac = k_feeder_mac,
              pds_encap_type_t vnic_encap_type = PDS_ENCAP_TYPE_DOT1Q,
              pds_encap_type_t fabric_encap_type = PDS_ENCAP_TYPE_MPLSoUDP,
              bool src_dst_check = true, bool configure_policy = true);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Build routines
    void key_build(pds_vnic_key_t *key) const;
    void spec_build(pds_vnic_spec_t *spec) const;

    // Compare routines
    bool key_compare(const pds_vnic_key_t *key) const;
    bool spec_compare(const pds_vnic_spec_t *spec) const;
};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const pds_vnic_key_t *key) {
    os << " id: " << key->id;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_vnic_spec_t *spec) {
    os << &spec->key
       << " subnet id: " << spec->subnet.id
       << " vnic encap: " << pds_encap2str(&spec->vnic_encap)
       << " fabric encap: " << pds_encap2str(&spec->fabric_encap)
       << " mac: " << macaddr2str(spec->mac_addr)
       << " src dst check: " << spec->src_dst_check
       << " tx_mirror_session_bmap: " << +spec->tx_mirror_session_bmap
       << " rx_mirror_session_bmap: " << +spec->rx_mirror_session_bmap
       << " v4 meter id: " << spec->v4_meter.id
       << " v6 meter id: " << spec->v6_meter.id
       << " switch vnic: " << spec->switch_vnic;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_vnic_status_t *status) {
    os << " HW id: " << status->hw_id;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_vnic_stats_t *stats) {
    os << " rx pkts: " << stats->rx_pkts
       << " rx bytes: " << stats->rx_bytes
       << " tx pkts: " << stats->tx_pkts
       << " tx bytes: " << stats->tx_bytes;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_vnic_info_t *obj) {
    os << " VNIC info =>"
       << &obj->spec
       << &obj->status
       << &obj->stats
       << std::endl;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const vnic_feeder& obj) {
    os << "VNIC feeder =>"
       << " id: " << obj.id;
    return os;
}

// CRUD prototypes
API_CREATE(vnic);
API_READ(vnic);
API_UPDATE(vnic);
API_DELETE(vnic);

// Misc function prototypes
void sample_vnic_setup(pds_batch_ctxt_t bctxt);
void sample_vnic_teardown(pds_batch_ctxt_t bctxt);

}    // namespace api
}    // namespace test

#endif    // __TEST_API_UTILS_VNIC_HPP__
