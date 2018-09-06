//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef HAL_PLUGINS_NETWORK_EP_LEARN_FLOW_MISS_TRANS_HPP_
#define HAL_PLUGINS_NETWORK_EP_LEARN_FLOW_MISS_TRANS_HPP_

#include "nic/include/base.hpp"
#include <netinet/if_ether.h>
#include "nic/utils/fsm/fsm.hpp"
#include "nic/fte/fte.hpp"
#include "nic/include/hal_state.hpp"
#include "sdk/ht.hpp"
#include "sdk/twheel.hpp"
#include "sdk/periodic.hpp"
#include "../common/trans.hpp"

#define HAL_MAX_FMISS_LEARN_TRANS 512
#define FMISS_LEARN_TIMER_ID 253

using sdk::lib::ht_ctxt_t;
using hal::utils::fsm_state_t;
using hal::utils::fsm_state_ctx;
using hal::utils::fsm_event_data;
using hal::utils::fsm_state_machine_t;
using hal::utils::fsm_state_machine_def_t;

using hal::utils::fsm_timer_t;
using hal::utils::fsm_state_timer_ctx;
using sdk::lib::twheel;

namespace hal {
namespace eplearn {

enum fmiss_learn_fsm_state_t {
    FMISS_LEARN_INIT,
    FMISS_LEARN_BOUND,
    FMISS_LEARN_DONE,
};

#define FMISS_LEARN_FSM_EVENT_ENTRIES(ENTRY)                   \
    ENTRY(FMISS_LEARN_ADD,             0, "FLOW_MISS_LEARN_ADD")      \
    ENTRY(FMISS_LEARN_REMOVE,          1, "FLOW_MISS_LEARN_REMOVE")   \
    ENTRY(FMISS_LEARN_ERROR,           2, "FLOW_MISS_LEARN_ERROR")    \
    ENTRY(FMISS_LEARN_TIMEOUT,         3, "FLOW_MISS_LEARN_TIMEOUT")  \
    ENTRY(FMISS_IP_ADD,                4, "FLOW_MISS_IP_ADD")               \

DEFINE_ENUM(fmiss_learn_fsm_event_t, FMISS_LEARN_FSM_EVENT_ENTRIES)


class fmiss_learn_trans_t;

// Flow miss transaction key
typedef struct fmiss_learn_trans_key_s {
    vrf_id_t         vrf_id;    //tenant id
    l2seg_id_t       l2_segid;  // L2 segment id
    mac_addr_t       mac_addr;  // MAC address of the endpoint
    ip_addr_t        ip_addr;   //ip address

} __PACK__ fmiss_learn_trans_key_t;

struct fmiss_learn_event_data_t {
    fte::ctx_t        *fte_ctx;
    hal_handle_t      ep_handle;
    ip_addr_t         ip_addr;
    bool              in_fte_pipeline;
};

class fmiss_learn_trans_t : public trans_t {
public:
    static ht *fmiss_learn_key_ht(void) { return fmiss_learn_key_ht_; }
    static ht *fmiss_learn_ip_entry_ht(void) { return fmiss_learn_ip_entry_ht_; }
    static slab *fmiss_learn_slab(void) { return fmiss_learn_slab_; }

private:
    fmiss_learn_trans_key_t trans_key_;
    fsm_state_timer_ctx fmiss_learn_timer_ctx;
    uint8_t hw_addr_[ETHER_ADDR_LEN]; /* hardware address */
    ip_addr_t ip_addr_;

    ht_ctxt_t ht_ctxt_;           // id based hash table ctxt
    ht_ctxt_t ip_entry_ht_ctxt_;  // IP based hash table ctxt

    class fmiss_learn_fsm_t;    // forward declaration.
    class fmiss_learn_timer_t;  // Forward declaration.
    static fmiss_learn_fsm_t *fmiss_learn_fsm_;
    static trans_timer_t *fmiss_learn_timer_;

    static slab *fmiss_learn_slab_;
    static ht *fmiss_learn_key_ht_;
    static ht *fmiss_learn_ip_entry_ht_;

    class fmiss_learn_fsm_t {
        void _init_state_machine();

        void set_state_machine(fsm_state_machine_def_t sm_def) {
            this->sm_def = sm_def;
        }
        bool process_fmiss_learn_request(fsm_state_ctx ctx, fsm_event_data data);
        bool process_fmiss_learn_timeout(fsm_state_ctx ctx, fsm_event_data data);
        bool add_ip_entry(fsm_state_ctx ctx, fsm_event_data data);
        bool del_ip_entry(fsm_state_ctx ctx, fsm_event_data data);
        void bound_entry_func(fsm_state_ctx ctx);

       public:
        fsm_state_machine_def_t sm_def;
        fmiss_learn_fsm_t() { this->_init_state_machine(); }
        static void set_state_timeout(uint32_t state, uint32_t timeout);
    };

