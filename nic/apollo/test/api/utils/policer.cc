//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/policer.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// Policy feeder class routines
//----------------------------------------------------------------------------

void
policer_feeder::init(pds_obj_key_t key,
                     sdk::qos::policer_type_t type,
                     pds_policer_dir_t dir,
                     uint64_t rate,
                     uint64_t burst,
                     uint32_t num_policer) {
    memset(&this->spec, 0, sizeof(pds_policer_spec_t));
    this->spec.key = key;
    this->spec.type = type;
    this->spec.dir = dir;
    if (type == sdk::qos::POLICER_TYPE_PPS) {
        this->spec.pps = rate;
        this->spec.pps_burst = burst;
    } else {
        this->spec.bps = rate;
        this->spec.bps_burst = burst;
    }
    num_obj = num_policer;
}

void
policer_feeder::iter_next(int width) {
    this->spec.key = int2pdsobjkey(pdsobjkey2int(this->spec.key) + width);
    cur_iter_pos++;
}

void
policer_feeder::key_build(pds_obj_key_t *key) const {
    memcpy(key, &this->spec.key, sizeof(pds_obj_key_t));
}

void
policer_feeder::spec_build(pds_policer_spec_t *spec) const {
    memcpy(spec, &this->spec, sizeof(pds_policer_spec_t));
}

bool
policer_feeder::key_compare(const pds_obj_key_t *key) const {
    return (memcmp(key, &this->spec.key, sizeof(pds_obj_key_t)) == 0);
}

bool
policer_feeder::spec_compare(const pds_policer_spec_t *spec) const {
    if (spec->type != this->spec.type)
        return false;
    if (spec->dir != this->spec.dir)
        return false;
    if (spec->type == sdk::qos::POLICER_TYPE_PPS) {
        if (spec->pps != this->spec.pps)
            return false;
        //if (spec->pps_burst != this->spec.pps_burst)
        //    return false;
    } else {
        if (spec->bps != this->spec.bps)
            return false;
        //if (spec->bps_burst != this->spec.bps_burst)
        //    return false;
    }

    return true;
}

bool
policer_feeder::status_compare(const pds_policer_status_t *status1,
                               const pds_policer_status_t *status2) const {
    return true;
}

//----------------------------------------------------------------------------
// Misc routines
//----------------------------------------------------------------------------

// do not modify these sample values as rest of system is sync with these
static policer_feeder k_pol_feeder;

void sample_policer_setup(pds_batch_ctxt_t bctxt) {
    pds_obj_key_t pol_key = int2pdsobjkey(20000);

    // setup and teardown parameters should be in sync
    k_pol_feeder.init(pol_key, sdk::qos::POLICER_TYPE_PPS,
                      PDS_POLICER_DIR_INGRESS, 5000, 15000, 5);
    many_create(bctxt, k_pol_feeder);

    pol_key = int2pdsobjkey(pdsobjkey2int(pol_key) + 5);
    k_pol_feeder.init(pol_key, sdk::qos::POLICER_TYPE_BPS,
                      PDS_POLICER_DIR_INGRESS, 500000, 1500000, 5);
    many_create(bctxt, k_pol_feeder);

    pol_key = int2pdsobjkey(pdsobjkey2int(pol_key) + 5);
    k_pol_feeder.init(pol_key, sdk::qos::POLICER_TYPE_PPS,
                      PDS_POLICER_DIR_EGRESS, 5000, 15000, 5);
    many_create(bctxt, k_pol_feeder);

    pol_key = int2pdsobjkey(pdsobjkey2int(pol_key) + 5);
    k_pol_feeder.init(pol_key, sdk::qos::POLICER_TYPE_BPS,
                      PDS_POLICER_DIR_EGRESS, 500000, 1500000, 5);
    many_create(bctxt, k_pol_feeder);
}

void sample_policer_teardown(pds_batch_ctxt_t bctxt) {
    pds_obj_key_t pol_key = int2pdsobjkey(20000);

    // this feeder base values doesn't matter in case of deletes
    k_pol_feeder.init(pol_key, sdk::qos::POLICER_TYPE_PPS,
                      PDS_POLICER_DIR_INGRESS, 5000, 15000, 5);
    many_delete(bctxt, k_pol_feeder);

    pol_key = int2pdsobjkey(pdsobjkey2int(pol_key) + 5);
    k_pol_feeder.init(pol_key, sdk::qos::POLICER_TYPE_BPS,
                      PDS_POLICER_DIR_INGRESS, 500000, 1500000, 5);
    many_delete(bctxt, k_pol_feeder);

    pol_key = int2pdsobjkey(pdsobjkey2int(pol_key) + 5);
    k_pol_feeder.init(pol_key, sdk::qos::POLICER_TYPE_PPS,
                      PDS_POLICER_DIR_EGRESS, 5000, 15000, 5);
    many_delete(bctxt, k_pol_feeder);

    pol_key = int2pdsobjkey(pdsobjkey2int(pol_key) + 5);
    k_pol_feeder.init(pol_key, sdk::qos::POLICER_TYPE_BPS,
                      PDS_POLICER_DIR_EGRESS, 500000, 1500000, 5);
    many_delete(bctxt, k_pol_feeder);
}

}    // namespace api
}    // namespace test
