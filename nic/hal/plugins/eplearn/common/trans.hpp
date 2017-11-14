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

class trans_t {
private:
    hal_spinlock_t slock_;  // lock to protect this structure
    bool marked_for_delete_;

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
                delete trans;
            }
        }
        HAL_SPINLOCK_UNLOCK(&trans->slock_);
    }
    bool transaction_completed() { return sm_->state_machine_competed(); }

    uint32_t get_current_state_timeout() {
        return this->sm_->get_current_state_timeout();
    }

};

#endif /* HAL_PLUGINS_NETWORK_EP_LEARN_COMMON_TRANS_HPP_ */
