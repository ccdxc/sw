// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
#include <iostream>
#include <vector>
#include <map>
#include <string.h>
#include "lib/pal/pal.hpp"
#include "include/sdk/lock.hpp"
#include "lib/dpdk/sim/sim.hpp"
#include "platform/utils/mpart_rsvd.hpp"

struct dpdk_sim_mbuf_data_s {
    uint64_t signature;
    uint64_t offset;
    struct   dpdk_sim_mbuf_data_s *next;
};

typedef struct dpdk_sim_mbuf_data_s dpdk_sim_mbuf_data_t;
typedef std::map<uint64_t, uint64_t> dpdk_sim_mbuf_map_t;
typedef std::map<void *, std::vector<uint64_t>> dpdk_sim_desc_map_t;

static dpdk_sim_desc_map_t desc_map;
static uint64_t desc_paddr_offset = 0;
static uint64_t pmem_base = 0;
static uint32_t desc_size;

static dpdk_sim_mbuf_data_t mbuf_data_head = {0};
static dpdk_sim_mbuf_map_t mbuf_map;
static uint32_t mbuf_paddr_offset = 0;
static sdk_spinlock_t slock;
static std::map<std::string, uint32_t> lif_map;

#define DPDK_SIM_MEM_START (MREGION_BASE_ADDR + MREGION_RSVD_START_OFFSET)
#define DPDK_SIM_SIGNATURE 0xbfb0beb1bdb2bcb3ULL
#define DPDK_SIM_DESC_MAX_PMEM_SIZE (2 << 20) // 2 MB
#define DPDK_SIM_MBUF_PMEM_BASE     (pmem_base + DPDK_SIM_DESC_MAX_PMEM_SIZE)
#define DPDK_SIM_MBUF_MAX_PMEM_SIZE (30 << 20) // 30 MB
#define DPDK_MAX_PACKET_SIZE 10240
#define DPDK_SIM_APP_MEM_SIZE   (DPDK_SIM_DESC_MAX_PMEM_SIZE + \
                                 DPDK_SIM_MBUF_MAX_PMEM_SIZE)

#define PRINT_DBG(fmt, args...)
#define PRINT_ERR(fmt, args...) printf(fmt, ##args)

extern "C" {

int
dpdk_sim_init(void)
{
    int err;
    int app_id = 0;

    if (std::getenv("DPDK_SIM_APP_ID")) {
        std::string dpdk_id_str(std::getenv("DPDK_SIM_APP_ID"));
        app_id = std::stoi(dpdk_id_str);
    }

    /* initialize PAL */
    err = sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_SIM);
    SDK_ASSERT(err == sdk::lib::PAL_RET_OK);
    pmem_base = DPDK_SIM_MEM_START + (app_id * DPDK_SIM_APP_MEM_SIZE);
    return 0;
}

int
dpdk_sim_mbuf_init(uint32_t ndescs)
{
    dpdk_sim_mbuf_data_t *m;
    //uint32_t dsize = 9200; // TODO , multiple pools
    uint32_t pdsize = DPDK_MAX_PACKET_SIZE;
    //dsize + DPDK_SIM_HDR_SIZE;

    //ndescs += 512;
    SDK_ASSERT((mbuf_paddr_offset + (pdsize * ndescs)) <= DPDK_SIM_MBUF_MAX_PMEM_SIZE);
    m = (dpdk_sim_mbuf_data_t *)malloc(ndescs * sizeof(dpdk_sim_mbuf_data_t));
    SDK_ASSERT(m != NULL);
    SDK_SPINLOCK_INIT(&slock, PTHREAD_PROCESS_PRIVATE);

    for(uint32_t i = 0; i < ndescs; i++) {
        m->signature = DPDK_SIM_SIGNATURE;
        m->offset = mbuf_paddr_offset;
        m->next = mbuf_data_head.next;
        mbuf_data_head.next = m;
        mbuf_paddr_offset += pdsize;
        m++;
    }
    desc_size = pdsize;
    return 0;
}



