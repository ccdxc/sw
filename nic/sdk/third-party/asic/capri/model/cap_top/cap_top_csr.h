
        
#ifndef CAP_TOP_CSR_H
#define CAP_TOP_CSR_H

#include "cap_blk_reg_model.h"
#include "cap_csr_base.h" 
using namespace std;
#include "cap_ppa_csr.h"
#include "cap_pict_csr.h"
#include "cap_wa_csr.h"
#include "cap_mpu_csr.h"
#include "cap_pbm_csr.h"
#include "cap_emmc_csr.h"
#include "cap_pxb_csr.h"
#include "cap_te_csr.h"
#include "cap_txs_csr.h"
#include "cap_hese_csr.h"
#include "cap_ms_csr.h"
#include "cap_apb_csr.h"
#include "cap_pt_csr.h"
#include "cap_mpns_csr.h"
#include "cap_bx_csr.h"
#include "cap_em_csr.h"
#include "cap_mc_csr.h"
#include "cap_pbc_csr.h"
#include "cap_intr_csr.h"
#include "cap_mss_csr.h"
#include "cap_pp_csr.h"
#include "cap_pr_csr.h"
#include "cap_sema_csr.h"
#include "cap_ap_csr.h"
#include "cap_hens_csr.h"
#include "cap_dpr_csr.h"
#include "cap_dpp_csr.h"
#include "cap_mx_csr.h"
#include "cap_pics_csr.h"
#include "cap_mpse_csr.h"

    class cap_ppa_lgrp_csr_t : public cap_block_base {
    public:
        cap_ppa_lgrp_csr_t(string name = "cap_ppa_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_ppa_csr_t ppa[2];
    }; // cap_ppa_lgrp_csr_t
    
    class cap_sgi_lgrp_csr_t : public cap_block_base {
    public:
        cap_sgi_lgrp_csr_t(string name = "cap_sgi_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_sgi_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_te_csr_t te[6];
        cap_mpu_csr_t mpu[6];
    }; // cap_sgi_lgrp_csr_t
    
    class cap_rpc_lgrp_csr_t : public cap_block_base {
    public:
        cap_rpc_lgrp_csr_t(string name = "cap_rpc_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_rpc_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pics_csr_t pics;
    }; // cap_rpc_lgrp_csr_t
    
    class cap_intr_lgrp_csr_t : public cap_block_base {
    public:
        cap_intr_lgrp_csr_t(string name = "cap_intr_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_intr_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_intr_csr_t intr;
    }; // cap_intr_lgrp_csr_t
    
    class cap_pxb_lgrp_csr_t : public cap_block_base {
    public:
        cap_pxb_lgrp_csr_t(string name = "cap_pxb_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pxb_csr_t pxb;
    }; // cap_pxb_lgrp_csr_t
    
    class cap_sge_lgrp_csr_t : public cap_block_base {
    public:
        cap_sge_lgrp_csr_t(string name = "cap_sge_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_sge_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_te_csr_t te[6];
        cap_mpu_csr_t mpu[6];
    }; // cap_sge_lgrp_csr_t
    
    class cap_pr_lgrp_csr_t : public cap_block_base {
    public:
        cap_pr_lgrp_csr_t(string name = "cap_pr_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pr_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pr_csr_t pr;
    }; // cap_pr_lgrp_csr_t
    
    class cap_pp_lgrp_csr_t : public cap_block_base {
    public:
        cap_pp_lgrp_csr_t(string name = "cap_pp_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pp_csr_t pp;
    }; // cap_pp_lgrp_csr_t
    
    class cap_pt_lgrp_csr_t : public cap_block_base {
    public:
        cap_pt_lgrp_csr_t(string name = "cap_pt_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pt_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pt_csr_t pt;
    }; // cap_pt_lgrp_csr_t
    
    class cap_tsi_lgrp_csr_t : public cap_block_base {
    public:
        cap_tsi_lgrp_csr_t(string name = "cap_tsi_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_tsi_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pict_csr_t pict;
    }; // cap_tsi_lgrp_csr_t
    
    class cap_pcr_lgrp_csr_t : public cap_block_base {
    public:
        cap_pcr_lgrp_csr_t(string name = "cap_pcr_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pcr_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_te_csr_t te[8];
        cap_mpu_csr_t mpu[8];
    }; // cap_pcr_lgrp_csr_t
    
    class cap_txs_lgrp_csr_t : public cap_block_base {
    public:
        cap_txs_lgrp_csr_t(string name = "cap_txs_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_txs_csr_t txs;
    }; // cap_txs_lgrp_csr_t
    
    class cap_tse_lgrp_csr_t : public cap_block_base {
    public:
        cap_tse_lgrp_csr_t(string name = "cap_tse_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_tse_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pict_csr_t pict;
    }; // cap_tse_lgrp_csr_t
    
    class cap_pct_lgrp_csr_t : public cap_block_base {
    public:
        cap_pct_lgrp_csr_t(string name = "cap_pct_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pct_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_te_csr_t te[8];
        cap_mpu_csr_t mpu[8];
    }; // cap_pct_lgrp_csr_t
    
    class cap_pb_lgrp_csr_t : public cap_block_base {
    public:
        cap_pb_lgrp_csr_t(string name = "cap_pb_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pb_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pbc_csr_t pbc;
    }; // cap_pb_lgrp_csr_t
    
    class cap_pm_lgrp_csr_t : public cap_block_base {
    public:
        cap_pm_lgrp_csr_t(string name = "cap_pm_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pm_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pbm_csr_t pbm;
    }; // cap_pm_lgrp_csr_t
    
    class cap_db_lgrp_csr_t : public cap_block_base {
    public:
        cap_db_lgrp_csr_t(string name = "cap_db_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_db_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_wa_csr_t wa;
    }; // cap_db_lgrp_csr_t
    
    class cap_ssi_lgrp_csr_t : public cap_block_base {
    public:
        cap_ssi_lgrp_csr_t(string name = "cap_ssi_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_ssi_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pics_csr_t pics;
    }; // cap_ssi_lgrp_csr_t
    
    class cap_sse_lgrp_csr_t : public cap_block_base {
    public:
        cap_sse_lgrp_csr_t(string name = "cap_sse_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_sse_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pics_csr_t pics;
    }; // cap_sse_lgrp_csr_t
    
    class cap_bx_lgrp_csr_t : public cap_block_base {
    public:
        cap_bx_lgrp_csr_t(string name = "cap_bx_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_bx_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_bx_csr_t bx;
    }; // cap_bx_lgrp_csr_t
    
    class cap_md_lgrp_csr_t : public cap_block_base {
    public:
        cap_md_lgrp_csr_t(string name = "cap_md_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_md_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_hens_csr_t hens;
        cap_hese_csr_t hese;
    }; // cap_md_lgrp_csr_t
    
    class cap_tpc_lgrp_csr_t : public cap_block_base {
    public:
        cap_tpc_lgrp_csr_t(string name = "cap_tpc_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_tpc_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_pics_csr_t pics;
    }; // cap_tpc_lgrp_csr_t
    
    class cap_dpr_lgrp_csr_t : public cap_block_base {
    public:
        cap_dpr_lgrp_csr_t(string name = "cap_dpr_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_dpr_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_dpr_csr_t dpr[2];
    }; // cap_dpr_lgrp_csr_t
    
    class cap_mc_lgrp_csr_t : public cap_block_base {
    public:
        cap_mc_lgrp_csr_t(string name = "cap_mc_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_mc_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_mc_csr_t mc[8];
    }; // cap_mc_lgrp_csr_t
    
    class cap_dpp_lgrp_csr_t : public cap_block_base {
    public:
        cap_dpp_lgrp_csr_t(string name = "cap_dpp_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_dpp_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_dpp_csr_t dpp[2];
    }; // cap_dpp_lgrp_csr_t
    
    class cap_sema_lgrp_csr_t : public cap_block_base {
    public:
        cap_sema_lgrp_csr_t(string name = "cap_sema_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_sema_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_sema_csr_t sema;
    }; // cap_sema_lgrp_csr_t
    
    class cap_mp_lgrp_csr_t : public cap_block_base {
    public:
        cap_mp_lgrp_csr_t(string name = "cap_mp_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_mp_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_mpns_csr_t mpns;
        cap_mpse_csr_t mpse;
    }; // cap_mp_lgrp_csr_t
    
    class cap_ms_lgrp_csr_t : public cap_block_base {
    public:
        cap_ms_lgrp_csr_t(string name = "cap_ms_lgrp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_ms_lgrp_csr_t();
        virtual void init();
        virtual void register_model(int chip_id);
    
        cap_apb_csr_t apb;
        cap_emmc_csr_t emmc;
        cap_ap_csr_t ap;
        cap_ms_csr_t ms;
        cap_mss_csr_t mss;
        cap_em_csr_t em;
    }; // cap_ms_lgrp_csr_t
    
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
        
    cap_ppa_lgrp_csr_t ppa;
    cap_sgi_lgrp_csr_t sgi;
    cap_rpc_lgrp_csr_t rpc;
    cap_intr_lgrp_csr_t intr;
    cap_pxb_lgrp_csr_t pxb;
    cap_sge_lgrp_csr_t sge;
    cap_pr_lgrp_csr_t pr;
    cap_pp_lgrp_csr_t pp;
    cap_pt_lgrp_csr_t pt;
    cap_tsi_lgrp_csr_t tsi;
    cap_pcr_lgrp_csr_t pcr;
    cap_txs_lgrp_csr_t txs;
    cap_tse_lgrp_csr_t tse;
    cap_pct_lgrp_csr_t pct;
    cap_pb_lgrp_csr_t pb;
    cap_pm_lgrp_csr_t pm;
    cap_db_lgrp_csr_t db;
    cap_ssi_lgrp_csr_t ssi;
    cap_sse_lgrp_csr_t sse;
    cap_bx_lgrp_csr_t bx;
    cap_md_lgrp_csr_t md;
    cap_tpc_lgrp_csr_t tpc;
    cap_dpr_lgrp_csr_t dpr;
    cap_mc_lgrp_csr_t mc;
    cap_dpp_lgrp_csr_t dpp;
    cap_sema_lgrp_csr_t sema;
    cap_mp_lgrp_csr_t mp;
    cap_ms_lgrp_csr_t ms;
    cap_mx_lgrp_csr_t mx;
};        
#endif // CAP_TOP_CSR_H
        