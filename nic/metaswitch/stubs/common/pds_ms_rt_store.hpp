//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS Metaswitch RT store object used by Mgmt
//---------------------------------------------------------------

#ifndef __PDS_MS_RT_STORE_HPP__
#define __PDS_MS_RT_STORE_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"

namespace pds_ms {

// object to store vector of RTs. will be inherited into subnet store and
// vpc stores to store evi and vrf RTs to configure ORF entry
class rt_store_t {
public:
    void add (const uint8_t *rt) {list_.push_back(ms_rt_t(rt));}
    void del (const uint8_t *rt) {
        list_.erase (std::remove_if (list_.begin(),
                                     list_.end(),
                                     [rt] (ms_rt_t& obj)
                                     {return (obj.equal(rt));}),
                        list_.end());
    }
    bool find (uint8_t *rt) {
        for (auto& obj: list_) {
            if (obj.equal(rt)) {return true;}
        }
        return false;
    }
    const std::vector<ms_rt_t>& list(void) const {return list_;}
    std::vector<ms_rt_t>& list(void) {return list_;}

private:
    std::vector<ms_rt_t> list_; // list to store RTs
};
} // end namespace

#endif
