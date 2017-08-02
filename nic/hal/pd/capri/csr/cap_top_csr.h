
        
#ifndef CAP_TOP_CSR_H
#define CAP_TOP_CSR_H

#include "cap_blk_reg_model.h"
#include "cap_csr_base.h" 
#include "cap_ppa_csr.h"
#include "cap_te_csr.h"
#include "cap_mpu_csr.h"
#include "cap_pics_csr.h"
#include "cap_pict_csr.h"
#include "cap_pxb_csr.h"
#include "cap_te_csr.h"
#include "cap_mpu_csr.h"
#include "cap_pr_csr.h"
#include "cap_pp_csr.h"
#include "cap_pt_csr.h"
#include "cap_pict_csr.h"
#include "cap_te_csr.h"
#include "cap_mpu_csr.h"
#include "cap_txs_csr.h"
#include "cap_pict_csr.h"
#include "cap_te_csr.h"
#include "cap_mpu_csr.h"
#include "cap_pbc_csr.h"
#include "cap_pbm_csr.h"
#include "cap_wa_csr.h"
#include "cap_pics_csr.h"
#include "cap_pics_csr.h"
#include "cap_he_csr.h"
#include "cap_pics_csr.h"
#include "cap_pict_csr.h"
#include "cap_dpr_csr.h"
#include "cap_dpp_csr.h"

