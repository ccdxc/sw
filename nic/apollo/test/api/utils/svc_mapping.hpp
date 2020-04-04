//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_SVC_MAPPING_HPP__
#define __TEST_API_UTILS_SVC_MAPPING_HPP__

#include "nic/apollo/api/include/pds_service.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/feeder.hpp"

namespace test {
namespace api {

// svc mapping test feeder class
class svc_mapping_feeder : public feeder {
public:
    pds_svc_mapping_spec_t spec;

    // Constructor
    svc_mapping_feeder() { };
    svc_mapping_feeder(const svc_mapping_feeder& feeder) {
        init(feeder.spec.key, ipaddr2str(&feeder.spec.vip),
             feeder.spec.svc_port, feeder.spec.skey.vpc,
             ipaddr2str(&feeder.spec.skey.backend_ip),
             feeder.spec.skey.backend_port,
             ipaddr2str(&feeder.spec.backend_provider_ip), feeder.num_obj);
    }

    // Initialize feeder with the base set of values
    void init(pds_obj_key_t key, std::string vip_str, uint16_t svc_port,
              pds_obj_key_t backend_vpc, std::string backend_ip,
              uint16_t backend_port, std::string backend_pip,
              uint32_t num_svc_mapping=100);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Build routines
    void key_build(pds_obj_key_t *key) const;
    void spec_build(pds_svc_mapping_spec_t *spec) const;

    // Compare routines
    bool key_compare(const pds_obj_key_t *key) const;
    bool spec_compare(const pds_svc_mapping_spec_t *spec) const;
    bool status_compare(const pds_svc_mapping_status_t *status1,
                        const pds_svc_mapping_status_t *status2) const;
};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const pds_svc_mapping_key_t *key) {
    os << " vpc: " << key->vpc.str()
       << " backend ip: " << key->backend_ip
       << " backend port: " << key->backend_port;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_svc_mapping_spec_t *spec) {
    os << &spec->key
       << " vip: " << spec->vip
       << " svc port " << spec->svc_port
       << " backend provider ip " << spec->backend_provider_ip;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_svc_mapping_info_t *obj) {
    os << "Svc mapping info => "
       << &obj->spec
       << std::endl;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const svc_mapping_feeder& obj) {
    os << "Svc Mapping feeder => " << &obj.spec;
    return os;
}

// CRUD prototypes
API_CREATE(svc_mapping);
API_READ(svc_mapping);
API_UPDATE(svc_mapping);
API_DELETE(svc_mapping);

}    // namespace api
}    // namespace test

#endif    // __TEST_API_UTILS_SVC_MAPPING_HPP__
