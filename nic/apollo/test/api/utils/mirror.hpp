//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_MIRROR_HPP__
#define __TEST_API_UTILS_MIRROR_HPP__

#include "nic/apollo/api/include/pds_mirror.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/feeder.hpp"

namespace test {
namespace api {

// MIRROR test feeder class
class mirror_session_feeder : public feeder {
public:
    pds_obj_key_t key;
    pds_obj_key_t vpc;
    pds_mirror_session_type_t type;
    uint32_t snap_len;
    pds_obj_key_t tep;
    ip_addr_t dst_ip;
    ip_addr_t src_ip;
    uint32_t span_id;
    uint32_t dscp;
    pds_obj_key_t interface;
    pds_encap_t encap;

    //Constructor
    mirror_session_feeder() { };

    // initalize feeder with base set of values
    void init(pds_obj_key_t key, uint8_t max_ms,
              pds_obj_key_t interface, uint16_t vlan_tag,
              std::string src_ip, pds_obj_key_t tep,
              uint32_t span_id = 1, uint32_t dscp = 1);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Build routines
    void key_build(pds_obj_key_t *key) const;
    void spec_build(pds_mirror_session_spec_t *spec) const;

    // Compare routines
    bool key_compare(const pds_obj_key_t *key) const;
    bool spec_compare(const pds_mirror_session_spec_t *spec) const;
    bool status_compare(const pds_mirror_session_status_t *status1,
                        const pds_mirror_session_status_t *status2) const;

    bool read_unsupported(void) const {
        return (::asic_mock_mode() ? true : false);
    }

};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const mirror_session_feeder& obj) {
    os << "MIRROR feeder =>"
       << "id: " << std::string(obj.key.str()) << "  "
       << "type: " << obj.type
       << "snap_len: " << obj.snap_len;
    return os;
}

// CRUD prototypes
API_CREATE(mirror_session);
API_READ(mirror_session);
API_UPDATE(mirror_session);
API_DELETE(mirror_session);

}    // namespace api
}    // namespace test

#endif    // __TEST_API_UTILS_MIRROR_HPP__
