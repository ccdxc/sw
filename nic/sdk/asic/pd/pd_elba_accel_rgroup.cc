// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#include <map>
#include <unordered_map>
#include "include/sdk/lock.hpp"
#include "lib/periodic/periodic.hpp"
#include "platform/elba/elba_barco_rings.hpp"
#include "asic/pd/pd.hpp"
#include "asic/pd/pd_accel_rgroup.hpp"
#include "asic/rw/asicrw.hpp"
#include "third-party/asic/elba/model/elb_top/elb_top_csr.h"

/*
 * Operations on user selected group of Accelerator rings and sub-rings.
 */

using namespace sdk::asic::pd;
using namespace sdk::platform::elba;

namespace sdk {
namespace asic {
namespace pd {
namespace accel {

#if 0 /* TBD-ELBA-REBASE: */
/*
 * Write a config 32-bit register
 */
#define ACCEL_CFG_WRITE32(cfg, val)                                         \
    do {                                                                    \
        elb_hens_csr_dhs_crypto_ctl_t&  ctl = accel_ctl();                  \
        ctl.cfg.fld(val);                                                   \
        ctl.cfg.write();                                                    \
    } while (false)

/*
 * Read a config 32-bit register
 */
#define ACCEL_CFG_READ32(cfg, val)                                          \
    do {                                                                    \
        elb_hens_csr_dhs_crypto_ctl_t&  ctl = accel_ctl();                  \
        ctl.cfg.read();                                                     \
        val = ctl.cfg.fld().convert_to<uint32_t>();                         \
    } while (false)


/*
 * Read a config 64-bit register
 */
#define ACCEL_CFG_READ64(cfg_lo, cfg_hi, val)                               \
    do {                                                                    \
        elb_hens_csr_dhs_crypto_ctl_t&  ctl = accel_ctl();                  \
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
        elb_hens_csr_dhs_crypto_ctl_t&  ctl = accel_ctl();                  \
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
        elb_hens_csr_dhs_crypto_ctl_t&  ctl = accel_ctl();                  \
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
            return SDK_RET_INVALID_ARG;                                     \
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
            if (ret_val != SDK_RET_OK) break;                               \
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
            if (ret_val != SDK_RET_OK) break;                               \
            iter++;                                                         \
        }                                                                   \
    } while (false)

/*
 * Get handle to Accelerator control block
 */
static inline elb_hens_csr_dhs_crypto_ctl_t&
accel_ctl(void)
{
    elb_top_csr_t&  elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    return elb0.md.hens.dhs_crypto_ctl;
}

/*
 * Get sw_reset capability
 */
static inline bool
accel_ring_sw_reset_capable(uint8_t ring_type)
{
    bool    sw_reset_capable;
    bool    sw_enable_capable;

    asicpd_barco_get_capabilities((barco_rings_t) ring_type,
                                  &sw_reset_capable,
                                  &sw_enable_capable);
    return sw_reset_capable;
}

/*
 * Write to shadow pndx
 */
static inline sdk_ret_t
accel_shadow_pndx_write(uint8_t ring_type,
                        uint32_t val)
{
    uint64_t    shadow_pndx_pa;
    uint32_t    shadow_pndx_size;
    uint32_t    desc_size;
    uint32_t    opaque_tag_size;

    elba_barco_get_meta_config_info((barco_rings_t) ring_type,
                                    &shadow_pndx_pa,
                                    &shadow_pndx_size,
                                    &desc_size,
                                    &opaque_tag_size);
    if (shadow_pndx_size) {
        assert(shadow_pndx_size <= sizeof(val));
        if (sdk::asic::asic_mem_write(shadow_pndx_pa, (uint8_t *)&val,
                                shadow_pndx_size)) {
            SDK_TRACE_ERR("Failed to write shadow pndx @ {:x} size {}",
                          shadow_pndx_pa, shadow_pndx_size);
            return SDK_RET_HW_PROGRAM_ERR; /* TBD-REBASE: SDK_RET_HW_FAIL */
        }
    }
    return SDK_RET_OK;
}

static accel_ring_cp_t          ring_cp(BARCO_RING_CP);
static accel_ring_cp_hot_t      ring_cp_hot(BARCO_RING_CP_HOT);
static accel_ring_dc_t          ring_dc(BARCO_RING_DC);
static accel_ring_dc_hot_t      ring_dc_hot(BARCO_RING_DC_HOT);
static accel_ring_xts0_t        ring_xts0(BARCO_RING_XTS0);
static accel_ring_xts1_t        ring_xts1(BARCO_RING_XTS1);
static accel_ring_gcm0_t        ring_gcm0(BARCO_RING_GCM0);
static accel_ring_gcm1_t        ring_gcm1(BARCO_RING_GCM1);


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

// TBD: Function isn't implemented in Elba, returning OK
sdk_ret_t
asicpd_accel_rgroup_init (int tid)
{
    return SDK_RET_OK;
}

// TBD: Function isn't implemented in Elba, returning OK
sdk_ret_t
asicpd_accel_rgroup_fini (int tid)
{
    return SDK_RET_OK;
}

/*
 * Add a new ring group.
 */
sdk_ret_t
asicpd_accel_rgroup_add(const char *rgroup_name,
                 uint64_t metrics_mem_addr,
                 uint64_t metrics_mem_mem_size
                 )
{
    accel_rgroup_t  *rgroup;

    if (accel_rgroup_find(rgroup_name)) {
        return SDK_RET_ENTRY_EXISTS;
    }

    rgroup = new (std::nothrow) accel_rgroup_t();
    if (!rgroup) {
        SDK_TRACE_ERR("Failed to allocate rgroup {}", rgroup_name);
        return SDK_RET_OOM;
    }

    rgroup_map.insert(std::make_pair(std::string(rgroup_name), rgroup));
    return SDK_RET_OK;
}

/*
 * Delete a ring group.
 */
sdk_ret_t
asicpd_accel_rgroup_del(const char *rgroup_name)
{
    accel_rgroup_t  *rgroup;

    rgroup = accel_rgroup_find(rgroup_name);
    if (rgroup) {
        rgroup_map.erase(std::string(rgroup_name));
        delete rgroup;
    }
    return SDK_RET_OK;
}

/*
 * Add a ring by name to a ring group.
 */
sdk_ret_t
asicpd_accel_rgroup_ring_add(const char *rgroup_name,
                      const char *ring_name,
                      uint32_t ring_handle)
{
    accel_rgroup_t  *rgroup;
    sdk_ret_t       ret_val = SDK_RET_ENTRY_NOT_FOUND;

    rgroup = accel_rgroup_find(rgroup_name);
    if (rgroup) {
        ret_val = rgroup->ring_add(std::string(ring_name), ring_handle);
    }

    return ret_val;
}

/*
 * Remove a ring by name from a ring group.
 */
sdk_ret_t
accel_rgroup_ring_del(const char *rgroup_name,
                      const char *ring_name)
{
    accel_rgroup_t  *rgroup;

    rgroup = accel_rgroup_find(rgroup_name);
    if (rgroup) {
        rgroup->ring_del(std::string(ring_name));
    }
    return SDK_RET_OK;
}

/*
 * Invoke ring_reset on all rings in a group, taking the argument sub_ring
 * into considerations.
 */
sdk_ret_t
asicpd_accel_rgroup_reset_set(const char *rgroup_name,
                       uint32_t sub_ring,
                       uint32_t *last_ring_handle,
                       uint32_t *last_sub_ring,
                       bool reset_sense)
{
    accel_rgroup_t  *rgroup;
    sdk_ret_t       ret_val = SDK_RET_ENTRY_NOT_FOUND;

    rgroup = accel_rgroup_find(rgroup_name);
    if (rgroup) {
        ret_val = rgroup->reset_set(sub_ring, last_ring_handle,
                                    last_sub_ring, reset_sense);
    }

    return ret_val;
}

/*
 * Invoke ring_enable on all rings in a group, taking the argument sub_ring
 * into considerations.
 */
sdk_ret_t
asicpd_accel_rgroup_enable_set(const char *rgroup_name,
                        uint32_t sub_ring,
                        uint32_t *last_ring_handle,
                        uint32_t *last_sub_ring,
                        bool enable_sense)
{
    accel_rgroup_t  *rgroup;
    sdk_ret_t       ret_val = SDK_RET_ENTRY_NOT_FOUND;

    rgroup = accel_rgroup_find(rgroup_name);
    if (rgroup) {
        ret_val = rgroup->enable_set(sub_ring, last_ring_handle,
                                     last_sub_ring, enable_sense);
    }

    return ret_val;
}

/*
 * Invoke ring_pndx_set on all rings in a group, taking the argument sub_ring
 * into considerations.
 */
sdk_ret_t
asicpd_accel_rgroup_pndx_set(const char *rgroup_name,
                      uint32_t sub_ring,
                      uint32_t *last_ring_handle,
                      uint32_t *last_sub_ring,
                      uint32_t val,
                      bool conditional)
{
    accel_rgroup_t  *rgroup;
    sdk_ret_t       ret_val = SDK_RET_ENTRY_NOT_FOUND;

    rgroup = accel_rgroup_find(rgroup_name);
    if (rgroup) {
        ret_val = rgroup->pndx_set(sub_ring, last_ring_handle,
                                   last_sub_ring, val, conditional);
    }

    return ret_val;
}

/*
 * Retrieve config info for all rings in a group.
 */
sdk_ret_t
asicpd_accel_rgroup_info_get(const char *rgroup_name,
                      uint32_t sub_ring,
                      accel_rgroup_ring_info_cb_t cb_func,
                      void *user_ctx)
{
    accel_rgroup_t  *rgroup;
    sdk_ret_t       ret_val = SDK_RET_ENTRY_NOT_FOUND;

    rgroup = accel_rgroup_find(rgroup_name);
    if (rgroup) {
        ret_val = rgroup->info_get(sub_ring, cb_func, user_ctx);
    }

    return ret_val;
}

/*
 * Retrieve indices for all rings in a group.
 */
sdk_ret_t
asicpd_accel_rgroup_indices_get(const char *rgroup_name,
                         uint32_t sub_ring,
                         accel_rgroup_ring_indices_cb_t cb_func,
                         void *user_ctx)
{
    accel_rgroup_t  *rgroup;
    sdk_ret_t       ret_val = SDK_RET_ENTRY_NOT_FOUND;

    rgroup = accel_rgroup_find(rgroup_name);
    if (rgroup) {
        ret_val = rgroup->indices_get(sub_ring, cb_func, user_ctx);
    }

    return ret_val;
}

/*
 * Retrieve metrics for all rings in a group.
 */
sdk_ret_t
asicpd_accel_rgroup_metrics_get(const char *rgroup_name,
                         uint32_t sub_ring,
                         accel_rgroup_ring_metrics_cb_t cb_func,
                         void *user_ctx)
{
    accel_rgroup_t  *rgroup;
    sdk_ret_t       ret_val = SDK_RET_ENTRY_NOT_FOUND;

    rgroup = accel_rgroup_find(rgroup_name);
    if (rgroup) {
        ret_val = rgroup->metrics_get(sub_ring, cb_func, user_ctx);
    }

    return ret_val;
}

/*
 * Add a ring by name to a ring group. The ring must have
 * supported ring_ops.
 */
sdk_ret_t
accel_rgroup_t::ring_add(const std::string& ring_name,
                         uint32_t ring_handle)
{
    accel_ring_ops_t            *ring_ops;
    accel_rgroup_elem_iter_c    elem_iter;

    ring_ops = accel_ring_ops_find(ring_name);
    if (!ring_ops) {
        return SDK_RET_INVALID_ARG;
    }

    elem_iter = elem_map.find(ring_name);
    if (elem_iter != elem_map.end()) {
        return SDK_RET_ENTRY_EXISTS;
    }

    elem_map.insert(std::make_pair(ring_name,
                                   std::make_pair(ring_ops, ring_handle)));
    return SDK_RET_OK;
}

/*
 * Remove a ring by name from a ring group.
 */
sdk_ret_t
accel_rgroup_t::ring_del(const std::string& ring_name)
{
    accel_rgroup_elem_iter_c    elem_iter;

    elem_iter = elem_map.find(ring_name);
    if (elem_iter == elem_map.end()) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    elem_map.erase(ring_name);
    return SDK_RET_OK;
}

/*
 * Invoke ring_reset on all rings in a group, taking the argument sub_ring
 * into considerations.
 */
sdk_ret_t
accel_rgroup_t::reset_set(uint32_t sub_ring,
                          uint32_t *last_ring_handle,
                          uint32_t *last_sub_ring,
                          bool reset_sense)
{
    sdk_ret_t   ret_val = SDK_RET_OK;
    FOR_EACH_RGROUP_RING_INVOKE_RET_LAST(reset_set, sub_ring, ret_val,
                                         last_ring_handle, last_sub_ring,
                                         reset_sense);
    return ret_val;
}

/*
 * Invoke ring_enable on all rings in a group, taking the argument sub_ring
 * into considerations.
 */
sdk_ret_t
accel_rgroup_t::enable_set(uint32_t sub_ring,
                           uint32_t *last_ring_handle,
                           uint32_t *last_sub_ring,
                           bool enable_sense)
{
    sdk_ret_t   ret_val = SDK_RET_OK;
    FOR_EACH_RGROUP_RING_INVOKE_RET_LAST(enable_set, sub_ring, ret_val,
                                         last_ring_handle, last_sub_ring,
                                         enable_sense);
    return ret_val;
}

/*
 * Invoke ring_pndx_set on all rings in a group, taking the argument sub_ring
 * into considerations.
 */
sdk_ret_t
accel_rgroup_t::pndx_set(uint32_t sub_ring,
                         uint32_t *last_ring_handle,
                         uint32_t *last_sub_ring,
                         uint32_t val,
                         bool conditional)
{
    sdk_ret_t   ret_val = SDK_RET_OK;
    FOR_EACH_RGROUP_RING_INVOKE_RET_LAST(pndx_set, sub_ring, ret_val,
                                         last_ring_handle, last_sub_ring, val,
                                         conditional);
    return ret_val;
}

/*
 * Retrieve config info on all rings in a group.
 */
sdk_ret_t
accel_rgroup_t::info_get(uint32_t sub_ring,
                         accel_rgroup_ring_info_cb_t cb_func,
                         void *user_ctx)
{
    sdk_ret_t   ret_val = SDK_RET_OK;
    FOR_EACH_RGROUP_RING_INVOKE(info_get, sub_ring, ret_val,
                                cb_func, user_ctx);
    return ret_val;
}

/*
 * Retrieve current ring indices on all rings in a group.
 */
sdk_ret_t
accel_rgroup_t::indices_get(uint32_t sub_ring,
                            accel_rgroup_ring_indices_cb_t cb_func,
                            void *user_ctx)
{
    sdk_ret_t   ret_val = SDK_RET_OK;
    FOR_EACH_RGROUP_RING_INVOKE(indices_get, sub_ring, ret_val,
                                cb_func, user_ctx);
    return ret_val;
}

/*
 * Retrieve current ring metrics on all rings in a group.
 */
sdk_ret_t
accel_rgroup_t::metrics_get(uint32_t sub_ring,
                            accel_rgroup_ring_metrics_cb_t cb_func,
                            void *user_ctx)
{
    sdk_ret_t   ret_val = SDK_RET_OK;
    FOR_EACH_RGROUP_RING_INVOKE(metrics_get, sub_ring, ret_val,
                                cb_func, user_ctx);
    return ret_val;
}

/*
 * All supported ring_ops methods begin here
 */
sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
accel_ring_cp_t::info_get(uint32_t ring_handle,
                          uint32_t sub_ring,
                          accel_rgroup_ring_info_cb_t cb_func,
                          void *usr_ctx)
{
    accel_rgroup_ring_info_t    info = {0};

    info.ring_handle = ring_handle;
    info.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_READ64(cp_cfg_q_base_adr_w0, cp_cfg_q_base_adr_w1, info.base_pa);
    ACCEL_CFG_READ64(cp_cfg_host_opaque_tag_adr_w0, cp_cfg_host_opaque_tag_adr_w1,
                      info.opaque_tag_pa);
    ACCEL_CFG_OFFSET(cp_cfg_q_pd_idx, info.pndx_pa);
    elba_barco_get_meta_config_info((barco_rings_t) ring_type,
                                    &info.shadow_pndx_pa,
                                    &info.pndx_size,
                                    &info.desc_size,
                                    &info.opaque_tag_size);

    asicpd_barco_get_capabilities((barco_rings_t) ring_type,
                                  &info.sw_reset_capable,
                                  &info.sw_enable_capable);
    assert(info.pndx_size);
    CPDC_RING_SIZE_READ32(cp_cfg_dist, info.ring_size,
                          BARCO_CRYPTO_CP_DIST_DESC_Q_SIZE_SHFT,
                          BARCO_CRYPTO_CP_DIST_DESC_Q_SIZE_MASK);
    (*cb_func)(usr_ctx, info);
    return SDK_RET_OK;
}

sdk_ret_t
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

