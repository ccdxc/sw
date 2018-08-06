
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdio>

#include "eth_common.h"
#include "sdk/pal.hpp"


struct eth_rx_qstate qstate_ethrx = { 0 };
struct eth_tx_qstate qstate_ethtx = { 0 };
struct eth_admin_qstate qstate_ethadmin = { 0 };


void write_qstate(char **argv)
{
    uint64_t addr = std::strtoul(argv[3], NULL, 0);

    switch (std::strtoul(argv[2], NULL, 0)) {
        case 0:
            sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_ethrx, sizeof(qstate_ethrx));
            qstate_ethrx.pc_offset = 3;
            qstate_ethrx.cosA = 0;
            qstate_ethrx.cosB = 0;
            qstate_ethrx.host = 1;
            qstate_ethrx.total = 1;
            qstate_ethrx.pid = 0;
            qstate_ethrx.enable = 1;
            qstate_ethrx.c_index0 = 0;
            qstate_ethrx.comp_index = 0;
            qstate_ethrx.ring_base = std::strtoul(argv[4], NULL, 16);
            qstate_ethrx.ring_size = std::strtoul(argv[5], NULL, 0);
            qstate_ethrx.cq_ring_base = std::strtoul(argv[6], NULL, 16);
            qstate_ethrx.intr_assert_addr = std::strtoul(argv[7], NULL, 16);
            qstate_ethrx.color = 1;
            sdk::lib::pal_mem_write(addr, (uint8_t *)&qstate_ethrx, sizeof(qstate_ethrx));
            break;
        case 1:
            sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_ethtx, sizeof(qstate_ethtx));
            qstate_ethtx.pc_offset = 4;
            qstate_ethtx.cosA = 0;
            qstate_ethtx.cosB = 0;
            qstate_ethtx.host = 1;
            qstate_ethtx.total = 1;
            qstate_ethtx.pid = 0;
            qstate_ethtx.enable = 1;
            qstate_ethtx.p_index0 = 0;
            qstate_ethtx.c_index0 = 0;
            qstate_ethtx.comp_index = 0;
            qstate_ethtx.ring_base = std::strtoul(argv[4], NULL, 16);
            qstate_ethtx.ring_size = std::strtoul(argv[5], NULL, 0);
            qstate_ethtx.cq_ring_base = std::strtoul(argv[6], NULL, 16);
            qstate_ethtx.intr_assert_addr = std::strtoul(argv[7], NULL, 16);
            qstate_ethtx.sg_ring_base = std::strtoul(argv[8], NULL, 16);
            qstate_ethtx.spurious_db_cnt = 0;
            qstate_ethtx.color = 1;
            sdk::lib::pal_mem_write(addr, (uint8_t *)&qstate_ethtx, sizeof(qstate_ethtx));
            break;
        default:
            std::cerr << "Invalid Queue Type!" << std::endl;
            break;
    }
}

