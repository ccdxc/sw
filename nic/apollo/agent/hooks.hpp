// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_HOOKS_HPP__
#define __AGENT_HOOKS_HPP__
#include <dlfcn.h>
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"

namespace hooks {

typedef enum agent_op_e {
    LOCAL_MAPPING_CREATE = 0,
    REMOTE_MAPPING_CREATE,
    BATCH_START,
    INIT_DONE
} agent_op_t;

typedef sdk_ret_t (*hooks_func_t)(agent_op_t op, void *obj, void *arg);
extern hooks_func_t hooks_func;

static inline sdk_ret_t
local_mapping_create (pds_local_mapping_spec_t *spec)
{
    if (hooks_func) {
        return hooks_func(LOCAL_MAPPING_CREATE, (void *)spec, NULL);
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
remote_mapping_create (pds_remote_mapping_spec_t *spec)
{
    if (hooks_func) {
        return hooks_func(REMOTE_MAPPING_CREATE, (void *)spec, NULL);
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
batch_start (pds_epoch_t epoch)
{
    if (hooks_func) {
        return hooks_func(BATCH_START, (void *)&epoch, NULL);
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
load (void)
{
    void *so, *func;
    char *sofile;

    sofile = std::getenv("AGENT_TEST_HOOKS_LIB");
    if (sofile == NULL) {
        return SDK_RET_OK;
    }
    so = dlopen(sofile, RTLD_NOW|RTLD_GLOBAL|RTLD_NOLOAD);
    if (so == NULL) {
        so = dlopen(sofile, RTLD_NOW|RTLD_GLOBAL);
    }
    if (so == NULL) {
        return SDK_RET_ERR;
    }
    func = dlsym(so, "g_agent_hooks_func");
    if (func) {
        hooks_func = (hooks_func_t)func;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
agent_init_done (const char *pipeline)
{
    sdk_ret_t ret;

    ret = load();
    if (ret != SDK_RET_OK) {
        return ret;
    }
    if (hooks_func) {
        return hooks_func(INIT_DONE, (void *)pipeline, NULL);
    }
    return ret;
}

}    // namespace hooks

#endif    // __AGENT_HOOKS_HPP__
