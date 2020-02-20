//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Cookie passed in PDS APIs to and from HAL
//---------------------------------------------------------------

#ifndef __PDS_MS_COOKIE_HPP__
#define __PDS_MS_COOKIE_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_slab_object.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_object_store.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include <string>

namespace pds_ms {

// Cookies are short-lived until the PDS async callback is received.    
//
// Max outstanding cookies
constexpr int k_max_cookie = 1000;

class cookie_t : public slab_obj_t<cookie_t> {
public:    
    bool ips_mock = false; // Is this part of a gtest with ips_feeder
    std::vector<std::unique_ptr<pds_ms::base_obj_t>> objs;
    void print_debug_str(void);
    std::function<void(bool,bool)> send_ips_reply;
};

inline void cookie_t::print_debug_str(void) {
    if (objs.size() > 0) {PDS_TRACE_DEBUG ("Update store for object(s):");}

    for (auto& obj_uptr: objs) {
        obj_uptr->print_debug_str();
    }
}

}
#endif
