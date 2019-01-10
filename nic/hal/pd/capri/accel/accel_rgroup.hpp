// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __PD_ACCEL_RGROUP_HPP__
#define __PD_ACCEL_RGROUP_HPP__

/*
 * Operations on user selected group of Accelerator rings and sub-rings.
 */

#include <map>
#include <unordered_map>
#include "nic/include/accel_ring.h"
#include "gen/proto/types.pb.h"
#include "nic/hal/plugins/cfg/accel/accel_rgroup.hpp"

namespace hal {
namespace pd {

#define ACCEL_RGROUP_NO_SUPP        {return HAL_RET_NOT_SUPPORTED;}
#define ACCEL_RGROUP_SUPP

#define ACCEL_RING_OPS_METHOD(SUPPORT)                                      \
    virtual hal_ret_t reset_set(uint32_t ring_handle,                       \
                                uint32_t sub_ring,                          \
                                uint32_t *last_ring_handle,                 \
                                uint32_t *last_sub_ring,                    \
                                bool reset_sense) SUPPORT;                  \
    virtual hal_ret_t enable_set(uint32_t ring_handle,                      \
                                 uint32_t sub_ring,                         \
                                 uint32_t *last_ring_handle,                \
                                 uint32_t *last_sub_ring,                   \
                                 bool enable_sense) SUPPORT;                \
    virtual hal_ret_t pndx_set(uint32_t ring_handle,                        \
                               uint32_t sub_ring,                           \
                               uint32_t *last_ring_handle,                  \
                               uint32_t *last_sub_ring,                     \
                               uint32_t val,                                \
                               bool conditional) SUPPORT;                   \
    virtual hal_ret_t info_get(uint32_t ring_handle,                        \
                               uint32_t sub_ring,                           \
                               accel_rgroup_ring_info_cb_t cb_func,         \
                               void *usr_ctx) SUPPORT;                      \
    virtual hal_ret_t indices_get(uint32_t ring_handle,                     \
                                  uint32_t sub_ring,                        \
                                  accel_rgroup_ring_indices_cb_t cb_func,   \
                                  void *usr_ctx) SUPPORT;                   \
    virtual hal_ret_t metrics_get(uint32_t ring_handle,                     \
                                  uint32_t sub_ring,                        \
                                  accel_rgroup_ring_metrics_cb_t cb_func,   \
                                  void *usr_ctx) SUPPORT;                   \

/*
 * Ring operations base class
 */
class accel_ring_ops_t
{
public:
    accel_ring_ops_t(types::BarcoRings ring_type) :
        ring_type(ring_type), soft_resets(0) {}
    ACCEL_RING_OPS_METHOD(ACCEL_RGROUP_NO_SUPP);

protected:
    types::BarcoRings       ring_type;
    uint64_t                soft_resets;
};

/*
 * Ring operations derived classes
 */
class accel_ring_cp_t : public accel_ring_ops_t
{
public:
    accel_ring_cp_t(types::BarcoRings ring_type) : accel_ring_ops_t(ring_type) {}
    ACCEL_RING_OPS_METHOD(ACCEL_RGROUP_SUPP);
};

class accel_ring_cp_hot_t : public accel_ring_ops_t
{
public:
    accel_ring_cp_hot_t(types::BarcoRings ring_type) : accel_ring_ops_t(ring_type) {}
    ACCEL_RING_OPS_METHOD(ACCEL_RGROUP_SUPP);
};

class accel_ring_dc_t : public accel_ring_ops_t
{
public:
    accel_ring_dc_t(types::BarcoRings ring_type) : accel_ring_ops_t(ring_type) {}
    ACCEL_RING_OPS_METHOD(ACCEL_RGROUP_SUPP);
};

class accel_ring_dc_hot_t : public accel_ring_ops_t
{
public:
    accel_ring_dc_hot_t(types::BarcoRings ring_type) : accel_ring_ops_t(ring_type) {}
    ACCEL_RING_OPS_METHOD(ACCEL_RGROUP_SUPP);
};

class accel_ring_xts0_t : public accel_ring_ops_t
{
public:
    accel_ring_xts0_t(types::BarcoRings ring_type) : accel_ring_ops_t(ring_type) {}
    ACCEL_RING_OPS_METHOD(ACCEL_RGROUP_SUPP);
};

class accel_ring_xts1_t : public accel_ring_ops_t
{
public:
    accel_ring_xts1_t(types::BarcoRings ring_type) : accel_ring_ops_t(ring_type) {}
    ACCEL_RING_OPS_METHOD(ACCEL_RGROUP_SUPP);
};

class accel_ring_gcm0_t : public accel_ring_ops_t
{
public:
    accel_ring_gcm0_t(types::BarcoRings ring_type) : accel_ring_ops_t(ring_type) {}
    ACCEL_RING_OPS_METHOD(ACCEL_RGROUP_SUPP);
};

class accel_ring_gcm1_t : public accel_ring_ops_t
{
public:
    accel_ring_gcm1_t(types::BarcoRings ring_type) : accel_ring_ops_t(ring_type) {}
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
    ~accel_rgroup_t()
    {
        elem_map.clear();
    }
    hal_ret_t ring_add(const std::string& ring_name,
                       uint32_t ring_handle);
    hal_ret_t ring_del(const std::string& ring_name);
    hal_ret_t reset_set(uint32_t sub_ring,
                        uint32_t *last_ring_handle,
                        uint32_t *last_sub_ring,
                        bool reset_sense);
    hal_ret_t enable_set(uint32_t sub_ring,
                         uint32_t *last_ring_handle,
                         uint32_t *last_sub_ring,
                         bool enable_sense);
    hal_ret_t pndx_set(uint32_t sub_ring,
                       uint32_t *last_ring_handle,
                       uint32_t *last_sub_ring,
                       uint32_t val,
                       bool conditional);
    hal_ret_t info_get(uint32_t sub_ring,
                       accel_rgroup_ring_info_cb_t cb_func,
                       void *user_ctx);
    hal_ret_t indices_get(uint32_t sub_ring,
                          accel_rgroup_ring_indices_cb_t cb_func,
                          void *user_ctx);
    hal_ret_t metrics_get(uint32_t sub_ring,
                          accel_rgroup_ring_metrics_cb_t cb_func,
                          void *user_ctx);

private:
    accel_rgroup_elem_map_t     elem_map;
};

/*
 * Map of rgroup name to ring group
 */
typedef std::unordered_map<std::string, accel_rgroup_t*>    accel_rgroup_map_t;
typedef accel_rgroup_map_t::iterator                        accel_rgroup_iter_t;
typedef accel_rgroup_map_t::const_iterator                  accel_rgroup_iter_c;


hal_ret_t accel_rgroup_add(const char *rgroup_name);
hal_ret_t accel_rgroup_del(const char *rgroup_name);
hal_ret_t accel_rgroup_ring_add(const char *rgroup_name,
                                const char *ring_name,
                                uint32_t ring_handle);
hal_ret_t accel_rgroup_ring_del(const char *rgroup_name,
                                const char *ring_name);
hal_ret_t accel_rgroup_reset_set(const char *rgroup_name,
                                 uint32_t sub_ring,
                                 uint32_t *last_ring_handle,
                                 uint32_t *last_sub_ring,
                                 bool reset_sense);
hal_ret_t accel_rgroup_enable_set(const char *rgroup_name,
                                  uint32_t sub_ring,
                                  uint32_t *last_ring_handle,
                                  uint32_t *last_sub_ring,
                                  bool enable_sense);
hal_ret_t accel_rgroup_pndx_set(const char *rgroup_name,
                                uint32_t sub_ring,
                                uint32_t *last_ring_handle,
                                uint32_t *last_sub_ring,
                                uint32_t val,
                                bool conditional);
hal_ret_t accel_rgroup_info_get(const char *rgroup_name,
                                uint32_t sub_ring,
                                accel_rgroup_ring_info_cb_t cb_func,
                                void *user_ctx);
hal_ret_t accel_rgroup_indices_get(const char *rgroup_name,
                                   uint32_t sub_ring,
                                   accel_rgroup_ring_indices_cb_t cb_func,
                                   void *user_ctx);
hal_ret_t accel_rgroup_metrics_get(const char *rgroup_name,
                                   uint32_t sub_ring,
                                   accel_rgroup_ring_metrics_cb_t cb_func,
                                   void *user_ctx);

}    // namespace pd
}    // namespace hal

#endif /*  __PD_ACCEL_RGROUP_HPP__ */
