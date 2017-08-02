
#include "cap_top_csr.h"

cap_top_csr_t::cap_ppa_lgrp_csr_t::cap_ppa_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_ppa_lgrp_csr_t::~cap_ppa_lgrp_csr_t() { } 

void cap_top_csr_t::cap_ppa_lgrp_csr_t::init() {

    ppa[0].set_attributes(this,"ppa[0]",  0x3400000);
        
    ppa[1].set_attributes(this,"ppa[1]",  0x3480000);
        
} // cap_ppa_lgrp_csr_t::init()

void cap_top_csr_t::cap_ppa_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_ppa_csr_t,chip_id, 0, &ppa[0]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_ppa_csr_t,chip_id, 1, &ppa[1]);
        
} // cap_ppa_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_sgi_lgrp_csr_t::cap_sgi_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_sgi_lgrp_csr_t::~cap_sgi_lgrp_csr_t() { } 

void cap_top_csr_t::cap_sgi_lgrp_csr_t::init() {

    te[0].set_attributes(this,"te[0]",  0x35c0000);
        
    te[1].set_attributes(this,"te[1]",  0x35e0000);
        
    te[2].set_attributes(this,"te[2]",  0x3600000);
        
    te[3].set_attributes(this,"te[3]",  0x3620000);
        
    te[4].set_attributes(this,"te[4]",  0x3640000);
        
    te[5].set_attributes(this,"te[5]",  0x3660000);
        
    mpu[0].set_attributes(this,"mpu[0]",  0x36e0000);
        
    mpu[1].set_attributes(this,"mpu[1]",  0x36f0000);
        
    mpu[2].set_attributes(this,"mpu[2]",  0x3700000);
        
    mpu[3].set_attributes(this,"mpu[3]",  0x3710000);
        
    mpu[4].set_attributes(this,"mpu[4]",  0x3720000);
        
    mpu[5].set_attributes(this,"mpu[5]",  0x3730000);
        
} // cap_sgi_lgrp_csr_t::init()

void cap_top_csr_t::cap_sgi_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 0, &te[0]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 1, &te[1]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 2, &te[2]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 3, &te[3]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 4, &te[4]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 5, &te[5]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 0, &mpu[0]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 1, &mpu[1]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 2, &mpu[2]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 3, &mpu[3]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 4, &mpu[4]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 5, &mpu[5]);
        
} // cap_sgi_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_rpc_lgrp_csr_t::cap_rpc_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_rpc_lgrp_csr_t::~cap_rpc_lgrp_csr_t() { } 

void cap_top_csr_t::cap_rpc_lgrp_csr_t::init() {

    pics.set_attributes(this,"pics",  0x4800000);
        
    pict.set_attributes(this,"pict",  0x5800000);
        
} // cap_rpc_lgrp_csr_t::init()

void cap_top_csr_t::cap_rpc_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_pics_csr_t,chip_id, 0, &pics);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_pict_csr_t,chip_id, 0, &pict);
        
} // cap_rpc_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_pxb_lgrp_csr_t::cap_pxb_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_pxb_lgrp_csr_t::~cap_pxb_lgrp_csr_t() { } 

void cap_top_csr_t::cap_pxb_lgrp_csr_t::init() {

    pxb.set_attributes(this,"pxb",  0x7100000);
        
} // cap_pxb_lgrp_csr_t::init()

void cap_top_csr_t::cap_pxb_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_pxb_csr_t,chip_id, 0, &pxb);
        
} // cap_pxb_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_sge_lgrp_csr_t::cap_sge_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_sge_lgrp_csr_t::~cap_sge_lgrp_csr_t() { } 