void read_qstate(char **argv)
{
    uint64_t addr = std::strtoul(argv[3], NULL, 0);

    switch (std::strtoul(argv[2], NULL, 0)) {
        case 0:
            sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_ethrx, sizeof(qstate_ethrx));
            printf("pc_offset=0x%0x\n"
                 "rsvd0=0x%0x\n"
                 "cosA=0x%0x\ncosB=0x%0x\ncos_sel=0x%0x\n"
                 "eval_last=0x%0x\n"
                 "host=0x%0x\ntotal=0x%0x\n"
                 "pid=0x%0x\n"
                 "p_index0=0x%0x\nc_index0=0x%0x\n"
                 "comp_index=0x%0x\nc_index1=0x%0x\n"
                 "enable=0x%0x\ncolor=0x%0x\n"
                 "ring_base=0x%0lx\nring_size=0x%0x\n"
                 "cq_ring_base=0x%0lx\nintr_assert_addr=0x%0x\n"
                 "rss_type=0x%0x\n",
                 qstate_ethrx.pc_offset,
                 qstate_ethrx.rsvd0,
                 qstate_ethrx.cosA, qstate_ethrx.cosB, qstate_ethrx.cos_sel,
                 qstate_ethrx.eval_last,
                 qstate_ethrx.host, qstate_ethrx.total,
                 qstate_ethrx.pid,
                 qstate_ethrx.p_index0, qstate_ethrx.c_index0,
                 qstate_ethrx.comp_index, qstate_ethrx.c_index1,
                 qstate_ethrx.enable, qstate_ethrx.color,
                 qstate_ethrx.ring_base, qstate_ethrx.ring_size,
                 qstate_ethrx.cq_ring_base, qstate_ethrx.intr_assert_addr,
                 qstate_ethrx.rss_type);
            break;
        case 1:
            sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_ethtx, sizeof(qstate_ethtx));
            printf("pc_offset=0x%0x\n"
                 "rsvd0=0x%0x\n"
                 "cosA=0x%0x\ncosB=0x%0x\ncos_sel=0x%0x\n"
                 "eval_last=0x%0x\n"
                 "host=0x%0x\ntotal=0x%0x\n"
                 "pid=0x%0x\n"
                 "p_index0=0x%0x\nc_index0=0x%0x\n"
                 "comp_index=0x%0x\nci_fetch=0x%0x\n"
                 "enable=0x%0x\ncolor=0x%0x\n"
                 "ring_base=0x%0lx\nring_size=0x%0x\n"
                 "cq_ring_base=0x%0lx\nintr_assert_addr=0x%0x\n"
                 "sg_ring_base=0x%0lx\n"
                 "spurious_db_cnt=0x%0x\n",
                 qstate_ethtx.pc_offset,
                 qstate_ethtx.rsvd0,
                 qstate_ethtx.cosA, qstate_ethtx.cosB, qstate_ethtx.cos_sel,
                 qstate_ethtx.eval_last,
                 qstate_ethtx.host, qstate_ethtx.total,
                 qstate_ethtx.pid,
                 qstate_ethtx.p_index0, qstate_ethtx.c_index0,
                 qstate_ethtx.comp_index, qstate_ethtx.ci_fetch,
                 qstate_ethtx.enable, qstate_ethtx.color,
                 qstate_ethtx.ring_base, qstate_ethtx.ring_size,
                 qstate_ethtx.cq_ring_base, qstate_ethtx.intr_assert_addr,
                 qstate_ethtx.sg_ring_base,
                 qstate_ethtx.spurious_db_cnt);
            break;
        case 2:
            sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate_ethadmin, sizeof(qstate_ethadmin));
            printf("pc_offset=0x%0x\n"
                 "rsvd0=0x%0x\n"
                 "cosA=0x%0x\ncosB=0x%0x\ncos_sel=0x%0x\n"
                 "eval_last=0x%0x\n"
                 "host=0x%0x\ntotal=0x%0x\n"
                 "pid=0x%0x\n"
                 "p_index0=0x%0x\nc_index0=0x%0x\n"
                 "comp_index=0x%0x\nci_fetch=0x%0x\n"
                 "enable=0x%0x\ncolor=0x%0x\n"
                 "ring_base=0x%0lx\nring_size=0x%0x\n"
                 "cq_ring_base=0x%0lx\nintr_assert_addr=0x%0x\n"
                 "nicmgr_qstate_addr=0x%0lx\n",
                 qstate_ethadmin.pc_offset,
                 qstate_ethadmin.rsvd0,
                 qstate_ethadmin.cosA, qstate_ethadmin.cosB, qstate_ethadmin.cos_sel,
                 qstate_ethadmin.eval_last,
                 qstate_ethadmin.host, qstate_ethadmin.total,
                 qstate_ethadmin.pid,
                 qstate_ethadmin.p_index0, qstate_ethadmin.c_index0,
                 qstate_ethadmin.comp_index, qstate_ethadmin.ci_fetch,
                 qstate_ethadmin.enable, qstate_ethadmin.color,
                 qstate_ethadmin.ring_base, qstate_ethadmin.ring_size,
                 qstate_ethadmin.cq_ring_base, qstate_ethadmin.intr_assert_addr,
                 qstate_ethadmin.nicmgr_qstate_addr);
            break;
        default:
            std::cerr << "Invalid Queue Type!" << std::endl;
            break;
    }
}

int main (int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " op args ..." << std::endl;
        std::cerr << "   read_qstate    <qtype> <qstate_addr>" << std::endl;
        std::cerr << "   write_qstate   <qtype> <qstate_addr> <ring_base> <ring_size> <cq_ring_base> <intr_assert_addr>" << std::endl;
        std::cerr << "   memrd          <addr> <size_in_bytes>" << std::endl;
        std::cerr << "   memwr          <addr> <size_in_bytes> <bytes> ..." << std::endl;
        std::cerr << "   memwr32        <addr> <32-bit value>" << std::endl;
        std::cerr << "   memwr64        <addr> <64-bit value>" << std::endl;
        std::cerr << "   memdump        <addr> <size_in_bytes>" << std::endl;
        std::cerr << "   bzero          <addr> <size_in_bytes>" << std::endl;
        std::cerr << "   find           <addr> <size_in_bytes> <pattern>" << std::endl;
        std::cerr << "   nfind          <addr> <size_in_bytes> <pattern>" << std::endl;
        exit(1);
    }

    std::cout << std::endl;

