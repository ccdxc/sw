/*
* Copyright (c) 2020, Pensando Systems Inc.
*/

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/lib/logger/logger.hpp"

#include "impl.hpp"


int
debug_logger (uint32_t mod_id, sdk::lib::sdk_trace_level_e trace_level,
              const char *format, ...)
{
    return 0;
}

void
usage()
{
    printf("Usage:\n");
    printf("   qinfo          <lif>\n");
    printf("   qstate         <lif> <qtype> <qid>\n");
    printf("   eqstate        <addr>\n");
    printf("   qdump          <lif> <qtype> <qid> <ring>\n");
    printf("   debug          <lif> <qtype> <qid> <enable>\n");
    printf("   nvme_qstate    <lif> <qtype> <qid>\n");
    printf("   rdma_qstate    <lif> <qtype> <qid>\n");
    printf("   virtio_qstate  <lif> <qtype> <qid>\n");
    printf("   qpoll          <lif> <qtype>\n");
    printf("   stats          <lif>\n");
    printf("   stats_reset    <lif>\n");
    printf("   memrd          <addr> <size_in_bytes>\n");
    printf("   memwr          <addr> <size_in_bytes> <bytes> ...\n");
    printf("   memdump        <addr> <size_in_bytes>\n");
    printf("   memzero        <addr> <size_in_bytes>\n");
    printf("   memfill        <addr> <size_in_bytes> <pattern_size_in_bytes> <pattern>\n");
    printf("   memfind        <addr> <size_in_bytes> <pattern_size_in_bytes> <pattern>\n");
    printf("   memnfind       <addr> <size_in_bytes> <pattern_size_in_bytes> <pattern>\n");
    printf("   port_config    <addr>\n");
    printf("   port_status    <addr>\n");
    printf("   lif_status     <addr>\n");
    printf("   rss            <lif>\n");
    printf("   rss_debug      <lif> <enable>\n");
    exit(1);
}

