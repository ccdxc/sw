//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __HAL_HPP__
#define __HAL_HPP__

#include <string>
#include "lib/thread/thread.hpp"
#include "nic/include/base.hpp"
#include "nic/include/hal_mem.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/hal/lib/hal_handle.hpp"
#include "nic/hal/core/core.hpp"

extern std::string g_grpc_server_addr;
void svc_reg(hal::hal_cfg_t *hal_cfg);

namespace hal {

extern hal::hal_cfg_t g_hal_cfg;

using sdk::lib::thread;

typedef enum hal_status_e {
    HAL_STATUS_NONE,
    HAL_STATUS_ASIC_INIT_DONE,
    HAL_STATUS_MEM_INIT_DONE,
    HAL_STATUS_PACKET_BUFFER_INIT_DONE,
    HAL_STATUS_DATA_PLANE_INIT_DONE,
    HAL_STATUS_SCHEDULER_INIT_DONE,
    HAL_STATUS_INIT_ERR,
    HAL_STATUS_UP
} hal_status_t;

// TODO: remove these two from here and move to HAL core
thread *hal_get_current_thread(void);

#if 0
//------------------------------------------------------------------------------
// parse HAL initialization
//------------------------------------------------------------------------------
extern hal_ret_t hal_parse_ini(const char *inifile __IN__,
                               hal_cfg_t *hal_cfg __OUT__);
#endif

//------------------------------------------------------------------------------
// hal initialization main routine
//------------------------------------------------------------------------------
extern hal_ret_t hal_init(hal_cfg_t *cfg __IN__);

//------------------------------------------------------------------------------
// hal uninitialization main routine
//------------------------------------------------------------------------------
extern hal_ret_t hal_destroy(void);

//------------------------------------------------------------------------------
// hal wait-for-cleanup API
//------------------------------------------------------------------------------
extern hal_ret_t hal_wait(void);

//------------------------------------------------------------------------------
// return's HAL memory manager
//------------------------------------------------------------------------------
shmmgr *hal_mmgr(void);

// meta class for each object
typedef hal_ret_t (*marshall_cb_t)(void *obj, uint8_t *mem,
                                   uint32_t len, uint32_t *mlen);
typedef uint32_t (*unmarshall_cb_t)(void *obj, uint32_t len);
class hal_obj_meta {
public:
    hal_obj_meta(hal_slab_t slab_id,
                 marshall_cb_t mcb,
                 unmarshall_cb_t umcb) {
        this->slab_id_ = slab_id;
        this->marshall_cb_ = mcb;
        this->unmarshall_cb_ = umcb;
    }
    ~hal_obj_meta() {}
    hal_slab_t slab_id(void) const { return slab_id_; }
    marshall_cb_t marshall_cb(void) const { return marshall_cb_; }
    unmarshall_cb_t unmarshall_cb(void) const { return unmarshall_cb_; }

private:
    hal_slab_t         slab_id_;
    marshall_cb_t      marshall_cb_;      // serialize the state
    unmarshall_cb_t    unmarshall_cb_;    // deserialize the state
};
void hal_obj_meta_init(void);
extern hal_obj_meta *g_obj_meta[HAL_OBJ_ID_MAX];

//------------------------------------------------------------------------------
// initialize HAL state
//------------------------------------------------------------------------------
hal_ret_t hal_state_init(hal_cfg_t *hal_cfg);

// hal_state APIs
slab *hal_handle_slab(void);
slab *hal_handle_ht_entry_slab(void);
ht *hal_handle_id_ht(void);
void hal_handle_cfg_db_lock(bool readlock, bool lock);
hal_ret_t hal_cfg_db_open(cfg_op_t cfg_op);
hal_ret_t hal_cfg_db_close(void);
hal::cfg_op_t hal_cfg_db_get_mode(void);

void asiccfg_init_completion_event(sdk_status_t status);
void svc_init_done(void);

}    // namespace hal

#endif    // __HAL_HPP__

