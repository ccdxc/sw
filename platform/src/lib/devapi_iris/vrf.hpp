
#ifndef __VRF_HPP__
#define __VRF_HPP__

#include "lib/indexer/indexer.hpp"
#include "gen/proto/types.pb.h"
#include "devapi_object.hpp"

namespace iris {

class devapi_uplink;
class devapi_vrf : public devapi_object {
private:
    uint32_t id_;
    types::VrfType type_;
    devapi_uplink *uplink_;
    static sdk::lib::indexer *allocator_;      // for vrf id
    static constexpr uint64_t max_vrfs = 32;

private:
    sdk_ret_t init_(types::VrfType type, devapi_uplink *uplink);
    devapi_vrf() {}
    ~devapi_vrf() {}

public:
    static devapi_vrf *factory(types::VrfType type, devapi_uplink *up);
    static void destroy(devapi_vrf *vrf);

    sdk_ret_t vrf_halcreate(void);
    sdk_ret_t vrf_haldelete(void);

    void deallocate_id(void);
    uint64_t get_id(void);

    static void probe(void);
    devapi_uplink *get_uplink(void) { return uplink_; }
};

}    // namespace iris

using iris::devapi_vrf;

#endif // __VRF_HPP__