uint64_t
dpdk_sim_get_bar_addr(const char *dev_name)
{
    FILE *fp = NULL;
    const char *cfg_path = std::getenv("CONFIG_PATH");
    uint64_t pa;
    uint32_t lif_id;
    char buf[256], name[64];
    static std::map<std::string, std::string> name_map;
    bool found = false;

    // dpdk name to lif name mapping
    name_map["net_ionic0"] = "cpu_mnic0";
    name_map["net_ionic1"] = "cpu_mnic1";

    std::map<std::string, std::string>::iterator i = name_map.find(dev_name);
    SDK_ASSERT(i != name_map.end());
    SDK_ASSERT(cfg_path);
    snprintf(buf, sizeof(buf), "%s/gen/device_info.txt", cfg_path);
    if ((fp = fopen(buf, "r")) != NULL) {
        while(fgets(buf, sizeof(buf),  fp)) {
            if (strncmp(buf, i->second.c_str(), strlen(i->second.c_str()))) {
                continue;
            }
            sscanf(buf, "%s 0x%lx %u", name, &pa, &lif_id);
            if (strcmp(name, i->second.c_str()) == 0) {
                lif_map[dev_name] = lif_id;
                found = true;
                break;
            }
        }
        // entry not found
        SDK_ASSERT(found == true);
    } else {
        SDK_ASSERT(0);
    }
    if (fp) {
        fclose(fp);
    }
    PRINT_DBG("name %s pa %lx lif_id %d\n", name, pa, lif_id);
    return pa;
}

uint32_t
dpdk_sim_read(uint64_t paddr, uint32_t sz)
{
    if (sz == 4) {
        uint32_t data;

        sdk::lib::pal_mem_read(paddr, (uint8_t *)&data, sz);
        return data;
    } else if (sz == 2) {
        uint16_t data;

        sdk::lib::pal_mem_read(paddr, (uint8_t *)&data, sz);
        return data;
    } else if (sz == 1) {
        uint8_t data;

        sdk::lib::pal_mem_read(paddr, &data, sz);
        return data;
    } else {
        SDK_ASSERT(0);
    }
}

void
dpdk_sim_read_mem(void *dst, uint64_t paddr, uint32_t sz)
{
    sdk::lib::pal_mem_read(paddr, (uint8_t *)dst, sz);
}

void
dpdk_sim_write(uint64_t val, uint64_t paddr, uint32_t sz)
{
    if (sz == 8) {
        sdk::lib::pal_mem_write(paddr, (uint8_t *)&val, sz);
    } else if (sz == 4) {
        uint32_t v = val;
        sdk::lib::pal_mem_write(paddr, (uint8_t *)&v, sz);
    } else if (sz == 1) {
        uint8_t v = val;
        sdk::lib::pal_mem_write(paddr, (uint8_t *)&v, sz);
    } else {
        SDK_ASSERT(0);
    }
}

void
dpdk_sim_write_mem(void *src, uint64_t paddr, uint32_t sz)
{
    sdk::lib::pal_mem_write(paddr, (uint8_t *)src, sz);
}

static uint64_t
alloc_desc(uint64_t size)
{
    uint64_t paddr = pmem_base + desc_paddr_offset;

    if ((desc_paddr_offset + size) >= DPDK_SIM_DESC_MAX_PMEM_SIZE) {
        SDK_ASSERT(0);
    }
    desc_paddr_offset += size;
    return paddr;
}

