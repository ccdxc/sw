/*
 * Stage 0, Table 0
 * Implementation of Virtio LIF RX qstate read
 */
#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "virtio_defines.h"
#include "INGRESS_common_p4plus_stage0_app_header_table_k.h"

struct phv_                 p;
struct common_p4plus_stage0_app_header_table_k_ k;
struct common_p4plus_stage0_app_header_table_d  d;

#define D(field)    d.{u.read_virtq_rx_qstate_d.##field}
#define K(field)    k.{##field}



%%

    .param      virtio_rx_read_virtq_avail_start

virtio_rx_read_qstate_start:

    phvwr       p.virtio_s2s_t0_phv_pkt_len, K(p4_to_p4plus_packet_len)
    phvwr       p.virtio_s2s_t0_phv_rx_intr_assert_addr, D(rx_intr_assert_addr).wx
    phvwri      p.virtio_s2s_t0_phv_rx_intr_assert_data, 0x01000000

    phvwr       p.to_s2_qid, K(p4_rxdma_intr_qid)
    phvwr       p.to_s2_lif, K(p4_intr_global_lif)
    phvwr       p.to_s1_qstate_addr, K(p4_rxdma_intr_qstate_addr)

    phvwr       p.to_s2_rx_virtq_avail_addr, D(rx_virtq_avail_addr).dx
    phvwr       p.to_s3_rx_virtq_desc_addr, D(rx_virtq_desc_addr).dx
    phvwr       p.to_s4_rx_virtq_desc_addr, D(rx_virtq_desc_addr).dx

    phvwr       p.to_s4_rx_virtq_used_addr, D(rx_virtq_used_addr).dx
    phvwr       p.to_s5_rx_virtq_used_addr, D(rx_virtq_used_addr).dx
    phvwr       p.virtio_s2s_t0_phv_rx_queue_size_mask, D(rx_queue_size_mask).hx

virtio_rx_read_qstate_done:

    add         r1, r0, D(rx_virtq_avail_addr).dx

	CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                        virtio_rx_read_virtq_avail_start,
	                    r1, TABLE_SIZE_32_BITS)
	nop.e
    nop
