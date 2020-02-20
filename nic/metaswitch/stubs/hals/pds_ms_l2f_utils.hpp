//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// L2F Utils for Metaswitch joins and Stores
//---------------------------------------------------------------

#ifndef __PDSA_L2F_UTILS_HPP__
#define __PDSA_L2F_UTILS_HPP__

#include <l2f_fte.hpp>
#include <l2f_bdpi_slave_join.hpp>
#include <l2f_bd.hpp>

namespace pds_ms {

static inline l2f::BdpiSlaveJoin* get_l2f_bdpi_join() {
    auto& bdpi_set = l2f::Fte::get().get_bdpi_joins();
    if (unlikely(bdpi_set.size()!=1)) {
        PDS_TRACE_ERR("Unexpected number of BDPI joins %d", 
                      bdpi_set.size());
        return nullptr;
    }
    return *(bdpi_set.begin());
}

static inline l2f::FdbMacStore* get_l2f_fdb_mac_store() {
    auto bdpi_join = get_l2f_bdpi_join();
    if (bdpi_join == nullptr) {return nullptr;}
    return &bdpi_join->get_fdb_mac_store();
}

static inline l2f::MaiSlaveJoin* get_l2f_mai_join() {
    auto& mai_set = l2f::Fte::get().get_mai_joins();
    if (unlikely(mai_set.size()!=1)) {
        PDS_TRACE_ERR("Unexpected number of MAI joins %d", 
                      mai_set.size());
        return nullptr;
    }
    return *(mai_set.begin());
}

} // End namespace

#endif
