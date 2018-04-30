#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
struct smbdc_req_tx_s1_t0_k k;
struct smbdc_sqwqe_mr_t d;


d = {
    wrid = 0x1010101020202020;
    direction = SMBDC_MR_DIR_WRITE;
    need_invalidate = 0;
    num_sg_lists = 4;
    begin_offset = 0x100;
    end_offset = 0x200;
    sg0.base_addr = 0x1234123412341234;
    sg0.num_pages = 0x1111;
    sg1.base_addr = 0x5656565656565656;
    sg1.num_pages = 0x2222;
    sg2.base_addr = 0x7878787878787878;
    sg2.num_pages = 0x3333;
    sg3.base_addr = 0x9090909090909090;
    sg3.num_pages = 0x4444;
};
