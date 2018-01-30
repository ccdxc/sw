// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <sys/mman.h>
#include <dlfcn.h>
#include <unistd.h>
#include "lib/pal/pal.hpp"
#include "lib/pal/pal_internal.hpp"

namespace sdk {
namespace lib {

static void *gl_lib_handle;
extern pal_info_t   gl_pal_info;
typedef int (*connect_fn_t)();
typedef bool (*read_reg_fn_t)(uint64_t addr, uint32_t& data);
typedef bool (*write_reg_fn_t)(uint64_t addr, uint32_t  data);
typedef bool (*read_mem_fn_t)(uint64_t addr, uint8_t * data, uint32_t size);
typedef bool (*write_mem_fn_t)(uint64_t addr, uint8_t * data, uint32_t size);
typedef bool (*ring_doorbell_fn_t)(uint64_t addr, uint64_t data);
typedef bool (*step_cpu_pkt_fn_t)(const uint8_t* pkt, size_t pkt_len);

typedef struct pal_sim_vectors_s {
    connect_fn_t        connect;
    read_reg_fn_t           read_reg;
    write_reg_fn_t          write_reg;
    read_mem_fn_t           read_mem;
    write_mem_fn_t          write_mem;
    ring_doorbell_fn_t      ring_doorbell;
    step_cpu_pkt_fn_t       step_cpu_pkt; 
} pal_sim_vectors_t;

static pal_sim_vectors_t   gl_sim_vecs;

pal_ret_t
pal_init_sim_vectors ()
{
    gl_sim_vecs.connect =
            (connect_fn_t)dlsym(gl_lib_handle, "lib_model_connect");
    gl_sim_vecs.read_reg = (read_reg_fn_t)dlsym(gl_lib_handle, "read_reg");
    gl_sim_vecs.write_reg = (write_reg_fn_t)dlsym(gl_lib_handle, "write_reg");
    gl_sim_vecs.read_mem = (read_mem_fn_t)dlsym(gl_lib_handle, "read_mem");
    gl_sim_vecs.write_mem = (write_mem_fn_t)dlsym(gl_lib_handle, "write_mem");
    gl_sim_vecs.ring_doorbell = 
            (ring_doorbell_fn_t)dlsym(gl_lib_handle, "step_doorbell");
    gl_sim_vecs.step_cpu_pkt = 
            (step_cpu_pkt_fn_t)dlsym(gl_lib_handle, "step_cpu_pkt");

    return PAL_RET_OK;
}

pal_ret_t
pal_sim_reg_read (uint64_t addr, uint32_t *data, uint32_t num_words)
{
    if (!(*gl_sim_vecs.read_reg)(addr, *data)) {
        return PAL_RET_NOK;
    }
    return PAL_RET_OK;
}

pal_ret_t
pal_sim_reg_write (uint64_t addr, uint32_t *data, uint32_t num_words)
{
    if (!(*gl_sim_vecs.write_reg)(addr, *data)) {
        return PAL_RET_NOK;
    }
    return PAL_RET_OK;
}

pal_ret_t
pal_sim_mem_read (uint64_t addr, uint8_t * data, uint32_t size)
{
    if (!(*gl_sim_vecs.read_mem)(addr, data, size)) {
        return PAL_RET_NOK;
    }
    return PAL_RET_OK;
}

pal_ret_t
pal_sim_mem_write (uint64_t addr, uint8_t * data, uint32_t size)
{
    if (!(*gl_sim_vecs.write_mem)(addr, data, size)) {
        return PAL_RET_NOK;
    }
    return PAL_RET_OK;
}

pal_ret_t
pal_sim_ring_doorbell (uint64_t addr, uint64_t data)
{
    (*gl_sim_vecs.ring_doorbell)(addr, data);
    return PAL_RET_OK;
}

pal_ret_t
pal_sim_step_cpu_pkt (const uint8_t* pkt, size_t pkt_len)
{
    gl_sim_vecs.step_cpu_pkt(pkt, pkt_len);
    return PAL_RET_OK;
}

pal_ret_t
pal_sim_connect (void)
{
    int     rc;

    if (getenv("CAPRI_MOCK_MODE")) {
        return PAL_RET_OK;
    }

    SDK_TRACE_DEBUG("Connecting to ASIC SIM\n");
    do {
        rc = (*gl_sim_vecs.connect)();
        if (rc != -1) {
            SDK_TRACE_DEBUG("Connected to the ASIC model...\n");
            break;
        }
        SDK_TRACE_DEBUG("Failed to connect to asic, retrying in 1 sec ...\n");
        sleep(1);
    } while (1);

    return PAL_RET_OK;
}

pal_ret_t
pal_sim_init_rwvectors (void)
{
    gl_pal_info.rwvecs.reg_read = pal_sim_reg_read;
    gl_pal_info.rwvecs.reg_write = pal_sim_reg_write;
    gl_pal_info.rwvecs.mem_read = pal_sim_mem_read;
    gl_pal_info.rwvecs.mem_write = pal_sim_mem_write;
    gl_pal_info.rwvecs.ring_doorbell = pal_sim_ring_doorbell;
    gl_pal_info.rwvecs.step_cpu_pkt = pal_sim_step_cpu_pkt;

    pal_init_sim_vectors();
    
    return PAL_RET_OK;
}

pal_ret_t
pal_sim_dlopen (void)
{
    gl_lib_handle = dlopen("libmodel_client.so", RTLD_NOW | RTLD_GLOBAL);
    SDK_ASSERT(gl_lib_handle);
    return PAL_RET_OK;
}

pal_ret_t
pal_init_sim (void)
{
    pal_ret_t   rv;

    rv = pal_sim_dlopen();
    SDK_ASSERT(IS_PAL_API_SUCCESS(rv));

    rv = pal_sim_init_rwvectors();
    SDK_ASSERT(IS_PAL_API_SUCCESS(rv));

    rv = pal_sim_connect();
    SDK_ASSERT(IS_PAL_API_SUCCESS(rv));

    return PAL_RET_OK;
}

}    // namespace lib
}    // namespace sdk