    (*cb_func)(usr_ctx, indices);
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
accel_ring_cp_hot_t::info_get(uint32_t ring_handle,
                              uint32_t sub_ring,
                              accel_rgroup_ring_info_cb_t cb_func,
                              void *usr_ctx)
{
    accel_rgroup_ring_info_t    info = {0};

    info.ring_handle = ring_handle;
    info.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_READ64(cp_cfg_hotq_base_adr_w0, cp_cfg_hotq_base_adr_w1,
                     info.base_pa);
    ACCEL_CFG_OFFSET(cp_cfg_hotq_pd_idx, info.pndx_pa);
    elba_barco_get_meta_config_info((barco_rings_t) ring_type,
                                    &info.shadow_pndx_pa,
                                    &info.pndx_size,
                                    &info.desc_size,
                                    &info.opaque_tag_size);
    asicpd_barco_get_capabilities((barco_rings_t) ring_type,
                                  &info.sw_reset_capable,
                                  &info.sw_enable_capable);
    assert(info.pndx_size);
    CPDC_RING_SIZE_READ32(cp_cfg_dist, info.ring_size,
                          BARCO_CRYPTO_CP_DIST_DESC_HOTQ_SIZE_SHFT,
                          BARCO_CRYPTO_CP_DIST_DESC_HOTQ_SIZE_MASK);
    (*cb_func)(usr_ctx, info);
    return SDK_RET_OK;
}

sdk_ret_t
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

