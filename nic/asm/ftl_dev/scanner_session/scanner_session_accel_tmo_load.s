#include "ftl_dev_shared_asm.h"

struct phv_                              p;
struct s1_tbl2_k                         k;
struct s1_tbl2_session_accel_tmo_load_d  d;

/*
 * Registers usage
 */

%%
    .align

session_accel_tmo_load:

    // In the event of aging timeout values in the middle of being
    // updated by software -- which is very rare -- temporarily
    // switch to using default values. This is necessitated by
    // the fact, due to MPU staging layout, our FSM exec (which
    // is stage 1 table 0) will have advanced its various state
    // variables and can no longer be backtracked.
    //
    // Note also that session_fsm_exec has control of the loading of
    // next tables so it will also be responsible for clearing any
    // tableX_valid bits as applicable.
     
    seq         c1, d.cb_select, r0
    
    // Software guarantees that whenever there is a switch between
    // the normal template and accelerated template, cb_select of the
    // new one would always be set before clearing cb_select in the other.
    
    nop.c1.e
_if0:
    sne         c2, d.cb_activate, SCANNER_AGE_TMO_CB_ACTIVATE  // delay slot
    bcf         [c2], _endif0

    SCANNER_KIVEC_FORCE_EXPIRED_TS_LOAD()    
    SESSION_KIVEC_AGE_TMO_LOAD(2)            
    SESSION_KIVEC_SESSION_BASE_TMO_LOAD_e(2)            

_endif0:
    SESSION_KIVEC_AGE_TMO_DFLT_LOAD(2)
    SESSION_KIVEC_SESSION_BASE_DFLT_TMO_LOAD_e(2)            

