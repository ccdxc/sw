/*
 * 	Implements the reading of SESQ entry to queue the desc to TCP tx q
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct phv_ p;
struct tcp_tx_read_sesq_ci_k k;
struct tcp_tx_read_sesq_ci_read_sesq_ci_d d;
	
	
%%
        .align
        .param          tcp_tx_sesq_read_descr_stage2_start
        .param          tcp_tx_sesq_consume_stage2_start
tcp_tx_sesq_read_ci_stage1_start:

        sne             c1, k.common_phv_pending_asesq, r0

        add.c1          r3, r0, d.{desc_addr}.dx
        add.c1          r3, r3, NIC_DESC_ENTRY_0_OFFSET
        add.!c1         r3, r0, d.desc_addr
        phvwr           p.to_s3_sesq_desc_addr, r3

        CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(1, tcp_tx_sesq_consume_stage2_start)

        // TLS is queueing with descriptor address pointing to area after
        // scratch, so don't add the offset here
        //add             r3, d.desc_addr, NIC_DESC_ENTRY_0_OFFSET
        CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                    tcp_tx_sesq_read_descr_stage2_start, r3, TABLE_SIZE_512_BITS)

        nop.e
        nop