    (*cb_func)(usr_ctx, indices);
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
accel_ring_dc_t::info_get(uint32_t ring_handle,
                          uint32_t sub_ring,
                          accel_rgroup_ring_info_cb_t cb_func,
                          void *usr_ctx)
{
    accel_rgroup_ring_info_t    info = {0};

    info.ring_handle = ring_handle;
    info.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_READ64(dc_cfg_q_base_adr_w0, dc_cfg_q_base_adr_w1, info.base_pa);
    ACCEL_CFG_READ64(dc_cfg_host_opaque_tag_adr_w0, dc_cfg_host_opaque_tag_adr_w1,
                     info.opaque_tag_pa);
    ACCEL_CFG_OFFSET(dc_cfg_q_pd_idx, info.pndx_pa);
    elba_barco_get_meta_config_info((barco_rings_t) ring_type,
                                    &info.shadow_pndx_pa,
                                    &info.pndx_size,
                                    &info.desc_size,
                                    &info.opaque_tag_size);
    elba_barco_get_capabilities((barco_rings_t) ring_type,
                                &info.sw_reset_capable,
                                &info.sw_enable_capable);
    assert(info.pndx_size);
    CPDC_RING_SIZE_READ32(dc_cfg_dist, info.ring_size,
                          BARCO_CRYPTO_DC_DIST_DESC_Q_SIZE_SHFT,
                          BARCO_CRYPTO_DC_DIST_DESC_Q_SIZE_MASK);
    (*cb_func)(usr_ctx, info);
    return SDK_RET_OK;
}

sdk_ret_t
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

