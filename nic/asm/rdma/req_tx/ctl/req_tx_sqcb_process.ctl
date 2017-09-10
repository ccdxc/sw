#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
struct req_tx_s0_k_t k;
struct sqcb0_t d;

/*
k = {
    params.lif = 123;
    params.qid = 456;
    params.qtype = 3;
};
*/

d = {
    fence = 1;
    curr_wqe_ptr = 0x0102030405060708;
};
