// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <cstring>
#include <sys/mman.h>
#include <dlfcn.h>
#include <unistd.h>
#include "lib/pal/pal.hpp"
#include "lib/pal/pal_internal.hpp"
#include "lib/thread/thread.hpp"

namespace sdk {
namespace lib {

static void *gl_lib_handle;
extern pal_info_t   gl_pal_info;
typedef int (*connect_fn_t)();
typedef int (*disconnect_fn_t)();
typedef bool (*read_reg_fn_t)(uint64_t addr, uint32_t& data);
typedef bool (*write_reg_fn_t)(uint64_t addr, uint32_t  data);
typedef bool (*read_mem_fn_t)(uint64_t addr, uint8_t * data, uint32_t size);
typedef bool (*write_mem_fn_t)(uint64_t addr, uint8_t * data, uint32_t size);
typedef bool (*ring_dbfn_t)(uint64_t addr, uint64_t data);
typedef bool (*step_cpu_pkt_fn_t)(const uint8_t* pkt, size_t pkt_len);
typedef void* (*mem_map_fn_t)(uint64_t addr, uint32_t size);

typedef struct pal_sim_vectors_s {
    connect_fn_t            connect;
    disconnect_fn_t         disconnect;
    read_reg_fn_t           read_reg;
    write_reg_fn_t          write_reg;
    read_mem_fn_t           read_mem;
    write_mem_fn_t          write_mem;
    ring_dbfn_t             ring_doorbell;
    step_cpu_pkt_fn_t       step_cpu_pkt;
    mem_map_fn_t            mem_map;
} pal_sim_vectors_t;

static pal_sim_vectors_t   gl_sim_vecs;

pal_ret_t
pal_init_sim_vectors ()
{
    gl_sim_vecs.connect =
            (connect_fn_t)dlsym(gl_lib_handle, "lib_model_connect");
    gl_sim_vecs.disconnect =
            (disconnect_fn_t)dlsym(gl_lib_handle, "lib_model_conn_close");
    gl_sim_vecs.read_reg = (read_reg_fn_t)dlsym(gl_lib_handle, "read_reg");
    gl_sim_vecs.write_reg = (write_reg_fn_t)dlsym(gl_lib_handle, "write_reg");
    gl_sim_vecs.read_mem = (read_mem_fn_t)dlsym(gl_lib_handle, "read_mem");
    gl_sim_vecs.write_mem = (write_mem_fn_t)dlsym(gl_lib_handle, "write_mem");
    gl_sim_vecs.ring_doorbell =
            (ring_dbfn_t)dlsym(gl_lib_handle, "step_doorbell");
    gl_sim_vecs.step_cpu_pkt =
            (step_cpu_pkt_fn_t)dlsym(gl_lib_handle, "step_cpu_pkt");
    gl_sim_vecs.mem_map = (mem_map_fn_t)dlsym(gl_lib_handle, "mem_map");
    return PAL_RET_OK;
}

inline pal_ret_t
pal_sim_physical_addr_to_virtual_addr(uint64_t phy_addr,
                                      uint64_t *virtual_addr)
{
    *virtual_addr = phy_addr;
    return PAL_RET_OK;;
}

inline pal_ret_t
pal_sim_virtual_addr_to_physical_addr(uint64_t virtual_addr,
                                      uint64_t *phy_addr)
{
    *phy_addr = virtual_addr;
    return PAL_RET_OK;;
}

pal_ret_t
pal_sim_reg_read (uint64_t addr, uint32_t *data, uint32_t num_words)
{
    while (num_words--) {
        if (!(*gl_sim_vecs.read_reg)(addr, *data)) {
            return PAL_RET_NOK;
        }
        addr += sizeof(uint32_t);
        data++;
    }
    return PAL_RET_OK;
}

pal_ret_t
pal_sim_reg_write (uint64_t addr, uint32_t *data, uint32_t num_words)
{
    while (num_words--) {
        if (!(*gl_sim_vecs.write_reg)(addr, *data)) {
            return PAL_RET_NOK;
        }
        addr += sizeof(uint32_t);
        data++;
    }
    return PAL_RET_OK;
}

/* Note: The flags are ignored for model */
pal_ret_t
pal_sim_mem_read (uint64_t addr, uint8_t * data, uint32_t size, uint32_t flags)
{
    if (!(*gl_sim_vecs.read_mem)(addr, data, size)) {
        return PAL_RET_NOK;
    }
    return PAL_RET_OK;
}

/* Note: The flags are ignored for model */
pal_ret_t
pal_sim_mem_write (uint64_t addr, uint8_t * data, uint32_t size, uint32_t flags)
{
    if (!(*gl_sim_vecs.write_mem)(addr, data, size)) {
        return PAL_RET_NOK;
    }
    return PAL_RET_OK;
}

/* Note: The flags are ignored for model */
pal_ret_t
pal_sim_mem_set (uint64_t addr, uint8_t data, uint32_t size, uint32_t flags)
{
    uint32_t i = 0;

    /* We could recycle i, but choosing this instead for understandability */
    uint32_t bytes_written = 0;
    uint8_t buffer[8];

    /* Prepare a buffer of size 8 bytes */
    for(i = 0; i < 8; i++) {
	buffer[i] = data;
    }

    /* Perform writes of 8 byte length */
    for (bytes_written = 0;
         bytes_written < size;
         bytes_written = bytes_written + 8) {
    	if (!(*gl_sim_vecs.write_mem)(addr + bytes_written, buffer, 8)) {
            return PAL_RET_NOK;
    	}
    }

    if (!(*gl_sim_vecs.write_mem)(addr + bytes_written,
                                  buffer,
                                  (size - bytes_written) % 8)) {
        return PAL_RET_NOK;
    }

    return PAL_RET_OK;
}

void*
pal_sim_mem_map(const uint64_t pa, const uint32_t sz)
{
    return (*gl_sim_vecs.mem_map)(pa, sz);
}

pal_ret_t
pal_sim_ring_db16 (uint64_t addr, uint16_t data)
{
    (*gl_sim_vecs.ring_doorbell)(addr, (uint64_t) data);
    return PAL_RET_OK;
}

pal_ret_t
pal_sim_ring_db32 (uint64_t addr, uint32_t data)
{
    (*gl_sim_vecs.ring_doorbell)(addr, (uint64_t) data);
    return PAL_RET_OK;
}

pal_ret_t
pal_sim_ring_db64 (uint64_t addr, uint64_t data)
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