    (*cb_func)(usr_ctx, indices);
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
accel_ring_dc_hot_t::info_get(uint32_t ring_handle,
                              uint32_t sub_ring,
                              accel_rgroup_ring_info_cb_t cb_func,
                              void *usr_ctx)
{
    accel_rgroup_ring_info_t    info = {0};

    info.ring_handle = ring_handle;
    info.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_READ64(dc_cfg_hotq_base_adr_w0, dc_cfg_hotq_base_adr_w1,
                     info.base_pa);
    ACCEL_CFG_OFFSET(dc_cfg_hotq_pd_idx, info.pndx_pa);
    elba_barco_get_meta_config_info((barco_rings_t)ring_type, &info.shadow_pndx_pa,
                                     &info.pndx_size, &info.desc_size,
                                     &info.opaque_tag_size);
    asicpd_barco_get_capabilities((barco_rings_t)ring_type, &info.sw_reset_capable,
                                 &info.sw_enable_capable);
    assert(info.pndx_size);
    CPDC_RING_SIZE_READ32(dc_cfg_dist, info.ring_size,
                          BARCO_CRYPTO_DC_DIST_DESC_HOTQ_SIZE_SHFT,
                          BARCO_CRYPTO_DC_DIST_DESC_HOTQ_SIZE_MASK);
    (*cb_func)(usr_ctx, info);
    return SDK_RET_OK;
}

sdk_ret_t
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

