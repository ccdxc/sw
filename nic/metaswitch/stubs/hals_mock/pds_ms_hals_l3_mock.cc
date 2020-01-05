//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Mock implementation of Metaswitch L3 HALS stub integration
//---------------------------------------------------------------

#include <hals_c_includes.hpp>
#include "nic/metaswitch/stubs/hals/pds_ms_hals_l3.hpp"

namespace pds_ms {

//-------------------------------------------------------------------------
// NHPI
//-------------------------------------------------------------------------
NBB_BYTE hals_l3_integ_subcomp_t::nhpi_add_update_ecmp(ATG_NHPI_ADD_UPDATE_ECMP *add_update_ecmp_ips) { 
    return ATG_OK;
}

NBB_BYTE hals_l3_integ_subcomp_t::nhpi_delete_ecmp(ATG_NHPI_DELETE_ECMP *delete_ecmp_ips) {
    return ATG_OK;
}

NBB_BYTE hals_l3_integ_subcomp_t::nhpi_destroy_ecmp(NBB_CORRELATOR ecmp_corr) {
    return ATG_OK;
}

NBB_BYTE hals_l3_integ_subcomp_t::nhpi_add_ecmp_nh(NBB_CORRELATOR ecmp_corr,
                                                   ATG_NHPI_APPENDED_NEXT_HOP *next_hop,
                                                   NBB_BYTE cascaded) {
    return ATG_OK;
}

NBB_BYTE hals_l3_integ_subcomp_t::nhpi_delete_ecmp_nh(NBB_CORRELATOR ecmp_corr,
                                                      NBB_CORRELATOR nh_corr) {
    return ATG_OK;
}

NBB_BYTE hals_l3_integ_subcomp_t::ropi_update_route(ATG_ROPI_UPDATE_ROUTE *update_route) {
    return ATG_OK;
}
    
NBB_BYTE hals_l3_integ_subcomp_t::ropi_delete_route(ATG_ROPI_ROUTE_ID route_id) {
    return ATG_OK;
}

} // End namespace
