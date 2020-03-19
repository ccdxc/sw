// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#include "nic/sdk/platform/capri/capri_barco_rings.hpp"
#include "nic/hal/plugins/cfg/aclqos/barco_rings.hpp"
#include "nic/hal/pd/capri/accel/accel_rgroup.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "third-party/asic/capri/model/cap_top/cap_top_csr.h"

/*
 * Operations on user selected group of Accelerator rings and sub-rings.
 */

#define ACCEL_RGROUP_METRICS_TIMER_MS   500

namespace hal {

namespace pd {

/*
 * Write a config 32-bit register
 */
#define ACCEL_CFG_WRITE32(cfg, val)                                         \
    do {                                                                    \
        cap_hens_csr_dhs_crypto_ctl_t&  ctl = accel_ctl();                  \
        ctl.cfg.fld(val);                                                   \
        ctl.cfg.write();                                                    \
    } while (false)

/*
 * Read a config 32-bit register
 */
#define ACCEL_CFG_READ32(cfg, val)                                          \
    do {                                                                    \
        cap_hens_csr_dhs_crypto_ctl_t&  ctl = accel_ctl();                  \
        ctl.cfg.read();                                                     \
        val = ctl.cfg.fld().convert_to<uint32_t>();                         \
    } while (false)


#define ACCEL_CFG_NAMED_READ32(cfg, reg_val)                                \
    do {                                                                    \
        cap_hens_csr_dhs_crypto_ctl_t&  ctl = accel_ctl();                  \
        ctl.cfg.read();                                                     \
        reg_val.val = ctl.cfg.fld().convert_to<uint32_t>();                 \
        strncpy(reg_val.name, #cfg, sizeof(reg_val.name));                  \
        reg_val.name[sizeof(reg_val.name)-1] = '\0';                        \
    } while (false)

/*
 * Read and write back (to clear) a 32-bit register (if it had non-zero value)
 */
#define ACCEL_CFG_NAMED_RD_CLR32(cfg, reg_val)                              \
    do {                                                                    \
        ACCEL_CFG_NAMED_READ32(cfg, reg_val);                               \
        if (reg_val.val) {                                                  \
            ACCEL_CFG_WRITE32(cfg, 0);                                      \
        }                                                                   \
    } while (false)

/*
 * Read a config 64-bit register
 */
#define ACCEL_CFG_READ64(cfg_lo, cfg_hi, val)                               \
    do {                                                                    \
        cap_hens_csr_dhs_crypto_ctl_t&  ctl = accel_ctl();                  \
        ctl.cfg_lo.read();                                                  \
        ((uint32_t *)&val)[0] = ctl.cfg_lo.fld().convert_to<uint32_t>();    \
        ctl.cfg_hi.read();                                                  \
        ((uint32_t *)&val)[1] = ctl.cfg_hi.fld().convert_to<uint32_t>();    \
    } while (false)


/*
 * Read-modify-write a config 32-bit register
 */
#define ACCEL_CFG_RMW32(cfg, val, sense)                                    \
    do {                                                                    \
        cap_hens_csr_dhs_crypto_ctl_t&  ctl = accel_ctl();                  \
        ctl.cfg.read();                                                     \
        uint32_t reg_val = ctl.cfg.fld().convert_to<uint32_t>();            \
        ctl.cfg.fld(sense ? reg_val | (val) : reg_val & ~(val));            \
        ctl.cfg.write();                                                    \
    } while (false)

/*
 * Get a config register address
 */
#define ACCEL_CFG_OFFSET(cfg, offs)                                         \
    do {                                                                    \
        cap_hens_csr_dhs_crypto_ctl_t&  ctl = accel_ctl();                  \
        offs = ctl.cfg.get_offset();                                        \
    } while (false)

/*
 * Read CP/DC ring size which is stored with mask and shift
 */
#define CPDC_RING_SIZE_READ32(cfg, val, shft, mask)                         \
    do {                                                                    \
        ACCEL_CFG_READ32(cfg, val);                                         \
        val = (val >> (shft)) & (mask);                                     \
        if (!val) val = (mask) + 1;                                         \
    } while (false)

/*
 * Sub-ring validation
 * Note: on Capri, each Barco master ring consists of only one single sub-ring.
 * Post-Capri chips may support multiple sub-rings per master ring.
 */
#define SUB_RING_VALIDATE_RETURN(sub_ring)                                  \
    do {                                                                    \
        if (((sub_ring) != ACCEL_SUB_RING_ALL) && ((sub_ring) > 0)) {       \
            return HAL_RET_INVALID_ARG;                                     \
        }                                                                   \
    } while (false)

/*
 * For each ring in a group, invoke the given function
 */
#define FOR_EACH_RGROUP_RING_INVOKE(func, sub_ring, ret_val, ...)           \
    do {                                                                    \
        accel_rgroup_elem_iter_c iter = elem_map.begin();                   \
        while (iter != elem_map.end()) {                                    \
            ret_val = iter->second.first->func(iter->second.second,         \
                                               sub_ring, __VA_ARGS__);      \
            if (ret_val != HAL_RET_OK) break;                               \
            iter++;                                                         \
        }                                                                   \
    } while (false)

#define FOR_EACH_RGROUP_RING_INVOKE_RET_LAST(func, sub_ring, ret_val,       \
                                             last_ring_handle,              \
                                             last_sub_ring, ...)            \
    do {                                                                    \
        accel_rgroup_elem_iter_c iter = elem_map.begin();                   \
        while (iter != elem_map.end()) {                                    \
            ret_val = iter->second.first->func(iter->second.second,         \
                                               sub_ring, last_ring_handle,  \
                                               last_sub_ring, __VA_ARGS__); \
            if (ret_val != HAL_RET_OK) break;                               \
            iter++;                                                         \
        }                                                                   \
    } while (false)
    
/*
 * Calculate offset into metrics memory
 */
#define ACCEL_METRICS_MEM_OFFSET(ring, sub_ring)                            \
    (((ring) * (ACCEL_SUB_RING_MAX * sizeof(accel_ring_metrics_t))) +       \
     ((sub_ring) * sizeof(accel_ring_metrics_t)))

/*
 * Write to metrics memory
 */
#define ACCEL_METRICS_MEM_WRITE(addr, buf, size)                            \
    do {                                                                    \
        sdk::asic::asic_mem_write(addr, buf, size);                         \
    } while (false)

/*
 * Get handle to Accelerator control block
 */
static inline cap_hens_csr_dhs_crypto_ctl_t&
accel_ctl(void)
{
    cap_top_csr_t&  cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    return cap0.md.hens.dhs_crypto_ctl;
}

/*
 * Get sw_reset capability
 */
static inline bool
accel_ring_sw_reset_capable(types::BarcoRings ring_type)
{
    bool    sw_reset_capable;
    bool    sw_enable_capable;

    capri_barco_get_capabilities((barco_rings_t)ring_type, &sw_reset_capable, &sw_enable_capable);
    return sw_reset_capable;
}

/*
 * Retrieve ring meta config info from HAL
 */
static void
accel_ring_meta_config_get(types::BarcoRings ring_type,
                           accel_rgroup_ring_info_t *ret_info)
{
    barco_ring_meta_config_t    meta = {0};

    capri_barco_get_meta_config_info((barco_rings_t)ring_type, &meta);
    ret_info->base_pa = meta.ring_base;
    ret_info->opaque_tag_pa = meta.opaque_tag_addr;
    ret_info->shadow_pndx_pa = meta.shadow_pndx_addr;
    ret_info->pndx_pa = meta.producer_idx_addr;
    ret_info->pndx_size = meta.pndx_size;
    ret_info->desc_size = meta.desc_size;
    ret_info->ring_size = meta.ring_size;
    ret_info->opaque_tag_size = meta.opaque_tag_size;
}

/*
 * Write to shadow pndx
 */
static inline hal_ret_t
accel_shadow_pndx_write(types::BarcoRings ring_type,
                        uint32_t val)
{
    barco_ring_meta_config_t meta = {0};

    capri_barco_get_meta_config_info((barco_rings_t)ring_type, &meta);
    if (meta.pndx_size) {
        assert(meta.pndx_size <= sizeof(val));
        if (sdk::asic::asic_mem_write(meta.shadow_pndx_addr, (uint8_t *)&val,
                                meta.pndx_size)) {
            HAL_TRACE_ERR("Failed to write shadow pndx @ {:x} size {}",
                          meta.shadow_pndx_addr, meta.pndx_size);
            return HAL_RET_HW_FAIL;
        }
    }
    return HAL_RET_OK;
}

static void accel_rgroup_timer(void *timer,
                               uint32_t timer_id,
                               void *user_ctx);

static accel_ring_cp_t          ring_cp(types::BarcoRings::BARCO_RING_CP);
static accel_ring_cp_hot_t      ring_cp_hot(types::BarcoRings::BARCO_RING_CP_HOT);
static accel_ring_dc_t          ring_dc(types::BarcoRings::BARCO_RING_DC);
static accel_ring_dc_hot_t      ring_dc_hot(types::BarcoRings::BARCO_RING_DC_HOT);
static accel_ring_xts0_t        ring_xts0(types::BarcoRings::BARCO_RING_XTS0);
static accel_ring_xts1_t        ring_xts1(types::BarcoRings::BARCO_RING_XTS1);
static accel_ring_gcm0_t        ring_gcm0(types::BarcoRings::BARCO_RING_GCM0);
static accel_ring_gcm1_t        ring_gcm1(types::BarcoRings::BARCO_RING_GCM1);


static accel_ring_ops_map_t     supported_ring_ops_map = {
    {"cp",      &ring_cp},
    {"cp_hot",  &ring_cp_hot},
    {"dc",      &ring_dc},
    {"dc_hot",  &ring_dc_hot},
    {"xts0",    &ring_xts0},
    {"xts1",    &ring_xts1},
    {"gcm0",    &ring_gcm0},
    {"gcm1",    &ring_gcm1},
};

static accel_rgroup_map_t       rgroup_map;
static void                     *rgroup_timer;

/*
 * Lock to guard rgroup addition/deletion interference with background timer.
 */
static sdk_spinlock_t           rgroup_lock;
static bool                     lock_inited;

#define ACCEL_RGROUP_LOCK()                                                 \
    SDK_SPINLOCK_LOCK(&rgroup_lock)

#define ACCEL_RGROUP_UNLOCK()                                               \
    SDK_SPINLOCK_UNLOCK(&rgroup_lock)

hal_ret_t
accel_rgroup_init(int tid)
{
    if (!lock_inited) {
        SDK_ASSERT(!SDK_SPINLOCK_INIT(&rgroup_lock, PTHREAD_PROCESS_PRIVATE));
        lock_inited = true;
    }
    if (!rgroup_timer && (tid == hal::HAL_THREAD_ID_PERIODIC)) {

        while (!sdk::lib::periodic_thread_is_running()) {
            pthread_yield();
        }

        rgroup_timer = sdk::lib::timer_schedule(HAL_TIMER_ID_ACCEL_RGROUP,
                                 ACCEL_RGROUP_METRICS_TIMER_MS, nullptr,
                                 accel_rgroup_timer, true);
        if (!rgroup_timer) {
            HAL_TRACE_ERR("Failed to start periodic rgroup timer");
            return HAL_RET_ERR;
        }
    }
    return HAL_RET_OK;
}


hal_ret_t
accel_rgroup_fini(int tid)
{
     if (rgroup_timer && (tid == hal::HAL_THREAD_ID_PERIODIC)) {
         sdk::lib::timer_delete(rgroup_timer);
         rgroup_timer = nullptr;
     }

     if (lock_inited) {
         SDK_SPINLOCK_DESTROY(&rgroup_lock);
         lock_inited = false;
     }
     return HAL_RET_OK;
}


/*
 * Located supported ring_ops given a ring name.
 */
accel_ring_ops_t *
accel_ring_ops_find(const std::string& ring_name)
{
    accel_ring_ops_iter_c   iter;

    iter = supported_ring_ops_map.find(ring_name);
    return iter == supported_ring_ops_map.end() ? nullptr : iter->second;
}

/*
 * Located an existing ring group given a group name.
 */
accel_rgroup_t *
accel_rgroup_find(const char *rgroup_name)
{
    accel_rgroup_iter_c iter;

    iter = rgroup_map.find(rgroup_name);
    return iter == rgroup_map.end() ? nullptr : iter->second;
}

/*
 * Add a new ring group.
 */
hal_ret_t
accel_rgroup_add(const char *rgroup_name,
                 uint64_t metrics_mem_addr,
                 uint32_t metrics_mem_size)
{
    accel_rgroup_t  *rgroup;
    hal_ret_t       ret_val;

    ACCEL_RGROUP_LOCK();
    if (accel_rgroup_find(rgroup_name)) {
        ret_val = HAL_RET_ENTRY_EXISTS;
        goto done;
    }

    if (metrics_mem_size) {
        if (metrics_mem_size < ACCEL_METRICS_MEM_OFFSET(ACCEL_RING_ID_MAX, 0)) {
            HAL_TRACE_ERR("rgroup {} metrics_mem_size {} too small",
                          rgroup_name, metrics_mem_size);
            ret_val = HAL_RET_INVALID_ARG;
            goto done;
        }
    }

    rgroup = new (std::nothrow) accel_rgroup_t(metrics_mem_addr,
                                               metrics_mem_size);
    if (!rgroup) {
        HAL_TRACE_ERR("Failed to allocate rgroup {}", rgroup_name);
        ret_val = HAL_RET_OOM;
        goto done;
    }
    rgroup_map.insert(std::make_pair(std::string(rgroup_name), rgroup));
    ret_val = HAL_RET_OK;

done:
    ACCEL_RGROUP_UNLOCK();
    return ret_val;
}

/*
 * Delete a ring group.
 */
hal_ret_t
accel_rgroup_del(const char *rgroup_name)
{
    accel_rgroup_t  *rgroup;

    ACCEL_RGROUP_LOCK();
    rgroup = accel_rgroup_find(rgroup_name);
    if (rgroup) {
        rgroup_map.erase(std::string(rgroup_name));
        delete rgroup;
    }
    ACCEL_RGROUP_UNLOCK();
    return HAL_RET_OK;
}

/*
 * Add a ring by name to a ring group.
 */
hal_ret_t
accel_rgroup_ring_add(const char *rgroup_name,
                      const char *ring_name,
                      uint32_t ring_handle)
{
    accel_rgroup_t  *rgroup;
    hal_ret_t       ret_val = HAL_RET_ENTRY_NOT_FOUND;

    ACCEL_RGROUP_LOCK();
    rgroup = accel_rgroup_find(rgroup_name);
    if (rgroup) {
        ret_val = rgroup->ring_add(std::string(ring_name), ring_handle);
    }
    ACCEL_RGROUP_UNLOCK();

    return ret_val;
}

/*
 * Remove a ring by name from a ring group.
 */
hal_ret_t
accel_rgroup_ring_del(const char *rgroup_name,
                      const char *ring_name)
{
    accel_rgroup_t  *rgroup;

    ACCEL_RGROUP_LOCK();
    rgroup = accel_rgroup_find(rgroup_name);
    if (rgroup) {
        rgroup->ring_del(std::string(ring_name));
    }
    ACCEL_RGROUP_UNLOCK();
    return HAL_RET_OK;
}

/*
 * Invoke ring_reset on all rings in a group, taking the argument sub_ring
 * into considerations.
 */
hal_ret_t
accel_rgroup_reset_set(const char *rgroup_name,
                       uint32_t sub_ring,
                       uint32_t *last_ring_handle,
                       uint32_t *last_sub_ring,
                       bool reset_sense)
{
    accel_rgroup_t  *rgroup;
    hal_ret_t       ret_val = HAL_RET_ENTRY_NOT_FOUND;

    ACCEL_RGROUP_LOCK();
    rgroup = accel_rgroup_find(rgroup_name);
    if (rgroup) {
        ret_val = rgroup->reset_set(sub_ring, last_ring_handle,
                                    last_sub_ring, reset_sense);
    }
    ACCEL_RGROUP_UNLOCK();
    return ret_val;
}

/*
 * Invoke ring_enable on all rings in a group, taking the argument sub_ring
 * into considerations.
 */
hal_ret_t
accel_rgroup_enable_set(const char *rgroup_name,
                        uint32_t sub_ring,
                        uint32_t *last_ring_handle,
                        uint32_t *last_sub_ring,
                        bool enable_sense)
{
    accel_rgroup_t  *rgroup;
    hal_ret_t       ret_val = HAL_RET_ENTRY_NOT_FOUND;

    ACCEL_RGROUP_LOCK();
    rgroup = accel_rgroup_find(rgroup_name);
    if (rgroup) {
        ret_val = rgroup->enable_set(sub_ring, last_ring_handle,
                                     last_sub_ring, enable_sense);
    }
    ACCEL_RGROUP_UNLOCK();
    return ret_val;
}

/*
 * Invoke ring_pndx_set on all rings in a group, taking the argument sub_ring
 * into considerations.
 */
hal_ret_t
accel_rgroup_pndx_set(const char *rgroup_name,
                      uint32_t sub_ring,
                      uint32_t *last_ring_handle,
                      uint32_t *last_sub_ring,
                      uint32_t val,
                      bool conditional)
{
    accel_rgroup_t  *rgroup;
    hal_ret_t       ret_val = HAL_RET_ENTRY_NOT_FOUND;

    ACCEL_RGROUP_LOCK();
    rgroup = accel_rgroup_find(rgroup_name);
    if (rgroup) {
        ret_val = rgroup->pndx_set(sub_ring, last_ring_handle,
                                   last_sub_ring, val, conditional);
    }
    ACCEL_RGROUP_UNLOCK();
    return ret_val;
}

/*
 * Retrieve config info for all rings in a group.
 */
hal_ret_t 
accel_rgroup_info_get(const char *rgroup_name,
                      uint32_t sub_ring,
                      accel_rgroup_ring_info_cb_t cb_func,
                      void *user_ctx)
{
    accel_rgroup_t  *rgroup;
    hal_ret_t       ret_val = HAL_RET_ENTRY_NOT_FOUND;

    ACCEL_RGROUP_LOCK();
    rgroup = accel_rgroup_find(rgroup_name);
    if (rgroup) {
        ret_val = rgroup->info_get(sub_ring, cb_func, user_ctx);
    }
    ACCEL_RGROUP_UNLOCK();
    return ret_val;
}

/*
 * Retrieve indices for all rings in a group.
 */
hal_ret_t 
accel_rgroup_indices_get(const char *rgroup_name,
                         uint32_t sub_ring,
                         accel_rgroup_ring_indices_cb_t cb_func,
                         void *user_ctx)
{
    accel_rgroup_t  *rgroup;
    hal_ret_t       ret_val = HAL_RET_ENTRY_NOT_FOUND;

    ACCEL_RGROUP_LOCK();
    rgroup = accel_rgroup_find(rgroup_name);
    if (rgroup) {
        ret_val = rgroup->indices_get(sub_ring, cb_func, user_ctx);
    }
    ACCEL_RGROUP_UNLOCK();
    return ret_val;
}

/*
 * Retrieve metrics for all rings in a group.
 */
hal_ret_t 
accel_rgroup_metrics_get(const char *rgroup_name,
                         uint32_t sub_ring,
                         accel_rgroup_ring_metrics_cb_t cb_func,
                         void *user_ctx)
{
    accel_rgroup_t  *rgroup;
    hal_ret_t       ret_val = HAL_RET_ENTRY_NOT_FOUND;

    ACCEL_RGROUP_LOCK();
    rgroup = accel_rgroup_find(rgroup_name);
    if (rgroup) {
        ret_val = rgroup->metrics_get(sub_ring, cb_func, user_ctx);
    }
    ACCEL_RGROUP_UNLOCK();
    return ret_val;
}

/*
 * Retrieve miscellaneous config/status for all rings in a group.
 */
hal_ret_t 
accel_rgroup_misc_get(const char *rgroup_name,
                      uint32_t sub_ring,
                      accel_rgroup_ring_misc_cb_t cb_func,
                      void *user_ctx)
{
    accel_rgroup_t  *rgroup;
    hal_ret_t       ret_val = HAL_RET_ENTRY_NOT_FOUND;

    ACCEL_RGROUP_LOCK();
    rgroup = accel_rgroup_find(rgroup_name);
    if (rgroup) {
        ret_val = rgroup->misc_get(sub_ring, cb_func, user_ctx);
    }
    ACCEL_RGROUP_UNLOCK();
    return ret_val;
}

/*
 * Add a ring by name to a ring group. The ring must have 
 * supported ring_ops.
 */
hal_ret_t
accel_rgroup_t::ring_add(const std::string& ring_name,
                         uint32_t ring_handle)
{
    accel_ring_ops_t            *ring_ops;
    accel_rgroup_elem_iter_c    elem_iter;

    ring_ops = accel_ring_ops_find(ring_name);
    if (!ring_ops) {
        return HAL_RET_INVALID_ARG;
    }

    elem_iter = elem_map.find(ring_name);
    if (elem_iter != elem_map.end()) {
        return HAL_RET_ENTRY_EXISTS;
    }

    elem_map.insert(std::make_pair(ring_name, 
                                   std::make_pair(ring_ops, ring_handle)));
    return HAL_RET_OK;
}

/*
 * Remove a ring by name from a ring group.
 */
hal_ret_t
accel_rgroup_t::ring_del(const std::string& ring_name)
{
    accel_rgroup_elem_iter_c    elem_iter;

    elem_iter = elem_map.find(ring_name);
    if (elem_iter == elem_map.end()) {
        return HAL_RET_ENTRY_NOT_FOUND;
    }

    elem_map.erase(ring_name);
    return HAL_RET_OK;
}

/*
 * Invoke ring_reset on all rings in a group, taking the argument sub_ring
 * into considerations.
 */
hal_ret_t
accel_rgroup_t::reset_set(uint32_t sub_ring,
                          uint32_t *last_ring_handle,
                          uint32_t *last_sub_ring,
                          bool reset_sense)
{
    hal_ret_t   ret_val = HAL_RET_OK;
    FOR_EACH_RGROUP_RING_INVOKE_RET_LAST(reset_set, sub_ring, ret_val,
                                         last_ring_handle, last_sub_ring,
                                         reset_sense);
    return ret_val;
}

/*
 * Invoke ring_enable on all rings in a group, taking the argument sub_ring
 * into considerations.
 */
hal_ret_t
accel_rgroup_t::enable_set(uint32_t sub_ring,
                           uint32_t *last_ring_handle,
                           uint32_t *last_sub_ring,
                           bool enable_sense)
{
    hal_ret_t   ret_val = HAL_RET_OK;
    FOR_EACH_RGROUP_RING_INVOKE_RET_LAST(enable_set, sub_ring, ret_val,
                                         last_ring_handle, last_sub_ring,
                                         enable_sense);
    return ret_val;
}

/*
 * Invoke ring_pndx_set on all rings in a group, taking the argument sub_ring
 * into considerations.
 */
hal_ret_t
accel_rgroup_t::pndx_set(uint32_t sub_ring,
                         uint32_t *last_ring_handle,
                         uint32_t *last_sub_ring,
                         uint32_t val,
                         bool conditional)
{
    hal_ret_t   ret_val = HAL_RET_OK;
    FOR_EACH_RGROUP_RING_INVOKE_RET_LAST(pndx_set, sub_ring, ret_val,
                                         last_ring_handle, last_sub_ring, val,
                                         conditional);
    return ret_val;
}

/*
 * Retrieve config info on all rings in a group.
 */
hal_ret_t 
accel_rgroup_t::info_get(uint32_t sub_ring,
                         accel_rgroup_ring_info_cb_t cb_func,
                         void *user_ctx)
{
    hal_ret_t   ret_val = HAL_RET_OK;
    FOR_EACH_RGROUP_RING_INVOKE(info_get, sub_ring, ret_val,
                                cb_func, user_ctx);
    return ret_val;
}

/*
 * Retrieve current ring indices on all rings in a group.
 */
hal_ret_t 
accel_rgroup_t::indices_get(uint32_t sub_ring,
                            accel_rgroup_ring_indices_cb_t cb_func,
                            void *user_ctx)
{
    hal_ret_t   ret_val = HAL_RET_OK;
    FOR_EACH_RGROUP_RING_INVOKE(indices_get, sub_ring, ret_val,
                                cb_func, user_ctx);
    return ret_val;
}

/*
 * Retrieve current ring metrics on all rings in a group.
 */
hal_ret_t 
accel_rgroup_t::metrics_get(uint32_t sub_ring,
                            accel_rgroup_ring_metrics_cb_t cb_func,
                            void *user_ctx)
{
    hal_ret_t   ret_val = HAL_RET_OK;
    FOR_EACH_RGROUP_RING_INVOKE(metrics_get, sub_ring, ret_val,
                                cb_func, user_ctx);
    return ret_val;
}

/*
 * Retrieve  miscellaneous config/status on all rings in a group.
 */
hal_ret_t 
accel_rgroup_t::misc_get(uint32_t sub_ring,
                         accel_rgroup_ring_misc_cb_t cb_func,
                         void *user_ctx)
{
    hal_ret_t   ret_val = HAL_RET_OK;
    FOR_EACH_RGROUP_RING_INVOKE(misc_get, sub_ring, ret_val,
                                cb_func, user_ctx);
    return ret_val;
}

/*
 * All supported ring_ops methods begin here
 */
hal_ret_t 
accel_ring_cp_t::reset_set(uint32_t ring_handle,
                           uint32_t sub_ring,
                           uint32_t *last_ring_handle,
                           uint32_t *last_sub_ring,
                           bool reset_sense)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    ACCEL_CFG_RMW32(cp_cfg_glb, BARCO_CRYPTO_CP_CFG_GLB_SOFT_RESET, reset_sense);
    soft_resets += reset_sense ? 1 : 0;
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_cp_t::enable_set(uint32_t ring_handle,
                            uint32_t sub_ring,
                            uint32_t *last_ring_handle,
                            uint32_t *last_sub_ring,
                            bool enable_sense)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    ACCEL_CFG_RMW32(cp_cfg_dist, BARCO_CRYPTO_CP_DIST_DESC_Q_EN, enable_sense);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_cp_t::pndx_set(uint32_t ring_handle,
                          uint32_t sub_ring,
                          uint32_t *last_ring_handle,
                          uint32_t *last_sub_ring,
                          uint32_t val,
                          bool conditional)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    if (!conditional || accel_ring_sw_reset_capable(ring_type)) {
        ACCEL_CFG_WRITE32(cp_cfg_q_pd_idx, val);
        return accel_shadow_pndx_write(ring_type, val);
    }
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_cp_t::info_get(uint32_t ring_handle,
                          uint32_t sub_ring,
                          accel_rgroup_ring_info_cb_t cb_func,
                          void *usr_ctx)
{
    accel_rgroup_ring_info_t    info = {0};

    info.ring_handle = ring_handle;
    info.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    accel_ring_meta_config_get(ring_type, &info);
    capri_barco_get_capabilities((barco_rings_t)ring_type, &info.sw_reset_capable,
                                 &info.sw_enable_capable);
    assert(info.pndx_size);
    (*cb_func)(usr_ctx, info);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_cp_t::indices_get(uint32_t ring_handle,
                             uint32_t sub_ring,
                             accel_rgroup_ring_indices_cb_t cb_func,
                             void *usr_ctx)
{
    accel_rgroup_ring_indices_t indices = {0};

    indices.ring_handle = ring_handle;
    indices.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_READ32(cp_cfg_q_pd_idx, indices.pndx);
    ACCEL_CFG_READ32(cp_sta_q_cp_idx, indices.cndx);
    ACCEL_CFG_READ32(cp_sta_q_cp_idx_early, indices.endx);

    (*cb_func)(usr_ctx, indices);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_cp_t::metrics_get(uint32_t ring_handle,
                             uint32_t sub_ring,
                             accel_rgroup_ring_metrics_cb_t cb_func,
                             void *usr_ctx)
{
    accel_rgroup_ring_metrics_t metrics = {0};

    metrics.ring_handle = ring_handle;
    metrics.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    metrics.soft_resets = soft_resets;
    ACCEL_CFG_READ64(cp_sta_in_bcnt_w0,  cp_sta_in_bcnt_w1,  metrics.input_bytes);
    ACCEL_CFG_READ64(cp_sta_out_bcnt_w0, cp_sta_out_bcnt_w1, metrics.output_bytes);

    (*cb_func)(usr_ctx, metrics);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_cp_t::misc_get(uint32_t ring_handle,
                          uint32_t sub_ring,
                          accel_rgroup_ring_misc_cb_t cb_func,
                          void *usr_ctx)
{
    accel_rgroup_ring_misc_t misc = {0};

    misc.ring_handle = ring_handle;
    misc.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    /*
     * Note that some registers require clearing after read
     */
    ACCEL_CFG_NAMED_READ32(cp_cfg_glb,             misc.reg_val[0]);
    ACCEL_CFG_NAMED_READ32(cp_cfg_dist,            misc.reg_val[1]);
    ACCEL_CFG_NAMED_READ32(cp_cfg_host,            misc.reg_val[2]);
    ACCEL_CFG_NAMED_READ32(cp_cfg_limit,           misc.reg_val[3]);
    ACCEL_CFG_NAMED_READ32(cp_cfg_q_base_adr_w0,   misc.reg_val[4]);
    ACCEL_CFG_NAMED_READ32(cp_cfg_q_base_adr_w1,   misc.reg_val[5]);
    ACCEL_CFG_NAMED_READ32(cp_cfg_ueng_w0,         misc.reg_val[6]);
    ACCEL_CFG_NAMED_READ32(cp_cfg_ueng_w1,         misc.reg_val[7]);
    ACCEL_CFG_NAMED_READ32(cp_int,                 misc.reg_val[8]);
    ACCEL_CFG_NAMED_READ32(cp_int_mask,            misc.reg_val[9]);
    ACCEL_CFG_NAMED_READ32(cp_cfg_axi_settings_w0, misc.reg_val[10]);
    ACCEL_CFG_NAMED_READ32(cp_cfg_axi_settings_w1, misc.reg_val[11]);
    ACCEL_CFG_NAMED_READ32(cp_cfg_axi_timeout,     misc.reg_val[12]);

    ACCEL_CFG_NAMED_READ32(cp_sta_q_cp_idx_early,  misc.reg_val[13]);
    ACCEL_CFG_NAMED_RD_CLR32(cp_int_ecc_error,     misc.reg_val[14]);
    ACCEL_CFG_NAMED_RD_CLR32(cp_int_axi_error_w0,  misc.reg_val[15]);
    ACCEL_CFG_NAMED_RD_CLR32(cp_int_axi_error_w1,  misc.reg_val[16]);
    ACCEL_CFG_NAMED_RD_CLR32(cp_int_ueng_error_w0, misc.reg_val[17]);
    ACCEL_CFG_NAMED_RD_CLR32(cp_int_ueng_error_w1, misc.reg_val[18]);
    ACCEL_CFG_NAMED_READ32(cp_sta_ecc_error,       misc.reg_val[19]);
    ACCEL_CFG_NAMED_READ32(cp_sta_bist_done_pass,  misc.reg_val[20]);
    ACCEL_CFG_NAMED_READ32(cp_sta_bist_done_fail,  misc.reg_val[21]);

    ACCEL_CFG_NAMED_READ32(cp_sta_debug_w0,        misc.reg_val[22]);
    ACCEL_CFG_NAMED_READ32(cp_sta_debug_w1,        misc.reg_val[23]);
    ACCEL_CFG_NAMED_READ32(cp_sta_debug_w2,        misc.reg_val[24]);
    ACCEL_CFG_NAMED_READ32(cp_sta_debug_w3,        misc.reg_val[25]);
    ACCEL_CFG_NAMED_READ32(cp_sta_debug_w4,        misc.reg_val[26]);
    ACCEL_CFG_NAMED_READ32(cp_sta_debug_w5,        misc.reg_val[27]);
    ACCEL_CFG_NAMED_READ32(cp_sta_debug_w6,        misc.reg_val[28]);
    ACCEL_CFG_NAMED_READ32(cp_sta_debug_w7,        misc.reg_val[29]);
    misc.num_reg_vals = 30;

    (*cb_func)(usr_ctx, misc);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_cp_hot_t::reset_set(uint32_t ring_handle,
                               uint32_t sub_ring,
                               uint32_t *last_ring_handle,
                               uint32_t *last_sub_ring,
                               bool reset_sense)
{
    // reset as part of cp so nothing to do
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_cp_hot_t::enable_set(uint32_t ring_handle,
                                uint32_t sub_ring,
                                uint32_t *last_ring_handle,
                                uint32_t *last_sub_ring,
                                bool enable_sense)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    ACCEL_CFG_RMW32(cp_cfg_dist, BARCO_CRYPTO_CP_DIST_DESC_HOTQ_EN, enable_sense);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_cp_hot_t::pndx_set(uint32_t ring_handle,
                              uint32_t sub_ring,
                              uint32_t *last_ring_handle,
                              uint32_t *last_sub_ring,
                              uint32_t val,
                              bool conditional)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    if (!conditional || accel_ring_sw_reset_capable(ring_type)) {
        ACCEL_CFG_WRITE32(cp_cfg_hotq_pd_idx, val);
        return accel_shadow_pndx_write(ring_type, val);
    }
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_cp_hot_t::info_get(uint32_t ring_handle,
                              uint32_t sub_ring,
                              accel_rgroup_ring_info_cb_t cb_func,
                              void *usr_ctx)
{
    accel_rgroup_ring_info_t    info = {0};

    info.ring_handle = ring_handle;
    info.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    accel_ring_meta_config_get(ring_type, &info);
    capri_barco_get_capabilities((barco_rings_t)ring_type, &info.sw_reset_capable,
                                 &info.sw_enable_capable);
    assert(info.pndx_size);
    (*cb_func)(usr_ctx, info);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_cp_hot_t::indices_get(uint32_t ring_handle,
                                 uint32_t sub_ring,
                                 accel_rgroup_ring_indices_cb_t cb_func,
                                 void *usr_ctx)
{
    accel_rgroup_ring_indices_t indices = {0};

    indices.ring_handle = ring_handle;
    indices.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_READ32(cp_cfg_hotq_pd_idx, indices.pndx);
    ACCEL_CFG_READ32(cp_sta_hotq_cp_idx, indices.cndx);
    ACCEL_CFG_READ32(cp_sta_hotq_cp_idx_early, indices.endx);

    (*cb_func)(usr_ctx, indices);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_cp_hot_t::metrics_get(uint32_t ring_handle,
                                 uint32_t sub_ring,
                                 accel_rgroup_ring_metrics_cb_t cb_func,
                                 void *usr_ctx)
{
    accel_rgroup_ring_metrics_t metrics = {0};

    /*
     * CP hot ring does not have bytes metrics
     */
    metrics.ring_handle = ring_handle;
    metrics.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    metrics.soft_resets = soft_resets;
    (*cb_func)(usr_ctx, metrics);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_cp_hot_t::misc_get(uint32_t ring_handle,
                              uint32_t sub_ring,
                              accel_rgroup_ring_misc_cb_t cb_func,
                              void *usr_ctx)
{
    accel_rgroup_ring_misc_t misc = {0};

    misc.ring_handle = ring_handle;
    misc.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_NAMED_READ32(cp_cfg_hotq_base_adr_w0,  misc.reg_val[0]);
    ACCEL_CFG_NAMED_READ32(cp_cfg_hotq_base_adr_w1,  misc.reg_val[1]);
    ACCEL_CFG_NAMED_READ32(cp_sta_hotq_cp_idx_early, misc.reg_val[2]);
    misc.num_reg_vals = 3;

    (*cb_func)(usr_ctx, misc);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_dc_t::reset_set(uint32_t ring_handle,
                           uint32_t sub_ring,
                           uint32_t *last_ring_handle,
                           uint32_t *last_sub_ring,
                           bool reset_sense)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    ACCEL_CFG_RMW32(dc_cfg_glb, BARCO_CRYPTO_DC_CFG_GLB_SOFT_RESET, reset_sense);
    soft_resets += reset_sense ? 1 : 0;
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_dc_t::enable_set(uint32_t ring_handle,
                            uint32_t sub_ring,
                            uint32_t *last_ring_handle,
                            uint32_t *last_sub_ring,
                            bool enable_sense)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    ACCEL_CFG_RMW32(dc_cfg_dist, BARCO_CRYPTO_DC_DIST_DESC_Q_EN, enable_sense);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_dc_t::pndx_set(uint32_t ring_handle,
                          uint32_t sub_ring,
                          uint32_t *last_ring_handle,
                          uint32_t *last_sub_ring,
                          uint32_t val,
                          bool conditional)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    if (!conditional || accel_ring_sw_reset_capable(ring_type)) {
        ACCEL_CFG_WRITE32(dc_cfg_q_pd_idx, val);
        return accel_shadow_pndx_write(ring_type, val);
    }
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_dc_t::info_get(uint32_t ring_handle,
                          uint32_t sub_ring,
                          accel_rgroup_ring_info_cb_t cb_func,
                          void *usr_ctx)
{
    accel_rgroup_ring_info_t    info = {0};

    info.ring_handle = ring_handle;
    info.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    accel_ring_meta_config_get(ring_type, &info);
    capri_barco_get_capabilities((barco_rings_t)ring_type, &info.sw_reset_capable,
                                 &info.sw_enable_capable);
    assert(info.pndx_size);
    (*cb_func)(usr_ctx, info);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_dc_t::indices_get(uint32_t ring_handle,
                             uint32_t sub_ring,
                             accel_rgroup_ring_indices_cb_t cb_func,
                             void *usr_ctx)
{
    accel_rgroup_ring_indices_t indices = {0};

    indices.ring_handle = ring_handle;
    indices.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_READ32(dc_cfg_q_pd_idx, indices.pndx);
    ACCEL_CFG_READ32(dc_sta_q_cp_idx, indices.cndx);
    ACCEL_CFG_READ32(dc_sta_q_cp_idx_early, indices.endx);

    (*cb_func)(usr_ctx, indices);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_dc_t::metrics_get(uint32_t ring_handle,
                             uint32_t sub_ring,
                             accel_rgroup_ring_metrics_cb_t cb_func,
                             void *usr_ctx)
{
    accel_rgroup_ring_metrics_t metrics = {0};

    metrics.ring_handle = ring_handle;
    metrics.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    metrics.soft_resets = soft_resets;
    ACCEL_CFG_READ64(dc_sta_in_bcnt_w0,  dc_sta_in_bcnt_w1,  metrics.input_bytes);
    ACCEL_CFG_READ64(dc_sta_out_bcnt_w0, dc_sta_out_bcnt_w1, metrics.output_bytes);

    (*cb_func)(usr_ctx, metrics);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_dc_t::misc_get(uint32_t ring_handle,
                          uint32_t sub_ring,
                          accel_rgroup_ring_misc_cb_t cb_func,
                          void *usr_ctx)
{
    accel_rgroup_ring_misc_t misc = {0};

    misc.ring_handle = ring_handle;
    misc.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    /*
     * Note that some registers require clearing after read
     */
    ACCEL_CFG_NAMED_READ32(dc_cfg_glb,             misc.reg_val[0]);
    ACCEL_CFG_NAMED_READ32(dc_cfg_dist,            misc.reg_val[1]);
    ACCEL_CFG_NAMED_READ32(dc_cfg_host,            misc.reg_val[2]);
    ACCEL_CFG_NAMED_READ32(dc_cfg_limit,           misc.reg_val[3]);
    ACCEL_CFG_NAMED_READ32(dc_cfg_q_base_adr_w0,   misc.reg_val[4]);
    ACCEL_CFG_NAMED_READ32(dc_cfg_q_base_adr_w1,   misc.reg_val[5]);
    ACCEL_CFG_NAMED_READ32(dc_cfg_ueng_w0,         misc.reg_val[6]);
    ACCEL_CFG_NAMED_READ32(dc_cfg_ueng_w1,         misc.reg_val[7]);
    ACCEL_CFG_NAMED_READ32(dc_int,                 misc.reg_val[8]);
    ACCEL_CFG_NAMED_READ32(dc_int_mask,            misc.reg_val[9]);
    ACCEL_CFG_NAMED_READ32(dc_cfg_axi_settings_w0, misc.reg_val[10]);
    ACCEL_CFG_NAMED_READ32(dc_cfg_axi_settings_w1, misc.reg_val[11]);
    ACCEL_CFG_NAMED_READ32(dc_cfg_axi_timeout,     misc.reg_val[12]);

    ACCEL_CFG_NAMED_READ32(dc_sta_q_cp_idx_early,  misc.reg_val[13]);
    ACCEL_CFG_NAMED_RD_CLR32(dc_int_ecc_error,     misc.reg_val[14]);
    ACCEL_CFG_NAMED_RD_CLR32(dc_int_axi_error_w0,  misc.reg_val[15]);
    ACCEL_CFG_NAMED_RD_CLR32(dc_int_axi_error_w1,  misc.reg_val[16]);
    ACCEL_CFG_NAMED_RD_CLR32(dc_int_ueng_error_w0, misc.reg_val[17]);
    ACCEL_CFG_NAMED_RD_CLR32(dc_int_ueng_error_w1, misc.reg_val[18]);
    ACCEL_CFG_NAMED_READ32(dc_sta_ecc_error,       misc.reg_val[19]);
    ACCEL_CFG_NAMED_READ32(dc_sta_bist_done_pass,  misc.reg_val[20]);
    ACCEL_CFG_NAMED_READ32(dc_sta_bist_done_fail,  misc.reg_val[21]);

    ACCEL_CFG_NAMED_READ32(dc_sta_debug_w0,        misc.reg_val[22]);
    ACCEL_CFG_NAMED_READ32(dc_sta_debug_w1,        misc.reg_val[23]);
    ACCEL_CFG_NAMED_READ32(dc_sta_debug_w2,        misc.reg_val[24]);
    ACCEL_CFG_NAMED_READ32(dc_sta_debug_w3,        misc.reg_val[25]);
    ACCEL_CFG_NAMED_READ32(dc_sta_debug_w4,        misc.reg_val[26]);
    ACCEL_CFG_NAMED_READ32(dc_sta_debug_w5,        misc.reg_val[27]);
    ACCEL_CFG_NAMED_READ32(dc_sta_debug_w6,        misc.reg_val[28]);
    ACCEL_CFG_NAMED_READ32(dc_sta_debug_w7,        misc.reg_val[29]);
    misc.num_reg_vals = 30;

    (*cb_func)(usr_ctx, misc);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_dc_hot_t::reset_set(uint32_t ring_handle,
                               uint32_t sub_ring,
                               uint32_t *last_ring_handle,
                               uint32_t *last_sub_ring,
                               bool reset_sense)
{
    // reset as part of dc so nothing to do
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_dc_hot_t::enable_set(uint32_t ring_handle,
                                uint32_t sub_ring,
                                uint32_t *last_ring_handle,
                                uint32_t *last_sub_ring,
                                bool enable_sense)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    ACCEL_CFG_RMW32(dc_cfg_dist, BARCO_CRYPTO_DC_DIST_DESC_HOTQ_EN, enable_sense);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_dc_hot_t::pndx_set(uint32_t ring_handle,
                              uint32_t sub_ring,
                              uint32_t *last_ring_handle,
                              uint32_t *last_sub_ring,
                              uint32_t val,
                              bool conditional)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    if (!conditional || accel_ring_sw_reset_capable(ring_type)) {
        ACCEL_CFG_WRITE32(dc_cfg_hotq_pd_idx, val);
        return accel_shadow_pndx_write(ring_type, val);
    }
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_dc_hot_t::info_get(uint32_t ring_handle,
                              uint32_t sub_ring,
                              accel_rgroup_ring_info_cb_t cb_func,
                              void *usr_ctx)
{
    accel_rgroup_ring_info_t    info = {0};

    info.ring_handle = ring_handle;
    info.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    accel_ring_meta_config_get(ring_type, &info);
    capri_barco_get_capabilities((barco_rings_t)ring_type, &info.sw_reset_capable,
                                 &info.sw_enable_capable);
    assert(info.pndx_size);
    (*cb_func)(usr_ctx, info);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_dc_hot_t::indices_get(uint32_t ring_handle,
                                 uint32_t sub_ring,
                                 accel_rgroup_ring_indices_cb_t cb_func,
                                 void *usr_ctx)
{
    accel_rgroup_ring_indices_t indices = {0};

    indices.ring_handle = ring_handle;
    indices.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_READ32(dc_cfg_hotq_pd_idx, indices.pndx);
    ACCEL_CFG_READ32(dc_sta_hotq_cp_idx, indices.cndx);
    ACCEL_CFG_READ32(dc_sta_hotq_cp_idx_early, indices.endx);

    (*cb_func)(usr_ctx, indices);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_dc_hot_t::metrics_get(uint32_t ring_handle,
                                 uint32_t sub_ring,
                                 accel_rgroup_ring_metrics_cb_t cb_func,
                                 void *usr_ctx)
{
    accel_rgroup_ring_metrics_t metrics = {0};

    /*
     * DC hot ring does not have bytes metrics
     */
    metrics.ring_handle = ring_handle;
    metrics.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    metrics.soft_resets = soft_resets;
    (*cb_func)(usr_ctx, metrics);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_dc_hot_t::misc_get(uint32_t ring_handle,
                              uint32_t sub_ring,
                              accel_rgroup_ring_misc_cb_t cb_func,
                              void *usr_ctx)
{
    accel_rgroup_ring_misc_t misc = {0};

    misc.ring_handle = ring_handle;
    misc.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_NAMED_READ32(dc_cfg_hotq_base_adr_w0,  misc.reg_val[0]);
    ACCEL_CFG_NAMED_READ32(dc_cfg_hotq_base_adr_w1,  misc.reg_val[1]);
    ACCEL_CFG_NAMED_READ32(dc_sta_hotq_cp_idx_early, misc.reg_val[2]);
    misc.num_reg_vals = 3;

    (*cb_func)(usr_ctx, misc);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_xts0_t::reset_set(uint32_t ring_handle,
                             uint32_t sub_ring,
                             uint32_t *last_ring_handle,
                             uint32_t *last_sub_ring,
                             bool reset_sense)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    ACCEL_CFG_RMW32(xts_enc_soft_rst, 0xffffffff, reset_sense);
    soft_resets += reset_sense ? 1 : 0;
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_xts0_t::enable_set(uint32_t ring_handle,
                              uint32_t sub_ring,
                              uint32_t *last_ring_handle,
                              uint32_t *last_sub_ring,
                              bool enable_sense)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    // XTS0 does not implement ring_enable
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_xts0_t::pndx_set(uint32_t ring_handle,
                            uint32_t sub_ring,
                            uint32_t *last_ring_handle,
                            uint32_t *last_sub_ring,
                            uint32_t val,
                            bool conditional)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    if (!conditional || accel_ring_sw_reset_capable(ring_type)) {
        ACCEL_CFG_WRITE32(xts_enc_producer_idx, val);
        return accel_shadow_pndx_write(ring_type, val);
    }
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_xts0_t::info_get(uint32_t ring_handle,
                            uint32_t sub_ring,
                            accel_rgroup_ring_info_cb_t cb_func,
                            void *usr_ctx)
{
    accel_rgroup_ring_info_t    info = {0};

    info.ring_handle = ring_handle;
    info.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    accel_ring_meta_config_get(ring_type, &info);
    capri_barco_get_capabilities((barco_rings_t)ring_type, &info.sw_reset_capable,
                                 &info.sw_enable_capable);
    assert(info.pndx_size);
    (*cb_func)(usr_ctx, info);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_xts0_t::indices_get(uint32_t ring_handle,
                               uint32_t sub_ring,
                               accel_rgroup_ring_indices_cb_t cb_func,
                               void *usr_ctx)
{
    accel_rgroup_ring_indices_t indices = {0};

    indices.ring_handle = ring_handle;
    indices.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_READ32(xts_enc_producer_idx, indices.pndx);
    ACCEL_CFG_READ32(xts_enc_consumer_idx, indices.cndx);
    ACCEL_CFG_READ32(xts_enc_error_idx, indices.endx);

    (*cb_func)(usr_ctx, indices);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_xts0_t::metrics_get(uint32_t ring_handle,
                               uint32_t sub_ring,
                               accel_rgroup_ring_metrics_cb_t cb_func,
                               void *usr_ctx)
{
    accel_rgroup_ring_metrics_t metrics = {0};

    /*
     * XTS does not have bytes metrics
     */
    metrics.ring_handle = ring_handle;
    metrics.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    metrics.soft_resets = soft_resets;
    (*cb_func)(usr_ctx, metrics);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_xts0_t::misc_get(uint32_t ring_handle,
                            uint32_t sub_ring,
                            accel_rgroup_ring_misc_cb_t cb_func,
                            void *usr_ctx)
{
    accel_rgroup_ring_misc_t misc = {0};

    misc.ring_handle = ring_handle;
    misc.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_NAMED_READ32(xts_enc_ring_base_w0, misc.reg_val[0]);
    ACCEL_CFG_NAMED_READ32(xts_enc_ring_base_w1, misc.reg_val[1]);
    ACCEL_CFG_NAMED_READ32(xts_enc_ring_size,    misc.reg_val[2]);
    ACCEL_CFG_NAMED_READ32(xts_enc_ci_addr_w0,   misc.reg_val[3]);
    ACCEL_CFG_NAMED_READ32(xts_enc_ci_addr_w1,   misc.reg_val[4]);
    ACCEL_CFG_NAMED_READ32(xts_enc_status,       misc.reg_val[5]);
    ACCEL_CFG_NAMED_READ32(xts_enc_error_idx,    misc.reg_val[6]);
    misc.num_reg_vals = 7;

    (*cb_func)(usr_ctx, misc);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_xts1_t::reset_set(uint32_t ring_handle,
                             uint32_t sub_ring,
                             uint32_t *last_ring_handle,
                             uint32_t *last_sub_ring,
                             bool reset_sense)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    ACCEL_CFG_RMW32(xts_soft_rst, 0xffffffff, reset_sense);
    soft_resets += reset_sense ? 1 : 0;
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_xts1_t::enable_set(uint32_t ring_handle,
                              uint32_t sub_ring,
                              uint32_t *last_ring_handle,
                              uint32_t *last_sub_ring,
                              bool enable_sense)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    // XTS1 does not implement ring_enable
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_xts1_t::pndx_set(uint32_t ring_handle,
                            uint32_t sub_ring,
                            uint32_t *last_ring_handle,
                            uint32_t *last_sub_ring,
                            uint32_t val,
                            bool conditional)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    if (!conditional || accel_ring_sw_reset_capable(ring_type)) {
        ACCEL_CFG_WRITE32(xts_producer_idx, val);
        return accel_shadow_pndx_write(ring_type, val);
    }
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_xts1_t::info_get(uint32_t ring_handle,
                            uint32_t sub_ring,
                            accel_rgroup_ring_info_cb_t cb_func,
                            void *usr_ctx)
{
    accel_rgroup_ring_info_t    info = {0};

    info.ring_handle = ring_handle;
    info.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    accel_ring_meta_config_get(ring_type, &info);
    capri_barco_get_capabilities((barco_rings_t)ring_type, &info.sw_reset_capable,
                                 &info.sw_enable_capable);
    assert(info.pndx_size);
    (*cb_func)(usr_ctx, info);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_xts1_t::indices_get(uint32_t ring_handle,
                               uint32_t sub_ring,
                               accel_rgroup_ring_indices_cb_t cb_func,
                               void *usr_ctx)
{
    accel_rgroup_ring_indices_t indices = {0};

    indices.ring_handle = ring_handle;
    indices.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_READ32(xts_producer_idx, indices.pndx);
    ACCEL_CFG_READ32(xts_consumer_idx, indices.cndx);
    ACCEL_CFG_READ32(xts_error_idx, indices.endx);

    (*cb_func)(usr_ctx, indices);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_xts1_t::metrics_get(uint32_t ring_handle,
                               uint32_t sub_ring,
                               accel_rgroup_ring_metrics_cb_t cb_func,
                               void *usr_ctx)
{
    accel_rgroup_ring_metrics_t metrics = {0};

    /*
     * XTS does not have bytes metrics
     */
    metrics.ring_handle = ring_handle;
    metrics.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    metrics.soft_resets = soft_resets;
    (*cb_func)(usr_ctx, metrics);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_xts1_t::misc_get(uint32_t ring_handle,
                            uint32_t sub_ring,
                            accel_rgroup_ring_misc_cb_t cb_func,
                            void *usr_ctx)
{
    accel_rgroup_ring_misc_t misc = {0};

    misc.ring_handle = ring_handle;
    misc.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_NAMED_READ32(xts_ring_base_w0, misc.reg_val[0]);
    ACCEL_CFG_NAMED_READ32(xts_ring_base_w1, misc.reg_val[1]);
    ACCEL_CFG_NAMED_READ32(xts_ring_size,    misc.reg_val[2]);
    ACCEL_CFG_NAMED_READ32(xts_ci_addr_w0,   misc.reg_val[3]);
    ACCEL_CFG_NAMED_READ32(xts_ci_addr_w1,   misc.reg_val[4]);
    ACCEL_CFG_NAMED_READ32(xts_status,       misc.reg_val[5]);
    ACCEL_CFG_NAMED_READ32(xts_error_idx,    misc.reg_val[6]);
    misc.num_reg_vals = 7;

    (*cb_func)(usr_ctx, misc);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_gcm0_t::reset_set(uint32_t ring_handle,
                             uint32_t sub_ring,
                             uint32_t *last_ring_handle,
                             uint32_t *last_sub_ring,
                             bool reset_sense)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    ACCEL_CFG_RMW32(gcm0_soft_rst, 0xffffffff, reset_sense);
    soft_resets += reset_sense ? 1 : 0;
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_gcm0_t::enable_set(uint32_t ring_handle,
                              uint32_t sub_ring,
                              uint32_t *last_ring_handle,
                              uint32_t *last_sub_ring,
                              bool enable_sense)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    // GCM0 does not implement ring_enable
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_gcm0_t::pndx_set(uint32_t ring_handle,
                            uint32_t sub_ring,
                            uint32_t *last_ring_handle,
                            uint32_t *last_sub_ring,
                            uint32_t val,
                            bool conditional)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    if (!conditional || accel_ring_sw_reset_capable(ring_type)) {
        ACCEL_CFG_WRITE32(gcm0_producer_idx, val);
        return accel_shadow_pndx_write(ring_type, val);
    }
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_gcm0_t::info_get(uint32_t ring_handle,
                            uint32_t sub_ring,
                            accel_rgroup_ring_info_cb_t cb_func,
                            void *usr_ctx)
{
    accel_rgroup_ring_info_t    info = {0};

    info.ring_handle = ring_handle;
    info.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    accel_ring_meta_config_get(ring_type, &info);
    capri_barco_get_capabilities((barco_rings_t)ring_type, &info.sw_reset_capable,
                                 &info.sw_enable_capable);
    assert(info.pndx_size);
    (*cb_func)(usr_ctx, info);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_gcm0_t::metrics_get(uint32_t ring_handle,
                               uint32_t sub_ring,
                               accel_rgroup_ring_metrics_cb_t cb_func,
                               void *usr_ctx)
{
    accel_rgroup_ring_metrics_t metrics = {0};

    /*
     * GCM does not have bytes metrics
     */
    metrics.ring_handle = ring_handle;
    metrics.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    metrics.soft_resets = soft_resets;
    (*cb_func)(usr_ctx, metrics);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_gcm0_t::indices_get(uint32_t ring_handle,
                               uint32_t sub_ring,
                               accel_rgroup_ring_indices_cb_t cb_func,
                               void *usr_ctx)
{
    accel_rgroup_ring_indices_t indices = {0};

    indices.ring_handle = ring_handle;
    indices.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_READ32(gcm0_producer_idx, indices.pndx);
    ACCEL_CFG_READ32(gcm0_consumer_idx, indices.cndx);
    ACCEL_CFG_READ32(gcm0_error_idx, indices.endx);

    (*cb_func)(usr_ctx, indices);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_gcm0_t::misc_get(uint32_t ring_handle,
                            uint32_t sub_ring,
                            accel_rgroup_ring_misc_cb_t cb_func,
                            void *usr_ctx)
{
    accel_rgroup_ring_misc_t misc = {0};

    misc.ring_handle = ring_handle;
    misc.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_NAMED_READ32(gcm0_ring_base_w0, misc.reg_val[0]);
    ACCEL_CFG_NAMED_READ32(gcm0_ring_base_w1, misc.reg_val[1]);
    ACCEL_CFG_NAMED_READ32(gcm0_ring_size,    misc.reg_val[2]);
    ACCEL_CFG_NAMED_READ32(gcm0_ci_addr_w0,   misc.reg_val[3]);
    ACCEL_CFG_NAMED_READ32(gcm0_ci_addr_w1,   misc.reg_val[4]);
    ACCEL_CFG_NAMED_READ32(gcm0_status,       misc.reg_val[5]);
    ACCEL_CFG_NAMED_READ32(gcm0_error_idx,    misc.reg_val[6]);
    misc.num_reg_vals = 7;

    (*cb_func)(usr_ctx, misc);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_gcm1_t::reset_set(uint32_t ring_handle,
                             uint32_t sub_ring,
                             uint32_t *last_ring_handle,
                             uint32_t *last_sub_ring,
                             bool reset_sense)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    ACCEL_CFG_RMW32(gcm1_soft_rst, 0xffffffff, reset_sense);
    soft_resets += reset_sense ? 1 : 0;
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_gcm1_t::enable_set(uint32_t ring_handle,
                              uint32_t sub_ring,
                              uint32_t *last_ring_handle,
                              uint32_t *last_sub_ring,
                              bool enable_sense)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    // GCM1 does not implement ring_enable
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_gcm1_t::pndx_set(uint32_t ring_handle,
                            uint32_t sub_ring,
                            uint32_t *last_ring_handle,
                            uint32_t *last_sub_ring,
                            uint32_t val,
                            bool conditional)
{
    *last_ring_handle = ring_handle;
    *last_sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);
    if (!conditional || accel_ring_sw_reset_capable(ring_type)) {
        ACCEL_CFG_WRITE32(gcm1_producer_idx, val);
        return accel_shadow_pndx_write(ring_type, val);
    }
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_gcm1_t::info_get(uint32_t ring_handle,
                            uint32_t sub_ring,
                            accel_rgroup_ring_info_cb_t cb_func,
                            void *usr_ctx)
{
    accel_rgroup_ring_info_t    info = {0};

    info.ring_handle = ring_handle;
    info.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    accel_ring_meta_config_get(ring_type, &info);
    capri_barco_get_capabilities((barco_rings_t)ring_type, &info.sw_reset_capable,
                                 &info.sw_enable_capable);
    assert(info.pndx_size);
    (*cb_func)(usr_ctx, info);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_gcm1_t::indices_get(uint32_t ring_handle,
                               uint32_t sub_ring,
                               accel_rgroup_ring_indices_cb_t cb_func,
                               void *usr_ctx)
{
    accel_rgroup_ring_indices_t indices = {0};

    indices.ring_handle = ring_handle;
    indices.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_READ32(gcm1_producer_idx, indices.pndx);
    ACCEL_CFG_READ32(gcm1_consumer_idx, indices.cndx);
    ACCEL_CFG_READ32(gcm1_error_idx, indices.endx);

    (*cb_func)(usr_ctx, indices);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_gcm1_t::metrics_get(uint32_t ring_handle,
                               uint32_t sub_ring,
                               accel_rgroup_ring_metrics_cb_t cb_func,
                               void *usr_ctx)
{
    accel_rgroup_ring_metrics_t metrics = {0};

    /*
     * GCM does not have bytes metrics
     */
    metrics.ring_handle = ring_handle;
    metrics.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    metrics.soft_resets = soft_resets;
    (*cb_func)(usr_ctx, metrics);
    return HAL_RET_OK;
}

hal_ret_t 
accel_ring_gcm1_t::misc_get(uint32_t ring_handle,
                            uint32_t sub_ring,
                            accel_rgroup_ring_misc_cb_t cb_func,
                            void *usr_ctx)
{
    accel_rgroup_ring_misc_t misc = {0};

    misc.ring_handle = ring_handle;
    misc.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_NAMED_READ32(gcm1_ring_base_w0, misc.reg_val[0]);
    ACCEL_CFG_NAMED_READ32(gcm1_ring_base_w1, misc.reg_val[1]);
    ACCEL_CFG_NAMED_READ32(gcm1_ring_size,    misc.reg_val[2]);
    ACCEL_CFG_NAMED_READ32(gcm1_ci_addr_w0,   misc.reg_val[3]);
    ACCEL_CFG_NAMED_READ32(gcm1_ci_addr_w1,   misc.reg_val[4]);
    ACCEL_CFG_NAMED_READ32(gcm1_status,       misc.reg_val[5]);
    ACCEL_CFG_NAMED_READ32(gcm1_error_idx,    misc.reg_val[6]);
    misc.num_reg_vals = 7;

    (*cb_func)(usr_ctx, misc);
    return HAL_RET_OK;
}

static void
rgroup_indices_get_cb(void *user_ctx,
                      const accel_rgroup_ring_indices_t& indices)
{
    accel_ring_metrics_t *fill_ctx;

    if ((indices.ring_handle < ACCEL_RING_ID_MAX) &&
        (indices.sub_ring < ACCEL_SUB_RING_MAX)) {

        fill_ctx = (accel_ring_metrics_t *)((char *)user_ctx +
            ACCEL_METRICS_MEM_OFFSET(indices.ring_handle, indices.sub_ring));
        fill_ctx->pndx = indices.pndx;
        fill_ctx->cndx = indices.cndx;
    }
}

static void
rgroup_metrics_get_cb(void *user_ctx,
                      const accel_rgroup_ring_metrics_t& metrics)
{
    accel_ring_metrics_t *fill_ctx;

    if ((metrics.ring_handle < ACCEL_RING_ID_MAX) &&
        (metrics.sub_ring < ACCEL_SUB_RING_MAX)) {

        fill_ctx = (accel_ring_metrics_t *)((char *)user_ctx +
            ACCEL_METRICS_MEM_OFFSET(metrics.ring_handle, metrics.sub_ring));
        fill_ctx->input_bytes  = metrics.input_bytes;
        fill_ctx->output_bytes = metrics.output_bytes;
        fill_ctx->soft_resets  = metrics.soft_resets;
    }
}

static void
accel_rgroup_timer(void *timer,
                   uint32_t timer_id,
                   void *user_ctx)
{
    accel_rgroup_t           *rgroup;
    accel_rgroup_iter_c      iter;
    accel_ring_metrics_t     metrics[ACCEL_RING_ID_MAX][ACCEL_SUB_RING_MAX];

    ACCEL_RGROUP_LOCK();
    iter = rgroup_map.begin();
    while (iter != rgroup_map.end()) {
        rgroup = iter->second;
        if (rgroup->metrics_size_get() >= sizeof(metrics)) {
            memset(metrics, 0, sizeof(metrics));
            rgroup->indices_get(ACCEL_SUB_RING_ALL,
                                rgroup_indices_get_cb, metrics);
            rgroup->metrics_get(ACCEL_SUB_RING_ALL,
                                rgroup_metrics_get_cb, metrics);
            ACCEL_METRICS_MEM_WRITE(rgroup->metrics_addr_get(),
                                    (uint8_t *)metrics, sizeof(metrics));
        }
        iter++;
    }
    ACCEL_RGROUP_UNLOCK();
}


}/// aameppac  pd

} // nmmesaacehhal
