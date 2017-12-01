// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/utils/thread/thread.hpp"
#include "nic/include/hal_pd.hpp"
#include "nic/hal/pd/asic_pd.hpp"
#include "nic/include/pal.hpp"

namespace hal {
namespace pd {

// asic model's cfg port socket descriptor
static std::atomic<bool> g_asic_rw_ready_;

#define HAL_ASIC_RW_Q_SIZE                           128
#define HAL_ASIC_RW_OPERATION_MEM_READ               0
#define HAL_ASIC_RW_OPERATION_MEM_WRITE              1
#define HAL_ASIC_RW_OPERATION_REG_READ               2
#define HAL_ASIC_RW_OPERATION_REG_WRITE              3
#define HAL_ASIC_RW_OPERATION_PORT                   4
#define HAL_ASIC_RW_OPERATION_RING_DOORBELL          5

//------------------------------------------------------------------------------
// custom struct passed between control thread and asic-rw thread
// for port related operations
//------------------------------------------------------------------------------
typedef struct asic_rw_port_entry_ {
    uint32_t    port_num;  // port number
    uint32_t    speed;     // port speed
    uint32_t    type;      // buffer type between model client and server
    uint32_t    num_lanes; // number of lanes for port
    uint32_t    val;       // custom value per operation
} asic_rw_port_entry_t;

//------------------------------------------------------------------------------
// asic read-write entry, one such entry is added to the asic read/write
// thread's queue whenever asic read/write is attempted by any HAL thread
//------------------------------------------------------------------------------
typedef struct asic_rw_entry_ {
    uint8_t              opn;        // operation requested to perform
    std::atomic<bool>    done;       // TRUE if thread performed operation
    hal_ret_t            status;     // result status of operation requested
    uint64_t             addr;       // address to write to or read from
    uint32_t             len;        // length of data to read or write
    uint8_t              *data;      // data to write or buffer to copy data to for mem read/write
    uint32_t             reg_data;   // data to write or buffer to copy data to for mem read/write
    asic_rw_port_entry_t port_entry; // port data
} asic_rw_entry_t;

//------------------------------------------------------------------------------
// asic read-write thread maintains one queue per HAL thread to serve
// read/write operations by HAL thread, thus avoiding locking altogether
//------------------------------------------------------------------------------
typedef struct asic_rw_queue_s {
    std::atomic<uint32_t>    nentries;    // no. of entries in the queue
    uint16_t                 pindx;       // producer index
    uint16_t                 cindx;       // consumer index
    asic_rw_entry_t          entries[HAL_ASIC_RW_Q_SIZE];    // entries
} asic_rw_queue_t;

// per producer read/write request queues
asic_rw_queue_t    g_asic_rw_workq[HAL_THREAD_ID_MAX];

// check if this thread is the hal-control thread for given chip
// Returns true if:
//    this thread's id matches with hal-control thread's id
bool
is_hal_ctrl_thread()
{
    hal::utils::thread *curr_thread = hal::utils::thread::current_thread();
    hal::utils::thread *ctrl_thread = g_hal_threads[HAL_THREAD_ID_CONTROL];

    if (ctrl_thread == NULL || curr_thread == NULL) {
        assert(0);
    }

    if (curr_thread->thread_id() == ctrl_thread->thread_id()) {
        return true;
    }

    return false;
}

// check if this thread is the asic-rw thread for given chip
// Returns true if:
//    this thread's id matches with asic-rw thread's id
bool
is_asic_rw_thread()
{
    hal::utils::thread *curr_thread;
    hal::utils::thread *asic_rw_thread;
    
    curr_thread = hal::utils::thread::current_thread();
    asic_rw_thread = g_hal_threads[HAL_THREAD_ID_ASIC_RW];

    if (curr_thread == NULL) {
        // running in single-threaded mode
        return true;
    }

    if (asic_rw_thread == NULL ||
        asic_rw_thread->is_running() == false) {
        assert(0);
    }

    if (curr_thread->thread_id() == asic_rw_thread->thread_id()) {
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
// return TRUE is asicrw thread is ready to do read/write operations
//------------------------------------------------------------------------------
bool
is_asic_rw_ready (void)
{
    return g_asic_rw_ready_.load();
}

//------------------------------------------------------------------------------
// read data from specified address in the memory to given buffer
// NOTE: this is always a blocking call and this API runs in the calling
//       thread's context
//------------------------------------------------------------------------------
static hal_ret_t
asic_do_read (uint8_t opn, uint64_t addr, uint8_t *data, uint32_t len)
{
    uint16_t           pindx;

    hal::utils::thread *curr_thread = hal::utils::thread::current_thread();

    uint32_t           curr_tid = curr_thread->thread_id();
    asic_rw_entry_t    *rw_entry;

    if (!data) {
        return HAL_RET_INVALID_ARG;
    }

    if (g_asic_rw_workq[curr_tid].nentries >= HAL_ASIC_RW_Q_SIZE) {
        HAL_TRACE_ERR("ASIC rwq for thread {}, tid {} full, read failed",
                      curr_thread->name(), curr_tid);
        return HAL_RET_HW_PROG_ERR;
    }
    pindx = g_asic_rw_workq[curr_tid].pindx;

    rw_entry = &g_asic_rw_workq[curr_tid].entries[pindx];
    rw_entry->opn = opn;
    rw_entry->status = HAL_RET_ERR;
    rw_entry->addr = addr;
    rw_entry->len = len;
    rw_entry->data = data;
    rw_entry->done.store(false);

    g_asic_rw_workq[curr_tid].nentries++;

    while (rw_entry->done.load() == false) {
        if (curr_thread->can_yield()) {
            pthread_yield();
        }
    }

    // move the producer index to next slot.
    // consumer is unaware of the blocking/non-blocking call and always
    // moves to the next slot.
    g_asic_rw_workq[curr_tid].pindx++;
    if (g_asic_rw_workq[curr_tid].pindx >= HAL_ASIC_RW_Q_SIZE) {
        g_asic_rw_workq[curr_tid].pindx = 0;
    }

    return rw_entry->status;
}

//------------------------------------------------------------------------------
// public API for register read operations
// NOTE: this is always a blocking call and this API runs in the calling
//       thread's context
//------------------------------------------------------------------------------
static hal_ret_t
asic_reg_read_impl (uint64_t addr, uint32_t *data)
{
    hal_ret_t rc = HAL_RET_OK;

    if (is_asic_rw_thread() == false) {
        rc = asic_do_read(HAL_ASIC_RW_OPERATION_REG_READ,
                          addr, (uint8_t *)data, 0);
    } else {
        pal_reg_read(addr, data);
    }

    if (rc != HAL_RET_OK) {
        HAL_TRACE_ERR("Error reading reg addr:{}", addr);
        HAL_ASSERT(0);
    }

    return rc;
}

uint32_t
asic_reg_read (uint64_t addr)
{
    uint32_t data = 0;
    asic_reg_read_impl(addr, &data);
    return data;
}

hal_ret_t
asic_reg_read (uint64_t addr, uint32_t *data)
{
    return asic_reg_read_impl(addr, data);
}

//------------------------------------------------------------------------------
// public API for memory read operations
// NOTE: this is always a blocking call and this API runs in the calling
//       thread's context
//------------------------------------------------------------------------------
hal_ret_t
asic_mem_read (uint64_t addr, uint8_t *data, uint32_t len)
{
    hal_ret_t   rc = HAL_RET_OK;

    if (is_asic_rw_thread() == false) {
        rc = asic_do_read(HAL_ASIC_RW_OPERATION_MEM_READ, addr, data, len);
    } else {
        pal_ret_t prc = pal_mem_read(addr, data, len);
        rc = IS_PAL_API_SUCCESS(prc) ? HAL_RET_OK : HAL_RET_ERR;
    }

    if (rc != HAL_RET_OK) {
        HAL_TRACE_ERR("Error reading mem addr:{} data:{}", addr, data);
    }
    return rc;
}

//------------------------------------------------------------------------------
// write given data at specified address in the memory
// NOTE: this API runs in the calling thread's context and supports both
// blocking and non-blocking writes
//------------------------------------------------------------------------------
static hal_ret_t
asic_do_write (uint8_t opn, uint64_t addr, uint8_t *data,
               uint32_t len, bool blocking)
{
    hal_ret_t          ret;
    uint16_t           pindx;
    hal::utils::thread *curr_thread = hal::utils::thread::current_thread();
    uint32_t           curr_tid = curr_thread->thread_id();
    asic_rw_entry_t    *rw_entry;

    if (g_asic_rw_workq[curr_tid].nentries >= HAL_ASIC_RW_Q_SIZE) {
        HAL_TRACE_ERR("ASIC rwq for thread {}, tid {} full, write failed",
                      curr_thread->name(), curr_tid);
        return HAL_RET_HW_PROG_ERR;
    }
    pindx = g_asic_rw_workq[curr_tid].pindx;

    rw_entry = &g_asic_rw_workq[curr_tid].entries[pindx];
    rw_entry->opn = opn;
    rw_entry->status = HAL_RET_ERR;
    rw_entry->addr = addr;
    rw_entry->len = len;
    if (!len) {
        rw_entry->reg_data = *((uint32_t *)data);
    } else {
        rw_entry->data = data;
    }
    rw_entry->done.store(false);

    g_asic_rw_workq[curr_tid].nentries++;

    if (blocking) {
        while (rw_entry->done.load() == false) {
            if (curr_thread->can_yield()) {
                pthread_yield();
            }
        }
        ret = rw_entry->status;
    } else {
        ret = HAL_RET_OK;
    }

    // move the producer index to next slot.
    // consumer is unaware of the blocking/non-blocking call and always
    // moves to the next slot.
    g_asic_rw_workq[curr_tid].pindx++;
    if (g_asic_rw_workq[curr_tid].pindx >= HAL_ASIC_RW_Q_SIZE) {
        g_asic_rw_workq[curr_tid].pindx = 0;
    }

    return ret;
}


//------------------------------------------------------------------------------
// public API for register write operations
// NOTE: this API runs in the calling thread's context and supports both
// blocking and non-blocking writes
//------------------------------------------------------------------------------
hal_ret_t
asic_reg_write (uint64_t addr, uint32_t data, bool blocking)
{
    hal_ret_t rc = HAL_RET_OK;

    if (is_asic_rw_thread() == false) {
        rc = asic_do_write(HAL_ASIC_RW_OPERATION_REG_WRITE,
                           addr, (uint8_t *)&data, 0, blocking);
    } else {
        pal_ret_t prc = pal_reg_write(addr, data);
        rc = IS_PAL_API_SUCCESS(prc) ? HAL_RET_OK : HAL_RET_ERR;
    }
    if (rc != HAL_RET_OK) {
        HAL_TRACE_ERR("Error writing addr:{} data:{}", addr, data);
        HAL_ASSERT(0);
    }

    return rc;
}

//------------------------------------------------------------------------------
// public API for memory write operations
// NOTE: this API runs in the calling thread's context and supports both
// blocking and non-blocking writes
//------------------------------------------------------------------------------
hal_ret_t
asic_mem_write (uint64_t addr, uint8_t *data, uint32_t len, bool blocking)
{
    hal_ret_t rc = HAL_RET_OK;

    if (is_asic_rw_thread() == false) {
        rc = asic_do_write(HAL_ASIC_RW_OPERATION_MEM_WRITE,
                           addr, data, len, blocking);
    } else {
        pal_ret_t prc = pal_mem_write(addr, data, len);
        rc = IS_PAL_API_SUCCESS(prc) ? HAL_RET_OK : HAL_RET_ERR;
    }

    if (rc != HAL_RET_OK) {
        HAL_TRACE_ERR("Error writing mem addr:{} data:{}", addr, data);
        HAL_ASSERT(0);
    }

    return rc;
}

//------------------------------------------------------------------------------
// public API for ringing doorbell
//------------------------------------------------------------------------------
hal_ret_t
asic_ring_doorbell (uint64_t addr, uint64_t data, bool blocking)
{
    hal_ret_t rc = HAL_RET_OK;

    if (is_asic_rw_thread() == false) {
        rc = asic_do_write(HAL_ASIC_RW_OPERATION_RING_DOORBELL,
                           addr, (uint8_t *)&data, sizeof(data), blocking);
    } else {
        pal_ret_t prc = pal_ring_doorbell(addr, data);
        rc = IS_PAL_API_SUCCESS(prc) ? HAL_RET_OK : HAL_RET_ERR;
    }

    if (rc != HAL_RET_OK) {
        HAL_TRACE_ERR("Error ringing doorbell addr:{} data:{}", addr, data);
        HAL_ASSERT(0);
    }

    return rc;
}

//------------------------------------------------------------------------------
// public API for saving cpu packet.
//------------------------------------------------------------------------------
hal_ret_t
asic_step_cpu_pkt(const uint8_t* pkt, size_t pkt_len)
{
    pal_ret_t prc = pal_step_cpu_pkt(pkt, pkt_len);
    return IS_PAL_API_SUCCESS(prc) ? HAL_RET_OK : HAL_RET_ERR;
}

hal_ret_t
asic_port_cfg (uint32_t port_num,
               uint32_t speed,
               uint32_t type,
               uint32_t num_lanes,
               uint32_t val)
{
    hal_ret_t          ret = HAL_RET_OK;
    uint16_t           pindx  = 0;
    asic_rw_entry_t    *rw_entry = NULL;
    uint32_t           op = HAL_ASIC_RW_OPERATION_PORT;

    hal::utils::thread *curr_thread = hal::utils::thread::current_thread();
    uint32_t           curr_tid = curr_thread->thread_id();

    if (g_asic_rw_workq[curr_tid].nentries >= HAL_ASIC_RW_Q_SIZE) {
        HAL_TRACE_ERR("ASIC rwq for thread {}, tid {} full, write failed",
                      curr_thread->name(), curr_tid);
        return HAL_RET_HW_PROG_ERR;
    }

    pindx = g_asic_rw_workq[curr_tid].pindx;
    rw_entry = &g_asic_rw_workq[curr_tid].entries[pindx];

    rw_entry->opn = (uint8_t)op;
    rw_entry->status = HAL_RET_ERR;

    asic_rw_port_entry_t *port_entry = &rw_entry->port_entry;
    port_entry->port_num  = port_num;
    port_entry->speed     = speed;
    port_entry->type      = type;
    port_entry->num_lanes = num_lanes;
    port_entry->val       = val;

    rw_entry->done.store(false);

    g_asic_rw_workq[curr_tid].nentries++;

    while (rw_entry->done.load() == false) {
        if (curr_thread->can_yield()) {
            pthread_yield();
        }
    }
    ret = rw_entry->status;

    // move the producer index to next slot.
    // consumer is unaware of the blocking/non-blocking call and always
    // moves to the next slot.
    g_asic_rw_workq[curr_tid].pindx++;
    if (g_asic_rw_workq[curr_tid].pindx >= HAL_ASIC_RW_Q_SIZE) {
        g_asic_rw_workq[curr_tid].pindx = 0;
    }

    return ret;
}

//------------------------------------------------------------------------------
// ASIC read-write thread's forever loop to server read and write requests from
// other HAL threads
//------------------------------------------------------------------------------
static void
asic_rw_loop (void)
{
    uint32_t            qid;
    uint16_t            cindx;
    bool                work_done;
    pal_ret_t           rv;
    asic_rw_entry_t     *rw_entry;
    uint32_t            regval;

    while (TRUE) {
        work_done = false;
        for (qid = 0; qid < HAL_THREAD_ID_MAX; qid++) {
            if (!g_asic_rw_workq[qid].nentries) {
                // no read/write requests
                continue;
            }

            // found a read/write request to serve
            cindx = g_asic_rw_workq[qid].cindx;
            rw_entry = &g_asic_rw_workq[qid].entries[cindx];
            switch (rw_entry->opn) {
            case HAL_ASIC_RW_OPERATION_MEM_READ:
                rv = pal_mem_read(rw_entry->addr, rw_entry->data, rw_entry->len);
                break;

            case HAL_ASIC_RW_OPERATION_MEM_WRITE:
                rv = pal_mem_write(rw_entry->addr, rw_entry->data, rw_entry->len);
                break;

            case HAL_ASIC_RW_OPERATION_REG_READ:
                rv = pal_reg_read(rw_entry->addr, &regval);
                if (IS_PAL_API_SUCCESS(rv)) {
                    *(uint32_t *)rw_entry->data = regval;
                }
                break;

            case HAL_ASIC_RW_OPERATION_REG_WRITE:
                regval = rw_entry->reg_data;
                rv = pal_reg_write(rw_entry->addr, regval);
                break;

            case HAL_ASIC_RW_OPERATION_PORT:
                // TODO: This needs to be moved to asic library.
                #if 0
                lib_model_mac_msg_send(
                        rw_entry->port_entry.port_num,
                        rw_entry->port_entry.speed,
                        rw_entry->port_entry.type,
                        rw_entry->port_entry.num_lanes,
                        rw_entry->port_entry.val);
                #endif
                break;

            case HAL_ASIC_RW_OPERATION_RING_DOORBELL:
                rv = pal_ring_doorbell(rw_entry->addr,
                                       *(uint64_t *)rw_entry->data);
                break;

            default:
                HAL_TRACE_ERR("Invalid operation {}", rw_entry->opn);
                HAL_ASSERT(0);
                break;
            }

            // populate the results
            rw_entry->status =  IS_PAL_API_SUCCESS(rv) ? HAL_RET_OK : HAL_RET_ERR;
            rw_entry->done.store(true);

            // advance to next entry in the queue
            g_asic_rw_workq[qid].cindx++;
            if (g_asic_rw_workq[qid].cindx >= HAL_ASIC_RW_Q_SIZE) {
                g_asic_rw_workq[qid].cindx = 0;
            }
            g_asic_rw_workq[qid].nentries--;
            work_done = true;
        }

        // all queues scanned once, check if any work was found
        if (!work_done) {
            // didn't find any work, yield and give chance to other threads
            pthread_yield();
        }
    }
}

//------------------------------------------------------------------------------
// attempt to connect to ASIC model in sim mode
//------------------------------------------------------------------------------
void
asic_rw_init(hal_cfg_t *hal_cfg)
{
    asic_cfg_t  asic_cfg;
    pal_ret_t   palrv;

    // Initialize PAL
    palrv = pal_init(hal_cfg);
    HAL_ABORT(IS_PAL_API_SUCCESS(palrv));

    // do asic initialization
    asic_cfg.loader_info_file = hal_cfg->loader_info_file;
    HAL_ABORT(asic_init(&asic_cfg) == HAL_RET_OK);
    return;
}

//------------------------------------------------------------------------------
// ASIC read/write thread's entry point
//------------------------------------------------------------------------------
void *
asic_rw_start (void *ctxt)
{
    HAL_THREAD_INIT(ctxt);

    hal_cfg_t *hal_cfg =
                (hal_cfg_t *)hal::utils::thread::current_thread()->data();
    if (hal_cfg == NULL) {
        return NULL;
    }

    // asic init code
    asic_rw_init(hal_cfg);

    // announce asic-rw thread as ready
    g_asic_rw_ready_.store(true);

    // keep polling the queue and serve the read/write requests
    asic_rw_loop();

    return NULL;
}

}    // namespace pd
}    // namespace hal
