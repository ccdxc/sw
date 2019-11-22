//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Cookie passed in PDS APIs to and from HAL
//---------------------------------------------------------------

#ifndef __PDSA_COOKIE_HPP__
#define __PDSA_COOKIE_HPP__

#include "nic/metaswitch/stubs/common/pdsa_slab_object.hpp"
#include "nic/metaswitch/stubs/common/pdsa_state.hpp"
#include "nic/metaswitch/stubs/common/pdsa_object_store.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include <string>

namespace pdsa_stub {

// Cookies are short-lived until the PDS async callback is received.    
//
// Max outstanding cookies
constexpr int k_max_cookie = 1000;

class cookie_t : public slab_obj_t<cookie_t> {
public:    
    std::vector<std::unique_ptr<pdsa_stub::base_obj_t>> objs;
    void print_debug_str(void);

    bool op_delete = false;
    NBB_IPS* ips = nullptr;
    NBB_PROC_ID nbb_stub_pid;
    NBB_PROC_ID nbb_send_pid;
    NBB_QUEUE_ID nbb_qid;
};

inline void cookie_t::print_debug_str(void) {
    if (op_delete) {SDK_TRACE_DEBUG ("Delete objects:");}
    else {SDK_TRACE_DEBUG ("Add/Upd objects:");}

    for (auto& obj_uptr: objs) {
        obj_uptr->print_debug_str();
    }
}

}
#endif
