//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __CORE_STATE_HPP__
#define __CORE_STATE_HPP__

#include <unordered_map>
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/sdk/lib/eventmgr/eventmgr.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_service.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_route.hpp"

using std::unordered_map;
using std::make_pair;
using sdk::lib::slab;
using sdk::lib::eventmgr;

namespace core {

typedef sdk_ret_t (*vpc_walk_cb_t)(pds_vpc_spec_t *spec, void *ctxt);
typedef sdk_ret_t (*vpc_peer_walk_cb_t)(pds_vpc_peer_spec_t *spec, void *ctxt);
typedef sdk_ret_t (*subnet_walk_cb_t)(pds_subnet_spec_t *spec, void *ctxt);
typedef sdk_ret_t (*service_walk_cb_t)(pds_svc_mapping_spec_t *spec, void *ctxt);
typedef sdk_ret_t (*route_table_walk_cb_t)(pds_route_table_spec_t *spec, void *ctxt);
typedef sdk_ret_t (*tep_walk_cb_t)(pds_tep_spec_t *spec, void *ctxt);

typedef slab *slab_ptr_t;

typedef enum slab_id_e {
    SLAB_ID_MIN,
    SLAB_ID_VPC = SLAB_ID_MIN,
    SLAB_ID_VPC_PEER,
    SLAB_ID_SUBNET,
    SLAB_ID_SERVICE,
    SLAB_ID_TEP,
    SLAB_ID_ROUTE,
    SLAB_ID_IF,
    SLAB_ID_MAX
} slab_id_t;

typedef unordered_map<pds_obj_key_t, pds_vpc_spec_t *, pds_obj_key_hash> vpc_db_t;
typedef unordered_map<pds_obj_key_t, pds_vpc_peer_spec_t *, pds_obj_key_hash> vpc_peer_db_t;
typedef unordered_map<pds_obj_key_t , pds_subnet_spec_t *, pds_obj_key_hash> subnet_db_t;
typedef unordered_map<pds_svc_mapping_key_t, pds_svc_mapping_spec_t *, pds_svc_mapping_hash_fn> service_db_t;
typedef unordered_map<pds_obj_key_t, pds_tep_spec_t *, pds_obj_key_hash> tep_db_t;
typedef unordered_map<pds_obj_key_t, pds_route_table_spec_t *, pds_obj_key_hash> route_table_db_t;

typedef vpc_db_t::const_iterator vpc_it_t;

class cfg_db {
public:
    static cfg_db *factory(void);
    static void destroy(cfg_db *cfg_db);

    pds_device_spec_t *device(void) { return &device_; }
    tep_db_t *tep_map(void) { return tep_map_; }
    vpc_db_t *vpc_map(void) { return vpc_map_; }
    vpc_peer_db_t *vpc_peer_map(void) { return vpc_peer_map_; }
    subnet_db_t *subnet_map(void) { return subnet_map_; }
    service_db_t *service_map(void) { return service_map_; }
    route_table_db_t *route_table_map(void) { return route_table_map_; }
    const pds_obj_key_t& underlay_vpc(void) const { return underlay_vpc_; }
    void set_underlay_vpc(pds_obj_key_t key) { underlay_vpc_ = key; }
    void reset_underlay_vpc(void) { underlay_vpc_.reset(); }

    sdk_ret_t slab_walk(sdk::lib::slab_walk_cb_t walk_cb, void *ctxt);

    slab_ptr_t tep_slab(void) const {
        return slabs_[SLAB_ID_TEP];
    }
    slab_ptr_t vpc_slab(void) const {
        return slabs_[SLAB_ID_VPC];
    }
    slab_ptr_t vpc_peer_slab(void) const {
        return slabs_[SLAB_ID_VPC_PEER];
    }
    slab_ptr_t subnet_slab(void) const {
        return slabs_[SLAB_ID_SUBNET];
    }
    slab_ptr_t service_slab(void) const {
        return slabs_[SLAB_ID_SERVICE];
    }
    slab_ptr_t if_slab(void) const {
        return slabs_[SLAB_ID_IF];
    }
    slab_ptr_t route_table_slab(void) const {
        return slabs_[SLAB_ID_ROUTE];
    }

private:
    cfg_db();
    ~cfg_db();
    bool init(void);

private:
    tep_db_t *tep_map_;
    vpc_db_t *vpc_map_;
    vpc_peer_db_t *vpc_peer_map_;
    pds_obj_key_t underlay_vpc_;
    subnet_db_t *subnet_map_;
    service_db_t *service_map_;
    route_table_db_t *route_table_map_;
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

