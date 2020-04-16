/*
* Copyright (c) 2020, Pensando Systems Inc.
*/

#include <cstdint>
#include <string>

// Common
#define QTYPE_MAX               (8)
#define PAL_barrier()           asm volatile("dsb sy" ::: "memory")

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

typedef struct {
    uint64_t base;
    uint32_t size;
    uint32_t length;
} queue_info_t;

bool get_lif_qstate(uint16_t lif, queue_info_t qinfo[QTYPE_MAX]);
std::string hal_cfg_path();
std::string mpart_cfg_path();
void qinfo(uint16_t lif);
void *memrev(void *block, size_t elnum);
uint8_t *parse_byte_array(char **argv, uint32_t size);
void mem_rd(uint64_t addr, uint32_t size);
void mem_wr(uint64_t addr, uint32_t size, uint8_t *src);
void mem_dump(uint64_t addr, uint32_t size, char *filepath);
void mem_bzero(uint64_t addr, uint32_t size);
void mem_fill(uint64_t addr, uint32_t size, uint8_t *pattern, uint32_t pattern_sz);
void mem_find(uint64_t addr, uint32_t size, uint8_t *pattern, uint32_t pattern_sz);
void mem_nfind(uint64_t addr, uint32_t size, uint8_t *pattern, uint32_t pattern_sz);

// pd
void pd_init();
void p4pd_common_p4plus_rxdma_rss_params_table_entry_show(uint32_t hw_lif_id);
int p4pd_common_p4plus_rxdma_rss_params_table_entry_add(uint32_t hw_lif_id,
    uint8_t debug);
int p4pd_common_p4plus_rxdma_rss_indir_table_entry_show(uint32_t hw_lif_id);

// eth
void eth_qstate(uint16_t lif, uint8_t qtype, uint32_t qid);
void eth_qpoll(uint16_t lif, uint8_t qtype);
void eth_qdump(uint16_t lif, uint8_t qtype, uint32_t qid, uint8_t ring);
void eth_debug(uint16_t lif, uint8_t qtype, uint32_t qid, uint8_t enable);
void eth_eqstate(uint64_t addr);
void eth_stats(uint16_t lif);
void eth_stats_reset(uint16_t lif);
void eth_port_config(uint64_t addr);
void eth_port_status(uint64_t addr);
void eth_lif_status(uint64_t addr);

// rdma
void rdma_qstate(uint16_t lif, uint8_t qtype, uint32_t qid);
void rdma_qstate_all(uint16_t lif, uint8_t qtype);

// nvme
void nvme_qstate(uint16_t lif, uint8_t qtype, uint32_t qid);

// virtio
void virtio_qstate(uint16_t lif, uint8_t qtype, uint32_t qid);
