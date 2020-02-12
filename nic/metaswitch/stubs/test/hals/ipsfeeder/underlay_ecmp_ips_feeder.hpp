//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __UNDERLAY_ECMP_IPS_FEEDER_HPP__
#define __UNDERLAY_ECMP_IPS_FEEDER_HPP__

#include "nic/metaswitch/stubs/test/hals/underlay_ecmp_test_params.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include <hals_c_includes.hpp>
#include "nic/metaswitch/stubs/hals/pds_ms_hals_l3.hpp"
#include "nic/apollo/test/base/utils.hpp"
#include <nbase.h>

extern NBB_ULONG hals_proc_id;

namespace pds_ms_test {

class underlay_ecmp_ips_feeder_t final : public underlay_ecmp_input_params_t {
public:
    ATG_NHPI_ADD_UPDATE_ECMP* generate_add_upd_ips(void);

    void fill_add_update_sz_array(
                              NBB_BUF_SIZE (&size_array)[OFL_ATG_NHPI_ADD_UPDATE_ECMP + 1]);
    void init(std::vector<nhinfo_t>&& nhs) override;

    void cleanup(void) override;

    void trigger_create(void) override {
        op_create_ = true;
        auto add_upd = generate_add_upd_ips();
        hal_is.nhpi_add_update_ecmp(add_upd);
    }

    void trigger_delete(void) override {
        NBB_CORRELATOR pathset_id_corr;
        uint32_t pathset_id = 1;
        NBB_CORR_PUT_VALUE(pathset_id_corr, pathset_id);
        hal_is.nhpi_destroy_ecmp(pathset_id_corr);
    }

    void trigger_update(void) override {
        op_create_ = false;
        auto add_upd = generate_add_upd_ips();
        hal_is.nhpi_add_update_ecmp(add_upd);
    }
    void next(void) override {};

    bool ips_mock() override {return true;}

private:
    void fill_add_update_sz_array(bool op_create,
                  NBB_BUF_SIZE (&size_array)[OFL_ATG_NHPI_ADD_UPDATE_ECMP + 1]
                  );

    pds_ms::hals_l3_integ_subcomp_t hal_is;
    bool op_create_ = false;
    std::vector<uint32_t> ms_iflist;
    NBB_SAVED_CONTEXT saved_context;
};

} // End Namespace

#endif
