#include "../include/tcp-constants.h"
#include "../include/tcp-phv.h"
#include "../include/tcp-shared-state.h"
#include "../include/tcp-macros.h"
#include "../include/tcp-table.h"
#include "../include/tcp-sched.h"
	
 /* d is the data returned by lookup result */
struct d_struct {
        TCB_TX_SHARED_STATE
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
	flags				: 8  ;
	addr				: ADDRESS_WIDTH ;
	offset				: OFFSET_WIDTH ;
	len				: LEN_WIDTH ;
	sesq_event			: 1  ;
	timer_event			: 1  ;
	asq_event			: 1  ;
	txq_event			: 1  ;
	snd_una				: SEQ_NUMBER_WIDTH	;\
	rcv_nxt				: SEQ_NUMBER_WIDTH	;\
	rcv_mss_shft			: 4	                ;\
	pingpong			: 1                     ;\
	quick				: 4	                ;\
	ooo_datalen			: COUNTER16 	        ;\

        write_seq                       : SEQ_NUMBER_WIDTH      ;\
	snd_nxt				: SEQ_NUMBER_WIDTH	;\
	snd_wnd				: SEQ_NUMBER_WIDTH	;\
	snd_up				: SEQ_NUMBER_WIDTH	;\
        snd_cwnd                        : WINDOW_WIDTH          ;\
	packets_out			: COUNTER16	        ;\
	sacked_out			: COUNTER16	        ;\
	lost_out			: COUNTER8	        ;\
	retrans_out			: COUNTER8	        ;\
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;

d = {
  retx_ci	        = 6;
  retx_pi               = 7;
  sched_flag            = TCP_SCHED_FLAG_PENDING_TX;
  retx_snd_una		= 0x16;



  retx_tail_desc	= 0xD0;
  retx_snd_nxt_cursor   = 0x298;
  retx_snd_una_cursor   = 0x298;


};

k = {
  sesq_event		= 0x01;
  addr			= 0xA0;
  offset		= 0x40;
  len			= 0xB0;

  snd_una		= 0x16;
  rcv_nxt		= 0x10;
  snd_nxt 		= 0x20;
  snd_cwnd     		= 0x10;
  snd_wnd		= 0x10;

  rcv_mss_shft		= 1;
};

r4 = 0xC0;
