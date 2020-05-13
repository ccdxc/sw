//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements nicmgr shared memory state
///
//----------------------------------------------------------------------------

#include "nicmgr_shm.hpp"

#ifdef IRIS
#include "logger.hpp"
#define SHM_LOG_DEBUG               NIC_LOG_DEBUG
#define SHM_LOG_ERR                 NIC_LOG_ERR
#define FMT_U                       "{}"
#define FMT_D                       "{}"
#define FMT_S                       "{}"
#else
#include "nic/apollo/core/trace.hpp"
#define SHM_LOG_DEBUG               PDS_TRACE_DEBUG
#define SHM_LOG_ERR                 PDS_TRACE_ERR
#define FMT_U                       "%u"
#define FMT_D                       "%d"
#define FMT_S                       "%s"
#endif

#define NICMGR_SHM_BYTES            1024
#define NICMGR_SHM_NAME             "nicmgr_shm"
#define NICMGR_SHM_STATE_NAME       "nicmgr_shm_state"

using namespace sdk::lib;
using namespace sdk::asic;

/*
 * State stored per nicmgr device type
 */
typedef struct shm_devstate_s {
    uint32_t            cpp_pid;
    uint32_t            base_lif_id;
    bool                lif_fully_created;
} shm_devstate_t;

/*
 * Overall state
 */
typedef struct shm_state_s {
    shm_devstate_t  dev_state[DEVICETYPE_MAX];
} shm_state_t;

class nicmgr_shm_t
{
public:
    nicmgr_shm_t() :
        shm_mmgr(nullptr),
        shm_state(nullptr)
    {
    }
    ~nicmgr_shm_t()
    {
    }

    sdk_ret_t
    init(bool shm_create)
    {
        shm_mode_e mode = shm_create ? SHM_CREATE_ONLY : SHM_OPEN_ONLY;
        const char *op = shm_create ? "create" : "open";

        try {
            if (shm_create) {
                shmmgr::remove(NICMGR_SHM_NAME);
            }
            shm_mmgr = shmmgr::factory(NICMGR_SHM_NAME, NICMGR_SHM_BYTES,
                                       mode, NULL);
        } catch (...) {
            SHM_LOG_ERR("nicmgr_shm " FMT_S " failed", op);
            return SDK_RET_NO_RESOURCE;
        }

        shm_state = (shm_state_t *)
                    shm_mmgr->segment_find(NICMGR_SHM_STATE_NAME, shm_create,
                                           sizeof(*shm_state));
        if (!shm_state) {
            SHM_LOG_ERR("shm_state " FMT_S " failed", op);
            return SDK_RET_NO_RESOURCE;
        }
        if (shm_create) {
            memset(shm_state, 0, sizeof(*shm_state));
        }

        SHM_LOG_DEBUG("nicmgr_shm " FMT_S " completed", op);
        return SDK_RET_OK;
    }

    sdk_ret_t
    cpp_pid_set(enum DeviceType type,
                pid_t pid)
    {
        shm_devstate_t *devstate = devstate_get(type);
        if (devstate) {
            uint32_t pid32 = (uint32_t)pid;
            SDK_ATOMIC_STORE_UINT32(&devstate->cpp_pid, &pid32);
            return SDK_RET_OK;
        }
        return SDK_RET_ERR;
    }

    void
    cpp_pid_clr(enum DeviceType type)
    {
        shm_devstate_t *devstate = devstate_get(type);
        if (devstate) {
            uint32_t zero = 0;
            SDK_ATOMIC_STORE_UINT32(&devstate->cpp_pid, &zero);
        }
    }

    bool
    is_cpp_pid(enum DeviceType type)
    {
        shm_devstate_t *devstate = devstate_get(type);
        if (devstate) {
            uint32_t pid;
            SDK_ATOMIC_LOAD_UINT32(&devstate->cpp_pid, &pid);
            return getpid() == (pid_t)pid;
        }
        return false;
    }

    sdk_ret_t
    base_lif_id_set(enum DeviceType type,
                    uint32_t base_lif_id)
    {
        shm_devstate_t *devstate = devstate_get(type);
        if (devstate) {
            SDK_ATOMIC_STORE_UINT32(&devstate->base_lif_id, &base_lif_id);
            return SDK_RET_OK;
        }
        return SDK_RET_ERR;
    }

