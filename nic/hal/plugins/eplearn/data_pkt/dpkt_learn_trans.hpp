//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef HAL_PLUGINS_NETWORK_EP_LEARN_DATA_PKT_TRANS_HPP_
#define HAL_PLUGINS_NETWORK_EP_LEARN_DATA_PKT_TRANS_HPP_

#include "nic/include/base.hpp"
#include <netinet/if_ether.h>
#include "nic/utils/fsm/fsm.hpp"
#include "nic/fte/fte.hpp"
#include "nic/include/hal_state.hpp"
#include "sdk/ht.hpp"
#include "sdk/twheel.hpp"
#include "sdk/periodic.hpp"
#include "../common/trans.hpp"

#define HAL_MAX_DPKT_LEARN_TRANS 512
#define DPKT_LEARN_TIMER_ID 253

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

enum dpkt_learn_fsm_state_t {
    DPKT_LEARN_INIT,
    DPKT_LEARN_BOUND,
    DPKT_LEARN_WAIT_FOR_DELETE,
    DPKT_LEARN_DONE,
};

#define DPKT_LEARN_FSM_EVENT_ENTRIES(ENTRY)                                    \
    ENTRY(DPKT_LEARN_ADD,               0, "DATA_PKT_LEARN_ADD")               \
    ENTRY(DPKT_LEARN_REMOVE,            1, "DATA_PKT_LEARN_REMOVE")            \
    ENTRY(DPKT_LEARN_SESSIONS_DELETED,  2, "DATA_PKT_LEARN_SESSIONS_DELETED")   \
    ENTRY(DPKT_LEARN_REBOUND,           3, "DATA_PKT_LEARN_REBOUND")            \
    ENTRY(DPKT_LEARN_ERROR,             4, "DATA_PKT_LEARN_ERROR")              \
    ENTRY(DPKT_LEARN_TIMEOUT,           5, "DATA_PKT_LEARN_TIMEOUT")            \
    ENTRY(DPKT_IP_ADD,                  7, "DATA_PKT_IP_ADD")                   \

DEFINE_ENUM(dpkt_learn_fsm_event_t, DPKT_LEARN_FSM_EVENT_ENTRIES)


class dpkt_learn_trans_t;

// Flow miss transaction key
typedef struct dpkt_learn_trans_key_s {
    vrf_id_t         vrf_id;    //tenant id
    l2seg_id_t       l2_segid;  // L2 segment id
    mac_addr_t       mac_addr;  // MAC address of the endpoint
    ip_addr_t        ip_addr;   //ip address

} __PACK__ dpkt_learn_trans_key_t;

struct dpkt_learn_event_data_t {
    fte::ctx_t        *fte_ctx;
    hal_handle_t      ep_handle;
    ip_addr_t         ip_addr;
    bool              in_fte_pipeline;
};

class dpkt_learn_trans_t : public trans_t {
public:
    static ht *dpkt_learn_key_ht(void) { return dpkt_learn_key_ht_; }
    static ht *dpkt_learn_ip_entry_ht(void) { return dpkt_learn_ip_entry_ht_; }
    static slab *dpkt_learn_slab(void) { return dpkt_learn_slab_; }

private:
    dpkt_learn_trans_key_t trans_key_;
    fsm_state_timer_ctx dpkt_learn_timer_ctx;
    uint8_t hw_addr_[ETHER_ADDR_LEN]; /* hardware address */
    ip_addr_t ip_addr_;

    ht_ctxt_t ht_ctxt_;           // id based hash table ctxt
    ht_ctxt_t ip_entry_ht_ctxt_;  // IP based hash table ctxt

    class dpkt_learn_fsm_t;    // forward declaration.
    class dpkt_learn_timer_t;  // Forward declaration.
    static dpkt_learn_fsm_t *dpkt_learn_fsm_;
    static trans_timer_t *dpkt_learn_timer_;

    static slab *dpkt_learn_slab_;
    static ht *dpkt_learn_key_ht_;
    static ht *dpkt_learn_ip_entry_ht_;

    class dpkt_learn_fsm_t {
        void _init_state_machine();

        void set_state_machine(fsm_state_machine_def_t sm_def) {
            this->sm_def = sm_def;
        }
        bool process_dpkt_learn_request(fsm_state_ctx ctx, fsm_event_data data);
        bool process_dpkt_learn_timeout(fsm_state_ctx ctx, fsm_event_data data);
        bool process_sessions_deleted(fsm_state_ctx ctx, fsm_event_data data);
        bool add_ip_entry(fsm_state_ctx ctx, fsm_event_data data);
        bool del_ip_entry(fsm_state_ctx ctx, fsm_event_data data);
        void bound_entry_func(fsm_state_ctx ctx);

