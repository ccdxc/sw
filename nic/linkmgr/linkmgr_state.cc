#include "linkmgr_state.hpp"
#include "linkmgr_src.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/utils/trace/trace.hpp"

namespace linkmgr {

/*
 ******************
 ***** cfg_db *****
 ******************
*/

//------------------------------------------------------------------------------
// init() function to instantiate all the config db init state
//------------------------------------------------------------------------------
bool
cfg_db::init(void)
{
    // initialize port related data structures
    port_id_ht_ = ht::factory(HAL_MAX_PORTS,
                              linkmgr::port_id_get_key_func,
                              linkmgr::port_id_compute_hash_func,
                              linkmgr::port_id_compare_key_func);
    if (NULL == port_id_ht_) {
        HAL_TRACE_ERR("{} port ht allocation failed", __FUNCTION__);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
cfg_db::cfg_db()
{
    port_id_ht_ = NULL;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
cfg_db*
cfg_db::factory(void)
{
    void    *mem = NULL;
    cfg_db  *cfg = NULL;

    mem = HAL_CALLOC(hal::HAL_MEM_ALLOC_INFRA, sizeof(cfg_db));
    HAL_ASSERT_RETURN((mem != NULL), NULL);

    cfg = new(mem) cfg_db();
    if (cfg->init() == false) {
        cfg->~cfg_db();
        HAL_FREE(hal::HAL_MEM_ALLOC_INFRA, mem);
        return NULL;
    }

    return cfg;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
cfg_db::~cfg_db()
{
    port_id_ht_ ? ht::destroy(port_id_ht_) : HAL_NOP;
}

//------------------------------------------------------------------------------
// API to call before processing any packet by FTE, any operation by config
// thread or periodic thread etc.
// NOTE: once opened, cfg db has to be closed properly and reserved version
//       should be released/committed or else next open will fail
//------------------------------------------------------------------------------
hal_ret_t
cfg_db::db_open(cfg_op_t cfg_op)
{
    // if the cfg db was already opened by this thread, error out
    if (hal::t_cfg_db_ctxt.cfg_db_open_) {
        HAL_TRACE_ERR("Failed to open cfg db, opened already, thread {}",
                      current_thread()->name());
        return HAL_RET_ERR;
    }

    // take a read lock irrespective of whether the db is open in read/write
    // mode, for write mode, we will eventually take a write lock when needed
    rlock();

    hal::t_cfg_db_ctxt.cfg_op_ = cfg_op;
    hal::t_cfg_db_ctxt.cfg_db_open_ = true;
    HAL_TRACE_DEBUG("{} acquired rlock, opened cfg db, cfg op : {}",
                    current_thread()->name(), cfg_op);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// API to call after processing any packet by FTE, any operation by config
// thread or periodic thread etc. If successful, this will make the currently
// reserved (and cached) version of the DB valid. In case of failure, the
// currently reserved version will not be marked as valid and object updates
// made with this reserved version are left as they are ... they are either
// cleaned up when we touch those objects next time, or by periodic thread that
// will release instances of objects with invalid versions (or versions that
// slide out of the valid-versions window)
//------------------------------------------------------------------------------
hal_ret_t
cfg_db::db_close(void)
{
    if (hal::t_cfg_db_ctxt.cfg_db_open_) {
        hal::t_cfg_db_ctxt.cfg_db_open_ = FALSE;
        hal::t_cfg_db_ctxt.cfg_op_ = hal::CFG_OP_NONE;
        runlock();
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// register a config object's meta information
//------------------------------------------------------------------------------
hal_ret_t
cfg_db::register_cfg_object(hal_obj_id_t obj_id, uint32_t obj_sz)
{
    if ((obj_id <= hal::HAL_OBJ_ID_NONE) || (obj_id >= hal::HAL_OBJ_ID_MAX)) {
        return HAL_RET_INVALID_ARG;
    }
    obj_meta_[obj_id].obj_sz = obj_sz;
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// return a config object's size given its id
//------------------------------------------------------------------------------
uint32_t
cfg_db::object_size(hal_obj_id_t obj_id) const
{
    if ((obj_id <= hal::HAL_OBJ_ID_NONE) || (obj_id >= hal::HAL_OBJ_ID_MAX)) {
        return 0;
    }
    return obj_meta_[obj_id].obj_sz;
}

/*
 ******************
 ***** mem_db *****
 ******************
*/

//------------------------------------------------------------------------------
// init() function to instantiate all the mem db init state
//------------------------------------------------------------------------------
bool
mem_db::init(void)
{
    // initialize slab for HAL handles
    hal_handle_slab_ = slab::factory("hal-handle",
                                     hal::HAL_SLAB_HANDLE, sizeof(hal::hal_handle),
                                     64, true, true, true);
    if (NULL == hal_handle_slab_) {
        HAL_TRACE_ERR("{} hal handle slab allocation failed", __FUNCTION__);
        return false;
    }

    hal_handle_ht_entry_slab_ = slab::factory("hal-handle-ht-entry",
                                              hal::HAL_SLAB_HANDLE_HT_ENTRY,
                                              sizeof(hal::hal_handle_ht_entry_t),
                                              64, true, true, true);
    if (NULL == hal_handle_ht_entry_slab_) {
        HAL_TRACE_ERR("{} hal handle ht entry allocation failed", __FUNCTION__);
        return false;
    }

	hal_handle_id_ht_entry_slab_ = slab::factory("hal-handle-id-ht-entry",
                                                 hal::HAL_SLAB_HANDLE_ID_HT_ENTRY,
                                                 sizeof(hal::hal_handle_id_ht_entry_t),
                                                 64, true, true, true);
    if (NULL == hal_handle_id_ht_entry_slab_) {
        HAL_TRACE_ERR("{} handle ht allocation failed", __FUNCTION__);
        return false;
    }

    // initialize port related data structures
    port_slab_ = slab::factory("port", hal::HAL_SLAB_PORT,
                               sizeof(linkmgr::port_t), 8,
                               false, true, true);
    if (NULL == port_slab_) {
        HAL_TRACE_ERR("{} port allocation failed", __FUNCTION__);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
mem_db::mem_db()
{
    hal_handle_slab_ = NULL;
    hal_handle_ht_entry_slab_ = NULL;
    hal_handle_id_ht_entry_slab_ = NULL;
    port_slab_ = NULL;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
mem_db *
mem_db::factory(void)
{
    void    *mem = NULL;
    mem_db  *memdb = NULL;

    mem = HAL_CALLOC(hal::HAL_MEM_ALLOC_INFRA, sizeof(mem_db));
    HAL_ASSERT_RETURN((mem != NULL), NULL);

    memdb = new(mem) mem_db();
    if (memdb->init() == false) {
        memdb->~mem_db();
        HAL_FREE(hal::HAL_MEM_ALLOC_INFRA, mem);
        return NULL;
    }

    return memdb;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
mem_db::~mem_db()
{
    hal_handle_slab_ ? slab::destroy(hal_handle_slab_) : HAL_NOP;
    hal_handle_ht_entry_slab_ ? slab::destroy(hal_handle_ht_entry_slab_) : HAL_NOP;
    hal_handle_id_ht_entry_slab_ ? slab::destroy(hal_handle_id_ht_entry_slab_) : HAL_NOP;
    port_slab_ ? slab::destroy(port_slab_) : HAL_NOP;
}

//----------------------------------------------------------------------------
// gives the slab of a slab id
//----------------------------------------------------------------------------
#define GET_SLAB(slab_name)                                 \
    if (slab_name && slab_name->get_slab_id() == slab_id) { \
        return slab_name;                                   \
    }

slab*
mem_db::get_slab(hal::hal_slab_t slab_id)
{
    GET_SLAB(hal_handle_slab_);
    GET_SLAB(hal_handle_ht_entry_slab_);
    GET_SLAB(hal_handle_id_ht_entry_slab_);
    GET_SLAB(port_slab_);

    return NULL;
}

/*
 *************************
 ***** linkmgr_state *****
 *************************
*/

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
linkmgr_state*
linkmgr_state::factory(void)
{
    void          *mem = NULL;
    linkmgr_state *state = NULL;

    mem = HAL_CALLOC(hal::HAL_MEM_ALLOC_INFRA, sizeof(linkmgr_state));
    HAL_ABORT(mem != NULL);
    state = new (mem) linkmgr_state();
    if (state->init() != HAL_RET_OK) {
        state->~linkmgr_state();
        HAL_FREE(hal::HAL_MEM_ALLOC_INFRA, mem);
	    return NULL;
    }

    return state;
}

hal_ret_t
linkmgr_state::init()
{
    hal_ret_t ret = HAL_RET_OK;

    cfg_db_ = cfg_db::factory();
    if (NULL == cfg_db_) {
        HAL_TRACE_ERR("{} cfg_db allocation failed", __FUNCTION__);
        return HAL_RET_ERR;
    }

    mem_db_ = mem_db::factory();
    if (NULL == mem_db_) {
        HAL_TRACE_ERR("{} mem_db allocation failed", __FUNCTION__);
        return HAL_RET_ERR;
    }

    hal_handle_id_ht_ = ht::factory(HAL_MAX_HANDLES,
                                    hal::hal_handle_id_get_key_func,
                                    hal::hal_handle_id_compute_hash_func,
                                    hal::hal_handle_id_compare_key_func);
    if (NULL == hal_handle_id_ht_) {
        HAL_TRACE_ERR("{} hal handle id ht allocation failed", __FUNCTION__);
        return HAL_RET_ERR;
    }

    return ret;
}

} /* linkmgr */