    static fmiss_learn_trans_t *fmiss_learn_trans_alloc(void) {
        fmiss_learn_trans_t *fmiss_learn_trans;

        fmiss_learn_trans = (fmiss_learn_trans_t *)fmiss_learn_slab_->alloc();
        if (fmiss_learn_trans == NULL) {
            return NULL;
        }
        return fmiss_learn_trans;
    }

    static fmiss_learn_trans_t *fmiss_learn_trans_init(fmiss_learn_trans_t *fmiss_learn_trans) {
        if (!fmiss_learn_trans) {
            return NULL;
        }
        fmiss_learn_trans->ht_ctxt_.reset();
        fmiss_learn_trans->ip_entry_ht_ctxt_.reset();
        return fmiss_learn_trans;
    }

    static fmiss_learn_trans_t *fmiss_learn_trans_alloc_init(void) {
        return fmiss_learn_trans_init(fmiss_learn_trans_alloc());
    }

    static hal_ret_t fmiss_learn_trans_free(fmiss_learn_trans_t *fmiss_learn_trans) {
        fmiss_learn_slab_->free(fmiss_learn_trans);
        return HAL_RET_OK;
    }

    static fsm_state_machine_def_t *get_sm_def_func() {
        return &fmiss_learn_fsm_->sm_def;
    }
    static fsm_timer_t *get_timer_func() { return fmiss_learn_timer_; }
    void process_event(fmiss_learn_fsm_event_t event, fsm_event_data data);

   public:
    fmiss_learn_trans_key_t *trans_key_ptr() { return &trans_key_; }
    void set_up_ip_entry_key(const ip_addr_t *ip_addr);
    bool protocol_address_match(const ip_addr_t *ip_addr);
    static void init_fmiss_learn_trans_key(const uint8_t *hw_addr, const ep_t *ep,
                                   ip_addr_t *ip_addr, fmiss_learn_trans_key_t *trans_key);
    ep_t* get_ep_entry();
    static ht* get_ip_ht() { return fmiss_learn_ip_entry_ht_;}


    static inline fmiss_learn_trans_t *find_fmiss_learn_trans_by_id(fmiss_learn_trans_key_t id) {
        return (fmiss_learn_trans_t *)fmiss_learn_key_ht_->lookup(&id);
    }

    static inline fmiss_learn_trans_t *find_fmiss_learntrans_by_handle(hal_handle_t handle) {
        return (fmiss_learn_trans_t *)fmiss_learn_ip_entry_ht_->lookup(
            &handle);
    }

    static fmiss_learn_trans_t* find_fmiss_learn_trans_by_key(trans_ip_entry_key_t *key) {
        return reinterpret_cast<fmiss_learn_trans_t *>(
                fmiss_learn_trans_t::fmiss_learn_ip_entry_ht()->lookup(key));
    }

    void *operator new(size_t size);
    void operator delete(void *p);
    explicit fmiss_learn_trans_t(fmiss_learn_trans_key_t *trans, fte::ctx_t &ctx);
    fmiss_learn_fsm_state_t get_state();
    void reset();

    static void set_state_timeout(uint32_t state, uint32_t timeout) {
        trans_t::set_state_timeout(get_sm_def_func(), state, timeout);
    }

    virtual void log_info(const char *message) {
        HAL_TRACE_INFO("(tenant id : {}, l2_segid : {}, "
                "macaddr : {}, ip : {}) : {} ",
                this->trans_key_ptr()->vrf_id, this->trans_key_ptr()->l2_segid,
                macaddr2str(this->trans_key_ptr()->mac_addr),
                        ipaddr2str(&ip_entry_key_ptr()->ip_addr), message);
    }
    virtual void log_error(const char *message) {
        HAL_TRACE_ERR("(tenant id : {}, l2_segid : {}, "
                "macaddr : {}, ip : {}) : {} ",
                this->trans_key_ptr()->vrf_id, this->trans_key_ptr()->l2_segid,
                macaddr2str(this->trans_key_ptr()->mac_addr),
                ipaddr2str(&ip_entry_key_ptr()->ip_addr), message);
    }
    virtual ~fmiss_learn_trans_t();

} __PACK__;

void *fmiss_learn_trans_get_key_func(void *entry);
uint32_t fmiss_learn_trans_compute_hash_func(void *key, uint32_t ht_size);
bool fmiss_learn_tras_compare_key_func(void *key1, void *key2);

void *fmiss_learn_trans_get_handle_key_func(void *entry);
uint32_t fmiss_learn_trans_compute_handle_hash_func(void *key, uint32_t ht_size);
bool fmis_learn_trans_compare_handle_key_func(void *key1, void *key2);

hal_ret_t fmiss_learn_process_ip_move(hal_handle_t ep_handle, const ip_addr_t *ip_addr);

}  // namespace eplearn
}  // namespace hal

#endif
