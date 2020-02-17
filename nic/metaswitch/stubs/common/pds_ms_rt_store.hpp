//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS Metaswitch RT store object used by Mgmt
//---------------------------------------------------------------

#ifndef __PDS_MS_RT_STORE_HPP__
#define __PDS_MS_RT_STORE_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"

namespace pds_ms {

// object to store vector of RTs. will be added into subnet store and
// vpc stores to store evi and vrf RTs to configure ORF entry
class rt_store_t {
public:
    void add (const uint8_t *rt_str) {list_.emplace_back(rt_str);}
    void del (const uint8_t *rt_str) {
        list_.erase (std::remove_if (list_.begin(),
                                     list_.end(),
                                     [rt_str] (ms_rt_t& obj)
                                     {return (obj.equal(rt_str));}),
                        list_.end());
    }
    bool find (uint8_t *rt_str) {
        for (auto& obj: list_) {
            if (obj.equal(rt_str)) {return true;}
        }
        return false;
    }
    // walk thourgh the rt list using lambda or function
    void walk (const std::function <void (ms_rt_t&)>& cb_fn) {
        for (auto& obj: list_) {
            cb_fn (obj);
        }
    }

private:
    std::vector<ms_rt_t> list_; // list to store RTs
};
} // end namespace

#endif
