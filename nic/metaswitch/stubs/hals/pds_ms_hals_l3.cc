//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS Metaswitch L3 HALS stub integration subcomponent APIs
//---------------------------------------------------------------

#include <hals_c_includes.hpp>
#include "nic/metaswitch/stubs/hals/pds_ms_hals_l3.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hals_ecmp.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"

namespace pds_ms {

using pds_ms::Error;
//-------------------------------------------------------------------------
// NHPI
//-------------------------------------------------------------------------
NBB_BYTE hals_l3_integ_subcomp_t::nhpi_add_update_ecmp(ATG_NHPI_ADD_UPDATE_ECMP *add_update_ecmp_ips) { 
    try {
        hals_ecmp_t ecmp;
        ecmp.handle_add_upd_ips(add_update_ecmp_ips);
    } catch (Error& e) {
        SDK_TRACE_ERR ("ECMP Add Update processing failed %s", e.what());
        add_update_ecmp_ips->return_code = ATG_UNSUCCESSFUL;
    }
    // Always return ATG_OK - fill the actual return code in the IPS
    return ATG_OK;
}

NBB_BYTE hals_l3_integ_subcomp_t::nhpi_delete_ecmp(ATG_NHPI_DELETE_ECMP *delete_ecmp_ips) {
    // Called in MS HALs stateless mode
    try {
        hals_ecmp_t ecmp;
        ecmp.handle_delete(delete_ecmp_ips->pathset_id);
    } catch (Error& e) {
        SDK_TRACE_ERR ("ECMP Delete processing failed %s", e.what());
    }
    // Always return ATG_OK - fill the actual return code in the IPS
    return ATG_OK;
}

NBB_BYTE hals_l3_integ_subcomp_t::nhpi_destroy_ecmp(NBB_CORRELATOR ecmp_corr) {
    // Called in MS HALs stateful mode
    try {
        hals_ecmp_t ecmp;
        ecmp.handle_delete(ecmp_corr);
    } catch (Error& e) {
        SDK_TRACE_ERR ("ECMP Destroy processing failed %s", e.what());
    }
    return ATG_OK;
}

NBB_BYTE hals_l3_integ_subcomp_t::nhpi_add_ecmp_nh(NBB_CORRELATOR ecmp_corr,
                                                   ATG_NHPI_APPENDED_NEXT_HOP *next_hop,
                                                   NBB_BYTE cascaded) {
    return ATG_OK;
};

NBB_BYTE hals_l3_integ_subcomp_t::nhpi_delete_ecmp_nh(NBB_CORRELATOR ecmp_corr,
                                                      NBB_CORRELATOR nh_corr) {
    return ATG_OK;
};
} // End namespace