static int
find_desc_map(uint64_t vaddr, uint32_t size, uint64_t *paddr)
{
    uint64_t va, pa, sz, eaddr;
    uint64_t endaddr = vaddr + size - 1;

    for (auto i = desc_map.begin(); i != desc_map.end(); i++) {
        va = (uint64_t)i->first;
        pa = i->second[0];
        sz = i->second[1];
        eaddr = va + sz - 1;
        PRINT_DBG("va %lx, pa %lx, size %u\n",  va, pa, sz);
        if ((vaddr >= va) && (endaddr <= eaddr)) {
            *paddr =  pa + vaddr - va;
            return 0;
        }
        // SDK_ASSERT if vaddr is already part of any map
        if ((vaddr >= va) && (vaddr <= eaddr)) {
            PRINT_ERR("vaddr %lx, eaddr %lx\n", vaddr, eaddr);
            SDK_ASSERT(0);
        }
        if ((va >= vaddr) && (va <= endaddr)) {
            PRINT_ERR("vaddr %lx, endaddr %lx\n", vaddr, endaddr);
            SDK_ASSERT(0);
        }
    }
    return -1;
}

uint64_t
dpdk_sim_desc_alloc(void *vaddr, uint64_t size)
{
    uint64_t paddr;
    std::vector<uint64_t> v;

    if ((find_desc_map((uint64_t)vaddr, size, &paddr)) == 0) {
        return paddr;
    }
    paddr = alloc_desc(size);
    v.push_back(paddr);
    v.push_back(size);
    desc_map[vaddr] = v;
    return paddr;
}

void
dpdk_sim_write_doorbell(const char *dev_name, uint32_t qtype, uint64_t data)
{
    uint64_t addr;
    uint32_t lif_id;

    std::map<std::string, uint32_t>::iterator l = lif_map.find(dev_name);
    SDK_ASSERT(l != lif_map.end());
    lif_id = l->second;
    addr = 0x400000 | (0xb << 17) | (lif_id << 6) | (qtype << 3);

    sdk::lib::pal_ring_db64(addr, data);
}

uint64_t
dpdk_sim_mbuf_alloc(void *buf_addr, uint32_t size)
{
    uint64_t pa;
    uint64_t k = (uint64_t)buf_addr;

    SDK_SPINLOCK_LOCK(&slock);
    dpdk_sim_mbuf_data_t *m = mbuf_data_head.next;

    SDK_ASSERT(size <= desc_size);
    SDK_ASSERT(m != NULL);
    SDK_ASSERT(mbuf_map.find(k) == mbuf_map.end());

    pa = m->offset + DPDK_SIM_MBUF_PMEM_BASE;
    mbuf_data_head.next = m->next;

    mbuf_map[k] = (uint64_t)m;
    SDK_SPINLOCK_UNLOCK(&slock);
    return pa;
}

void
dpdk_sim_mbuf_free(void *buf_addr)
{
    uint64_t k = (uint64_t)buf_addr;
    dpdk_sim_mbuf_data_t *m;

    SDK_SPINLOCK_LOCK(&slock);
    SDK_ASSERT(mbuf_map.find(k) != mbuf_map.end());
    m = (dpdk_sim_mbuf_data_t *)mbuf_map[k];

    m->next = mbuf_data_head.next;
    mbuf_data_head.next = m;

    mbuf_map.erase(k);
    SDK_SPINLOCK_UNLOCK(&slock);
}

void
dpdk_sim_mbuf_read(void *buf_addr, uint32_t offset, uint32_t size)
{
    uint64_t paddr;
    uint64_t k = (uint64_t)buf_addr;
    dpdk_sim_mbuf_data_t *m;

    SDK_SPINLOCK_LOCK(&slock);
    SDK_ASSERT(mbuf_map.find(k) != mbuf_map.end());
    m = (dpdk_sim_mbuf_data_t *)mbuf_map[k];
    paddr =  m->offset + DPDK_SIM_MBUF_PMEM_BASE;

    dpdk_sim_read_mem((char *)buf_addr + offset, paddr, size);
    SDK_SPINLOCK_UNLOCK(&slock);
    // paddr reference is no more required
    dpdk_sim_mbuf_free(buf_addr);
}

}   // extern "C"
