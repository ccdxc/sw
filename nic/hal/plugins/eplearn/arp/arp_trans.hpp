//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef HAL_PLUGINS_NETWORK_EP_LEARN_ARP_TRANS_HPP_
#define HAL_PLUGINS_NETWORK_EP_LEARN_ARP_TRANS_HPP_

#include "nic/include/base.hpp"
#include <netinet/if_ether.h>
#include "nic/utils/fsm/fsm.hpp"
#include "nic/fte/fte.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "lib/ht/ht.hpp"
#include "lib/twheel/twheel.hpp"
#include "lib/periodic/periodic.hpp"
#include "../common/trans.hpp"

#define HAL_MAX_ARP_TRANS 512
#define ARP_TIMER_ID 254

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

enum arp_fsm_state_t {
    ARP_INIT,
    ARP_BOUND,
    ARP_PROBING,
    RARP_INIT,
    ARP_DONE,
};

#define ARP_FSM_EVENT_ENTRIES(ENTRY)                   \
    ENTRY(ARP_ADD,             0, "ARP_ADD")                  \
    ENTRY(RARP_REQ,            1, "RARP_REQ")                 \
    ENTRY(RARP_REPLY,          2, "RARP_REPLY")               \
    ENTRY(ARP_REMOVE,          3, "ARP_REMOVE")               \
    ENTRY(ARP_ERROR,           4, "ARP_ERROR")                \
    ENTRY(ARP_DUPLICATE,       5, "ARP_DUPLICATE")            \
    ENTRY(ARP_TIMEOUT,         6, "ARP_TIMEOUT")              \
    ENTRY(ARP_IP_ADD,          7, "ARP_IP_ADD")               \
    ENTRY(ARP_IP_RESET_ADD,    8, "ARP_IP_RESET_ADD")         \
    ENTRY(ARP_PROBE_SENT,      9, "ARP_PROBE_SENT")           \

DEFINE_ENUM(arp_fsm_event_t, ARP_FSM_EVENT_ENTRIES)

enum arp_trans_type_t {
    ARP_TRANS_IPV4,
    ARP_TRANS_IPV6
};

class arp_trans_t;

// ARP transaction key
typedef struct arp_trans_key_s {
    vrf_id_t         vrf_id;    //tenant id
    l2seg_id_t       l2_segid;  // L2 segment id
    mac_addr_t       mac_addr;  // MAC address of the endpoint
    ip_addr_t        ip_addr;  // IP address of the endpoint
    arp_trans_type_t type;      // IPv4 or IPv6 trans type

} __PACK__ arp_trans_key_t;

struct arp_event_data_t {
    fte::ctx_t        *fte_ctx;
    hal_handle_t      ep_handle;
    ip_addr_t         ip_addr;
    bool              in_fte_pipeline;
};

class arp_trans_t : public trans_t {
public:
    static ht *arplearn_key_ht(void) { return arplearn_key_ht_; }
    static ht *arplearn_ip_entry_ht(void) { return arplearn_ip_entry_ht_; }
    static slab *arplearn_slab(void) { return arplearn_slab_; }
    static uint32_t arp_probe_timeout;

private:
    arp_trans_key_t trans_key_;
    fsm_state_timer_ctx arp_timer_ctx;
    uint8_t hw_addr_[ETHER_ADDR_LEN]; /* hardware address */
    ip_addr_t ip_addr_;

    ht_ctxt_t ht_ctxt_;           // id based hash table ctxt
    ht_ctxt_t ip_entry_ht_ctxt_;  // IP based hash table ctxt

    class arp_fsm_t;    // forward declaration.
    class arp_timer_t;  // Forward declaration.
    static arp_fsm_t *arp_fsm_;
    static trans_timer_t *arp_timer_;

    static slab *arplearn_slab_;
    static ht *arplearn_key_ht_;
    static ht *arplearn_ip_entry_ht_;

    class arp_fsm_t {
        void _init_state_machine();

