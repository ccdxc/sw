#include "ipfix_defines.h"
#include "ipfix/asm_out/INGRESS_p.h"
#include "ipfix/asm_out/ingress.h"
#include "../common-p4+/include/capri-macros.h"

struct ipfix_create_record_d d;
struct phv_                  p;

%%

ipfix_create_record:
    // construct IPv4 record
    phvwr       p.ipfix_record_ipv4_set_id, IPFIX_IPv4_RECORD_ID
    phvwr       p.ipfix_record_ipv4_len, IPFIX_IPv4_RECORD_SIZE

    phvwr       p.p4_txdma_intr_dma_cmd_ptr, \
                    CAPRI_PHV_START_OFFSET(phv2mem_cmd1_dma_cmd_pad) / 16

    add         r1, d.u.ipfix_create_record_d.pktaddr, \
                    d.u.ipfix_create_record_d.pktsize
    phvwr       p.phv2mem_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr       p.phv2mem_cmd1_dma_cmd_phv_start_addr, \
                    CAPRI_PHV_START_OFFSET(ipfix_record_ipv4_set_id)
    phvwr       p.phv2mem_cmd1_dma_cmd_phv_end_addr, \
                    CAPRI_PHV_END_OFFSET(ipfix_record_ipv4_dport)
    phvwr       p.phv2mem_cmd1_dma_cmd_addr, r1
    add         r1, r1, (CAPRI_PHV_END_OFFSET(ipfix_record_ipv4_dport) - \
                         CAPRI_PHV_START_OFFSET(ipfix_record_ipv4_set_id))

    phvwr       p.phv2mem_cmd2_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr       p.phv2mem_cmd2_dma_cmd_phv_start_addr, \
                    CAPRI_PHV_START_OFFSET(ipfix_record_ip_flow_state_index)
    phvwr       p.phv2mem_cmd2_dma_cmd_phv_end_addr, \
                    CAPRI_PHV_END_OFFSET(ipfix_record_common_drop_vector)
    phvwr       p.phv2mem_cmd2_dma_cmd_addr, r1

    add         r1, d.u.ipfix_create_record_d.pktsize, IPFIX_IPv4_RECORD_SIZE
    tblwr       d.u.ipfix_create_record_d.pktsize, r1

    // IPFIX header fixups before sending the packet out
    phvwr       p.ipfix_record_header_version, IPFIX_VERSION
    add         r1, r1, IPFIX_HEADER_SIZE
    phvwr       p.ipfix_record_header_len, r1
    phvwr       p.ipfix_record_header_export_time, r4[63:32]
    phvwr       p.ipfix_record_header_seq_num, 1
    phvwr       p.ipfix_record_header_domain_id, 1

    phvwr       p.phv2mem_cmd3_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr       p.phv2mem_cmd3_dma_cmd_phv_start_addr, \
                    CAPRI_PHV_START_OFFSET(ipfix_record_header_version)
    phvwr       p.phv2mem_cmd3_dma_cmd_phv_end_addr, \
                    CAPRI_PHV_END_OFFSET(ipfix_record_header_domain_id)
    phvwr       p.phv2mem_cmd3_dma_cmd_addr, d.u.ipfix_create_record_d.pktaddr
    phvwr       p.phv2mem_cmd3_dma_cmd_eop, 1

    phvwr       p.app_header_table0_valid, 0
    phvwr       p.app_header_table1_valid, 0
    phvwr       p.app_header_table2_valid, 0
    phvwr.e     p.app_header_table3_valid, 0
    nop
