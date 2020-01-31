/*
 * AES-CBC-HMAC-SHA2 Mac-then-encrypt post-mac pipeline:
 * 	Implements the reading of input descriptor to retirve the page addresses
 *  Stage 2, Table 3
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_                 p;
struct tx_table_s2_t3_d     d;
struct tx_table_s2_t3_k     k;


%%
	.param          tls_mac_read_aad_process
	.param		TLS_PROXY_PAD_BYTES_HBM_TABLE_BASE
	
tls_mac_post_read_idesc_process:	
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s5_debug_stage0_3_thread, CAPRI_MPU_STAGE_2, CAPRI_MPU_TABLE_3)
    CAPRI_CLEAR_TABLE3_VALID
    phvwr       p.to_s3_ipage,  d.{u.tls_read_idesc_d.A0}.dx
    CAPRI_OPERAND_DEBUG(d.{u.tls_read_idesc_d.A0}.dx)
    phvwr       p.idesc_A0, d.u.tls_read_idesc_d.A0

    /*
     * The input page now has AAD + Data-packet-payload + HMAC (Auth-tag) (total size in L0).
     * For AES-CBC encryption request, we have to skip the AAD and pass the offset of
     * the data-packet-payload as the beginning of the 'plaintext' to encrypt.
     * The encryption must also include the HMAC (auth-tag), and appropriate padding to the
     * AES_CBC_BLOCK_SIZE, adjust the L0 accordingly.
     */
    add         r2, d.{u.tls_read_idesc_d.O0}.wx, NTLS_AAD_SIZE
    phvwr       p.idesc_O0, r2.wx
    sub         r2, d.{u.tls_read_idesc_d.L0}.wx, NTLS_AAD_SIZE
    addi        r2, r2, TLS_AES_CBC_HMAC_SHA256_AUTH_TAG_SIZE
    phvwr       p.idesc_L0, r2.wx

    /*
     * Calculate padding bytes required and setup for DMA of padding bytes
     * at the end of the packet to the AES-CBC block-boundary (16 bytes).
     * We will setup the padding in the A1/O1/L1 which points to the pre-carved
     * TLS padding HBM memory so there is no DMA required.
     */
    andi  	r3, r2, (TLS_AES_CBC_BLOCK_SIZE -1)
    rsubi 	r4, r3, TLS_AES_CBC_BLOCK_SIZE

    /*
     * Set the padding in A1/O1/L1 of the idesc.
     */
    addui       r5, r0, hiword(TLS_PROXY_PAD_BYTES_HBM_TABLE_BASE)
    addi        r5, r5, loword(TLS_PROXY_PAD_BYTES_HBM_TABLE_BASE)
    phvwr       p.idesc_A1, r5.dx

     /*
      * The tls pad HBM memory is setup as 16-byte blocks of all 0s,1s,2s...15s
      * so based on the padding length, we set the offset O1 to appropriate value
      * to pick up the padding content.
      */
     CAPRI_OPERAND_DEBUG(r4)
	.brbegin
	    br		r4[3:0]
	    nop
	        .brcase 0
	            phvwri  p.idesc_O1, TLS_WORD_SWAP(0xe0)
	            b tls_mac_post_read_idesc_pad_done
	            nop
	        .brcase 1
	            phvwri  p.idesc_O1, 0x0
	            b tls_mac_post_read_idesc_pad_done
	            nop
	        .brcase 2
	            phvwri  p.idesc_O1, TLS_WORD_SWAP(0x10)
	            b tls_mac_post_read_idesc_pad_done
	            nop
	        .brcase 3
	            phvwri  p.idesc_O1, TLS_WORD_SWAP(0x20)
	            b tls_mac_post_read_idesc_pad_done
	            nop
	        .brcase 4
	            phvwri  p.idesc_O1, TLS_WORD_SWAP(0x30)
	            b tls_mac_post_read_idesc_pad_done
	            nop
	        .brcase 5
	            phvwri  p.idesc_O1, TLS_WORD_SWAP(0x40)
	            b tls_mac_post_read_idesc_pad_done
	            nop
	        .brcase 6
	            phvwri  p.idesc_O1, TLS_WORD_SWAP(0x50)
	            b tls_mac_post_read_idesc_pad_done
	            nop
	        .brcase 7
	            phvwri  p.idesc_O1, TLS_WORD_SWAP(0x60)
	            b tls_mac_post_read_idesc_pad_done
	            nop
	        .brcase 8
	            phvwri  p.idesc_O1, TLS_WORD_SWAP(0x70)
	            b tls_mac_post_read_idesc_pad_done
	            nop
	        .brcase 9
	            phvwri  p.idesc_O1, TLS_WORD_SWAP(0x80)
	            b tls_mac_post_read_idesc_pad_done
	            nop
	        .brcase 10
	            phvwri  p.idesc_O1, TLS_WORD_SWAP(0x90)
	            b tls_mac_post_read_idesc_pad_done
	            nop
	        .brcase 11
	            phvwri  p.idesc_O1, TLS_WORD_SWAP(0xa0)
	            b tls_mac_post_read_idesc_pad_done
	            nop
	        .brcase 12
	            phvwri  p.idesc_O1, TLS_WORD_SWAP(0xb0)
	            b tls_mac_post_read_idesc_pad_done
	            nop
	        .brcase 13
	            phvwri  p.idesc_O1, TLS_WORD_SWAP(0xc0)
	            b tls_mac_post_read_idesc_pad_done
	            nop
	        .brcase 14
	            phvwri  p.idesc_O1, TLS_WORD_SWAP(0xd0)
	            b tls_mac_post_read_idesc_pad_done
	            nop
	        .brcase 15
	            phvwri  p.idesc_O1, TLS_WORD_SWAP(0xe0)
	            b tls_mac_post_read_idesc_pad_done
	            nop
	.brend

tls_mac_post_read_idesc_pad_done:

    /*
     * Set the padding length in L1.
     */
    phvwr       p.idesc_L1, r4.wx

    /*
     * Setup the total TLS-header-length to be set in the opage for eventual out-packet.
     */
    add         r2, r2, r4

    /*
     * Set the output length (post-encrypt) in the output-desc L0
     */
    phvwr       p.odesc_L0, r2.wx

    addi        r2, r2, TLS_AES_CBC_RANDOM_IV_SIZE
    phvwr       p.to_s3_tls_hdr_len, r2.wx

    /*
     * Setup table read to the AAD from the input page, so we pick up values for TLS header
     */
    add         r2, r0, d.{u.tls_read_idesc_d.A0}.dx
    add         r2, r2, d.{u.tls_read_idesc_d.O0}.wx
    CAPRI_NEXT_TABLE_READ(3, TABLE_LOCK_EN, tls_mac_read_aad_process,
                          r2, TABLE_SIZE_512_BITS)
	
    nop.e
    nop