    (*cb_func)(usr_ctx, indices);
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
accel_ring_xts0_t::info_get(uint32_t ring_handle,
                            uint32_t sub_ring,
                            accel_rgroup_ring_info_cb_t cb_func,
                            void *usr_ctx)
{
    accel_rgroup_ring_info_t    info = {0};

    info.ring_handle = ring_handle;
    info.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_READ64(xts_enc_ring_base_w0, xts_enc_ring_base_w1, info.base_pa);
    ACCEL_CFG_READ64(xts_enc_opa_tag_addr_w0, xts_enc_opa_tag_addr_w1,
                     info.opaque_tag_pa);
    ACCEL_CFG_OFFSET(xts_enc_producer_idx, info.pndx_pa);
    elba_barco_get_meta_config_info((barco_rings_t)ring_type,
                                    &info.shadow_pndx_pa,
                                    &info.pndx_size,
                                    &info.desc_size,
                                    &info.opaque_tag_size);
    asicpd_barco_get_capabilities((barco_rings_t)ring_type,
                                  &info.sw_reset_capable,
                                  &info.sw_enable_capable);
    assert(info.pndx_size);
    ACCEL_CFG_READ32(xts_enc_ring_size, info.ring_size);

    (*cb_func)(usr_ctx, info);
    return SDK_RET_OK;
}

sdk_ret_t
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

