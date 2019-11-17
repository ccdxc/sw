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
#include <string>

namespace pdsa_stub {

// Cookies are short-lived until the PDS async callback is received.    
//
// Max outstanding cookies
constexpr int k_max_cookie = 1000;

class cookie_t : public slab_obj_t<cookie_t> {
public:    
    cookie_t(const char* str_) : str(str_) {};
    cookie_t(std::string&& str_) : str(std::move(str_)) {};

    std::vector<std::unique_ptr<pdsa_stub::base_obj_t>> objs;
    bool op_delete = false;
    NBB_IPS* ips = nullptr;
    NBB_PROC_ID nbb_stub_pid;
    NBB_PROC_ID nbb_send_pid;
    NBB_QUEUE_ID nbb_qid;
    // TODO: Do away with debug string (malloc) for every API call
    // Try to find something in the cookie context that can be converted 
    // to string only if TRACE is enabled
    std::string str; // debug string
};

}
#endif