    pds_tep_spec_t *find_in_tep_db(pds_obj_key_t *key);
    sdk_ret_t add_to_tep_db(pds_obj_key_t *key,
                            pds_tep_spec_t *spec);
    sdk_ret_t tep_db_walk(tep_walk_cb_t cb, void *ctxt);
    bool del_from_tep_db(pds_obj_key_t *key);
    slab_ptr_t tep_slab(void) const { return cfg_db_->tep_slab(); }

    pds_vpc_spec_t *find_in_vpc_db(pds_obj_key_t *key);
    sdk_ret_t add_to_vpc_db(pds_obj_key_t *key,
                            pds_vpc_spec_t *spec);
    sdk_ret_t vpc_db_walk(vpc_walk_cb_t cb, void *ctxt);
    bool del_from_vpc_db(pds_obj_key_t *key);
    slab_ptr_t vpc_slab(void) const { return cfg_db_->vpc_slab(); }

    pds_vpc_peer_spec_t *find_in_vpc_peer_db(pds_obj_key_t *key);
    sdk_ret_t add_to_vpc_peer_db(pds_obj_key_t *key,
                                 pds_vpc_peer_spec_t *spec);
    sdk_ret_t vpc_peer_db_walk(vpc_peer_walk_cb_t cb, void *ctxt);
    bool del_from_vpc_peer_db(pds_obj_key_t *key);
    slab_ptr_t vpc_peer_slab(void) const { return cfg_db_->vpc_peer_slab(); }

    pds_subnet_spec_t *find_in_subnet_db(pds_obj_key_t *key);
    sdk_ret_t add_to_subnet_db(pds_obj_key_t *key,
                               pds_subnet_spec_t *spec);
    sdk_ret_t subnet_db_walk(subnet_walk_cb_t cb, void *ctxt);
    bool del_from_subnet_db(pds_obj_key_t *key);
    slab_ptr_t subnet_slab(void) const { return cfg_db_->subnet_slab(); }

    pds_svc_mapping_spec_t *find_in_service_db(pds_svc_mapping_key_t *key);
    sdk_ret_t add_to_service_db(pds_svc_mapping_key_t *key,
                               pds_svc_mapping_spec_t *spec);
    sdk_ret_t service_db_walk(service_walk_cb_t cb, void *ctxt);
    bool del_from_service_db(pds_svc_mapping_key_t *key);
    slab_ptr_t service_slab(void) const { return cfg_db_->service_slab(); }

    slab_ptr_t if_slab(void) const { return cfg_db_->if_slab(); }

    pds_route_table_spec_t *find_in_route_table_db(pds_obj_key_t *key);
    sdk_ret_t add_to_route_table_db(pds_obj_key_t *key,
                                    pds_route_table_spec_t *spec);
    bool del_from_route_table_db(pds_obj_key_t *key);
    sdk_ret_t route_table_db_walk(route_table_walk_cb_t cb, void *ctxt);
    slab_ptr_t route_table_slab(void) const {
        return cfg_db_->route_table_slab();
    }

    const pds_obj_key_t& underlay_vpc(void) const {
        return cfg_db_->underlay_vpc();
    }
    void set_underlay_vpc(pds_obj_key_t key) {
        return cfg_db_->set_underlay_vpc(key);
    }
    void reset_underlay_vpc(void) {
        return cfg_db_->reset_underlay_vpc();
    }

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
    tep_db_t *tep_map(void) const { return cfg_db_->tep_map();  }
    vpc_db_t *vpc_map(void) const { return cfg_db_->vpc_map();  }
    vpc_peer_db_t *vpc_peer_map(void) const { return cfg_db_->vpc_peer_map();  }
    subnet_db_t *subnet_map(void) const { return cfg_db_->subnet_map();  }
    service_db_t *service_map(void) const { return cfg_db_->service_map();  }
    route_table_db_t *route_table_map(void) const { return
        cfg_db_->route_table_map();
    }

private:
    cfg_db   *cfg_db_;
    bool     pds_mock_mode_;    // true if we are in PDS HAL mock mode
    uint64_t epoch_;            // config epoch
    eventmgr *evmgr_;           // event manager instance
};

}    // namespace core

#endif    // __CORE_STATE_HPP__