void cap_top_csr_t::cap_sge_lgrp_csr_t::init() {

    te[0].set_attributes(this,"te[0]",  0x3500000);
        
    te[1].set_attributes(this,"te[1]",  0x3520000);
        
    te[2].set_attributes(this,"te[2]",  0x3540000);
        
    te[3].set_attributes(this,"te[3]",  0x3560000);
        
    te[4].set_attributes(this,"te[4]",  0x3580000);
        
    te[5].set_attributes(this,"te[5]",  0x35a0000);
        
    mpu[0].set_attributes(this,"mpu[0]",  0x3680000);
        
    mpu[1].set_attributes(this,"mpu[1]",  0x3690000);
        
    mpu[2].set_attributes(this,"mpu[2]",  0x36a0000);
        
    mpu[3].set_attributes(this,"mpu[3]",  0x36b0000);
        
    mpu[4].set_attributes(this,"mpu[4]",  0x36c0000);
        
    mpu[5].set_attributes(this,"mpu[5]",  0x36d0000);
        
} // cap_sge_lgrp_csr_t::init()

void cap_top_csr_t::cap_sge_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 6, &te[0]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 7, &te[1]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 8, &te[2]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 9, &te[3]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 10, &te[4]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 11, &te[5]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 6, &mpu[0]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 7, &mpu[1]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 8, &mpu[2]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 9, &mpu[3]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 10, &mpu[4]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 11, &mpu[5]);
        
} // cap_sge_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_pr_lgrp_csr_t::cap_pr_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_pr_lgrp_csr_t::~cap_pr_lgrp_csr_t() { } 

void cap_top_csr_t::cap_pr_lgrp_csr_t::init() {

    pr.set_attributes(this,"pr",  0x6100000);
        
} // cap_pr_lgrp_csr_t::init()

void cap_top_csr_t::cap_pr_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_pr_csr_t,chip_id, 0, &pr);
        
} // cap_pr_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_pp_lgrp_csr_t::cap_pp_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_pp_lgrp_csr_t::~cap_pp_lgrp_csr_t() { } 

void cap_top_csr_t::cap_pp_lgrp_csr_t::init() {

    pp.set_attributes(this,"pp",  0x7000000);
        
} // cap_pp_lgrp_csr_t::init()

void cap_top_csr_t::cap_pp_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_pp_csr_t,chip_id, 0, &pp);
        
} // cap_pp_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_pt_lgrp_csr_t::cap_pt_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_pt_lgrp_csr_t::~cap_pt_lgrp_csr_t() { } 

void cap_top_csr_t::cap_pt_lgrp_csr_t::init() {

    pt.set_attributes(this,"pt",  0x6200000);
        
} // cap_pt_lgrp_csr_t::init()

void cap_top_csr_t::cap_pt_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_pt_csr_t,chip_id, 0, &pt);
        
} // cap_pt_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_tsi_lgrp_csr_t::cap_tsi_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_tsi_lgrp_csr_t::~cap_tsi_lgrp_csr_t() { } 

void cap_top_csr_t::cap_tsi_lgrp_csr_t::init() {

    pict.set_attributes(this,"pict",  0x3800000);
        
} // cap_tsi_lgrp_csr_t::init()

void cap_top_csr_t::cap_tsi_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_pict_csr_t,chip_id, 1, &pict);
        
} // cap_tsi_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_pcr_lgrp_csr_t::cap_pcr_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_pcr_lgrp_csr_t::~cap_pcr_lgrp_csr_t() { } 

