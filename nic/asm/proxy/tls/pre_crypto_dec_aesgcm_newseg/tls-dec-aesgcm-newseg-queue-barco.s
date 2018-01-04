/*
 * 	Implements the queueing of the request to Barco (if applicable)
 *  Stage 7, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"

#define D(field)    d.{u.queue_barco_d.##field}
#define K(field)    k.{##field}


#define DMA_GROUP_INPUT_SEGMENT         0
#define DMA_GROUP_FIRST_INPUT_SEGMENT   1
#define DMA_GROUP_PENDING_SEGMENT       2
#define DMA_GROUP_BARCO_SEND            3

struct phv_             p;
struct tx_table_s7_t0_d d;
struct tx_table_s7_t0_k k;

%%

    .param              BRQ_BASE

/*
    GPR Usage:
    r7  - Determine the last group of DMA commands to setup EOP
*/

tls_dec_aesgcm_queue_barco:

    /* Check if we need to skip all DMA
        - Case when the segment contained no payoad at all
            (corner case)
    */

    add             r1, r0, K(tls_global_phv_skip_dma)
    beqi            r1, 1, tls_dec_aesgcm_queue_barco_done
    nop

    add             r7, r0, r0

    /* Unconditionally setup idesc DMA command as the first */
    phvwri          p.p4_txdma_intr_dma_cmd_ptr, (CAPRI_PHV_START_OFFSET(dma_cmd_tls_rec_list_dma_cmd_type) / 16)


    add             r1, r0, K(tls_global_phv_no_payload)
    beqi            r1, 1, tls_dec_aesgcm_no_payload
    nop
    /* Setup idesc DMA Command */
    add             r1, K(to_s7_idesc), PKT_DESC_AOL_OFFSET
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_idesc_dma_cmd, r1,   \
        idesc_A0, idesc_next_pkt)

    add             r7, r0, DMA_GROUP_INPUT_SEGMENT

    add             r1, r0, K(tls_global_phv_first_segment)
    bnei            r1, 1, tls_dec_aesgcm_non_first_segment
    nop

/* Set up incoming idesc to contain the fragment/TLS record */
    /*  If the idesc is the first in the list
        setup AOL0 with the AAD information
        The idesc AOL1 has already been setup in Stage 4 Table 0
    */
    phvwr           p.idesc_A0, K(to_s7_opage).dx
    phvwr           p.idesc_O0, 0
    addi            r1, r0, NTLS_AAD_SIZE
    phvwr           p.idesc_L0, r1.wx

/* Setup odesc and opage reference in idesc (head of TLS record)  */
    phvwr           p.odesc_opage_ref_odesc, K(to_s7_odesc)
    phvwr           p.odesc_opage_ref_opage, K(to_s7_opage)
    add             r1, K(to_s7_idesc), PKT_DESC_ODESC_OFFSET
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_odesc_opage_ref_dma_cmd, r1,   \
        odesc_opage_ref_odesc, odesc_opage_ref_opage)

    add             r7, r0, DMA_GROUP_FIRST_INPUT_SEGMENT

tls_dec_aesgcm_non_first_segment:

tls_dec_aesgcm_no_payload:


/* ==== DMA commands when there are pending bytes in the current segment ==== */
    /* Selectively setup when segment_pending is TRUE */
    add             r1, r0, K(tls_global_phv_segment_pending)
    bnei            r1, 1, tls_dec_aesgcm_no_pending_bytes_in_segment
    nop

    /* curr_idesc already setup in TBD */
    addi            r1, r0, PKT_DESC_AOL_OFFSET
    add             r1, r1, K(to_s7_curr_idesc)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_curr_idesc_dma_cmd, r1,   \
        curr_idesc_A0, curr_idesc_L0)

/* Reference counting for ipage (tbl op instead? Needs to be atomic.) */
/* TODO - This needs to be done selectively only when the segment contains
part of the payload and not just the AuthTag */


/* Doorbell for TLS TXDMA pending work signalling */
	/* address will be in r4 */
	addi            r1, r0, \
            CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_INC, DB_SCHED_UPD_SET, 0, LIF_TLS)
	/* data will be in r3 */
	CAPRI_RING_DOORBELL_DATA(0, \
        K(s6_t0_to_s7_t0_fid_sbit0_ebit7...s6_t0_to_s7_t0_fid_sbit8_ebit15), \
        TLS_SCHED_RING_PENDING, 0)
    phvwr           p.phv_scratch_pend_dbell_data, r3
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_tls_txdma_penddbell_dma_cmd, r1,   \
        phv_scratch_pend_dbell_data, phv_scratch_pend_dbell_data)

    add             r7, r0, DMA_GROUP_PENDING_SEGMENT

tls_dec_aesgcm_no_pending_bytes_in_segment:

    add             r1, r0, K(tls_global_phv_enqueue_barco)
    bnei            r1, 1, tls_dec_aesgcm_no_barco_enqueue
    nop


tls_dec_aesgcm_equeue_barco:


/* ==== DMA commands when we are submitting the request to Barco ==== */

/* AAD, IV, Auth Tag */
    /* Setup explicit IV already setup ('salt' is in TLSCB 2nd-block, which is read separately */

    /* Bump up sequence number in CB used in AAD */
    phvwr           p.aad_aad_seq_num, D(explicit_iv)
    tbladd          D(explicit_iv), 1

    add             r1, r0, K(to_s7_opage)
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_aad_iv_authtag_dma_cmd, r1, aad_aad_seq_num, auth_tag_auth_tag_hi)


