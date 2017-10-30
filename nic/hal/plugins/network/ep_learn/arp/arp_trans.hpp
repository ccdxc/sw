#ifndef HAL_PLUGINS_NETWORK_EP_LEARN_ARP_TRANS_HPP_
#define HAL_PLUGINS_NETWORK_EP_LEARN_ARP_TRANS_HPP_

#include "nic/include/base.h"
#include <netinet/if_ether.h>
#include "nic/utils/fsm/fsm.hpp"
#include "nic/fte/fte.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/include/ht.hpp"
#include "nic/include/twheel.hpp"
#include "nic/include/periodic.hpp"

#define HAL_MAX_ARP_TRANS 512
#define ARP_TIMER_ID 254

using hal::utils::ht_ctxt_t;
using hal::utils::fsm_state_t;
using hal::utils::fsm_state_ctx;
using hal::utils::fsm_event_data;
using hal::utils::fsm_state_machine_t;
using hal::utils::fsm_state_machine_def_t;

using hal::utils::fsm_timer_t;
using hal::utils::fsm_state_timer_ctx;
using hal::utils::twheel;

namespace hal {
namespace network {

enum arp_fsm_state_t {
    ARP_INIT,
    ARP_BOUND,
    ARP_DONE,
};

enum arp_fsm_event_t {
    ARP_ADD,
    ARP_REMOVE,
    ARP_ERROR,
    ARP_DUPLICATE,
    ARP_TIMEOUT,
};

class arp_trans_t;

// ARP transaction key
typedef struct arp_trans_key_s {
    l2seg_id_t l2_segid;  // L2 segment id
    mac_addr_t mac_addr;  // MAC address of the endpoint
} __PACK__ arp_trans_key_t;

typedef struct arp_ip_entry_key_s {
    tenant_id_t tenant_id;  // VRF id
    ip_addr_t ip_addr;      // IP address of the endpoint
} __PACK__ arp_ip_entry_key_t;

struct arp_event_data_t {
    const fte::ctx_t    *fte_ctx;
};

class arp_trans_t {
private:
    hal_spinlock_t slock_;  // lock to protect this structure
    arp_trans_key_t trans_key_;
    uint8_t hw_addr_[ETHER_ADDR_LEN]; /* hardware address */
    uint8_t protocol_addr_[4];        /* protocol address */

    arp_ip_entry_key_t ip_entry_key_;  // HAL allocated handle
    ht_ctxt_t ht_ctxt_;           // id based hash table ctxt
    ht_ctxt_t ip_entry_ht_ctxt_;  // IP based hash table ctxt

    class arp_fsm_t;    // forward declaration.
    class arp_timer_t;  // Forward declaration.
    static arp_fsm_t *arp_fsm_;
    static arp_timer_t *arp_timer_;
    fsm_state_machine_t *sm_;

    class arp_timer_t : public fsm_timer_t {
       public:
        arp_timer_t() : fsm_timer_t(ARP_TIMER_ID) {}

        virtual fsm_state_timer_ctx add_timer(uint64_t timeout, void *ctx,
                                              bool periodic = false) {
            void *timer = hal::periodic::periodic_timer_schedule(
                ARP_TIMER_ID, timeout, ctx, timeout_handler, periodic);
            return reinterpret_cast<fsm_state_timer_ctx>(timer);
        }
        virtual void delete_timer(fsm_state_timer_ctx timer) {
            hal::periodic::periodic_timer_delete(timer);
        }
        static void timeout_handler(uint32_t timer_id, void *ctxt);

        ~arp_timer_t() {}
    };

    class arp_fsm_t {
        void _init_state_machine();

        void set_state_machine(fsm_state_machine_def_t sm_def) {
            this->sm_def = sm_def;
        }
        bool process_arp_request(fsm_state_ctx ctx, fsm_event_data data);
        void bound_entry_func(fsm_state_ctx ctx);

       public:
        fsm_state_machine_def_t sm_def;
        arp_fsm_t() { this->_init_state_machine(); }
    };

    static arp_trans_t *arp_trans_alloc(void) {
        arp_trans_t *arp_trans;

        arp_trans = (arp_trans_t *)g_hal_state->arplearn_slab()->alloc();
        if (arp_trans == NULL) {
            return NULL;
        }
        return arp_trans;
    }

    static arp_trans_t *arp_trans_init(arp_trans_t *arp_trans) {
        if (!arp_trans) {
            return NULL;
        }
        HAL_SPINLOCK_INIT(&arp_trans->slock_, PTHREAD_PROCESS_PRIVATE);

        // initialize meta information
        arp_trans->ht_ctxt_.reset();
        arp_trans->ip_entry_ht_ctxt_.reset();

        return arp_trans;
    }

    static arp_trans_t *arp_trans_alloc_init(void) {
        return arp_trans_init(arp_trans_alloc());
    }

    static hal_ret_t arp_trans_free(arp_trans_t *arp_trans) {
        HAL_SPINLOCK_DESTROY(&arp_trans->slock_);
        g_hal_state->arplearn_slab()->free(arp_trans);
        return HAL_RET_OK;
    }
    bool transaction_completed() { return sm_->state_machine_competed(); }
    static fsm_state_machine_def_t *get_sm_def_func() {
        return &arp_fsm_->sm_def;
    }
    static fsm_timer_t *get_timer_func() { return arp_timer_; }
    void process_event(arp_fsm_event_t event, fsm_event_data data);

   public:
    arp_trans_key_t *trans_key_ptr() { return &trans_key_; }
    arp_ip_entry_key_t *ip_entry_key_ptr() { return &ip_entry_key_; }
    static void init_arp_trans_key(const uint8_t *hw_addr, const ep_t *ep,
                                   arp_trans_key_t *trans_key);
    static void init_arp_ip_entry_key(const uint8_t *protocol_address,
                                            tenant_id_t tenant_id,
                                             arp_ip_entry_key_t *ip_entry_key);

    static inline arp_trans_t *find_arptrans_by_id(arp_trans_key_t id) {
        return (arp_trans_t *)g_hal_state->arplearn_key_ht()->lookup(&id);
    }

    static inline arp_trans_t *find_arptrans_by_handle(hal_handle_t handle) {
        return (arp_trans_t *)g_hal_state->arplearn_ip_entry_ht()->lookup(
            &handle);
    }

    void *operator new(size_t size);
    void operator delete(void *p);
    static void process_transaction(arp_trans_t *trans, arp_fsm_event_t event,
                                    fsm_event_data data);
    explicit arp_trans_t(uint8_t *hw_address, uint8_t *protocol_address,
                         fte::ctx_t &ctx);
    arp_fsm_state_t get_state();
    uint32_t get_current_state_timeout();
    ~arp_trans_t();

} __PACK__;

void *arptrans_get_key_func(void *entry);
uint32_t arptrans_compute_hash_func(void *key, uint32_t ht_size);
bool arptrans_compare_key_func(void *key1, void *key2);

void *arptrans_get_ip_entry_key_func(void *entry);
uint32_t arptrans_compute_ip_entry_hash_func(void *key, uint32_t ht_size);
bool arptrans_compare_ip_entry_key_func(void *key1, void *key2);

void *arptrans_get_handle_key_func(void *entry);
uint32_t arptrans_compute_handle_hash_func(void *key, uint32_t ht_size);
bool arptrans_compare_handle_key_func(void *key1, void *key2);

}  // namespace network
}  // namespace hal

#endif /* HAL_PLUGINS_NETWORK_EP_LEARN_ARP_TEST_ARP_TRANS_HPP_ */