void cap_top_csr_t::cap_pcr_lgrp_csr_t::init() {

    te[0].set_attributes(this,"te[0]",  0x6220000);
        
    te[1].set_attributes(this,"te[1]",  0x6240000);
        
    te[2].set_attributes(this,"te[2]",  0x6260000);
        
    te[3].set_attributes(this,"te[3]",  0x6280000);
        
    te[4].set_attributes(this,"te[4]",  0x62a0000);
        
    te[5].set_attributes(this,"te[5]",  0x62c0000);
        
    te[6].set_attributes(this,"te[6]",  0x62e0000);
        
    te[7].set_attributes(this,"te[7]",  0x6300000);
        
    mpu[0].set_attributes(this,"mpu[0]",  0x6420000);
        
    mpu[1].set_attributes(this,"mpu[1]",  0x6430000);
        
    mpu[2].set_attributes(this,"mpu[2]",  0x6440000);
        
    mpu[3].set_attributes(this,"mpu[3]",  0x6450000);
        
    mpu[4].set_attributes(this,"mpu[4]",  0x6460000);
        
    mpu[5].set_attributes(this,"mpu[5]",  0x6470000);
        
    mpu[6].set_attributes(this,"mpu[6]",  0x6480000);
        
    mpu[7].set_attributes(this,"mpu[7]",  0x6490000);
        
} // cap_pcr_lgrp_csr_t::init()

void cap_top_csr_t::cap_pcr_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 12, &te[0]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 13, &te[1]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 14, &te[2]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 15, &te[3]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 16, &te[4]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 17, &te[5]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 18, &te[6]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 19, &te[7]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 12, &mpu[0]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 13, &mpu[1]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 14, &mpu[2]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 15, &mpu[3]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 16, &mpu[4]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 17, &mpu[5]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 18, &mpu[6]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 19, &mpu[7]);
        
} // cap_pcr_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_txs_lgrp_csr_t::cap_txs_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_txs_lgrp_csr_t::~cap_txs_lgrp_csr_t() { } 

void cap_top_csr_t::cap_txs_lgrp_csr_t::init() {

    txs.set_attributes(this,"txs",  0x20000000);
        
} // cap_txs_lgrp_csr_t::init()

void cap_top_csr_t::cap_txs_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_txs_csr_t,chip_id, 0, &txs);
        
} // cap_txs_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_tse_lgrp_csr_t::cap_tse_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_tse_lgrp_csr_t::~cap_tse_lgrp_csr_t() { } 

void cap_top_csr_t::cap_tse_lgrp_csr_t::init() {

    pict.set_attributes(this,"pict",  0x3C00000);
        
} // cap_tse_lgrp_csr_t::init()

void cap_top_csr_t::cap_tse_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_pict_csr_t,chip_id, 2, &pict);
        
} // cap_tse_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_pct_lgrp_csr_t::cap_pct_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_pct_lgrp_csr_t::~cap_pct_lgrp_csr_t() { } 

void cap_top_csr_t::cap_pct_lgrp_csr_t::init() {

    te[0].set_attributes(this,"te[0]",  0x6320000);
        
    te[1].set_attributes(this,"te[1]",  0x6340000);
        
    te[2].set_attributes(this,"te[2]",  0x6360000);
        
    te[3].set_attributes(this,"te[3]",  0x6380000);
        
    te[4].set_attributes(this,"te[4]",  0x63a0000);
        
    te[5].set_attributes(this,"te[5]",  0x63c0000);
        
    te[6].set_attributes(this,"te[6]",  0x63e0000);
        
    te[7].set_attributes(this,"te[7]",  0x6400000);
        
    mpu[0].set_attributes(this,"mpu[0]",  0x64a0000);
        
    mpu[1].set_attributes(this,"mpu[1]",  0x64b0000);
        
    mpu[2].set_attributes(this,"mpu[2]",  0x64c0000);
        
    mpu[3].set_attributes(this,"mpu[3]",  0x64d0000);
        
    mpu[4].set_attributes(this,"mpu[4]",  0x64e0000);
        
    mpu[5].set_attributes(this,"mpu[5]",  0x64f0000);
        
    mpu[6].set_attributes(this,"mpu[6]",  0x6500000);
        
    mpu[7].set_attributes(this,"mpu[7]",  0x6510000);
        
} // cap_pct_lgrp_csr_t::init()