#ifdef __x86_64__
    assert(sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_SIM) == sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_HAPS) == sdk::lib::PAL_RET_OK);
#endif

    if (strcmp(argv[1], "read_qstate") == 0) {
        assert(argc == 4);
        read_qstate(argv);
    } else if (strcmp(argv[1], "write_qstate") == 0) {
        assert(argc == 8);
        write_qstate(argv);
    } else if (strcmp(argv[1], "memrd") == 0) {
        assert(argc == 4);
        uint64_t addr = strtoul(argv[2], NULL, 16);
        uint32_t size = strtoul(argv[3], NULL, 0);
        uint8_t *buf = (uint8_t *)calloc(1, size);
        assert (buf != NULL);
        sdk::lib::pal_mem_read(addr, buf, size);
        for (uint32_t i = 0; i < size / 16; i++) {
            printf("%0lx : ", addr + (i * 16));
            for (uint32_t j = 0; j < 16; j++) {
                printf("%02x ", buf[(i * 16) + j]);
            }
            printf("\n");
        }
        std::cout << std::endl;
        free(buf);
    } else if (strcmp(argv[1], "memwr") == 0) {
        assert(argc > 4);
        uint64_t addr = strtoul(argv[2], NULL, 16);
        uint32_t size = strtoul(argv[3], NULL, 0);
        assert(argc == (int)(4 + size));
        uint8_t *buf = (uint8_t *)calloc(1, size);
        assert (buf != NULL);
        for (uint32_t i = 0; i < size; i++) {
            buf[i] = strtoul(argv[4 + i], NULL, 16);
        }
        sdk::lib::pal_mem_write(addr, buf, size);
        free(buf);
    } else if (strcmp(argv[1], "memwr64") == 0) {
        assert(argc > 3);
        uint64_t addr = strtoul(argv[2], NULL, 16);
        uint32_t value = strtoul(argv[3], NULL, 16);
        sdk::lib::pal_ring_doorbell(addr, value);
    } else if (strcmp(argv[1], "memdump") == 0) {
        assert(argc == 5);
        uint64_t addr = strtoul(argv[2], NULL, 16);
        uint32_t size = strtoul(argv[3], NULL, 0);
        uint8_t *buf = (uint8_t *)calloc(1, size);
        assert (buf != NULL);
        sdk::lib::pal_mem_read(addr, buf, size);
        FILE * fp = fopen(argv[4], "wb");
        assert(fp != NULL);
        fwrite(buf, sizeof(buf[0]), size, fp);
        fclose(fp);
    } else if (strcmp(argv[1], "bzero") == 0) {
        assert(argc == 4);
        uint64_t addr = strtoul(argv[2], NULL, 16);
        uint32_t size = strtoul(argv[3], NULL, 0);
        uint8_t *buf = (uint8_t *)calloc(1, size);
        assert (buf != NULL);
        sdk::lib::pal_mem_write(addr, buf, size);
        free(buf);
    } else if (strcmp(argv[1], "find") == 0) {
        assert(argc == 5);
        uint64_t addr = strtoul(argv[2], NULL, 16);
        uint32_t size = strtoul(argv[3], NULL, 0);
        uint32_t pattern = strtoul(argv[4], NULL, 0);
        uint32_t val;
        for (uint32_t i = 0; i < size/4; i++) {
            sdk::lib::pal_mem_read(addr + i*4, (uint8_t *)&val, sizeof(val));
            if (val == pattern) {
                printf("0x%lx\n", addr + i*4);
                return 0;
            }
        }
        printf("Pattern 0x%x not found in region 0x%lx - 0x%lx\n",
            pattern, addr, addr+size);
        return -1;
    } else if (strcmp(argv[1], "nfind") == 0) {
        assert(argc == 5);
        uint64_t addr = strtoul(argv[2], NULL, 16);
        uint32_t size = strtoul(argv[3], NULL, 0);
        uint32_t pattern = strtoul(argv[4], NULL, 0);
        uint32_t val;
        for (uint32_t i = 0; i < size/4; i++) {
            sdk::lib::pal_mem_read(addr + i*4, (uint8_t *)&val, sizeof(val));
            if (val != pattern) {
                printf("0x%lx\n", addr + i*4);
                return 0;
            }
        }
        printf("Pattern 0x%x not found in region 0x%lx - 0x%lx\n",
            pattern, addr, addr+size);
        return -1;
    } else {
        assert(0);
    }

    return 0;
}
