#ifndef __LINKMGR_STATE_HPP__
#define __LINKMGR_STATE_HPP__

#include "nic/include/hal_cfg.hpp"
#include "nic/include/hal_mem.hpp"
#include "nic/hal/lib/hal_handle.hpp"
#include "sdk/slab.hpp"
#include "sdk/ht.hpp"
#include "sdk/catalog.hpp"

using sdk::lib::slab;
using sdk::lib::ht;
using hal::cfg_op_t;
using hal::hal_obj_id_t;

namespace linkmgr {

#define HAL_MAX_PORTS  8

// config database
class cfg_db {
public:
    static cfg_db *factory(void);
    ~cfg_db();

    void rlock(void) { rwlock_.rlock(); }
    void runlock(void) { rwlock_.runlock(); }
    void wlock(void) { rwlock_.wlock(); }
    void wunlock(void) { rwlock_.wunlock(); }

    // API to call before processing any packet by FTE, any operation by config
    hal_ret_t db_open(cfg_op_t cfg_op);

    // API to call after processing any packet by FTE, any operation by config
    // thread or periodic thread etc.
    hal_ret_t db_close(void);

    // API to register a config object with HAL infra
    hal_ret_t register_cfg_object(hal_obj_id_t obj_id, uint32_t obj_sz);
    uint32_t object_size(hal_obj_id_t obj_id) const;

    ht *port_id_ht(void) const { return port_id_ht_; }

private:
    bool init(void);
    cfg_db();

private:
    ht           *port_id_ht_;
    wp_rwlock    rwlock_;

    typedef struct obj_meta_s {
        uint32_t        obj_sz;
    } __PACK__ obj_meta_t;

    obj_meta_t    obj_meta_[hal::HAL_OBJ_ID_MAX];
};

// memory slabs and any other memory manager state
class mem_db {
public:
    static mem_db *factory(void);
    ~mem_db();

    slab *get_slab(hal::hal_slab_t slab_id);

    slab *hal_handle_slab(void) const { return hal_handle_slab_; }
    slab *hal_handle_ht_entry_slab(void) const { return hal_handle_ht_entry_slab_; }
    slab *hal_handle_id_ht_entry_slab(void) const { return hal_handle_id_ht_entry_slab_; }
    slab *port_slab(void) const { return port_slab_; }

private:
    bool init(void);
    mem_db();

private:
    slab    *hal_handle_slab_;
    slab    *hal_handle_ht_entry_slab_;
    slab    *hal_handle_id_ht_entry_slab_;
    slab    *port_slab_;
};

class linkmgr_state {
public:
    hal_ret_t init();

    sdk::lib::catalog* catalog() { return catalog_; }
    void set_catalog(sdk::lib::catalog *catalog) { catalog_ = catalog; }

    void cfg_db_rlock(bool lock) {
        (lock == true) ? cfg_db_->rlock() : cfg_db_->runlock();
    }

    void cfg_db_wlock(bool lock) {
        (lock == true) ? cfg_db_->wlock() : cfg_db_->wunlock();
    }

    hal_ret_t cfg_db_open(cfg_op_t cfg_op) { return cfg_db_->db_open(cfg_op); }
    hal_ret_t cfg_db_close(void) { return cfg_db_->db_close(); }

    ht *port_id_ht(void) const { return cfg_db_->port_id_ht(); }

    slab *hal_handle_id_ht_entry_slab(void) const {
        return mem_db_->hal_handle_id_ht_entry_slab();
    }

    // get APIs for port related state
    slab *port_slab(void) const { return mem_db_->port_slab(); }

    slab *hal_handle_ht_entry_slab(void) const { return mem_db_->hal_handle_ht_entry_slab(); }

    slab *hal_handle_slab(void) const { return mem_db_->hal_handle_slab(); }

    ht *hal_handle_id_ht(void) const { return hal_handle_id_ht_; };

    static linkmgr_state* factory();

private:
    sdk::lib::catalog  *catalog_;
    cfg_db               *cfg_db_;
    mem_db               *mem_db_;
    ht                   *hal_handle_id_ht_;
};

} /* namespace linkmgr */

#endif /* __LINKMGR_STATE_HPP__ */