    SDK_TRACE_DEBUG("Connecting to ASIC SIM");
    do {
        rc = (*gl_sim_vecs.connect)();
        if (rc != -1) {
            SDK_TRACE_DEBUG("Connected to the ASIC model...");
            break;
        }
        SDK_TRACE_DEBUG("Failed to connect to asic, retrying in 1 sec ...");
        sleep(1);
    } while (1);

    return PAL_RET_OK;
}

pal_ret_t
pal_sim_disconnect (void)
{
    int     rc;

    if (getenv("CAPRI_MOCK_MODE")) {
        return PAL_RET_OK;
    }

    SDK_TRACE_DEBUG("Disconnecting ASIC SIM");
    do {
        rc = (*gl_sim_vecs.disconnect)();
        if (rc != -1) {
            SDK_TRACE_DEBUG("Disconnected the ASIC model...");
            break;
        }
        SDK_TRACE_DEBUG("Failed to disconnect asic, retrying in 1 sec ...");
        sleep(1);
    } while (1);

    return PAL_RET_OK;
}

pal_ret_t
pal_sim_qsfp_read(const uint8_t *buffer, uint32_t size, uint32_t offset,
          qsfp_page_t page, uint32_t nretry, uint32_t port)
{
    // hexdump for QSFP-100G-CR4 5M
    const uint8_t dummy[] = {  0x11 ,0x07 ,0x04 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00
                              ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00
                              ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00
                              ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00
                              ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00
                              ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00
                              ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x08 ,0x00
                              ,0x00 ,0x10 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00
                              ,0x11 ,0x00 ,0x23 ,0x88 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x05 ,0xff ,0x00 ,0x00 ,0x00
                              ,0x00 ,0x00 ,0x05 ,0xa0 ,0x41 ,0x6d ,0x70 ,0x68 ,0x65 ,0x6e ,0x6f ,0x6c ,0x20 ,0x20 ,0x20 ,0x20
                              ,0x20 ,0x20 ,0x20 ,0x20 ,0x1f ,0x78 ,0xa7 ,0x14 ,0x4e ,0x44 ,0x41 ,0x41 ,0x46 ,0x4a ,0x2d ,0x30
                              ,0x30 ,0x30 ,0x34 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x42 ,0x20 ,0x07 ,0x0b ,0x0d ,0x14 ,0x55 ,0x0a
                              ,0x0b ,0x00 ,0x00 ,0x00 ,0x41 ,0x50 ,0x46 ,0x31 ,0x38 ,0x33 ,0x35 ,0x30 ,0x30 ,0x34 ,0x34 ,0x42
                              ,0x4c ,0x48 ,0x20 ,0x20 ,0x31 ,0x38 ,0x30 ,0x38 ,0x32 ,0x39 ,0x20 ,0x20 ,0x00 ,0x00 ,0x67 ,0x74
                              ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff
                              ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff};

    if(size > 256) {
        return PAL_RET_NOK;
    }

    memcpy((uint8_t*)buffer, dummy+offset, size);

    SDK_TRACE_DEBUG("PAL default pal_sim_qsfp_read");
    return PAL_RET_OK;
}

pal_ret_t
pal_sim_qsfp_write(const uint8_t *buffer, uint32_t size, uint32_t offset,
           qsfp_page_t page, uint32_t nretry, uint32_t port)
{
    SDK_TRACE_DEBUG("PAL default pal_sim_qsfp_write");
    return PAL_RET_OK;
}

pal_ret_t
pal_sim_qsfp_dom_read(const uint8_t *buffer, uint32_t size, uint32_t offset,
                      uint32_t nretry, uint32_t port)
{
    SDK_TRACE_DEBUG("PAL default pal_sim_qsfp_dom_read");
    return PAL_RET_OK;
}


static bool
pal_sim_is_qsfp_port_present(int port_no)
{
    return true;
}

pal_ret_t
pal_sim_init_rwvectors (void)
{
    gl_pal_info.rwvecs.reg_read = pal_sim_reg_read;
    gl_pal_info.rwvecs.reg_write = pal_sim_reg_write;
    gl_pal_info.rwvecs.mem_read = pal_sim_mem_read;
    gl_pal_info.rwvecs.mem_write = pal_sim_mem_write;
    gl_pal_info.rwvecs.ring_db16 = pal_sim_ring_db16;
    gl_pal_info.rwvecs.ring_db32 = pal_sim_ring_db32;
    gl_pal_info.rwvecs.ring_db64 = pal_sim_ring_db64;
    gl_pal_info.rwvecs.step_cpu_pkt = pal_sim_step_cpu_pkt;
    gl_pal_info.rwvecs.physical_addr_to_virtual_addr =
                        pal_sim_physical_addr_to_virtual_addr;
    gl_pal_info.rwvecs.virtual_addr_to_physical_addr =
                        pal_sim_virtual_addr_to_physical_addr;
    gl_pal_info.rwvecs.mem_set = pal_sim_mem_set;
    gl_pal_info.rwvecs.mem_map = pal_sim_mem_map;
    gl_pal_info.rwvecs.is_qsfp_port_present =
                            pal_sim_is_qsfp_port_present;
    gl_pal_info.rwvecs.qsfp_read = pal_sim_qsfp_read;
    gl_pal_info.rwvecs.qsfp_write = pal_sim_qsfp_write;
    gl_pal_info.rwvecs.qsfp_dom_read = pal_sim_qsfp_dom_read;
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

pal_ret_t
pal_teardown_sim (void)
{
    pal_ret_t   rv;

    rv = pal_sim_disconnect();
    SDK_ASSERT(IS_PAL_API_SUCCESS(rv));

    return PAL_RET_OK;
}

}    // namespace lib
}    // namespace sdk