/* Setup the new odesc for Barco output, when we have a full TLS record */

    add             r1, r0, K(to_s7_odesc)
    addi            r1, r1, PKT_DESC_AOL_OFFSET

    phvwr           p.odesc_A0, K(to_s7_opage).dx

    addi            r2, r0, (NIC_PAGE_HEADROOM - NTLS_AAD_SIZE)
    phvwr           p.odesc_O0, r2.wx

    /* Output length = AAD Length + (TLS Record Length - IV Length - AuthTag Length) */
    add             r3, r0, K(tls_global_phv_tls_rec_len_sbit0_ebit7...tls_global_phv_tls_rec_len_sbit8_ebit14)
    subi            r3, r3, (TLS_AES_GCM_AUTH_TAG_SIZE - NTLS_TLS_HEADER_SIZE)
    phvwr           p.odesc_L0, r3.wx

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_odesc_dma_cmd, r1, odesc_A0, odesc_next_pkt)
    

/* Setup Barco descriptor on the request ring */
    /* TODO: Setup record head always to the correct idesc */
    add             r1, r0, K(tls_global_phv_first_segment)
    seq             c1, r1, 1
    add.c1          r1, K(to_s7_idesc), PKT_DESC_AOL_OFFSET
    add.!c1         r1, \
                    K(s6_t0_to_s7_t0_rec_head_idesc_sbit0_ebit7...s6_t0_to_s7_t0_rec_head_idesc_sbit8_ebit63), \
                    PKT_DESC_AOL_OFFSET
    phvwr           p.barco_desc_input_list_address, r1.dx
    add             r1, K(to_s7_odesc), PKT_DESC_AOL_OFFSET
    phvwr           p.barco_desc_output_list_address, r1.dx
    phvwr           p.barco_desc_command, D(barco_command)
    phvwr           p.barco_desc_key_desc_index, D(barco_key_desc_index)

    addi            r1, r0, BARCO_INPUT_OPAGE_IV
    add             r1, r1, K(to_s7_opage)
    phvwr           p.barco_desc_iv_address, r1.dx
    addi            r1, r0, BARCO_INPUT_OPAGE_AUTHTAG
    add             r1, r1, K(to_s7_opage)
    phvwr           p.barco_desc_auth_tag_addr, r1.dx

    addi            r1, r0, NTLS_AAD_SIZE
    phvwr           p.barco_desc_header_size, r1.wx

    phvwr           p.barco_desc_status_address, K(to_s7_idesc).dx
	/* address will be in r4 */
	CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_INC, DB_SCHED_UPD_SET, 0, LIF_TLS)
    phvwr       p.barco_desc_doorbell_address, r4.dx

	/* data will be in r3 */
	CAPRI_RING_DOORBELL_DATA(0, \
        K(s6_t0_to_s7_t0_fid_sbit0_ebit7...s6_t0_to_s7_t0_fid_sbit8_ebit15), \
        TLS_SCHED_RING_BSQ, 0)
    phvwr       p.barco_desc_doorbell_data, r3.dx

    /* Compute address */
    add             r2, r0, \
        K(s6_t0_to_s7_t0_producer_idx_sbit0_ebit7...s6_t0_to_s7_t0_producer_idx_sbit8_ebit31)
    sll             r2, r2, NIC_BRQ_ENTRY_SIZE_SHIFT

    addui           r1, r0, hiword(BRQ_BASE)
    addi            r1, r1, loword(BRQ_BASE)
    add             r1, r1, r2
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_barco_desc_dma_cmd, r1,\
                barco_desc_input_list_address, barco_desc_second_key_desc_index)



/* Barco PI increment (doorbell) */
    addi            r1, r0, 1
    add             r1, r1, \
                    K(s6_t0_to_s7_t0_producer_idx_sbit0_ebit7...s6_t0_to_s7_t0_producer_idx_sbit8_ebit31)
    phvwr           p.barco_dbell_pi, r1.wx
    CAPRI_DMA_CMD_PHV2MEM_SETUP_I(dma_cmd_barco_pi_write_dma_cmd,
                                CAPRI_BARCO_MD_HENS_REG_GCM0_PRODUCER_IDX,
                                barco_dbell_pi, barco_dbell_pi)

    add             r7, r0, DMA_GROUP_BARCO_SEND


tls_dec_aesgcm_no_barco_enqueue:

/*  Setup EOP and/or fence */
    .brbegin
    br              r7[1:0]
    nop

    .brcase DMA_GROUP_INPUT_SEGMENT
    /* DMA for linked list insertion */
    CAPRI_DMA_CMD_STOP(dma_cmd_tls_rec_list_dma_cmd)
    b tls_dec_aesgcm_no_barco_enqueue_EOP_done
    nop

    .brcase DMA_GROUP_FIRST_INPUT_SEGMENT
    /* DMA for first segment */
    CAPRI_DMA_CMD_STOP(dma_cmd_odesc_opage_ref_dma_cmd)
    b tls_dec_aesgcm_no_barco_enqueue_EOP_done
    nop

    .brcase DMA_GROUP_PENDING_SEGMENT
    CAPRI_DMA_CMD_STOP(dma_cmd_tls_txdma_penddbell_dma_cmd)
    /* DMA for pending bytes in segment */
    b tls_dec_aesgcm_no_barco_enqueue_EOP_done
    nop

    .brcase DMA_GROUP_BARCO_SEND
    /* DMA for sending request to Barco */
    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd_barco_pi_write_dma_cmd)

    .brend
tls_dec_aesgcm_no_barco_enqueue_EOP_done:
    

tls_dec_aesgcm_queue_barco_done:
    /* Clear barrier */
    /* TODO - Only on the completion of the segment */
    tblwr           D(active_segment), 0

    nop.e
    nop
