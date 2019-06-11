#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t0_nvme_sesspredgst_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s2_t0_nvme_sesspredgst_tx_k_ k;
struct s2_t0_nvme_sesspredgst_tx_pdu_ctxt_process_d d;

#define DATA_LEN            r1
#define AOL_P               r2
#define TX_HWDGSTCB_P       r2

%%
    .param    nvme_tx_hwdgstcb_addr
    .param    nvme_sesspredgst_tx_dgstcb_process

.align
nvme_sesspredgst_tx_pdu_ctxt_process:
    // setup ddgst_desc data length based on nlb and lba_size and src_list_address
    // using pdu_ctxt

    // setup ddgst_desc command
    // total data size without including the ddgst size
    sll            DATA_LEN, d.nlb, d.log_lba_size
    or             r2, HW_CMD_DDGST, DATA_LEN, 16
    phvwr          p.ddgst_desc_cmd, r2.dx

    // setup ddgst_desc src list address
    mfspr          AOL_P, spr_tbladdr
    add            AOL_P, AOL_P, NVME_PDU_CTXT_AOL_DESC_LIST_OFFSET

    // Setup PDU CH and PSH fields based on pdu context
    // DMA cmd itself is composed in hdgst_process which has access to the tcp
    // page ptr
    seq            c1, d.pdu_opc, NVME_O_TCP_CH_PDU_TYPE_CAPSULECMD
    bcf            [!c1], exit
    phvwr          p.ddgst_desc_src, AOL_P.dx // BD Slot

capsule_cmd_pdu_hdr_setup:
    // Setup CH hdr with both dgsts enabled
    phvwri        p.{pdu_ch_pdu_type...pdu_ch_pdo}, NVME_O_TCP_CH_CMD_CAPSULE_DGST_EN_WORD0
    add           r3, DATA_LEN, (NVME_O_TCP_CMD_CAPSULE_DGST_EN_HDR_SIZE + NVME_O_TCP_DDGST_SIZE)
    phvwr         p.pdu_ch_plen, r3.wx

    // Setup PSH hdr with CCSQE
    phvwr         p.pdu_psh_cid, d.{cid}.hx
    phvwrpair     p.pdu_psh_psdt, SQE_DATA_XFER_TYPE_DATA_SGL, p.pdu_psh_opc, d.cmd_opc
    phvwr         p.pdu_psh_nsid, d.{nsid}.wx
    phvwr         p.pdu_psh_slba, d.{slba}.dx
    phvwr         p.pdu_psh_nlb, d.{nlb}.hx
    phvwrpair     p.pdu_psh_sgl1_len, DATA_LEN, \
                  p.{pdu_psh_sgl1_type...pdu_psh_sgl1_sub_type}, NVME_O_TCP_PSH_CMD_CAPSULE_SGL_TYPE_SUBTYPE

trigger_dgstcb:
    addui          TX_HWDGSTCB_P, r0, hiword(nvme_tx_hwdgstcb_addr)
    addi           TX_HWDGSTCB_P, TX_HWDGSTCB_P, loword(nvme_tx_hwdgstcb_addr)

    // setup hdgst_desc opaque_tag_address to dgst_cb's CI offset
    phvwr          p.hdgst_desc_opaque_tag_addr, TX_HWDGSTCB_P.dx

    // load tx_dgstcb to fetch PI indices for hdr and data dgst
    // to post to dgst engine
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_EN,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sesspredgst_tx_dgstcb_process,
                                TX_HWDGSTCB_P) // Exit Slot

exit:
    // TODO clear busy
    phvwr.e        p.p4_intr_global_drop, 1
    nop            // Exit Slot

