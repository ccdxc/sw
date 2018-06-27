// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "cap_top_csr.h"

cap_mx_lgrp_csr_t::cap_mx_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_mx_lgrp_csr_t::~cap_mx_lgrp_csr_t() { } 

void cap_mx_lgrp_csr_t::init() {
    mx[0].set_attributes(this,"mx[0]",  0x1d00000);
    mx[1].set_attributes(this,"mx[1]",  0x1e00000);
} // cap_mx_lgrp_csr_t::init()

void cap_mx_lgrp_csr_t::register_model(int chip_id) {
    CAP_BLK_REG_MODEL_REGISTER(cap_mx_csr_t,chip_id, 0, &mx[0]);
    CAP_BLK_REG_MODEL_REGISTER(cap_mx_csr_t,chip_id, 1, &mx[1]);
} // cap_mx_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_top_csr_t(string name , cap_csr_base * parent) : 
cap_block_base(name, parent) { }
cap_top_csr_t::~cap_top_csr_t() { }

void cap_top_csr_t::init(int chip_id) { 
    mx.set_attributes(this, "mx", 0);
    mx.register_model(chip_id);
    CAP_BLK_REG_MODEL_REGISTER(cap_mx_lgrp_csr_t, chip_id, 0, &mx); 

    vector<cap_csr_base *> all_top_blocks;
    for(auto i : get_children(1)) {
        for(auto j : i->get_children(1)) {
            all_top_blocks.push_back(j);
        }
    }

    for(auto i : all_top_blocks) {
        for(auto cmp_blk : all_top_blocks) {
            if(cmp_blk == i) continue;
            if( ((i->get_offset() >= cmp_blk->get_offset()) && (i->get_offset() <= cmp_blk->get_csr_end_addr())) ||
                    ((i->get_csr_end_addr() >= cmp_blk->get_offset()) && (i->get_csr_end_addr() <= cmp_blk->get_csr_end_addr())) ) {
                PLOG_ERR( hex << "Overlap :                         Current : " << i->get_hier_path() << " 0x" << i->get_offset() << " - 0x" << i->get_csr_end_addr() << 
                        " With : " << cmp_blk->get_hier_path() << " 0x" << cmp_blk->get_offset() << " - 0x"<< cmp_blk->get_csr_end_addr() << dec << endl);
            }
        }
        PLOG("csr", " path: " << hex << i->get_hier_path() << " address: 0x" << i->get_offset() << " - 0x" << i->get_csr_end_addr() << dec << endl);
    }
}
