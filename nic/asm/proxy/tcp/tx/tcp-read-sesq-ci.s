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
struct s1_t1_tcp_tx_k k;
struct s1_t1_tcp_tx_read_sesq_ci_d d;
	
	
%%
        .align
        .param          tcp_tx_sesq_consume_stage2_start
        .param          tcp_tx_read_descr_start
tcp_tx_sesq_read_ci_stage1_start:

        sne             c1, k.common_phv_pending_asesq, r0

        add.c1          r3, d.{desc_addr}.dx, NIC_DESC_ENTRY_0_OFFSET
        add.!c1         r3, r0, d.desc_addr

        sub             r4, r3, NIC_DESC_ENTRY_0_OFFSET
        phvwr           p.to_s3_sesq_desc_addr, r4

        CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(1, tcp_tx_sesq_consume_stage2_start)

		CAPRI_NEXT_TABLE_READ_e(0, TABLE_LOCK_DIS,
                        tcp_tx_read_descr_start, r3, TABLE_SIZE_512_BITS)

        nop.e
        nop
