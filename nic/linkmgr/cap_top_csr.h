// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __LINKMGR_CAP_TOP_CSR_H__
#define __LINKMGR_CAP_TOP_CSR_H__

#include "cap_blk_reg_model.h"
#include "cap_csr_base.h" 
using namespace std;
#include "cap_mx_csr.h"

class cap_mx_lgrp_csr_t : public cap_block_base {
public:
    cap_mx_lgrp_csr_t(string name = "cap_mx_lgrp_csr_t", cap_csr_base *parent = 0);
    virtual ~cap_mx_lgrp_csr_t();
    virtual void init();
    virtual void register_model(int chip_id);

    cap_mx_csr_t mx[2];
}; // cap_mx_lgrp_csr_t

class cap_top_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
        cap_top_csr_t(string name = "cap_top_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_top_csr_t();
        virtual void init(int chip_id);

        cap_mx_lgrp_csr_t mx;
};
#endif // __LINKMGR_CAP_TOP_CSR_H__

