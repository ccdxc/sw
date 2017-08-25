#include "common_phv.h"

struct sram_lif_entry_t d;

d = {
    pt_base_addr_page_id = 0x12345;
    log_num_pt_entries = 10;
    cqcb_base_addr_page_id = 0x6789a;
    log_num_cq_entries = 10;                                                         
    //prefetch_pool_base_addr_page_id = 0xbcdef;
    //log_num_prefetch_pool_entries = 10;
};
