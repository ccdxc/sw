//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __PDSA_TEST_UNDERLAY_ECMP_PDS_MOCK_HPP__
#define __PDSA_TEST_UNDERLAY_ECMP_PDS_MOCK_HPP__

#include "nic/metaswitch/stubs/test/hals/underlay_ecmp_test_params.hpp"
#include "nic/metaswitch/stubs/test/pdsmock/pds_api_mock.hpp"

namespace pdsa_test {

class underlay_ecmp_pds_mock_t final : public pds_mock_t {
public:
    void validate() override {
        validate_();
        reset_mock_fail();
    }
    void expect_create() override {
        op_create_ = true; op_delete_ = false;
        clear_batches();
        auto underlay_ecmp_input = dynamic_cast<underlay_ecmp_input_params_t*>
                                 (test_params()->test_input);
        generate_addupd_specs(*underlay_ecmp_input, expected_pds);
    }
    void expect_update() override {
        op_create_ = false; op_delete_ = false;
        clear_batches();
        auto underlay_ecmp_input = dynamic_cast<underlay_ecmp_input_params_t*>
                                 (test_params()->test_input);
        generate_addupd_specs(*underlay_ecmp_input, expected_pds);
    }
    void expect_delete() override {
        op_create_ = false; op_delete_ = true;
        clear_batches();
        auto underlay_ecmp_input = dynamic_cast<underlay_ecmp_input_params_t*>
                                 (test_params()->test_input);
        generate_del_specs(*underlay_ecmp_input, expected_pds);
    }
    void expect_create_pds_async_fail() override {
        mock_pds_batch_async_fail_ = true;
        expect_create();
    }
    void cleanup(void) override {
        pds_mock_t::cleanup();
    }
private:
    int           num_if_objs_ = 0;
    void generate_addupd_specs(const underlay_ecmp_input_params_t& input,
                               batch_spec_t& pds_batch);
    void generate_del_specs(const underlay_ecmp_input_params_t& input,
                            batch_spec_t& pds_batch);
    void validate_();
};

} // End namespace pdsa_test

#endif