using namespace std;
class cap_top_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_top_csr_t(string name = "cap_top_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_top_csr_t();
        virtual void init(int chip_id);
        
    class cap_ppa_lgrp_csr_t : public cap_block_base {
    public:
        cap_ppa_lgrp_csr_t(string name = "cap_ppa_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_ppa_csr_t ppa[2];
    }; // cap_ppa_lgrp_csr_t

    cap_ppa_lgrp_csr_t ppa;
    
    
    class cap_sgi_lgrp_csr_t : public cap_block_base {
    public:
        cap_sgi_lgrp_csr_t(string name = "cap_sgi_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_sgi_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_te_csr_t te[6];
        cap_mpu_csr_t mpu[6];
    }; // cap_sgi_lgrp_csr_t

    cap_sgi_lgrp_csr_t sgi;
    
    
    class cap_rpc_lgrp_csr_t : public cap_block_base {
    public:
        cap_rpc_lgrp_csr_t(string name = "cap_rpc_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_rpc_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pics_csr_t pics;
        cap_pict_csr_t pict;
    }; // cap_rpc_lgrp_csr_t

    cap_rpc_lgrp_csr_t rpc;
    
    
    class cap_pxb_lgrp_csr_t : public cap_block_base {
    public:
        cap_pxb_lgrp_csr_t(string name = "cap_pxb_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pxb_csr_t pxb;
    }; // cap_pxb_lgrp_csr_t

    cap_pxb_lgrp_csr_t pxb;
    
    
    class cap_sge_lgrp_csr_t : public cap_block_base {
    public:
        cap_sge_lgrp_csr_t(string name = "cap_sge_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_sge_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_te_csr_t te[6];
        cap_mpu_csr_t mpu[6];
    }; // cap_sge_lgrp_csr_t

    cap_sge_lgrp_csr_t sge;
    
    
    class cap_pr_lgrp_csr_t : public cap_block_base {
    public:
        cap_pr_lgrp_csr_t(string name = "cap_pr_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pr_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pr_csr_t pr;
    }; // cap_pr_lgrp_csr_t

    cap_pr_lgrp_csr_t pr;
    
    
    class cap_pp_lgrp_csr_t : public cap_block_base {
    public:
        cap_pp_lgrp_csr_t(string name = "cap_pp_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pp_csr_t pp;
    }; // cap_pp_lgrp_csr_t

    cap_pp_lgrp_csr_t pp;
    
    
    class cap_pt_lgrp_csr_t : public cap_block_base {
    public:
        cap_pt_lgrp_csr_t(string name = "cap_pt_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pt_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pt_csr_t pt;
    }; // cap_pt_lgrp_csr_t

    cap_pt_lgrp_csr_t pt;
    
    
    class cap_tsi_lgrp_csr_t : public cap_block_base {
    public:
        cap_tsi_lgrp_csr_t(string name = "cap_tsi_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_tsi_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pict_csr_t pict;
    }; // cap_tsi_lgrp_csr_t

    cap_tsi_lgrp_csr_t tsi;
    
    
    class cap_pcr_lgrp_csr_t : public cap_block_base {
    public:
        cap_pcr_lgrp_csr_t(string name = "cap_pcr_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pcr_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_te_csr_t te[8];
        cap_mpu_csr_t mpu[8];
    }; // cap_pcr_lgrp_csr_t

    cap_pcr_lgrp_csr_t pcr;
    
    
    class cap_txs_lgrp_csr_t : public cap_block_base {
    public:
        cap_txs_lgrp_csr_t(string name = "cap_txs_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_txs_csr_t txs;
    }; // cap_txs_lgrp_csr_t

    cap_txs_lgrp_csr_t txs;
    
    
    class cap_tse_lgrp_csr_t : public cap_block_base {
    public:
        cap_tse_lgrp_csr_t(string name = "cap_tse_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_tse_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pict_csr_t pict;
    }; // cap_tse_lgrp_csr_t

    cap_tse_lgrp_csr_t tse;
    
    
    class cap_pct_lgrp_csr_t : public cap_block_base {
    public:
        cap_pct_lgrp_csr_t(string name = "cap_pct_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pct_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_te_csr_t te[8];
        cap_mpu_csr_t mpu[8];
    }; // cap_pct_lgrp_csr_t

    cap_pct_lgrp_csr_t pct;
    
    
    class cap_pb_lgrp_csr_t : public cap_block_base {
    public:
        cap_pb_lgrp_csr_t(string name = "cap_pb_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pb_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pbc_csr_t pbc;
    }; // cap_pb_lgrp_csr_t

    cap_pb_lgrp_csr_t pb;
    
    
    class cap_pm_lgrp_csr_t : public cap_block_base {
    public:
        cap_pm_lgrp_csr_t(string name = "cap_pm_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pm_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pbm_csr_t pbm;
    }; // cap_pm_lgrp_csr_t

    cap_pm_lgrp_csr_t pm;
    
    
    class cap_db_lgrp_csr_t : public cap_block_base {
    public:
        cap_db_lgrp_csr_t(string name = "cap_db_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_db_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_wa_csr_t wa;
    }; // cap_db_lgrp_csr_t

    cap_db_lgrp_csr_t db;
    
    
    class cap_ssi_lgrp_csr_t : public cap_block_base {
    public:
        cap_ssi_lgrp_csr_t(string name = "cap_ssi_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_ssi_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pics_csr_t pics;
    }; // cap_ssi_lgrp_csr_t

    cap_ssi_lgrp_csr_t ssi;
    
    
    class cap_sse_lgrp_csr_t : public cap_block_base {
    public:
        cap_sse_lgrp_csr_t(string name = "cap_sse_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_sse_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pics_csr_t pics;
    }; // cap_sse_lgrp_csr_t

    cap_sse_lgrp_csr_t sse;
    
    
    class cap_he_lgrp_csr_t : public cap_block_base {
    public:
        cap_he_lgrp_csr_t(string name = "cap_he_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_he_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_he_csr_t he;
    }; // cap_he_lgrp_csr_t

    cap_he_lgrp_csr_t he;
    
    
    class cap_tpc_lgrp_csr_t : public cap_block_base {
    public:
        cap_tpc_lgrp_csr_t(string name = "cap_tpc_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_tpc_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pics_csr_t pics;
        cap_pict_csr_t pict;
    }; // cap_tpc_lgrp_csr_t

    cap_tpc_lgrp_csr_t tpc;
    
    
    class cap_dpr_lgrp_csr_t : public cap_block_base {
    public:
        cap_dpr_lgrp_csr_t(string name = "cap_dpr_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_dpr_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_dpr_csr_t dpr[2];
    }; // cap_dpr_lgrp_csr_t

    cap_dpr_lgrp_csr_t dpr;
    
    
    class cap_dpp_lgrp_csr_t : public cap_block_base {
    public:
        cap_dpp_lgrp_csr_t(string name = "cap_dpp_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_dpp_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_dpp_csr_t dpp[2];
    }; // cap_dpp_lgrp_csr_t

    cap_dpp_lgrp_csr_t dpp;
    
    
};        
#endif // CAP_TOP_CSR_H
        