        void set_state_machine(fsm_state_machine_def_t sm_def) {
            this->sm_def = sm_def;
        }
        bool process_arp_request(fsm_state_ctx ctx, fsm_event_data data);
        bool process_rarp_request(fsm_state_ctx ctx, fsm_event_data data);
        bool process_rarp_reply(fsm_state_ctx ctx, fsm_event_data data);
        bool process_arp_renewal_request(fsm_state_ctx ctx, fsm_event_data data);
        bool process_arp_timeout(fsm_state_ctx ctx, fsm_event_data data);
        bool process_arp_probe_timeout(fsm_state_ctx ctx, fsm_event_data data);
        bool reset_and_add_new_ip(fsm_state_ctx ctx, fsm_event_data data);
        bool add_ip_entry(fsm_state_ctx ctx, fsm_event_data data);
        bool del_ip_entry(fsm_state_ctx ctx, fsm_event_data data);
        void bound_entry_func(fsm_state_ctx ctx);

       public:
        fsm_state_machine_def_t sm_def;
        arp_fsm_t() { this->_init_state_machine(); }
        static void set_state_timeout(uint32_t state, uint32_t timeout);
    };

    static arp_trans_t *arp_trans_alloc(void) {
        arp_trans_t *arp_trans;

        arp_trans = (arp_trans_t *)arplearn_slab_->alloc();
        if (arp_trans == NULL) {
            return NULL;
        }
        return arp_trans;
    }

    static arp_trans_t *arp_trans_init(arp_trans_t *arp_trans) {
        if (!arp_trans) {
            return NULL;
        }
        //SDK_SPINLOCK_INIT(&arp_trans->slock_, PTHREAD_PROCESS_PRIVATE);

        // initialize meta information
        arp_trans->ht_ctxt_.reset();
        arp_trans->ip_entry_ht_ctxt_.reset();

        return arp_trans;
    }

    static arp_trans_t *arp_trans_alloc_init(void) {
        return arp_trans_init(arp_trans_alloc());
    }

    static hal_ret_t arp_trans_free(arp_trans_t *arp_trans) {
        //SDK_SPINLOCK_DESTROY(&arp_trans->slock_);
        arplearn_slab_->free(arp_trans);
        return HAL_RET_OK;
    }

    static fsm_state_machine_def_t *get_sm_def_func() {
        return &arp_fsm_->sm_def;
    }
    static fsm_timer_t *get_timer_func() { return arp_timer_; }
    void process_event(arp_fsm_event_t event, fsm_event_data data);
    void start_arp_timer();
    void stop_arp_timer();

   public:
    arp_trans_key_t *trans_key_ptr() { return &trans_key_; }
    void set_up_ip_entry_key(const ip_addr_t *ip_addr);
    bool protocol_address_match(const ip_addr_t *ip_addr);
    static void init_arp_trans_key(const uint8_t *hw_addr, const ep_t *ep,
                                   arp_trans_type_t type, ip_addr_t *ip_addr,
                                   arp_trans_key_t *trans_key);
    ep_t* get_ep_entry();
    static ht* get_ip_ht() { return arplearn_ip_entry_ht_;}


    static inline arp_trans_t *find_arptrans_by_id(arp_trans_key_t id) {
        return (arp_trans_t *)arplearn_key_ht_->lookup(&id);
    }

    static inline arp_trans_t *find_arptrans_by_handle(hal_handle_t handle) {
        return (arp_trans_t *)arplearn_ip_entry_ht_->lookup(
            &handle);
    }

    static arp_trans_t* find_arp_trans_by_key(trans_ip_entry_key_t *key) {
        return reinterpret_cast<arp_trans_t *>(
                arp_trans_t::arplearn_ip_entry_ht()->lookup(key));
    }

    void *operator new(size_t size);
    void operator delete(void *p);
    explicit arp_trans_t(arp_trans_key_t *trans_key, fte::ctx_t &ctx);
    arp_fsm_state_t get_state();
    void reset();

    static void set_state_timeout(uint32_t state, uint32_t timeout) {
        trans_t::set_state_timeout(get_sm_def_func(), state, timeout);
    }

    static void set_arp_probe_timeout(uint32_t timeout) {
        arp_probe_timeout = timeout * TIME_MSECS_PER_SEC;
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
    virtual ~arp_trans_t();

} __PACK__;

void *arptrans_get_key_func(void *entry);
uint32_t arptrans_key_size(void);

void *arptrans_get_handle_key_func(void *entry);
uint32_t arptrans_handle_key_size(void);

hal_ret_t arp_process_ip_move(hal_handle_t ep_handle, const ip_addr_t *ip_addr);

bool process_vmotion_rarp(fte::ctx_t* ctx);

}  // namespace eplearn
}  // namespace hal

#endif /* HAL_PLUGINS_NETWORK_EP_LEARN_ARP_TEST_ARP_TRANS_HPP_ */
