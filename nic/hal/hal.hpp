//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __HAL_HPP__
#define __HAL_HPP__

#include <string>
#include "sdk/thread.hpp"
#include "nic/include/base.hpp"
#include "nic/include/hal_mem.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/hal/lib/hal_handle.hpp"
#include "nic/hal/core/core.hpp"

namespace hal {

// TODO_CLEANUP - these don't belong here !!
class LIFManager;
extern LIFManager *g_lif_manager;
extern hal::hal_cfg_t g_hal_cfg;

using sdk::lib::thread;

extern thread *g_hal_threads[HAL_THREAD_ID_MAX];
thread *hal_get_current_thread(void);

//------------------------------------------------------------------------------
// parse HAL initialization
//------------------------------------------------------------------------------
extern hal_ret_t hal_parse_ini(const char *inifile __IN__,
                               hal_cfg_t *hal_cfg __OUT__);

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

}    // namespace hal

#endif    // __HAL_HPP__

