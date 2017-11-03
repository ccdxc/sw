// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/utils/eventmgr/eventmgr.hpp"

namespace hal {
namespace utils {

static void *
eventmap_get_key_func (void *entry)
{
    return (void *)&(((event_state_t *)entry)->event_id);
}

static uint32_t
eventmap_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(event_id_t)) % ht_size;
}

static bool
eventmap_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(event_id_t *)key1 == *(event_id_t *)key2) {
        return true;
    }
    return false;
}

// initialize event manager instance
int
eventmgr::init(uint32_t max_events)
{
    eventmap_ = ht::factory(max_events, eventmap_get_key_func,
                            eventmap_compute_hash_func,
                            eventmap_compare_key_func, true);
    if (eventmap_ == NULL) {
        return -1;
    }

    eventmap_slab_ = slab::factory("eventmgr", HAL_SLAB_EVENT_MAP,
                                   sizeof(event_state_t), 16, true,
                                   true, true, true);
    if (eventmap_slab_ == NULL) {
        delete eventmap_;
        return -1;
    }

    listener_state_slab_ = slab::factory("eventmgr", HAL_SLAB_EVENT_MAP_LISTENER,
                                         sizeof(event_listener_state_t), 16,
                                         true, true, true, true);
    if (listener_state_slab_ == NULL) {
        delete eventmap_;
        delete eventmap_slab_;
        return -1;
    }

    return 0;
}

// constructor
eventmgr::eventmgr()
{
}

// factory method for event manager class
eventmgr *
eventmgr::factory(uint32_t max_events)
{
    void        *mem;
    eventmgr    *new_eventmgr;

    mem = HAL_CALLOC(HAL_MEM_ALLOC_EVENT_MGR, sizeof(eventmgr));
    if (!mem) {
        return NULL;
    }

    new_eventmgr = new (mem) eventmgr();
    if (new_eventmgr->init(max_events) < 0) {
        new_eventmgr->~eventmgr();
        HAL_FREE(HAL_MEM_ALLOC_EVENT_MGR, new_eventmgr);
        return NULL;
    }

    return new_eventmgr;
}

eventmgr::~eventmgr()
{
    if (eventmap_) {
        delete eventmap_;
    }
    if (eventmap_slab_) {
        delete eventmap_slab_;
    }
    if (listener_state_slab_) {
        delete listener_state_slab_;
    }
}

hal_ret_t
eventmgr::add_eventmap_entry_(event_id_t event_id, void *lctxt)
{
    hal_ret_t                 ret;
    event_state_t             *event_state;
    event_listener_state_t    *lstate;

    event_state = (event_state_t *)eventmap_slab_->alloc();
    if (event_state == NULL) {
        HAL_TRACE_ERR("Failed to allocate memory for event map state");
        return HAL_RET_OOM;
    }
    lstate = (event_listener_state_t *)listener_state_slab_->alloc();
    if (lstate == NULL) {
        HAL_TRACE_ERR("Failed to allocate memory for event listener state");
        eventmap_slab_->free(event_state);
        return HAL_RET_OOM;
    }
    event_state->event_id = event_id;
    HAL_SPINLOCK_INIT(&event_state->slock, PTHREAD_PROCESS_PRIVATE);
    dllist_reset(&event_state->lctxt_head);
    event_state->ht_ctxt.reset();
    lstate->lctxt = lctxt;
    dllist_reset(&lstate->lentry);
    dllist_add(&event_state->lctxt_head, &lstate->lentry);
    ret = eventmap_->insert_with_key(&event_id, event_state,
                                     &event_state->ht_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Event {} subscription failed with err {}",
                      event_id, ret);
        HAL_SPINLOCK_DESTROY(&event_state->slock);
        listener_state_slab_->free(lstate);
        eventmap_slab_->free(event_state);
    }
    return ret;
}

hal_ret_t
eventmgr::subscribe(event_id_t event_id, void *lctxt)
{
    hal_ret_t                 ret;
    event_state_t             *event_state;
    event_listener_state_t    *lstate;
    dllist_ctxt_t             *node;
    bool                      exists = false;

    event_state = (event_state_t *)eventmap_->lookup(&event_id);
    if (event_state == NULL) {
        HAL_TRACE_DEBUG("First subscribtion to event {}", event_id);
        return add_eventmap_entry_(event_id, lctxt);
    } else {
        // walk the list of subscriptions and make sure this subscribtion
        // doesn't exist already
        HAL_SPINLOCK_LOCK(&event_state->slock);
        dllist_for_each(node, &event_state->lctxt_head) {
            lstate = dllist_entry(node, event_listener_state_t, lentry);
            if (lstate->lctxt == lctxt) {
                ret = HAL_RET_ENTRY_EXISTS;
                HAL_TRACE_DEBUG("Event {} subscription for the stream exists already",
                                event_id);
                exists = true;
                break;
            }
        }

        if (exists == false) {
            // add this stream to listeners
            lstate = (event_listener_state_t *)listener_state_slab_->alloc();
            if (lstate == NULL) {
                HAL_TRACE_ERR("Failed to allocate memory for event listener state");
                ret = HAL_RET_OOM;
            } else {
                lstate->lctxt = lctxt;
                dllist_reset(&lstate->lentry);
                dllist_add(&event_state->lctxt_head, &lstate->lentry);
            }
        }
        HAL_SPINLOCK_UNLOCK(&event_state->slock);
    }

    return ret;
}

hal_ret_t
eventmgr::unsubscribe(event_id_t event_id, void *lctxt)
{
    event_state_t             *event_state;
    event_listener_state_t    *lstate;
    dllist_ctxt_t             *curr, *next;

    event_state = (event_state_t *)eventmap_->lookup(&event_id);
    if (event_state == NULL) {
        HAL_TRACE_DEBUG("No subscribers for event {}");
        return HAL_RET_OK;
    }
    HAL_SPINLOCK_LOCK(&event_state->slock);
    dllist_for_each_safe(curr, next, &event_state->lctxt_head) {
        lstate = dllist_entry(curr, event_listener_state_t, lentry);
        if (lstate->lctxt == lctxt) {
            HAL_TRACE_DEBUG("Unsubscribed listener from event {}", event_id);
            utils::dllist_del(&lstate->lentry);
            listener_state_slab_->free(lstate);
            break;
        }
    }
    HAL_SPINLOCK_UNLOCK(&event_state->slock);
    return HAL_RET_OK;
}

hal_ret_t
eventmgr::unsubscribe_listener(void *lctxt)
{
    return HAL_RET_OK;
}

hal_ret_t
eventmgr::walk_listeners(event_id_t event_id, event_walk_cb_t walk_cb)
{
    event_state_t             *event_state;
    event_listener_state_t    *lstate;
    dllist_ctxt_t             *node;

    event_state = (event_state_t *)eventmap_->lookup(&event_id);
    if (event_state == NULL) {
        return HAL_RET_ENTRY_NOT_FOUND;
    }
    HAL_SPINLOCK_LOCK(&event_state->slock);
    dllist_for_each(node, &event_state->lctxt_head) {
        lstate = dllist_entry(node, event_listener_state_t, lentry);
        walk_cb(lstate->lctxt);
    }
    HAL_SPINLOCK_UNLOCK(&event_state->slock);

    return HAL_RET_OK;
}

}    // namespace utils
}    // namespace hal
