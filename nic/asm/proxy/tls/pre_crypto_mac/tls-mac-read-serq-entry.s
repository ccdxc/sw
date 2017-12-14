/*
 * AES-CBC-HMAC-SHA2 Mac-then-encrypt pre-mac pipeline:
 * 	Implements the reading of SERQ descriptor
 *  Stage 1, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct d_struct {
        idesc            : DESC_ADDRESS_WIDTH ; /* Descriptor address is written as 64-bits, so we have to use ADDRESS_WIDTH(64)
                                                 * instead of HBM_ADDRESS_WIDTH (32)
                                                 */
        pad              : 448  ;
};

struct phv_ p	;
struct d_struct d	;
struct tx_table_s1_t0_k k	    ;
%%
        .param          tls_mac_rx_serq_process
        .param          tls_mac_pkt_descriptor_process
        
tls_mac_read_serq_entry_process:
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s6_debug_stage0_3_thread, CAPRI_MPU_STAGE_1, CAPRI_MPU_TABLE_0)
    phvwr   p.to_s2_idesc, d.{idesc}
    phvwr   p.to_s4_idesc, d.{idesc}
    phvwr   p.to_s5_idesc, d.{idesc}

    add     r2, r0, d.{idesc}
    addi    r2, r2, PKT_DESC_AOL_OFFSET

table_read_rx_serq_mac: 
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_mac_rx_serq_process,
                                 k.tls_global_phv_qstate_addr, TLS_TCB_CRYPT_OFFSET,
                                 TABLE_SIZE_512_BITS)

table_read_idesc:
    CAPRI_NEXT_TABLE_READ(3, TABLE_LOCK_DIS, tls_mac_pkt_descriptor_process,
                          r2, TABLE_SIZE_512_BITS)
    nop.e
    nop
	