int
main(int argc, char **argv)
{
    if (argc < 2) {
        usage();
    }

    sdk::lib::logger::init(&debug_logger);

    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HW) ==
           sdk::lib::PAL_RET_OK);

    if (strcmp(argv[1], "qinfo") == 0) {
        if (argc != 3) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        qinfo(lif);
    } else if (strcmp(argv[1], "qstate") == 0) {
        if (argc != 5) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        uint8_t qtype = std::strtoul(argv[3], NULL, 0);
        uint32_t qid = std::strtoul(argv[4], NULL, 0);
        eth_qstate(lif, qtype, qid);
    } else if (strcmp(argv[1], "qdump") == 0) {
        if (argc != 6) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        uint8_t qtype = std::strtoul(argv[3], NULL, 0);
        uint32_t qid = std::strtoul(argv[4], NULL, 0);
        uint8_t ring = std::strtoul(argv[5], NULL, 0);
        eth_qdump(lif, qtype, qid, ring);
    } else if (strcmp(argv[1], "debug") == 0) {
        if (argc != 6) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        uint8_t qtype = std::strtoul(argv[3], NULL, 0);
        uint32_t qid = std::strtoul(argv[4], NULL, 0);
        uint8_t enable = std::strtoul(argv[5], NULL, 0);
        eth_debug(lif, qtype, qid, enable);
    } else if (strcmp(argv[1], "eqstate") == 0) {
        if (argc != 3) {
            usage();
        }
        uint64_t addr = std::strtoull(argv[2], NULL, 0);
        eth_eqstate(addr);
    } else if (strcmp(argv[1], "nvme_qstate") == 0) {
        if (argc != 5) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        uint8_t qtype = std::strtoul(argv[3], NULL, 0);
        uint32_t qid = std::strtoul(argv[4], NULL, 0);
        nvme_qstate(lif, qtype, qid);
    } else if (strcmp(argv[1], "rdma_qstate") == 0) {
        if (argc < 3 || argc > 5) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);

        if (argc == 3) {
            rdma_qstate_all(lif, QTYPE_MAX);
        } else if (argc == 4) {
            uint8_t qtype = std::strtoul(argv[3], NULL, 0);
            rdma_qstate_all(lif, qtype);
        } else {
            uint8_t qtype = std::strtoul(argv[3], NULL, 0);
            uint32_t qid = std::strtoul(argv[4], NULL, 0);
            rdma_qstate(lif, qtype, qid);
        }
    } else if (strcmp(argv[1], "virtio_qstate") == 0) {
        if (argc != 5) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        uint8_t qtype = std::strtoul(argv[3], NULL, 0);
        uint32_t qid = std::strtoul(argv[4], NULL, 0);
        virtio_qstate(lif, qtype, qid);
    } else if (strcmp(argv[1], "qpoll") == 0) {
        if (argc != 4) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        uint8_t qtype = std::strtoul(argv[3], NULL, 0);
        eth_qpoll(lif, qtype);
    } else if (strcmp(argv[1], "stats") == 0) {
        if (argc != 3) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        eth_stats(lif);
    } else if (strcmp(argv[1], "stats_reset") == 0) {
        if (argc != 3) {
            usage();
        }
        uint16_t lif = std::strtoul(argv[2], NULL, 0);
        eth_stats_reset(lif);
        eth_stats(lif);
    } else if (strcmp(argv[1], "memrd") == 0) {
        if (argc != 4) {
            usage();
        }
        uint64_t addr = strtoul(argv[2], NULL, 16);
        uint32_t size = strtoul(argv[3], NULL, 0);
        mem_rd(addr, size);
    } else if (strcmp(argv[1], "memwr") == 0) {
        if (argc <= 4) {
            usage();
        }
        uint64_t addr = strtoul(argv[2], NULL, 16);
        uint32_t size = strtoul(argv[3], NULL, 0);
        if (argc != (int)(3 + size)) {
            printf("Not enough bytes to write\n");
            usage();
        }
        uint8_t *buf = parse_byte_array(&argv[4], size);
        if (buf != NULL) {
            mem_wr(addr, size, buf);
            free(buf);
        }
    } else if (strcmp(argv[1], "memdump") == 0) {
        if (argc != 5) {
            usage();
        }
        uint64_t addr = strtoul(argv[2], NULL, 16);
        uint32_t size = strtoul(argv[3], NULL, 0);
        char *filepath = argv[4];
        mem_dump(addr, size, filepath);
    } else if (strcmp(argv[1], "memzero") == 0) {
        if (argc != 4) {
            usage();
        }
        uint64_t addr = strtoul(argv[2], NULL, 16);
        uint32_t size = strtoul(argv[3], NULL, 0);
        mem_bzero(addr, size);
    } else if (strcmp(argv[1], "memfill") == 0) {
        uint64_t addr = strtoul(argv[2], NULL, 16);
        uint32_t size = strtoul(argv[3], NULL, 0);
        uint32_t pattern_sz = strtoul(argv[4], NULL, 0);
        if (argc != (int)(5 + pattern_sz)) {
            printf("Not enough bytes in pattern\n");
            usage();
        }
        uint8_t *buf = parse_byte_array(&argv[5], pattern_sz);
        if (buf != NULL) {
            mem_fill(addr, size, buf, pattern_sz);
            free(buf);
        }
    } else if (strcmp(argv[1], "memfind") == 0) {
        if (argc <= 5) {
            usage();
        }
        uint64_t addr = strtoul(argv[2], NULL, 16);
        uint32_t size = strtoul(argv[3], NULL, 0);
        uint32_t pattern_sz = strtoul(argv[4], NULL, 0);
        if (argc != (int)(5 + pattern_sz)) {
            printf("Not enough bytes in pattern\n");
            usage();
        }
        uint8_t *buf = parse_byte_array(&argv[5], pattern_sz);
        if (buf != NULL) {
            mem_find(addr, size, buf, pattern_sz);
            free(buf);
        }
    } else if (strcmp(argv[1], "memnfind") == 0) {
        if (argc <= 5) {
            usage();
        }
        uint64_t addr = strtoul(argv[2], NULL, 16);
        uint32_t size = strtoul(argv[3], NULL, 0);
        uint32_t pattern_sz = strtoul(argv[4], NULL, 0);
        if (argc != (int)(5 + pattern_sz)) {
            printf("Not enough bytes in pattern\n");
            usage();
        }
        uint8_t *buf = parse_byte_array(&argv[5], pattern_sz);
        if (buf != NULL) {
            mem_nfind(addr, size, buf, pattern_sz);
            free(buf);
        }
    } else if (strcmp(argv[1], "port_config") == 0) {
        if (argc != 3) {
            usage();
        }
        uint64_t addr = strtoul(argv[2], NULL, 16);
        eth_port_config(addr);
    } else if (strcmp(argv[1], "port_status") == 0) {
        if (argc != 3) {
            usage();
        }
        uint64_t addr = strtoul(argv[2], NULL, 16);
        eth_port_status(addr);
    } else if (strcmp(argv[1], "lif_status") == 0) {
        if (argc != 3) {
            usage();
        }
        uint64_t addr = strtoul(argv[2], NULL, 16);
        eth_lif_status(addr);
    } else if (strcmp(argv[1], "rss") == 0) {
        if (argc != 3) {
            usage();
        }
        uint16_t lif_id = strtoul(argv[2], NULL, 0);
        printf("\n");
        p4pd_common_p4plus_rxdma_rss_params_table_entry_show(lif_id);
        p4pd_common_p4plus_rxdma_rss_indir_table_entry_show(lif_id);
        printf("\n");
    } else if (strcmp(argv[1], "rss_debug") == 0) {
        if (argc != 4) {
            usage();
        }
        uint16_t lif_id = strtoul(argv[2], NULL, 0);
        uint8_t enable = std::strtoul(argv[3], NULL, 0);
        p4pd_common_p4plus_rxdma_rss_params_table_entry_add(lif_id, enable);
        printf("\n");
        p4pd_common_p4plus_rxdma_rss_params_table_entry_show(lif_id);
        p4pd_common_p4plus_rxdma_rss_indir_table_entry_show(lif_id);
        printf("\n");
    } else {
        usage();
    }

    return 0;
}