       public:
        fsm_state_machine_def_t sm_def;
        dpkt_learn_fsm_t() { this->_init_state_machine(); }
        static void set_state_timeout(uint32_t state, uint32_t timeout);
    };

    static dpkt_learn_trans_t *dpkt_learn_trans_alloc(void) {
        dpkt_learn_trans_t *dpkt_learn_trans;

        dpkt_learn_trans = (dpkt_learn_trans_t *)dpkt_learn_slab_->alloc();
        if (dpkt_learn_trans == NULL) {
            return NULL;
        }
        return dpkt_learn_trans;
    }

    static dpkt_learn_trans_t *dpkt_learn_trans_init(dpkt_learn_trans_t *dpkt_learn_trans) {
        if (!dpkt_learn_trans) {
            return NULL;
        }
        dpkt_learn_trans->ht_ctxt_.reset();
        dpkt_learn_trans->ip_entry_ht_ctxt_.reset();
        return dpkt_learn_trans;
    }

    static dpkt_learn_trans_t *dpkt_learn_trans_alloc_init(void) {
        return dpkt_learn_trans_init(dpkt_learn_trans_alloc());
    }

    static hal_ret_t dpkt_learn_trans_free(dpkt_learn_trans_t *dpkt_learn_trans) {
        dpkt_learn_slab_->free(dpkt_learn_trans);
        return HAL_RET_OK;
    }

    static fsm_state_machine_def_t *get_sm_def_func() {
        return &dpkt_learn_fsm_->sm_def;
    }
    static fsm_timer_t *get_timer_func() { return dpkt_learn_timer_; }
    void process_event(dpkt_learn_fsm_event_t event, fsm_event_data data);

   public:
    dpkt_learn_trans_key_t *trans_key_ptr() { return &trans_key_; }
    void set_up_ip_entry_key(const ip_addr_t *ip_addr);
    bool protocol_address_match(const ip_addr_t *ip_addr);
    static void init_dpkt_learn_trans_key(const uint8_t *hw_addr, const ep_t *ep,
                                   ip_addr_t *ip_addr, dpkt_learn_trans_key_t *trans_key);
    ep_t* get_ep_entry();
    static ht* get_ip_ht() { return dpkt_learn_ip_entry_ht_;}


    static inline dpkt_learn_trans_t *find_dpkt_learn_trans_by_id(dpkt_learn_trans_key_t id) {
        return (dpkt_learn_trans_t *)dpkt_learn_key_ht_->lookup(&id);
    }

    static inline dpkt_learn_trans_t *find_dpkt_learntrans_by_handle(hal_handle_t handle) {
        return (dpkt_learn_trans_t *)dpkt_learn_ip_entry_ht_->lookup(
            &handle);
    }

    static dpkt_learn_trans_t* find_dpkt_learn_trans_by_key(trans_ip_entry_key_t *key) {
        return reinterpret_cast<dpkt_learn_trans_t *>(
                dpkt_learn_trans_t::dpkt_learn_ip_entry_ht()->lookup(key));
    }

    void *operator new(size_t size);
    void operator delete(void *p);
    explicit dpkt_learn_trans_t(dpkt_learn_trans_key_t *trans, fte::ctx_t &ctx);
    dpkt_learn_fsm_state_t get_state();
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
    virtual ~dpkt_learn_trans_t();

} __PACK__;

void *dpkt_learn_trans_get_key_func(void *entry);
uint32_t dpkt_learn_trans_compute_hash_func(void *key, uint32_t ht_size);
bool dpkt_learn_tras_compare_key_func(void *key1, void *key2);

void *dpkt_learn_trans_get_handle_key_func(void *entry);
uint32_t dpkt_learn_trans_compute_handle_hash_func(void *key, uint32_t ht_size);
bool fmis_learn_trans_compare_handle_key_func(void *key1, void *key2);

hal_ret_t dpkt_learn_process_ip_move(hal_handle_t ep_handle, const ip_addr_t *ip_addr);
void dpkt_learn_entry_delete_timeout_set(uint32_t timeout);

}  // namespace eplearn
}  // namespace hal

#endif