void cap_top_csr_t::cap_pct_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 20, &te[0]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 21, &te[1]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 22, &te[2]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 23, &te[3]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 24, &te[4]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 25, &te[5]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 26, &te[6]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_te_csr_t,chip_id, 27, &te[7]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 20, &mpu[0]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 21, &mpu[1]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 22, &mpu[2]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 23, &mpu[3]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 24, &mpu[4]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 25, &mpu[5]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 26, &mpu[6]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_mpu_csr_t,chip_id, 27, &mpu[7]);
        
} // cap_pct_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_pb_lgrp_csr_t::cap_pb_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_pb_lgrp_csr_t::~cap_pb_lgrp_csr_t() { } 

void cap_top_csr_t::cap_pb_lgrp_csr_t::init() {

    pbc.set_attributes(this,"pbc",  0x1400000);
        
} // cap_pb_lgrp_csr_t::init()

void cap_top_csr_t::cap_pb_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_pbc_csr_t,chip_id, 0, &pbc);
        
} // cap_pb_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_pm_lgrp_csr_t::cap_pm_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_pm_lgrp_csr_t::~cap_pm_lgrp_csr_t() { } 

void cap_top_csr_t::cap_pm_lgrp_csr_t::init() {

    pbm.set_attributes(this,"pbm",  0x1060000);
        
} // cap_pm_lgrp_csr_t::init()

void cap_top_csr_t::cap_pm_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_pbm_csr_t,chip_id, 0, &pbm);
        
} // cap_pm_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_db_lgrp_csr_t::cap_db_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_db_lgrp_csr_t::~cap_db_lgrp_csr_t() { } 

void cap_top_csr_t::cap_db_lgrp_csr_t::init() {

    wa.set_attributes(this,"wa",  0x68000000);
        
} // cap_db_lgrp_csr_t::init()

void cap_top_csr_t::cap_db_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_wa_csr_t,chip_id, 0, &wa);
        
} // cap_db_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_ssi_lgrp_csr_t::cap_ssi_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_ssi_lgrp_csr_t::~cap_ssi_lgrp_csr_t() { } 

void cap_top_csr_t::cap_ssi_lgrp_csr_t::init() {

    pics.set_attributes(this,"pics",  0x2000000);
        
} // cap_ssi_lgrp_csr_t::init()

void cap_top_csr_t::cap_ssi_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_pics_csr_t,chip_id, 1, &pics);
        
} // cap_ssi_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_sse_lgrp_csr_t::cap_sse_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_sse_lgrp_csr_t::~cap_sse_lgrp_csr_t() { } 

void cap_top_csr_t::cap_sse_lgrp_csr_t::init() {

    pics.set_attributes(this,"pics",  0x2800000);
        
} // cap_sse_lgrp_csr_t::init()

void cap_top_csr_t::cap_sse_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_pics_csr_t,chip_id, 2, &pics);
        
} // cap_sse_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_he_lgrp_csr_t::cap_he_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_he_lgrp_csr_t::~cap_he_lgrp_csr_t() { } 

void cap_top_csr_t::cap_he_lgrp_csr_t::init() {

    he.set_attributes(this,"he",  0x1C00000);
        
} // cap_he_lgrp_csr_t::init()

void cap_top_csr_t::cap_he_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_he_csr_t,chip_id, 0, &he);
        
} // cap_he_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_tpc_lgrp_csr_t::cap_tpc_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_tpc_lgrp_csr_t::~cap_tpc_lgrp_csr_t() { } 

void cap_top_csr_t::cap_tpc_lgrp_csr_t::init() {

    pics.set_attributes(this,"pics",  0x4000000);
        
    pict.set_attributes(this,"pict",  0x5000000);
        
} // cap_tpc_lgrp_csr_t::init()

void cap_top_csr_t::cap_tpc_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_pics_csr_t,chip_id, 3, &pics);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_pict_csr_t,chip_id, 3, &pict);
        
} // cap_tpc_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_dpr_lgrp_csr_t::cap_dpr_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_dpr_lgrp_csr_t::~cap_dpr_lgrp_csr_t() { } 