    (*cb_func)(usr_ctx, indices);
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
accel_ring_xts1_t::info_get(uint32_t ring_handle,
                            uint32_t sub_ring,
                            accel_rgroup_ring_info_cb_t cb_func,
                            void *usr_ctx)
{
    accel_rgroup_ring_info_t    info = {0};

    info.ring_handle = ring_handle;
    info.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_READ64(xts_ring_base_w0, xts_ring_base_w1, info.base_pa);
    ACCEL_CFG_READ64(xts_opa_tag_addr_w0, xts_opa_tag_addr_w1,
                     info.opaque_tag_pa);
    ACCEL_CFG_OFFSET(xts_producer_idx, info.pndx_pa);
    elba_barco_get_meta_config_info((barco_rings_t)ring_type,
                                    &info.shadow_pndx_pa,
                                    &info.pndx_size,
                                    &info.desc_size,
                                    &info.opaque_tag_size);
    asicpd_barco_get_capabilities((barco_rings_t)ring_type,
                                  &info.sw_reset_capable,
                                  &info.sw_enable_capable);
    assert(info.pndx_size);
    ACCEL_CFG_READ32(xts_ring_size, info.ring_size);

    (*cb_func)(usr_ctx, info);
    return SDK_RET_OK;
}

sdk_ret_t
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

