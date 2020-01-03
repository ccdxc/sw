// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "lib/thread/thread.hpp"
#include "lib/pal/pal.hpp"
#include "lib/utils/time_profile.hpp"
#include "asic/rw/asicrw.hpp"

using sdk::lib::pal_ret_t;
using sdk::lib::PAL_RET_OK;

namespace sdk {
namespace asic {

static bool g_asicrw_ready_;

#define ASICRW_NUM_QUEUES                16
#define ASICRW_Q_SIZE                    128
#define ASICRW_OP_MEM_READ               0
#define ASICRW_OP_MEM_WRITE              1
#define ASICRW_OP_REG_READ               2
#define ASICRW_OP_REG_WRITE              3
#define ASICRW_OP_RING_DOORBELL          4
#define ASICRW_OP_VMEM_READ              5
#define ASICRW_OP_VMEM_WRITE             6

//------------------------------------------------------------------------------
// asic read-write entry, one such entry is added to the asic read/write
// thread's queue whenever asic read/write is attempted by any thread
//------------------------------------------------------------------------------
typedef struct asicrw_entry_ {
    uint8_t              opn;        // operation requested to perform
    bool                 done;       // TRUE if thread performed operation
    sdk_ret_t            status;     // result status of operation requested
    uint64_t             addr;       // address to write to or read from
    uint32_t             len;        // length of data to read or write
    uint8_t              *data;      // data to write or buffer to copy data to for mem read/write
} asicrw_entry_t;

//------------------------------------------------------------------------------
// asic read-write thread maintains one queue per thread to serve read/write
// operations by thread, thus avoiding locking altogether
//------------------------------------------------------------------------------
typedef struct asicrw_queue_s {
    uint32_t                 nentries;    // no. of entries in the queue
    uint16_t                 pindx;       // producer index
    uint16_t                 cindx;       // consumer index
    asicrw_entry_t          entries[ASICRW_Q_SIZE];    // entries
} asicrw_queue_t;

// per producer read/write request queues
asicrw_queue_t    g_asicrw_workq[ASICRW_NUM_QUEUES];

//------------------------------------------------------------------------------
// return TRUE is asicrw thread is ready to do read/write operations
//------------------------------------------------------------------------------
bool
is_asicrw_ready (void)
{
    return SDK_ATOMIC_LOAD_BOOL(&g_asicrw_ready_);
}

//------------------------------------------------------------------------------
// read data from specified address in the memory to given buffer
// NOTE: this is always a blocking call and this API runs in the calling
//       thread's context
//------------------------------------------------------------------------------
static sdk_ret_t
asic_do_read (thread *curr_thread, uint8_t opn, uint64_t addr,
              uint8_t *data, uint32_t len)
{
    uint16_t          pindx = 0;
    asicrw_entry_t    *rw_entry;
    uint32_t          qid = curr_thread->thread_id();

    if (!data) {
        return SDK_RET_INVALID_ARG;
    }

    if (g_asicrw_workq[qid].nentries >= ASICRW_Q_SIZE) {
        SDK_TRACE_ERR("asic read operation failed, tid %u, addr 0x%llx, "
                      "data %p, len %u", qid, addr, data, len);
        return SDK_RET_HW_PROGRAM_ERR;
    }

    // move the producer index to next slot.
    // consumer is unaware of the blocking/non-blocking call and always
    // moves to the next slot.
    while (!SDK_ATOMIC_COMPARE_EXCHANGE_WEAK(&g_asicrw_workq[qid].pindx,
                                             &pindx,
                                             (pindx + 1)%ASICRW_Q_SIZE));

    rw_entry = &g_asicrw_workq[qid].entries[pindx];
    rw_entry->opn = opn;
    rw_entry->status = SDK_RET_ERR;
    rw_entry->addr = addr;
    rw_entry->len = len;
    rw_entry->data = data;
    SDK_ATOMIC_STORE_BOOL(&rw_entry->done, false);
    SDK_ATOMIC_FETCH_ADD(&g_asicrw_workq[qid].nentries, 1);
    while (SDK_ATOMIC_LOAD_BOOL(&rw_entry->done) == false) {
        if (curr_thread->can_yield()) {
            pthread_yield();
        }
    }
    return rw_entry->status;
}

//------------------------------------------------------------------------------
// public API for register read operations
//------------------------------------------------------------------------------
sdk_ret_t
asic_reg_read (uint64_t addr, uint32_t *data, uint32_t num_words,
               bool read_thru)
{
    sdk_ret_t    rc = SDK_RET_OK;
    thread       *curr_thread = sdk::lib::thread::current_thread();

    if ((read_thru == true) || (g_asicrw_ready_ == false) ||
        (curr_thread == NULL)) {
        // bypass asicrw thread
        time_profile_begin(sdk::utils::time_profile::PAL_REG_READ);
        pal_ret_t prc = sdk::lib::pal_reg_read(addr, data, num_words);
        rc = IS_PAL_API_SUCCESS(prc) ? SDK_RET_OK : SDK_RET_ERR;
        time_profile_end(sdk::utils::time_profile::PAL_REG_READ);
    } else {
        // go thru asicrw thread
        rc = asic_do_read(curr_thread, ASICRW_OP_REG_READ,
                          addr, (uint8_t *)data, num_words);
    }

    if (rc != SDK_RET_OK) {
        SDK_TRACE_ERR("Error reading reg addr 0x%llx", addr);
        SDK_ASSERT(0);
    }

    return rc;
}

//------------------------------------------------------------------------------
// public API for memory read operations
//------------------------------------------------------------------------------
sdk_ret_t
asic_mem_read (uint64_t addr, uint8_t *data, uint32_t len, bool read_thru)
{
    sdk_ret_t    rc = SDK_RET_OK;
    thread       *curr_thread = sdk::lib::thread::current_thread();

    time_profile_begin(sdk::utils::time_profile::ASIC_MEM_READ);

    if ((read_thru == true) || (g_asicrw_ready_ == false) ||
        (curr_thread == NULL)) {
        // bypass asicrw thread
        time_profile_begin(sdk::utils::time_profile::PAL_MEM_RD);
        pal_ret_t prc = sdk::lib::pal_mem_read(addr, data, len);
        time_profile_end(sdk::utils::time_profile::PAL_MEM_RD);
        rc = IS_PAL_API_SUCCESS(prc) ? SDK_RET_OK : SDK_RET_ERR;
    } else {
        // go thru asicrw thread
        rc = asic_do_read(curr_thread, ASICRW_OP_MEM_READ, addr, data, len);
    }

    if (rc != SDK_RET_OK) {
        SDK_TRACE_ERR("Error reading mem addr 0x%llx data %p", addr, data);
    }

    time_profile_end(sdk::utils::time_profile::ASIC_MEM_READ);
    return rc;
}

sdk_ret_t
asic_vmem_read (mem_addr_t addr, uint8_t *data, uint32_t len, bool read_thru)
{
    sdk_ret_t    rc = SDK_RET_OK;
    thread       *curr_thread = sdk::lib::thread::current_thread();

    time_profile_begin(sdk::utils::time_profile::ASIC_MEM_READ);

    if ((read_thru == true) || (g_asicrw_ready_ == false) ||
        (curr_thread == NULL)) {
        // bypass asicrw thread
        time_profile_begin(sdk::utils::time_profile::PAL_MEM_RD);
        memcpy(data, (void *)addr, len);
        time_profile_end(sdk::utils::time_profile::PAL_MEM_RD);
    } else {
        // go thru asicrw thread
        rc = asic_do_read(curr_thread, ASICRW_OP_VMEM_READ, addr, data, len);
    }

    if (rc != SDK_RET_OK) {
        SDK_TRACE_ERR("Error reading mem addr 0x%llx data %p", addr, data);
    }

    time_profile_end(sdk::utils::time_profile::ASIC_MEM_READ);
    return rc;
}

//------------------------------------------------------------------------------
// write given data at specified address in the memory
// NOTE: this API runs in the calling thread's context and supports both
// blocking and non-blocking writes
//------------------------------------------------------------------------------
static sdk_ret_t
asic_do_write (thread *curr_thread, uint8_t opn, uint64_t addr, uint8_t *data,
               uint32_t len, asic_write_mode_t mode)
{
    sdk_ret_t          ret;
    uint16_t           pindx = 0;
    asicrw_entry_t     *rw_entry;
    uint32_t           qid = curr_thread->thread_id();

    if (g_asicrw_workq[qid].nentries >= ASICRW_Q_SIZE) {
        SDK_TRACE_ERR("asic write operation failed, qid %u, addr 0x%llx, "
                      "data %p, len %u", qid, addr, data, len);
        return SDK_RET_HW_PROGRAM_ERR;
    }

    while (!SDK_ATOMIC_COMPARE_EXCHANGE_WEAK(&g_asicrw_workq[qid].pindx,
                                             &pindx,
                                             (pindx + 1)%ASICRW_Q_SIZE));

    rw_entry = &g_asicrw_workq[qid].entries[pindx];
    rw_entry->opn = opn;
    rw_entry->status = SDK_RET_ERR;
    rw_entry->addr = addr;
    rw_entry->len = len;
    rw_entry->data = data;
    SDK_ATOMIC_STORE_BOOL(&rw_entry->done, false);
    SDK_ATOMIC_FETCH_ADD(&g_asicrw_workq[qid].nentries, 1);

    if (mode == ASIC_WRITE_MODE_BLOCKING) {
        while (SDK_ATOMIC_LOAD_BOOL(&rw_entry->done) == false) {
            if (!curr_thread || curr_thread->can_yield()) {
                pthread_yield();
            }
        }
        ret = rw_entry->status;
    } else {
        ret = SDK_RET_OK;
    }

    return ret;
}

//------------------------------------------------------------------------------
// public API for register write operations
// NOTE: this API runs in the calling thread's context and supports both
// blocking and non-blocking writes
//------------------------------------------------------------------------------
sdk_ret_t
asic_reg_write (uint64_t addr, uint32_t *data, uint32_t num_words,
                asic_write_mode_t mode)
{
    sdk_ret_t    rc = SDK_RET_OK;
    thread       *curr_thread = sdk::lib::thread::current_thread();

    //SDK_TRACE_DEBUG("addr 0x%llx, data %p, len %u, mode %u",
                    //addr, data, num_words, mode);
    if ((mode == ASIC_WRITE_MODE_WRITE_THRU) || (g_asicrw_ready_ == false) ||
        (curr_thread == NULL)) {
        // bypass asicrw thread
        time_profile_begin(sdk::utils::time_profile::PAL_REG_WRITE);
        pal_ret_t prc = sdk::lib::pal_reg_write(addr, data, num_words);
        rc = IS_PAL_API_SUCCESS(prc) ? SDK_RET_OK : SDK_RET_ERR;
        time_profile_end(sdk::utils::time_profile::PAL_REG_WRITE);
    } else {
        // go thru asicrw thread
        rc = asic_do_write(curr_thread, ASICRW_OP_REG_WRITE,
                           addr, (uint8_t *)data, num_words, mode);
    }
    if (rc != SDK_RET_OK) {
        SDK_TRACE_ERR("Error writing addr 0x%llx, data %u", addr, *data);
        SDK_ASSERT(0);
    }

    return rc;
}

//------------------------------------------------------------------------------
// public API for memory write operations
//------------------------------------------------------------------------------
sdk_ret_t
asic_mem_write (uint64_t addr, uint8_t *data, uint32_t len,
                asic_write_mode_t mode)
{
    sdk_ret_t    rc = SDK_RET_OK;
    thread       *curr_thread = sdk::lib::thread::current_thread();

    time_profile_begin(sdk::utils::time_profile::ASIC_MEM_WRITE);
    //SDK_TRACE_DEBUG("addr 0x%llx, data %p, len %u, mode %u",
                    //addr, data, len, mode);
    if ((mode == ASIC_WRITE_MODE_WRITE_THRU) || (g_asicrw_ready_ == false) ||
        (curr_thread == NULL)) {
        // bypass asicrw thread
        time_profile_begin(sdk::utils::time_profile::PAL_MEM_WR);
        pal_ret_t prc = sdk::lib::pal_mem_write(addr, data, len);
        time_profile_end(sdk::utils::time_profile::PAL_MEM_WR);
        rc = IS_PAL_API_SUCCESS(prc) ? SDK_RET_OK : SDK_RET_ERR;
    } else {
        // go thru asicrw thread
        rc = asic_do_write(curr_thread, ASICRW_OP_MEM_WRITE,
                           addr, data, len, mode);
    }
    if (rc != SDK_RET_OK) {
        SDK_TRACE_ERR("Error writing mem addr 0x%llx, data %p", addr, data);
        SDK_ASSERT(0);
    }
    time_profile_end(sdk::utils::time_profile::ASIC_MEM_WRITE);

    return rc;
}

sdk_ret_t
asic_vmem_write (uint64_t addr, uint8_t *data, uint32_t len,
                 asic_write_mode_t mode)
{
    sdk_ret_t    rc = SDK_RET_OK;
    thread       *curr_thread = sdk::lib::thread::current_thread();

    time_profile_begin(sdk::utils::time_profile::ASIC_MEM_WRITE);
    //SDK_TRACE_DEBUG("addr 0x%llx, data %p, len %u, mode %u",
                    //addr, data, len, mode);
    if ((mode == ASIC_WRITE_MODE_WRITE_THRU) || (g_asicrw_ready_ == false) ||
        (curr_thread == NULL)) {
        // bypass asicrw thread
        time_profile_begin(sdk::utils::time_profile::PAL_MEM_WR);
        memcpy((void *)addr, data, len);
        time_profile_end(sdk::utils::time_profile::PAL_MEM_WR);
    } else {
        // go thru asicrw thread
        rc = asic_do_write(curr_thread, ASICRW_OP_VMEM_WRITE,
                           addr, data, len, mode);
    }
    if (rc != SDK_RET_OK) {
        SDK_TRACE_ERR("Error writing mem addr 0x%llx, data %p", addr, data);
        SDK_ASSERT(0);
    }
    time_profile_end(sdk::utils::time_profile::ASIC_MEM_WRITE);

    return rc;
}

//------------------------------------------------------------------------------
// public API for ringing doorbell
//------------------------------------------------------------------------------
sdk_ret_t
asic_ring_doorbell (uint64_t addr, uint64_t data, asic_write_mode_t mode)
{
    sdk_ret_t    rc = SDK_RET_OK;
    thread       *curr_thread = sdk::lib::thread::current_thread();

    //SDK_TRACE_DEBUG("addr 0x%llx, data %llx, mode %u", addr, data, mode);
    if ((mode == ASIC_WRITE_MODE_WRITE_THRU) || (g_asicrw_ready_ == false) ||
        (curr_thread == NULL)) {
        // bypass asicrw thread
        pal_ret_t prc = sdk::lib::pal_ring_db64(addr, data);
        rc = IS_PAL_API_SUCCESS(prc) ? SDK_RET_OK : SDK_RET_ERR;
    } else {
        // go thru asicrw thread
        rc = asic_do_write(curr_thread, ASICRW_OP_RING_DOORBELL,
                           addr, (uint8_t *)&data, sizeof(data), mode);
    }
    if (rc != SDK_RET_OK) {
        SDK_TRACE_ERR("Error ringing doorbell addr 0x%llx, data 0x%llx",
                       addr, data);
        SDK_ASSERT(0);
    }

    return rc;
}

//------------------------------------------------------------------------------
// asic read-write thread's forever loop to serve read and write requests from
// other threads
//------------------------------------------------------------------------------
static void
asicrw_loop (void *ctxt)
{
    uint32_t            qid;
    uint16_t            cindx;
    bool                work_done = false;
    pal_ret_t           rv = PAL_RET_OK;
    asicrw_entry_t     *rw_entry = NULL;
    sdk::lib::thread    *curr_thread = (sdk::lib::thread *)ctxt;

    while (TRUE) {
        work_done = false;
        for (qid = 0; qid < ASICRW_NUM_QUEUES; qid++) {
            if (!g_asicrw_workq[qid].nentries) {
                // no read/write requests
                continue;
            }

            // found a read/write request to serve
            cindx = g_asicrw_workq[qid].cindx;
            rw_entry = &g_asicrw_workq[qid].entries[cindx];
            switch (rw_entry->opn) {
            case ASICRW_OP_MEM_READ:
                time_profile_begin(sdk::utils::time_profile::PAL_MEM_RD);
                rv = sdk::lib::pal_mem_read(rw_entry->addr, rw_entry->data,
                                            rw_entry->len);
                time_profile_end(sdk::utils::time_profile::PAL_MEM_RD);
                break;

           case ASICRW_OP_VMEM_READ:
                rv = PAL_RET_OK;
                time_profile_begin(sdk::utils::time_profile::PAL_MEM_RD);
                memcpy(rw_entry->data, (void *)rw_entry->addr, rw_entry->len);
                time_profile_end(sdk::utils::time_profile::PAL_MEM_RD);
                break;

            case ASICRW_OP_MEM_WRITE:
                time_profile_begin(sdk::utils::time_profile::PAL_MEM_WR);
                rv = sdk::lib::pal_mem_write(rw_entry->addr, rw_entry->data,
                                             rw_entry->len);
                time_profile_end(sdk::utils::time_profile::PAL_MEM_WR);
                break;

            case ASICRW_OP_VMEM_WRITE:
                rv = PAL_RET_OK;
                time_profile_begin(sdk::utils::time_profile::PAL_MEM_WR);
                memcpy((void *)rw_entry->addr, rw_entry->data, rw_entry->len);
                time_profile_end(sdk::utils::time_profile::PAL_MEM_WR);
                break;

            case ASICRW_OP_REG_READ:
                time_profile_begin(sdk::utils::time_profile::PAL_REG_READ);
                rv = sdk::lib::pal_reg_read(rw_entry->addr,
                                            (uint32_t*)rw_entry->data,
                                            rw_entry->len);
                time_profile_end(sdk::utils::time_profile::PAL_REG_READ);
                break;

            case ASICRW_OP_REG_WRITE:
                time_profile_begin(sdk::utils::time_profile::PAL_REG_WRITE);
                rv = sdk::lib::pal_reg_write(rw_entry->addr,
                                             (uint32_t*)rw_entry->data,
                                             rw_entry->len);
                time_profile_end(sdk::utils::time_profile::PAL_REG_WRITE);
                break;

            case ASICRW_OP_RING_DOORBELL:
                rv = sdk::lib::pal_ring_db64(rw_entry->addr,
                                                 *(uint64_t *)rw_entry->data);
                break;

            default:
                SDK_TRACE_ERR("Invalid operation %u\n", rw_entry->opn);
                SDK_ASSERT(0);
                break;
            }

            // populate the results
            rw_entry->status =
                IS_PAL_API_SUCCESS(rv) ? SDK_RET_OK : SDK_RET_ERR;
            SDK_ATOMIC_STORE_BOOL(&rw_entry->done, true);

            // advance to next entry in the queue
            g_asicrw_workq[qid].cindx++;
            if (g_asicrw_workq[qid].cindx >= ASICRW_Q_SIZE) {
                g_asicrw_workq[qid].cindx = 0;
            }

            SDK_ATOMIC_FETCH_SUB(&g_asicrw_workq[qid].nentries, 1);
            work_done = true;
        }
        curr_thread->punch_heartbeat();

        // all queues scanned once, check if any work was found
        if (!work_done) {
            // didn't find any work, yield and give chance to other threads
            pthread_yield();
        }
    }
}

//------------------------------------------------------------------------------
// ASIC read/write thread's entry point
//------------------------------------------------------------------------------
void *
asicrw_start (void *ctxt)
{
    SDK_THREAD_INIT(ctxt);

    // announce asic-rw thread as ready
    SDK_ATOMIC_STORE_BOOL(&g_asicrw_ready_, true);

    // keep polling the queue and serve the read/write requests
    asicrw_loop(ctxt);

    return NULL;
}

}    // namespace asic
}    // namespace sdk
