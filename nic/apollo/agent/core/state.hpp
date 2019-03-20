//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __CORE_STATE_HPP__
#define __CORE_STATE_HPP__

#include <unordered_map>
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_vcn.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/include/pds_route.hpp"

using std::unordered_map;
using std::make_pair;
using sdk::lib::slab;

namespace core {

typedef sdk::sdk_ret_t (*pcn_walk_cb_t)(pds_vcn_spec_t *spec, void *ctxt);
typedef sdk::sdk_ret_t (*subnet_walk_cb_t)(pds_subnet_spec_t *spec, void *ctxt);
typedef sdk::sdk_ret_t (*vnic_walk_cb_t)(pds_vnic_spec_t *spec, void *ctxt);

typedef slab *slab_ptr_t;

typedef enum slab_id_e {
    SLAB_ID_MIN,
    SLAB_ID_PCN = SLAB_ID_MIN,
    SLAB_ID_SUBNET,
    SLAB_ID_TEP,
    SLAB_ID_VNIC,
    SLAB_ID_ROUTE,
    SLAB_ID_MAX
} slab_id_t;

typedef unordered_map<uint32_t, pds_vcn_spec_t *> pcn_db_t;
typedef unordered_map<uint32_t, pds_subnet_spec_t *> subnet_db_t;
typedef unordered_map<uint32_t, pds_tep_spec_t *> tep_db_t;
typedef unordered_map<uint32_t, pds_vnic_spec_t *> vnic_db_t;
typedef unordered_map<uint32_t, pds_route_table_spec_t *> route_table_db_t;

typedef pcn_db_t::const_iterator pcn_it_t;
typedef vnic_db_t::const_iterator vnic_it_t;

class cfg_db {
public:
    static cfg_db *factory(void);
    static void destroy(cfg_db *cfg_db);

    pds_device_spec_t *device(void) { return &device_; }
    tep_db_t *tep_map(void) { return tep_map_; }
    pcn_db_t *pcn_map(void) { return pcn_map_; }
    subnet_db_t *subnet_map(void) { return subnet_map_; }
    vnic_db_t *vnic_map(void) { return vnic_map_; }
    route_table_db_t *route_table_map(void) { return route_table_map_; }

    slab_ptr_t tep_slab(void) const {
        return slabs_[SLAB_ID_TEP];
    }
    slab_ptr_t pcn_slab(void) const {
        return slabs_[SLAB_ID_PCN];
    }
    slab_ptr_t subnet_slab(void) const {
        return slabs_[SLAB_ID_SUBNET];
    }
    slab_ptr_t vnic_slab(void) const {
        return slabs_[SLAB_ID_SUBNET];
    }
    slab_ptr_t route_table_slab(void) const {
        return slabs_[SLAB_ID_SUBNET];
    }

private:
    cfg_db();
    ~cfg_db();
    bool init(void);

private:
    tep_db_t *tep_map_;
    pcn_db_t *pcn_map_;
    subnet_db_t *subnet_map_;
    vnic_db_t *vnic_map_;
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

    pds_tep_spec_t *find_in_tep_db(uint32_t key);
    sdk_ret_t add_to_tep_db(uint32_t key,
                            pds_tep_spec_t *spec);
    bool del_from_tep_db(uint32_t key);
    slab_ptr_t tep_slab(void) const { return cfg_db_->tep_slab(); }

    pds_vcn_spec_t *find_in_pcn_db(pds_vcn_key_t *key);
    sdk_ret_t add_to_pcn_db(pds_vcn_key_t *key,
                            pds_vcn_spec_t *spec);
    sdk_ret_t pcn_db_walk(pcn_walk_cb_t cb, void *ctxt);
    bool del_from_pcn_db(pds_vcn_key_t *key);
    slab_ptr_t pcn_slab(void) const { return cfg_db_->pcn_slab(); }

    pds_subnet_spec_t *find_in_subnet_db(pds_subnet_key_t *key);
    sdk_ret_t add_to_subnet_db(pds_subnet_key_t *key,
                               pds_subnet_spec_t *spec);
    sdk_ret_t subnet_db_walk(subnet_walk_cb_t cb, void *ctxt);
    bool del_from_subnet_db(pds_subnet_key_t *key);
    slab_ptr_t subnet_slab(void) const { return cfg_db_->subnet_slab(); }

    pds_vnic_spec_t *find_in_vnic_db(pds_vnic_key_t *key);
    sdk_ret_t add_to_vnic_db(pds_vnic_key_t *key,
                             pds_vnic_spec_t *spec);
    sdk_ret_t vnic_db_walk(vnic_walk_cb_t cb, void *ctxt);
    bool del_from_vnic_db(pds_vnic_key_t *key);
    slab_ptr_t vnic_slab(void) const { return cfg_db_->vnic_slab(); }

    pds_route_table_spec_t *find_in_route_table_db(pds_route_table_key_t *key);
    sdk_ret_t add_to_route_table_db(pds_route_table_key_t *key,
                                    pds_route_table_spec_t *spec);
    bool del_from_route_table_db(pds_route_table_key_t *key);
    slab_ptr_t route_table_slab(void) const { return cfg_db_->route_table_slab(); }

private:
    void cleanup(void);
    tep_db_t *tep_map(void) const { return cfg_db_->tep_map();  }
    pcn_db_t *pcn_map(void) const { return cfg_db_->pcn_map();  }
    subnet_db_t *subnet_map(void) const { return cfg_db_->subnet_map();  }
    vnic_db_t *vnic_map(void) const { return cfg_db_->vnic_map();  }
    route_table_db_t *route_table_map(void) const { return cfg_db_->route_table_map();  }

private:
    cfg_db  *cfg_db_;
};

}    // namespace core

#endif    // __CORE_STATE_HPP__