    (*cb_func)(usr_ctx, indices);
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
accel_ring_gcm0_t::info_get(uint32_t ring_handle,
                            uint32_t sub_ring,
                            accel_rgroup_ring_info_cb_t cb_func,
                            void *usr_ctx)
{
    accel_rgroup_ring_info_t    info = {0};

    info.ring_handle = ring_handle;
    info.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_READ64(gcm0_ring_base_w0, gcm0_ring_base_w1, info.base_pa);
    ACCEL_CFG_READ64(gcm0_opa_tag_addr_w0, gcm0_opa_tag_addr_w1,
                     info.opaque_tag_pa);
    ACCEL_CFG_OFFSET(gcm0_producer_idx, info.pndx_pa);
    elba_barco_get_meta_config_info((barco_rings_t)ring_type,
                                    &info.shadow_pndx_pa,
                                    &info.pndx_size,
                                    &info.desc_size,
                                    &info.opaque_tag_size);
    elba_barco_get_capabilities((barco_rings_t)ring_type,
                                &info.sw_reset_capable,
                                &info.sw_enable_capable);
    assert(info.pndx_size);
    ACCEL_CFG_READ32(gcm0_ring_size, info.ring_size);

    (*cb_func)(usr_ctx, info);
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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

    (*cb_func)(usr_ctx, indices);
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}

sdk_ret_t
accel_ring_gcm1_t::info_get(uint32_t ring_handle,
                            uint32_t sub_ring,
                            accel_rgroup_ring_info_cb_t cb_func,
                            void *usr_ctx)
{
    accel_rgroup_ring_info_t    info = {0};

    info.ring_handle = ring_handle;
    info.sub_ring = ACCEL_SUB_RING0;
    SUB_RING_VALIDATE_RETURN(sub_ring);

    ACCEL_CFG_READ64(gcm1_ring_base_w0, gcm1_ring_base_w1, info.base_pa);
    ACCEL_CFG_READ64(gcm1_opa_tag_addr_w0, gcm1_opa_tag_addr_w1,
                     info.opaque_tag_pa);
    ACCEL_CFG_OFFSET(gcm1_producer_idx, info.pndx_pa);
    elba_barco_get_meta_config_info((barco_rings_t) ring_type,
                                    &info.shadow_pndx_pa,
                                    &info.pndx_size,
                                    &info.desc_size,
                                    &info.opaque_tag_size);
    elba_barco_get_capabilities((barco_rings_t)ring_type,
                                &info.sw_reset_capable,
                                &info.sw_enable_capable);
    assert(info.pndx_size);
    ACCEL_CFG_READ32(gcm1_ring_size, info.ring_size);

    (*cb_func)(usr_ctx, info);
    return SDK_RET_OK;
}

sdk_ret_t
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

    (*cb_func)(usr_ctx, indices);
    return SDK_RET_OK;
}

sdk_ret_t
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
    return SDK_RET_OK;
}
#else
sdk_ret_t
asicpd_accel_rgroup_ring_add (const char *rgroup_name, const char *ring_name,
                              uint32_t ring_handle)
{
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_accel_rgroup_indices_get (const char *rgroup_name, uint32_t sub_ring,
                                 accel_rgroup_ring_indices_cb_t cb_func,
                                 void *user_ctx)
{
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_accel_rgroup_del (const char *rgroup_name)
{
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_accel_rgroup_add (const char *rgroup_name, uint64_t metrics_mem_addr,
                         uint32_t metrics_mem_size)
{
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_accel_rgroup_ring_del (const char *rgroup_name, const char *ring_name)
{
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_accel_rgroup_pndx_set (const char *rgroup_name, uint32_t sub_ring,
                              uint32_t *last_ring_handle,
                              uint32_t *last_sub_ring, uint32_t val,
                              bool conditional)
{
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_accel_rgroup_misc_get (const char *rgroup_name, uint32_t sub_ring,
                              accel_rgroup_ring_misc_cb_t cb_func,
                              void *user_ctx)
{
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_accel_rgroup_fini (int tid)
{
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_accel_rgroup_info_get (const char *rgroup_name, uint32_t sub_ring,
                              accel_rgroup_ring_info_cb_t cb_func,
                              void *user_ctx)
{
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_accel_rgroup_metrics_get (const char *rgroup_name, uint32_t sub_ring,
                                 accel_rgroup_ring_metrics_cb_t cb_func,
                                 void *user_ctx)
{
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_accel_rgroup_enable_set (const char *rgroup_name, uint32_t sub_ring,
                                uint32_t *last_ring_handle,
                                uint32_t *last_sub_ring, bool enable_sense)
{
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_accel_rgroup_reset_set (const char *rgroup_name, uint32_t sub_ring,
                               uint32_t *last_ring_handle,
                               uint32_t *last_sub_ring, bool reset_sense)
{
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_accel_rgroup_init (int tid, uint32_t accel_total_rings)
{
    return SDK_RET_OK;
}
#endif

}   // namespace accel
}   // namespace pd
}   // namespace asic
}   // namespace sdk
