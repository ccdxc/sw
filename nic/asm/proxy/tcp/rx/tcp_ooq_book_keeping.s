/*
 *  Implements the RX stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s3_t0_tcp_rx_k.h"

struct phv_ p;
struct s3_t0_tcp_rx_k_ k;
struct s3_t0_tcp_rx_d d;

%%
    .align
    .param tcp_ooo_processing_launch_dummy1
tcp_ooq_book_keeping:
    seq c1, d.u.tcp_ooo_book_keeping_d.tail_index0, r0
    bcf [c1], tcp_ooo_book_keeping_begin_use_first_ooo_queue
    add r1, d.u.tcp_ooo_book_keeping_d.end_seq0, 1
    seq c2, r1, k.to_s3_seq
    bcf [c2], tcp_ooo_book_keeping_enqueue_tail_of_first_queue

    seq c1, d.u.tcp_ooo_book_keeping_d.tail_index1, r0
    bcf [c1], tcp_ooo_book_keeping_begin_use_second_ooo_queue
    add r1, d.u.tcp_ooo_book_keeping_d.end_seq1, 1
    seq c2, r1, k.to_s3_seq 
    bcf [c2], tcp_ooo_book_keeping_enqueue_tail_of_second_queue 


    seq c1, d.u.tcp_ooo_book_keeping_d.tail_index2, r0
    bcf [c1], tcp_ooo_book_keeping_begin_use_third_ooo_queue
    add r1, d.u.tcp_ooo_book_keeping_d.end_seq2, 1
    seq c2, r1, k.to_s3_seq 
    bcf [c2], tcp_ooo_book_keeping_enqueue_tail_of_third_queue 


    seq c1, d.u.tcp_ooo_book_keeping_d.tail_index3, r0
    bcf [c1], tcp_ooo_book_keeping_begin_use_fourth_ooo_queue
    add r1, d.u.tcp_ooo_book_keeping_d.end_seq3, 1
    seq c2, r1, k.to_s3_seq 
    bcf [c2], tcp_ooo_book_keeping_enqueue_tail_of_fourth_queue 
    nop
    tbladd.f d.u.tcp_ooo_book_keeping_d.ooo_alloc_fail, 1
    phvwri p.p4_intr_global_drop, 1
    // disable all tables
    nop.e
    nop


tcp_ooo_book_keeping_begin_use_first_ooo_queue:
    tblwr  d.u.tcp_ooo_book_keeping_d.start_seq0, k.to_s3_seq
    add r1, k.to_s3_seq, k.to_s3_payload_len
    tblwr  d.u.tcp_ooo_book_keeping_d.end_seq0, r1 
    tbladd d.u.tcp_ooo_book_keeping_d.tail_index0, 1  
    // launch table to allocate qbase_addr0
    nop.e
    nop

tcp_ooo_book_keeping_begin_use_second_ooo_queue:
    tblwr  d.u.tcp_ooo_book_keeping_d.start_seq1, k.to_s3_seq
    add r1, k.to_s3_seq, k.to_s3_payload_len
    tblwr  d.u.tcp_ooo_book_keeping_d.end_seq1, r1 
    tbladd d.u.tcp_ooo_book_keeping_d.tail_index1, 1  
    // launch table to allocate qbase_addr1
    nop.e
    nop

tcp_ooo_book_keeping_begin_use_third_ooo_queue:
    tblwr  d.u.tcp_ooo_book_keeping_d.start_seq2, k.to_s3_seq
    add r1, k.to_s3_seq, k.to_s3_payload_len
    tblwr  d.u.tcp_ooo_book_keeping_d.end_seq2, r1 
    tbladd d.u.tcp_ooo_book_keeping_d.tail_index2, 1  
    // launch table to allocate qbase_addr2
    nop.e
    nop

tcp_ooo_book_keeping_begin_use_fourth_ooo_queue:
    tblwr  d.u.tcp_ooo_book_keeping_d.start_seq3, k.to_s3_seq
    add r1, k.to_s3_seq, k.to_s3_payload_len
    tblwr  d.u.tcp_ooo_book_keeping_d.end_seq3, r1 
    tbladd d.u.tcp_ooo_book_keeping_d.tail_index3, 1  
    // launch table to allocate qbase_addr3
    nop.e
    nop

tcp_ooo_book_keeping_enqueue_tail_of_first_queue:
    seq c3, d.u.tcp_ooo_book_keeping_d.tail_index0, (TCP_OOO_QUEUE_SIZE - 1)
    bcf [c3], tcp_ooo_book_keeping_queue0_full 
    add r1, k.to_s3_seq, k.to_s3_payload_len
    tblwr  d.u.tcp_ooo_book_keeping_d.end_seq0, r1
    phvwr  p.to_s6_ooo_tail_index, d.u.tcp_ooo_book_keeping_d.tail_index0
    tbladd d.u.tcp_ooo_book_keeping_d.tail_index0, 1
    phvwr p.to_s6_ooo_queue_id, 0
    //Check if start of next is same as end of the current 
    // if they are same - trigger rx2tx
    seq c1, d.u.tcp_ooo_book_keeping_d.start_seq1, r1 
    phvwr.c1 p.to_s6_ooo_rx2tx_ready_qid, 1
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(1, tcp_ooo_processing_launch_dummy1)
    nop.e
    nop

tcp_ooo_book_keeping_queue0_full:
    tbladd d.u.tcp_ooo_book_keeping_d.ooo_queue0_full, 1
    phvwri p.p4_intr_global_drop, 1
    // disable all tables
    nop.e
    nop
 
tcp_ooo_book_keeping_enqueue_tail_of_second_queue:
    seq c3, d.u.tcp_ooo_book_keeping_d.tail_index1, (TCP_OOO_QUEUE_SIZE - 1)
    bcf [c3], tcp_ooo_book_keeping_queue1_full 
    add r1, k.to_s3_seq, k.to_s3_payload_len
    tblwr  d.u.tcp_ooo_book_keeping_d.end_seq1, r1
    phvwr  p.to_s6_ooo_tail_index, d.u.tcp_ooo_book_keeping_d.tail_index1
    tbladd d.u.tcp_ooo_book_keeping_d.tail_index1, 1
    phvwr p.to_s6_ooo_queue_id, 1
    //Check if start of next is same as end of the current 
    // if they are same - trigger rx2tx
    seq c1, d.u.tcp_ooo_book_keeping_d.start_seq2, r1 
    phvwr.c1 p.to_s6_ooo_rx2tx_ready_qid, 2
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(1, tcp_ooo_processing_launch_dummy1)
    nop.e
    nop

tcp_ooo_book_keeping_queue1_full:
    tbladd d.u.tcp_ooo_book_keeping_d.ooo_queue1_full, 1
    phvwri p.p4_intr_global_drop, 1
    // disable all tables
    nop.e
    nop
 
tcp_ooo_book_keeping_enqueue_tail_of_third_queue:
    seq c3, d.u.tcp_ooo_book_keeping_d.tail_index2, (TCP_OOO_QUEUE_SIZE - 1)
    bcf [c3], tcp_ooo_book_keeping_queue2_full 
    add r1, k.to_s3_seq, k.to_s3_payload_len
    tblwr  d.u.tcp_ooo_book_keeping_d.end_seq2, r1
    phvwr  p.to_s6_ooo_tail_index, d.u.tcp_ooo_book_keeping_d.tail_index2
    tbladd d.u.tcp_ooo_book_keeping_d.tail_index2, 1
    phvwr p.to_s6_ooo_queue_id, 2
    //Check if start of next is same as end of the current 
    // if they are same - trigger rx2tx
    seq c1, d.u.tcp_ooo_book_keeping_d.start_seq3, r1 
    phvwr.c1 p.to_s6_ooo_rx2tx_ready_qid, 3
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(1, tcp_ooo_processing_launch_dummy1)
    nop.e
    nop

tcp_ooo_book_keeping_queue2_full:
    tbladd d.u.tcp_ooo_book_keeping_d.ooo_queue2_full, 1
    phvwri p.p4_intr_global_drop, 1
    // disable all tables
    nop.e
    nop
 
tcp_ooo_book_keeping_enqueue_tail_of_fourth_queue:
    seq c3, d.u.tcp_ooo_book_keeping_d.tail_index3, (TCP_OOO_QUEUE_SIZE - 1)
    bcf [c3], tcp_ooo_book_keeping_queue3_full 
    add r1, k.to_s3_seq, k.to_s3_payload_len
    tblwr  d.u.tcp_ooo_book_keeping_d.end_seq3, r1
    phvwr  p.to_s6_ooo_tail_index, d.u.tcp_ooo_book_keeping_d.tail_index3
    tbladd d.u.tcp_ooo_book_keeping_d.tail_index3, 1
    phvwr p.to_s6_ooo_queue_id, 3
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(1, tcp_ooo_processing_launch_dummy1)
    nop.e
    nop

tcp_ooo_book_keeping_queue3_full:
    tbladd d.u.tcp_ooo_book_keeping_d.ooo_queue3_full, 1
    phvwri p.p4_intr_global_drop, 1
    // disable all tables
    nop.e
    nop


