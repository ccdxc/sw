//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __CORE_STATE_HPP__
#define __CORE_STATE_HPP__

#include <unordered_map>
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/sdk/lib/eventmgr/eventmgr.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_service.hpp"

using std::unordered_map;
using std::make_pair;
using sdk::lib::slab;
using sdk::lib::eventmgr;

namespace core {

typedef sdk_ret_t (*service_walk_cb_t)(pds_svc_mapping_spec_t *spec, void *ctxt);

typedef slab *slab_ptr_t;

typedef enum slab_id_e {
    SLAB_ID_MIN,
    SLAB_ID_SERVICE = SLAB_ID_MIN,
    SLAB_ID_IF,
    SLAB_ID_MAX
} slab_id_t;

typedef unordered_map<pds_svc_mapping_key_t, pds_svc_mapping_spec_t *, pds_svc_mapping_hash_fn> service_db_t;

class cfg_db {
public:
    static cfg_db *factory(void);
    static void destroy(cfg_db *cfg_db);

    pds_device_spec_t *device(void) { return &device_; }
    service_db_t *service_map(void) { return service_map_; }

    sdk_ret_t slab_walk(sdk::lib::slab_walk_cb_t walk_cb, void *ctxt);

    slab_ptr_t service_slab(void) const {
        return slabs_[SLAB_ID_SERVICE];
    }
    slab_ptr_t if_slab(void) const {
        return slabs_[SLAB_ID_IF];
    }

private:
    cfg_db();
    ~cfg_db();
    bool init(void);

private:
    service_db_t *service_map_;
    pds_device_spec_t device_;
    slab_ptr_t slabs_[SLAB_ID_MAX - SLAB_ID_MIN + 1];
};

class agent_state {
public:
    static sdk_ret_t init(void);
    static class agent_state *state(void);

    agent_state();
    ~agent_state();
    pds_device_spec_t *device(void) { return cfg_db_->device(); }

    pds_svc_mapping_spec_t *find_in_service_db(pds_svc_mapping_key_t *key);
    sdk_ret_t add_to_service_db(pds_svc_mapping_key_t *key,
                               pds_svc_mapping_spec_t *spec);
    sdk_ret_t service_db_walk(service_walk_cb_t cb, void *ctxt);
    bool del_from_service_db(pds_svc_mapping_key_t *key);
    slab_ptr_t service_slab(void) const { return cfg_db_->service_slab(); }

    slab_ptr_t if_slab(void) const { return cfg_db_->if_slab(); }

    sdk_ret_t slab_walk(sdk::lib::slab_walk_cb_t walk_cb, void *ctxt) {
        return cfg_db_->slab_walk(walk_cb, ctxt);
    }

    bool pds_mock_mode(void) const { return pds_mock_mode_;  }
    void pds_mock_mode_set(bool val) { pds_mock_mode_ = val; }
    uint64_t epoch(void) const { return epoch_; }
    uint64_t new_epoch(void) { return SDK_ATOMIC_INC_UINT64(&epoch_, 1); }
    eventmgr *event_mgr(void) { return evmgr_; }

private:
    void cleanup(void);
    service_db_t *service_map(void) const { return cfg_db_->service_map();  }

private:
    cfg_db   *cfg_db_;
    bool     pds_mock_mode_;    // true if we are in PDS HAL mock mode
    uint64_t epoch_;            // config epoch
    eventmgr *evmgr_;           // event manager instance
};

}    // namespace core

#endif    // __CORE_STATE_HPP__
