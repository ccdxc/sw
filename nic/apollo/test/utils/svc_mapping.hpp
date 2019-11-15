//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#ifndef __TEST_UTILS_SVC_MAPPING_HPP__
#define __TEST_UTILS_SVC_MAPPING_HPP__

#include "nic/apollo/api/include/pds_service.hpp"
#include "nic/apollo/api/service_utils.hpp"
#include "nic/apollo/test/utils/api_base.hpp"
#include "nic/apollo/test/utils/feeder.hpp"

namespace api_test {

// svc mapping test feeder class
class svc_mapping_feeder : public feeder {
public:
    pds_svc_mapping_spec_t spec;

    // Constructor
    svc_mapping_feeder() { };
    svc_mapping_feeder(const svc_mapping_feeder& feeder) {
        init(ipaddr2str(&feeder.spec.vip),
             feeder.spec.svc_port, feeder.spec.key.vpc.id,
             ipaddr2str(&feeder.spec.key.backend_ip),
             feeder.spec.key.backend_port,
             ipaddr2str(&feeder.spec.backend_provider_ip), feeder.num_obj);
    }

    // Initialize feeder with the base set of values
    void init(std::string vip_str, uint16_t svc_port,
              int backend_vpc_id, std::string backend_ip,
              uint16_t backend_port, std::string backend_pip,
              uint32_t num_svc_mapping=100);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Build routines
    void key_build(pds_svc_mapping_key_t *key) const;
    void spec_build(pds_svc_mapping_spec_t *spec) const;

    // Compare routines
    bool key_compare(const pds_svc_mapping_key_t *key) const;
    bool spec_compare(const pds_svc_mapping_spec_t *spec) const;
};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const svc_mapping_feeder& obj) {
    os << "Svc Mapping feeder =>" << &obj.spec << " ";
    return os;
}

// CRUD prototypes
API_CREATE(svc_mapping);
API_READ(svc_mapping);
API_UPDATE(svc_mapping);
API_DELETE(svc_mapping);

}    // namespace api_test

#endif    // __TEST_UTILS_SVC_MAPPING_HPP__
