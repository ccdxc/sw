#ifndef _TCP_MACROS_H_
#define _TCP_MACROS_H_

#include "capri-macros.h"
#include "cpu-macros.h"
#include "tcp_common.h"

#define TCP_NEXT_TABLE_READ  CAPRI_NEXT_TABLE_READ

#define TCP_END_CWND_REDUCTION                                          \
        sne		c1, d.ca_state, TCP_CA_CWR;			\
	seq		c2, d.undo_marker, r0;		                \
	addi		r1, r0,TCP_INFINITE_SSTHRESH;	                \
	slt		c3, r1, d.snd_ssthresh;				\
	setcf		c4, [c1 & c2 & c3];                             \
	phvwr.!c4	p.rx2tx_snd_wnd, d.snd_ssthresh;	        \
        phvwri		p.common_phv_ca_event, CA_EVENT_COMPLETE_CWR;



#define TCP_READ_IDX CAPRI_READ_IDX



#endif /* #ifndef _TCP_MACROS_H_ */
