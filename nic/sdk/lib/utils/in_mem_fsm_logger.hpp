//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// In memory FSM logger utility.
///
//----------------------------------------------------------------------------

#ifndef __SDK_IN_MEM_FSM_LOGGER_HPP__
#define __SDK_IN_MEM_FSM_LOGGER_HPP__

#include "include/sdk/timestamp.hpp"
#include "include/sdk/mem.hpp"

namespace sdk {
namespace utils {

typedef struct record_s {
    timespec_t real_ts;
    timespec_t mon_ts;
    char       data[0];
} __PACK__ record_t;

typedef bool (*in_mem_fsm_logger_walk_cb_t)(record_t *record, void *ctxt);

class in_mem_fsm_logger {
public:
    static in_mem_fsm_logger *factory(uint32_t capacity,
                                      uint32_t data_size) {
        void *mem = NULL;
        in_mem_fsm_logger *logger = NULL;

        mem = SDK_CALLOC(SDK_MEM_ALLOC_IN_MEM_FSM_LOGGER,
                         sizeof(in_mem_fsm_logger));
        if (!mem) {
            return NULL;
        }

        logger = new (mem) in_mem_fsm_logger();
        auto ret = logger->init_(capacity, data_size);
        if (ret != SDK_RET_OK) {
            logger->~in_mem_fsm_logger();
            SDK_FREE(SDK_MEM_ALLOC_IN_MEM_FSM_LOGGER, logger);
            return NULL;
        }
        return logger;
    }

    static void destroy(in_mem_fsm_logger *logger) {
        if (logger) {
            logger->cleanup_();
            logger->~in_mem_fsm_logger();
            SDK_FREE(SDK_MEM_ALLOC_IN_MEM_FSM_LOGGER, logger);
        }
    }

    uint32_t capacity(void) const {
        return capacity_;
    }

    uint32_t num_records(void) const {
        return num_records_;
    }

    bool is_full(void) const {
        return (num_records_ == capacity_);
    }

    void reset(void) {
        tail_ = head_ = num_records_ = 0;
    }

    void append(const char *data) {
        timespec_t mon_ts;

        clock_gettime(CLOCK_MONOTONIC_RAW, &mon_ts);
        append_(data, &mon_ts);
    }

    void walk(in_mem_fsm_logger_walk_cb_t cb, void *ctxt) {
        uint32_t count = 0;
        uint32_t i = head_;
        record_t *record = NULL;

        if (num_records_ == 0 || cb == NULL) {
            return;
        }

        while (count < num_records_) {
            record = (record_t *) (store_ + get_offset_(i));
            if (cb(record, ctxt)) {
                return;
            }
            count++;
            circ_incr_(&i);
        }
    }

private:
    char      *store_;
    uint32_t  head_;
    uint32_t  tail_;
    uint32_t  capacity_;
    uint32_t  num_records_;
    uint32_t  data_size_;

private:
    in_mem_fsm_logger() {
        store_ = NULL;
        head_= 0;
        tail_ = 0;
        capacity_ = 0;
        num_records_ = 0;
        data_size_ = 0;
    }

    ~in_mem_fsm_logger() {};

    sdk_ret_t init_(uint32_t capacity, uint32_t data_size) {
        SDK_ASSERT(capacity > 0 && data_size > 0);
        capacity_ = capacity;
        data_size_ = data_size;
        store_ = (char *)SDK_CALLOC(SDK_MEM_ALLOC_IN_MEM_FSM_LOGGER,
                         ((sizeof(record_t) + data_size_) * capacity_));
        return (store_ == NULL) ? SDK_RET_OOM : SDK_RET_OK;
    }

    void cleanup_(void) {
        if (store_ != NULL)
            SDK_FREE(SDK_MEM_ALLOC_IN_MEM_FSM_LOGGER, store_);
    }

    void append_(const char *data, timespec_t *mon_ts) {
        // move head forward to make room for tail to grow.
        if (is_full()) {
            circ_incr_(&head_);
            num_records_--;
        }

        // write record
        record_t *record = (record_t *)(store_ + get_offset_(tail_));
        record->mon_ts = *mon_ts;
        clock_gettime(CLOCK_REALTIME, &record->real_ts);
        memcpy(&record->data, data, data_size_);

        // book keeping
        circ_incr_(&tail_);
        num_records_++;
    }

    void circ_incr_(uint32_t *i) {
        *i = ((*i+1) % capacity_);
    }

    uint32_t get_offset_(uint32_t i) {
        return (i * (sizeof(record_t) + data_size_));
    }
};

}    // namespace utils
}    // namespace sdk

#endif  // __SDK_IN_MEM_FSM_LOGGER_HPP__