    void
    base_lif_id_clr(enum DeviceType type)
    {
        shm_devstate_t *devstate = devstate_get(type);
        if (devstate) {
            uint32_t zero = 0;
            SDK_ATOMIC_STORE_UINT32(&devstate->base_lif_id, &zero);
        }
    }

    uint32_t
    base_lif_id(enum DeviceType type)
    {
        shm_devstate_t *devstate = devstate_get(type);
        if (devstate) {
            uint32_t base_lif_id;
            SDK_ATOMIC_LOAD_UINT32(&devstate->base_lif_id, &base_lif_id);
            return base_lif_id;
        }
        return 0;
    }

    sdk_ret_t
    lif_fully_created_set(enum DeviceType type)
    {
        shm_devstate_t *devstate = devstate_get(type);
        if (devstate) {
            SDK_ATOMIC_STORE_BOOL(&devstate->lif_fully_created, true);
            return SDK_RET_OK;
        }
        return SDK_RET_ERR;
    }

    void
    lif_fully_created_clr(enum DeviceType type)
    {
        shm_devstate_t *devstate = devstate_get(type);
        if (devstate) {
            SDK_ATOMIC_STORE_BOOL(&devstate->lif_fully_created, false);
        }
    }

    bool
    lif_fully_created(enum DeviceType type)
    {
        shm_devstate_t *devstate = devstate_get(type);
        return devstate ? 
               SDK_ATOMIC_LOAD_BOOL(&devstate->lif_fully_created) : false;
    }

private:

    shm_devstate_t *devstate_get(enum DeviceType type)
    {
        SDK_ASSERT(type < DEVICETYPE_MAX);
        return shm_state ? &shm_state->dev_state[type] : nullptr;
    }

    sdk::lib::shmmgr        *shm_mmgr;
    shm_state_t             *shm_state;
};

static nicmgr_shm_t     nicmgr_shm;

sdk_ret_t
nicmgr_shm_init(pds_init_params_t *params)
{
    sdk_ret_t ret = nicmgr_shm.init(asic_is_hard_init());

    if (ret == SDK_RET_OK) {
        if (params->flow_age_pid) {
            ret = nicmgr_shm.cpp_pid_set(FTL, params->flow_age_pid);
            if (ret != SDK_RET_OK) {
                SHM_LOG_ERR("failed to set nicmgr_shm CPP pid " FMT_U
                            ": error " FMT_D, params->flow_age_pid, ret);
            }
        }
    }
    return ret;
}

sdk_ret_t
nicmgr_shm_cpp_pid_set(enum DeviceType type,
                            pid_t pid)
{
    return nicmgr_shm.cpp_pid_set(type, pid);
}

void
nicmgr_shm_cpp_pid_clr(enum DeviceType type)
{
    nicmgr_shm.cpp_pid_clr(type);
}

bool
nicmgr_shm_is_cpp_pid(enum DeviceType type)
{
    return nicmgr_shm.is_cpp_pid(type);
}

bool
nicmgr_shm_is_cpp_pid_any_dev(void)
{
    int dev_type;

    for (dev_type = DEVICETYPE_FIRST; dev_type < DEVICETYPE_MAX; dev_type++) {
        if (nicmgr_shm.is_cpp_pid((enum DeviceType)dev_type)) {
            return true;
        }
    }
    return false;
}

sdk_ret_t
nicmgr_shm_base_lif_id_set(enum DeviceType type,
                           uint32_t base_lif_id)
{
    return nicmgr_shm.base_lif_id_set(type, base_lif_id);
}

void
nicmgr_shm_base_lif_id_clr(enum DeviceType type)
{
    nicmgr_shm.base_lif_id_clr(type);
}

uint32_t
nicmgr_shm_base_lif_id(enum DeviceType type)
{
    return nicmgr_shm.base_lif_id(type);
}

sdk_ret_t
nicmgr_shm_lif_fully_created_set(enum DeviceType type)
{
    return nicmgr_shm.lif_fully_created_set(type);
}

void
nicmgr_shm_lif_fully_created_clr(enum DeviceType type)
{
    nicmgr_shm.lif_fully_created_clr(type);
}

bool
nicmgr_shm_lif_fully_created(enum DeviceType type)
{
    return nicmgr_shm.lif_fully_created(type);
}

