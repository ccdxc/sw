// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_MODULE_HPP__
#define __HAL_MODULE_HPP__

#include "nic/include/base.h"

namespace hal {

// HAL module identifiers
typedef enum {
    HAL_MODULE_ID_NONE,
    HAL_MODULE_ID_MIN,
    HAL_MODULE_ID_NW            = HAL_MODULE_ID_MIN,
    HAL_MODULE_ID_MULTICAST,
    HAL_MODULE_ID_ACLQOS,
    HAL_MODULE_ID_FIREWALL,
    HAL_MODULE_ID_DOS,
    HAL_MODULE_ID_L4LB,
    HAL_MODULE_ID_TELEMETRY,
    HAL_MODULE_ID_STATS,
    HAL_MODULE_ID_FTE,
    HAL_MODULE_ID_MAX,
} hal_module_id_t;

// per module information that HAL infra needs to maintain
typedef hal_ret_t (*module_init_cb_t)(void);
typedef hal_ret_t (*module_cleanup_cb_t)(void);
class hal_module_meta {
public:
    hal_module_meta(module_init_cb_t init_cb, module_cleanup_cb_t cleanup_cb) {
        this->init_cb_ = init_cb;
        this->cleanup_cb_ = cleanup_cb;
    }
    ~hal_module_meta() {}
    module_init_cb_t init_cb(void) const { return init_cb_; }
    module_cleanup_cb_t cleanup_cb(void) const { return cleanup_cb_; }

private:
    module_init_cb_t    init_cb_;
    module_cleanup_cb_t cleanup_cb_;
};
hal_ret_t hal_module_init(void);
extern hal_module_meta *g_module_meta[HAL_MODULE_ID_MAX];

}    // namespace hal

#endif    // __HAL_MODULE_HPP__