void cap_top_csr_t::cap_dpr_lgrp_csr_t::init() {

    dpr[0].set_attributes(this,"dpr[0]",  0x3200000);
        
    dpr[1].set_attributes(this,"dpr[1]",  0x3300000);
        
} // cap_dpr_lgrp_csr_t::init()

void cap_top_csr_t::cap_dpr_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_dpr_csr_t,chip_id, 0, &dpr[0]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_dpr_csr_t,chip_id, 1, &dpr[1]);
        
} // cap_dpr_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_dpp_lgrp_csr_t::cap_dpp_lgrp_csr_t(string name, cap_csr_base * parent) :cap_block_base(name, parent) {}
cap_top_csr_t::cap_dpp_lgrp_csr_t::~cap_dpp_lgrp_csr_t() { } 

void cap_top_csr_t::cap_dpp_lgrp_csr_t::init() {

    dpp[0].set_attributes(this,"dpp[0]",  0x3000000);
        
    dpp[1].set_attributes(this,"dpp[1]",  0x3100000);
        
} // cap_dpp_lgrp_csr_t::init()

void cap_top_csr_t::cap_dpp_lgrp_csr_t::register_model(int chip_id) {

    CAP_BLK_REG_MODEL_REGISTER(cap_dpp_csr_t,chip_id, 0, &dpp[0]);
        
    CAP_BLK_REG_MODEL_REGISTER(cap_dpp_csr_t,chip_id, 1, &dpp[1]);
        
} // cap_dpp_lgrp_csr_t::register_model(int chip_id)

cap_top_csr_t::cap_top_csr_t(string name , cap_csr_base * parent) : 
cap_block_base(name, parent) { }
cap_top_csr_t::~cap_top_csr_t() { }
void cap_top_csr_t::init(int chip_id) { 

    ppa.set_attributes(this, "ppa", 0);
    ppa.register_model(chip_id); 
    sgi.set_attributes(this, "sgi", 0);
    sgi.register_model(chip_id); 
    rpc.set_attributes(this, "rpc", 0);
    rpc.register_model(chip_id); 
    pxb.set_attributes(this, "pxb", 0);
    pxb.register_model(chip_id); 
    sge.set_attributes(this, "sge", 0);
    sge.register_model(chip_id); 
    pr.set_attributes(this, "pr", 0);
    pr.register_model(chip_id); 
    pp.set_attributes(this, "pp", 0);
    pp.register_model(chip_id); 
    pt.set_attributes(this, "pt", 0);
    pt.register_model(chip_id); 
    tsi.set_attributes(this, "tsi", 0);
    tsi.register_model(chip_id); 
    pcr.set_attributes(this, "pcr", 0);
    pcr.register_model(chip_id); 
    txs.set_attributes(this, "txs", 0);
    txs.register_model(chip_id); 
    tse.set_attributes(this, "tse", 0);
    tse.register_model(chip_id); 
    pct.set_attributes(this, "pct", 0);
    pct.register_model(chip_id); 
    pb.set_attributes(this, "pb", 0);
    pb.register_model(chip_id); 
    pm.set_attributes(this, "pm", 0);
    pm.register_model(chip_id); 
    db.set_attributes(this, "db", 0);
    db.register_model(chip_id); 
    ssi.set_attributes(this, "ssi", 0);
    ssi.register_model(chip_id); 
    sse.set_attributes(this, "sse", 0);
    sse.register_model(chip_id); 
    he.set_attributes(this, "he", 0);
    he.register_model(chip_id); 
    tpc.set_attributes(this, "tpc", 0);
    tpc.register_model(chip_id); 
    dpr.set_attributes(this, "dpr", 0);
    dpr.register_model(chip_id); 
    dpp.set_attributes(this, "dpp", 0);
    dpp.register_model(chip_id); 
}
