// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __PD_ACCEL_RGROUP_HPP__
#define __PD_ACCEL_RGROUP_HPP__

/*
 * Operations on user selected group of Accelerator rings and sub-rings.
 */

#include <map>
#include <unordered_map>
#include "include/sdk/accel_metrics.h"

namespace sdk {
namespace asic {
namespace pd {
namespace accel {

#define ACCEL_RGROUP_NO_SUPP        {return SDK_RET_INVALID_OP;}
#define ACCEL_RGROUP_SUPP

#define ACCEL_RING_OPS_METHOD(SUPPORT)                                      \
    virtual sdk_ret_t reset_set(uint32_t ring_handle,                       \
                                uint32_t sub_ring,                          \
                                uint32_t *last_ring_handle,                 \
                                uint32_t *last_sub_ring,                    \
                                bool reset_sense) SUPPORT;                  \
    virtual sdk_ret_t enable_set(uint32_t ring_handle,                      \
                                 uint32_t sub_ring,                         \
                                 uint32_t *last_ring_handle,                \
                                 uint32_t *last_sub_ring,                   \
                                 bool enable_sense) SUPPORT;                \
    virtual sdk_ret_t pndx_set(uint32_t ring_handle,                        \
                               uint32_t sub_ring,                           \
                               uint32_t *last_ring_handle,                  \
                               uint32_t *last_sub_ring,                     \
                               uint32_t val,                                \
                               bool conditional) SUPPORT;                   \
    virtual sdk_ret_t info_get(uint32_t ring_handle,                        \
                               uint32_t sub_ring,                           \
                               accel_rgroup_ring_info_cb_t cb_func,         \
                               void *usr_ctx) SUPPORT;                      \
    virtual sdk_ret_t indices_get(uint32_t ring_handle,                     \
                                  uint32_t sub_ring,                        \
                                  accel_rgroup_ring_indices_cb_t cb_func,   \
                                  void *usr_ctx) SUPPORT;                   \
    virtual sdk_ret_t metrics_get(uint32_t ring_handle,                     \
                                  uint32_t sub_ring,                        \
                                  accel_rgroup_ring_metrics_cb_t cb_func,   \
                                  void *usr_ctx) SUPPORT;                   \
    virtual sdk_ret_t misc_get(uint32_t ring_handle,                        \
                               uint32_t sub_ring,                           \
                               accel_rgroup_ring_misc_cb_t cb_func,         \
                               void *usr_ctx) SUPPORT;                      \
/*
 * Ring config info
 */
typedef struct {
    uint32_t    ring_handle;
    uint32_t    sub_ring;
    uint64_t    base_pa;
    uint64_t    pndx_pa;
    uint64_t    shadow_pndx_pa;
    uint64_t    opaque_tag_pa;
    uint32_t    opaque_tag_size;
    uint32_t    ring_size;
    uint32_t    desc_size;
    uint32_t    pndx_size;
    bool        sw_reset_capable;
    bool        sw_enable_capable;
} accel_rgroup_ring_info_t;

typedef void (*accel_rgroup_ring_info_cb_t)(void *user_ctx,
                                            const accel_rgroup_ring_info_t& info);
/*
 * Ring indices info
 */
typedef struct {
    uint32_t    ring_handle;
    uint32_t    sub_ring;
    uint32_t    pndx;          /* producer index */
    uint32_t    cndx;          /* consumer index */
    uint32_t    endx;          /* error index */
} accel_rgroup_ring_indices_t;

typedef void (*accel_rgroup_ring_indices_cb_t)(void *user_ctx,
                                               const accel_rgroup_ring_indices_t& indices);

/*
 * Ring metrics info
 */
typedef struct {
    uint32_t    ring_handle;
    uint32_t    sub_ring;
    uint64_t    input_bytes;
    uint64_t    output_bytes;
    uint64_t    soft_resets;
} accel_rgroup_ring_metrics_t;

typedef void (*accel_rgroup_ring_metrics_cb_t)(void *user_ctx,
                                               const accel_rgroup_ring_metrics_t& metrics);
/*
 * Ring miscellaneous info, identified by resource name
 */
typedef struct {
    uint32_t    ring_handle;
    uint32_t    sub_ring;
    uint32_t    num_reg_vals;
    accel_ring_reg_val_t reg_val[ACCEL_RING_NUM_REGS_MAX];
} accel_rgroup_ring_misc_t;

typedef void (*accel_rgroup_ring_misc_cb_t)(void *user_ctx,
                                            const accel_rgroup_ring_misc_t& misc);

/*
 * Ring operations base class
 */
class accel_ring_ops_t
{
public:
    accel_ring_ops_t(barco_rings_t ring_type) :
        ring_type(ring_type), soft_resets(0) {}
    ACCEL_RING_OPS_METHOD(ACCEL_RGROUP_NO_SUPP);

protected:
    barco_rings_t ring_type;
    uint64_t soft_resets;
};

/*
 * Ring operations derived classes
 */
class accel_ring_cp_t : public accel_ring_ops_t
{
public:
    accel_ring_cp_t(barco_rings_t ring_type) : accel_ring_ops_t(ring_type) {}
    ACCEL_RING_OPS_METHOD(ACCEL_RGROUP_SUPP);
};

class accel_ring_cp_hot_t : public accel_ring_ops_t
{
public:
    accel_ring_cp_hot_t(barco_rings_t ring_type) : accel_ring_ops_t(ring_type) {}
    ACCEL_RING_OPS_METHOD(ACCEL_RGROUP_SUPP);
};

class accel_ring_dc_t : public accel_ring_ops_t
{
public:
    accel_ring_dc_t(barco_rings_t ring_type) : accel_ring_ops_t(ring_type) {}
    ACCEL_RING_OPS_METHOD(ACCEL_RGROUP_SUPP);
};

class accel_ring_dc_hot_t : public accel_ring_ops_t
{
public:
    accel_ring_dc_hot_t(barco_rings_t ring_type) : accel_ring_ops_t(ring_type) {}
    ACCEL_RING_OPS_METHOD(ACCEL_RGROUP_SUPP);
};

class accel_ring_xts0_t : public accel_ring_ops_t
{
public:
    accel_ring_xts0_t(barco_rings_t ring_type) : accel_ring_ops_t(ring_type) {}
    ACCEL_RING_OPS_METHOD(ACCEL_RGROUP_SUPP);
};

class accel_ring_xts1_t : public accel_ring_ops_t
{
public:
    accel_ring_xts1_t(barco_rings_t ring_type) : accel_ring_ops_t(ring_type) {}
    ACCEL_RING_OPS_METHOD(ACCEL_RGROUP_SUPP);
};

class accel_ring_gcm0_t : public accel_ring_ops_t
{
public:
    accel_ring_gcm0_t(barco_rings_t ring_type) : accel_ring_ops_t(ring_type) {}
    ACCEL_RING_OPS_METHOD(ACCEL_RGROUP_SUPP);
};

class accel_ring_gcm1_t : public accel_ring_ops_t
{
public:
    accel_ring_gcm1_t(barco_rings_t ring_type) : accel_ring_ops_t(ring_type) {}
    ACCEL_RING_OPS_METHOD(ACCEL_RGROUP_SUPP);
};

/*
 * Map of ring name to ring ops
 */
typedef std::unordered_map<std::string, accel_ring_ops_t*>  accel_ring_ops_map_t;
typedef accel_ring_ops_map_t::iterator                      accel_ring_ops_iter_t;
typedef accel_ring_ops_map_t::const_iterator                accel_ring_ops_iter_c;

/*
 * Ring ops and user handle association
 */
typedef std::pair<accel_ring_ops_t*, uint32_t>  accel_rgroup_elem_t;

/*
 * Map of ring name to accel_rgroup_elem_t
 */
typedef std::map<std::string, accel_rgroup_elem_t>          accel_rgroup_elem_map_t;
typedef accel_rgroup_elem_map_t::iterator                   accel_rgroup_elem_iter_t;
typedef accel_rgroup_elem_map_t::const_iterator             accel_rgroup_elem_iter_c;

/*
 * Ring group
 */
class accel_rgroup_t
{
public:
    accel_rgroup_t(uint64_t metrics_mem_addr, uint32_t metrics_mem_size) :
        metrics_mem_addr_(metrics_mem_addr), metrics_mem_size_(metrics_mem_size)
    {
    }

