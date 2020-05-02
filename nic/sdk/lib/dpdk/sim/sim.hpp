// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __DPDK_SIM_HPP__
#define __DPDK_SIM_HPP__

#ifdef __cplusplus
extern "C" {
#endif
int dpdk_sim_init(void);
uint32_t dpdk_sim_read(uint64_t paddr, uint32_t sz);
void dpdk_sim_write(uint64_t val, uint64_t paddr, uint32_t sz);
uint64_t dpdk_sim_desc_alloc(void *vaddr, uint64_t size);
void dpdk_sim_write_mem(void *src, uint64_t paddr, uint32_t sz);
void dpdk_sim_read_mem(void *dst, uint64_t paddr, uint32_t sz);
void dpdk_sim_write_doorbell(const char *dev_name, uint32_t qtype, uint64_t data);
uint64_t dpdk_sim_mbuf_alloc(void *buf_addr, uint32_t size);
void dpdk_sim_mbuf_free(void *buf_addr);
uint64_t dpdk_sim_get_bar_addr(const char *dev_name);
void dpdk_sim_mbuf_read(void *buf_addr, uint32_t offset, uint32_t size);
int dpdk_sim_mbuf_init(uint32_t ndescs);
#ifdef __cplusplus
}    // extern "C"
#endif

#endif
