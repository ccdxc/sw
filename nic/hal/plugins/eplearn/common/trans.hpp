/*
 * trans.hpp
 *
 */

#ifndef HAL_PLUGINS_NETWORK_EP_LEARN_COMMON_TRANS_HPP_
#define HAL_PLUGINS_NETWORK_EP_LEARN_COMMON_TRANS_HPP_
#include "nic/include/base.h"
#include <netinet/if_ether.h>
#include "nic/utils/fsm/fsm.hpp"
#include "nic/include/periodic.hpp"

using hal::utils::fsm_state_machine_t;
using hal::utils::fsm_event_data;
using hal::utils::fsm_timer_t;
using hal::utils::fsm_state_timer_ctx;
using namespace hal;

namespace hal {
namespace eplearn {

typedef struct trans_ip_entry_key_s {
    vrf_id_t vrf_id;    // VRF id
    ip_addr_t ip_addr;  // IP address of the endpoint
} __PACK__ trans_ip_entry_key_t;


class trans_t {
private:
    hal_spinlock_t slock_;  // lock to protect this structure
    bool marked_for_delete_;
    trans_ip_entry_key_t ip_entry_key_;

public:
    fsm_state_machine_t *sm_;
    trans_t() {
        HAL_SPINLOCK_INIT(&this->slock_, PTHREAD_PROCESS_PRIVATE);
        marked_for_delete_ = false;
        sm_= nullptr;
    }
    virtual ~trans_t() {
        HAL_SPINLOCK_DESTROY(&this->slock_);
    }

    virtual void log_error(const char *message)  {}
    virtual void log_info(const char *message)  {}

    class trans_timer_t : public fsm_timer_t {
       public:
        trans_timer_t(uint32_t timer_id) : fsm_timer_t(timer_id) {
        }

        virtual fsm_state_timer_ctx add_timer(uint64_t timeout,
                                              fsm_state_machine_t *ctx,
                                              bool periodic = false) {
            void *timer = hal::periodic::periodic_timer_schedule(
                this->get_timer_id(), timeout, ctx, timeout_handler, periodic);
            return reinterpret_cast<fsm_state_timer_ctx>(timer);
        }
        virtual void delete_timer(fsm_state_timer_ctx timer) {
            hal::periodic::periodic_timer_delete(timer);
        }
        static void timeout_handler(uint32_t timer_id, void *ctxt) {
            fsm_state_machine_t* sm_ = reinterpret_cast<fsm_state_machine_t*>(ctxt);
            sm_->reset_timer();
            trans_t* trans =
                reinterpret_cast<trans_t*>(sm_->get_ctx());
            trans_t::process_transaction(trans, sm_->get_timeout_event(), NULL);
        }

        ~trans_timer_t() {}
    };

    static void process_transaction(trans_t *trans, uint32_t event,
                fsm_event_data data) {
        if (trans->marked_for_delete_) {
            return;
        }
        HAL_SPINLOCK_LOCK(&trans->slock_);
        if (!trans->marked_for_delete_) {
            trans->sm_->process_event(event, data);
            if (trans->sm_->state_machine_competed()) {
                trans->marked_for_delete_ = true;
                /* Assuming delayed delete is setup initiate the transaction delete. */
                trans->log_info("Transaction completed, deleting...");
                delete trans;
            }
        } else {
            trans->log_error("Transaction in marked for delete state, skipping processing");
        }
        HAL_SPINLOCK_UNLOCK(&trans->slock_);
    }
    bool transaction_completed() { return sm_->state_machine_competed(); }

    uint32_t get_current_state_timeout() {
        return this->sm_->get_current_state_timeout();
    }

    ep_t* get_ep_entry() {
        ep_t *other_ep_entry = NULL;
        ep_l3_entry_t *other_ep_l3_entry;
        ep_l3_key_t l3_key = {0};

        l3_key.vrf_id = this->ip_entry_key_.vrf_id;
        memcpy(&l3_key.ip_addr, &ip_entry_key_.ip_addr, sizeof(ip_addr_t));

        /* Find the EP entry for this IP address */
        other_ep_l3_entry = reinterpret_cast<ep_l3_entry_t *>(
            g_hal_state->ep_l3_entry_ht()->lookup(&l3_key));

        if (other_ep_l3_entry != NULL) {
            other_ep_entry = find_ep_by_handle(other_ep_l3_entry->ep_hal_handle);
        }

        return other_ep_entry;
    }

    trans_ip_entry_key_t *ip_entry_key_ptr() { return &ip_entry_key_; }

    static void init_ip_entry_key(const ip_addr_t *ip_addr, vrf_id_t vrf_id,
                                  trans_ip_entry_key_t *ip_entry_key) {
        *ip_entry_key = {0};
        memcpy(&ip_entry_key->ip_addr, ip_addr, sizeof(ip_addr_t));
        ip_entry_key->vrf_id = vrf_id;
    }

};

void *trans_get_ip_entry_key_func(void *entry);
uint32_t trans_compute_ip_entry_hash_func(void *key, uint32_t ht_size);
bool trans_compare_ip_entry_key_func(void *key1, void *key2);

}
}
#endif /* HAL_PLUGINS_NETWORK_EP_LEARN_COMMON_TRANS_HPP_ */