    ~accel_rgroup_t()
    {
        elem_map_.clear();
    }

    sdk_ret_t ring_add(const std::string& ring_name, uint32_t ring_handle);
    sdk_ret_t ring_del(const std::string& ring_name);

    sdk_ret_t reset_set(uint32_t sub_ring, uint32_t *last_ring_handle,
                        uint32_t *last_sub_ring, bool reset_sense);
    sdk_ret_t enable_set(uint32_t sub_ring, uint32_t *last_ring_handle,
                         uint32_t *last_sub_ring, bool enable_sense);
    sdk_ret_t pndx_set(uint32_t sub_ring, uint32_t *last_ring_handle,
                       uint32_t *last_sub_ring, uint32_t val, bool conditional);

    sdk_ret_t info_get(uint32_t sub_ring, accel_rgroup_ring_info_cb_t cb_func,
                       void *user_ctx);
    sdk_ret_t indices_get(uint32_t sub_ring,
                          accel_rgroup_ring_indices_cb_t cb_func,
                          void *user_ctx);
    sdk_ret_t metrics_get(uint32_t sub_ring,
                          accel_rgroup_ring_metrics_cb_t cb_func,
                          void *user_ctx);
    sdk_ret_t misc_get(uint32_t sub_ring, accel_rgroup_ring_misc_cb_t cb_func,
                       void *user_ctx);

