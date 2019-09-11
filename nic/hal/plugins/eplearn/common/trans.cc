//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "lib/ht/ht.hpp"
#include "nic/fte/fte.hpp"
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include "nic/hal/plugins/eplearn/common/trans.hpp"
#include "nic/hal/plugins/eplearn/eplearn.hpp"

namespace hal {
namespace eplearn {

void *trans_get_ip_entry_key_func(void *entry) {
    SDK_ASSERT(entry != NULL);
    return (void *)(((trans_t *)entry)->ip_entry_key_ptr());
}

uint32_t trans_compute_ip_entry_hash_func(void *key, uint32_t ht_size) {
    return sdk::lib::hash_algo::fnv_hash(key,
                                         sizeof(trans_ip_entry_key_t)) % ht_size;
}

bool trans_compare_ip_entry_key_func(void *key1, void *key2) {
    SDK_ASSERT((key1 != NULL) && (key2 != NULL));
    if (memcmp(key1, key2, sizeof(trans_ip_entry_key_t)) == 0) {
        return true;
    }
    return false;
}

void
trans_t::process_learning_transaction(trans_t *trans, fte::ctx_t &ctx, uint32_t event,
            fsm_event_data data) {
    eplearn_info_t *eplearn_info = (eplearn_info_t*)\
                ctx.feature_state(FTE_FEATURE_EP_LEARN);

    process_transaction(trans, event, data);
    if (!trans->trans_marked_for_delete() &&
            !ctx.completion_handler_registered()) {
        /* Register NOP event in case we have to delete this transaction */
        eplearn_info->trans_ctx[0].trans = trans;
        eplearn_info->trans_ctx[0].event = TRANS_NOP_EVENT;
        eplearn_info->trans_ctx_cnt++;
    }
}

void
trans_t::trans_completion_handler(fte::ctx_t& ctx, bool status) {
    eplearn_info_t *eplearn_info = (eplearn_info_t*)\
                ctx.feature_state(FTE_FEATURE_EP_LEARN);

    uint32_t event;

    for (uint32_t i = 0; i < eplearn_info->trans_ctx_cnt; i++) {
         /* Disabling out this code , learn even if packet is dropped! */
        if (0 && !status) {
            eplearn_info->trans_ctx[i].trans->log_info("Removing"
                    " transaction as FTE pipeline processing failed. ");
            event = eplearn_info->trans_ctx[i].trans->sm_->get_remove_event();
        } else if (eplearn_info->trans_ctx[i].event != TRANS_NOP_EVENT) {
            eplearn_info->trans_ctx[i].trans->log_info("Executing"
                    " completion handler ");
            event = eplearn_info->trans_ctx[i].event;
        } else {
            continue;
        }
        trans_t::process_transaction(eplearn_info->trans_ctx[i].trans,
                    event, (fsm_event_data)(&eplearn_info->trans_ctx[i].event_data));
    }
}


}  //  namespace eplearn
}  //  namespace hal