    uint64_t  metrics_addr_get(void) { return metrics_mem_addr_; }
    uint64_t  metrics_size_get(void) { return metrics_mem_size_; }

private:
    accel_rgroup_elem_map_t elem_map_;
    uint64_t metrics_mem_addr_;
    uint32_t metrics_mem_size_;
};

/*
 * Map of rgroup name to ring group
 */
typedef std::unordered_map<std::string, accel_rgroup_t*>    accel_rgroup_map_t;
typedef accel_rgroup_map_t::iterator                        accel_rgroup_iter_t;
typedef accel_rgroup_map_t::const_iterator                  accel_rgroup_iter_c;

sdk_ret_t asicpd_accel_rgroup_init(int tid, uint32_t accel_total_rings);
sdk_ret_t asicpd_accel_rgroup_fini(int tid);

sdk_ret_t asicpd_accel_rgroup_add(const char *rgroup_name,
                                  uint64_t metrics_mem_addr,
                                  uint32_t metrics_mem_size);
sdk_ret_t asicpd_accel_rgroup_del(const char *rgroup_name);

sdk_ret_t asicpd_accel_rgroup_ring_add(const char *rgroup_name,
                                       const char *ring_name,
                                       uint32_t ring_handle);
sdk_ret_t asicpd_accel_rgroup_ring_del(const char *rgroup_name,
                                       const char *ring_name);

sdk_ret_t asicpd_accel_rgroup_reset_set(const char *rgroup_name,
                                        uint32_t sub_ring,
                                        uint32_t *last_ring_handle,
                                        uint32_t *last_sub_ring,
                                        bool reset_sense);
sdk_ret_t asicpd_accel_rgroup_enable_set(const char *rgroup_name,
                                         uint32_t sub_ring,
                                         uint32_t *last_ring_handle,
                                         uint32_t *last_sub_ring,
                                         bool enable_sense);
sdk_ret_t asicpd_accel_rgroup_pndx_set(const char *rgroup_name,
                                       uint32_t sub_ring,
                                       uint32_t *last_ring_handle,
                                       uint32_t *last_sub_ring,
                                       uint32_t val, bool conditional);

sdk_ret_t asicpd_accel_rgroup_info_get(const char *rgroup_name,
                                       uint32_t sub_ring,
                                       accel_rgroup_ring_info_cb_t cb_func,
                                       void *user_ctx);
sdk_ret_t asicpd_accel_rgroup_indices_get(const char *rgroup_name,
                                          uint32_t sub_ring,
                                          accel_rgroup_ring_indices_cb_t cb_func,
                                          void *user_ctx);
sdk_ret_t asicpd_accel_rgroup_metrics_get(const char *rgroup_name,
                                          uint32_t sub_ring,
                                          accel_rgroup_ring_metrics_cb_t cb_func,
                                          void *user_ctx);
sdk_ret_t asicpd_accel_rgroup_misc_get(const char *rgroup_name,
                                       uint32_t sub_ring,
                                       accel_rgroup_ring_misc_cb_t cb_func,
                                       void *user_ctx);


}    // namespace accel
}    // namespace pd
}    // namespace asic
}    // namespace sdk

#endif /*  __PD_ACCEL_RGROUP_HPP__ */
