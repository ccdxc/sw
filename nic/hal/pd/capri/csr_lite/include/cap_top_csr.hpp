#ifndef CAP_TOP_CSR_H
#define CAP_TOP_CSR_H

#include "nic/gen/csr_lite/include/cap_bx_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_ap_helper.hpp"
#include "nic/gen/csr_lite/include/cap_em_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pxb_decoders_helper.hpp"
#include "nic/gen/csr_lite/include/cap_dpp_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_dppcsum_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_dpphdr_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_dpphdrfld_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_dppmem_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_dppstats_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_dpr_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_dprcfg_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_dprhdrfld_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_dprmem_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_dprstats_csr_helper.hpp"
#include "nic/gen/csr_lite/include/emmc_SDHOST_Memory_Map_helper.hpp"
#include "nic/gen/csr_lite/include/cap_hens_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_hese_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_intr_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_mpns_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_mpse_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_mpu_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_mx_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pbc_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pbchbm_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pbchbmeth_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pbchbmtx_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pbcport0_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pbcport1_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pbcport10_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pbcport11_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pbcport2_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pbcport3_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pbcport4_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pbcport5_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pbcport6_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pbcport7_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pbcport8_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pbcport9_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pbm_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_picc_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pics_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pict_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pp_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_ppa_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pr_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_prd_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_psp_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pt_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_ptd_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pxb_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pxc_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_pxp_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_sc_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_sema_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_sg_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_te_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_txs_csr_helper.hpp"
#include "nic/gen/csr_lite/include/cap_wa_csr_helper.hpp"

#define cap_emmc_csr_t          emmc_SDHOST_Memory_Map_t
#define cap_emmc_csr_helper_t   emmc_SDHOST_Memory_Map_helper_t

typedef enum {
    CAP_PPA_CSR_INST_PPA_0,
    CAP_PPA_CSR_INST_PPA_1,
    CAP_PPA_CSR_INST_MAX
} cap_ppa_csr_inst_id_t;

typedef enum {
    CAP_TE_CSR_INST_SGI_0,
    CAP_TE_CSR_INST_SGI_1,
    CAP_TE_CSR_INST_SGI_2,
    CAP_TE_CSR_INST_SGI_3,
    CAP_TE_CSR_INST_SGI_4,
    CAP_TE_CSR_INST_SGI_5,
    CAP_TE_CSR_INST_SGI_MAX
} cap_te_csr_inst_sgi_id_t;

typedef enum {
    CAP_TE_CSR_INST_SGE_0,
    CAP_TE_CSR_INST_SGE_1,
    CAP_TE_CSR_INST_SGE_2,
    CAP_TE_CSR_INST_SGE_3,
    CAP_TE_CSR_INST_SGE_4,
    CAP_TE_CSR_INST_SGE_5,
    CAP_TE_CSR_INST_SGE_MAX
} cap_te_csr_inst_sge_id_t;

typedef enum {
    CAP_TE_CSR_INST_PCR_0,
    CAP_TE_CSR_INST_PCR_1,
    CAP_TE_CSR_INST_PCR_2,
    CAP_TE_CSR_INST_PCR_3,
    CAP_TE_CSR_INST_PCR_4,
    CAP_TE_CSR_INST_PCR_5,
    CAP_TE_CSR_INST_PCR_6,
    CAP_TE_CSR_INST_PCR_7,
    CAP_TE_CSR_INST_PCR_MAX
} cap_te_csr_inst_pcr_id_t;

typedef enum {
    CAP_TE_CSR_INST_PCT_0,
    CAP_TE_CSR_INST_PCT_1,
    CAP_TE_CSR_INST_PCT_2,
    CAP_TE_CSR_INST_PCT_3,
    CAP_TE_CSR_INST_PCT_4,
    CAP_TE_CSR_INST_PCT_5,
    CAP_TE_CSR_INST_PCT_6,
    CAP_TE_CSR_INST_PCT_7,
    CAP_TE_CSR_INST_PCT_MAX
} cap_te_csr_inst_pct_id_t;

typedef enum {
    CAP_MPU_CSR_INST_SGI_0,
    CAP_MPU_CSR_INST_SGI_1,
    CAP_MPU_CSR_INST_SGI_2,
    CAP_MPU_CSR_INST_SGI_3,
    CAP_MPU_CSR_INST_SGI_4,
    CAP_MPU_CSR_INST_SGI_5,
    CAP_MPU_CSR_INST_SGI_MAX
} cap_mpu_csr_inst_sgi_id_t;

typedef enum {
    CAP_MPU_CSR_INST_SGE_0,
    CAP_MPU_CSR_INST_SGE_1,
    CAP_MPU_CSR_INST_SGE_2,
    CAP_MPU_CSR_INST_SGE_3,
    CAP_MPU_CSR_INST_SGE_4,
    CAP_MPU_CSR_INST_SGE_5,
    CAP_MPU_CSR_INST_SGE_MAX
} cap_mpu_csr_inst_sge_id_t;

typedef enum {
    CAP_MPU_CSR_INST_PCR_0,
    CAP_MPU_CSR_INST_PCR_1,
    CAP_MPU_CSR_INST_PCR_2,
    CAP_MPU_CSR_INST_PCR_3,
    CAP_MPU_CSR_INST_PCR_4,
    CAP_MPU_CSR_INST_PCR_5,
    CAP_MPU_CSR_INST_PCR_6,
    CAP_MPU_CSR_INST_PCR_7,
    CAP_MPU_CSR_INST_PCR_MAX
} cap_mpu_csr_inst_pcr_id_t;

typedef enum {
    CAP_MPU_CSR_INST_PCT_0,
    CAP_MPU_CSR_INST_PCT_1,
    CAP_MPU_CSR_INST_PCT_2,
    CAP_MPU_CSR_INST_PCT_3,
    CAP_MPU_CSR_INST_PCT_4,
    CAP_MPU_CSR_INST_PCT_5,
    CAP_MPU_CSR_INST_PCT_6,
    CAP_MPU_CSR_INST_PCT_7,
    CAP_MPU_CSR_INST_PCT_MAX
} cap_mpu_csr_inst_pct_id_t;

typedef enum {
    CAP_PICS_CSR_INST_RPC,
    CAP_PICS_CSR_INST_SSI,
    CAP_PICS_CSR_INST_SSE,
    CAP_PICS_CSR_INST_TPC,
    CAP_PICS_CSR_INST_MAX
} cap_pics_csr_inst_id_t;

typedef enum {
    CAP_PICT_CSR_INST_TSI,
    CAP_PICT_CSR_INST_TSE,
    CAP_PICT_CSR_INST_MAX
} cap_pict_csr_inst_id_t;

typedef enum {
    CAP_MC_CSR_INST_MC_0,
    CAP_MC_CSR_INST_MC_1,
    CAP_MC_CSR_INST_MC_2,
    CAP_MC_CSR_INST_MC_3,
    CAP_MC_CSR_INST_MC_4,
    CAP_MC_CSR_INST_MC_5,
    CAP_MC_CSR_INST_MC_6,
    CAP_MC_CSR_INST_MC_7,
    CAP_MC_CSR_INST_MAX
} cap_mc_csr_inst_id_t;

typedef enum {
    CAP_DPR_CSR_INST_DPR_0,
    CAP_DPR_CSR_INST_DPR_1,
    CAP_DPR_CSR_INST_MAX
} cap_dpr_csr_inst_id_t;

typedef enum {
    CAP_DPP_CSR_INST_DPP_0,
    CAP_DPP_CSR_INST_DPP_1,
    CAP_DPP_CSR_INST_MAX
} cap_dpp_csr_inst_id_t;

typedef enum {
    CAP_MX_CSR_INST_MX_0,
    CAP_MX_CSR_INST_MX_1,
    CAP_MX_CSR_INST_MAX
} cap_mx_csr_inst_id_t;

typedef enum {
    CAP_INTR_CSR_INST_INTR,
    CAP_INTR_CSR_INST_MAX
} cap_intr_csr_inst_id_t;

typedef enum {
    CAP_PXB_CSR_INST_PXB,
    CAP_PXB_CSR_INST_MAX
} cap_pxb_csr_inst_id_t;

typedef enum {
    CAP_PR_CSR_INST_PR,
    CAP_PR_CSR_INST_MAX
} cap_pr_csr_inst_id_t;

typedef enum {
    CAP_PP_CSR_INST_PP,
    CAP_PP_CSR_INST_MAX
} cap_pp_csr_inst_id_t;

typedef enum {
    CAP_PT_CSR_INST_PT,
    CAP_PT_CSR_INST_MAX
} cap_pt_csr_inst_id_t;

typedef enum {
    CAP_TXS_CSR_INST_TXS,
    CAP_TXS_CSR_INST_MAX
} cap_txs_csr_inst_id_t;

typedef enum {
    CAP_PBC_CSR_INST_PB,
    CAP_PBC_CSR_INST_MAX
} cap_pbc_csr_inst_id_t;

typedef enum {
    CAP_PBM_CSR_INST_PM,
    CAP_PBM_CSR_INST_MAX
} cap_pbm_csr_inst_id_t;

typedef enum {
    CAP_WA_CSR_INST_DB,
    CAP_WA_CSR_INST_MAX
} cap_wa_csr_inst_id_t;

typedef enum {
    CAP_BX_CSR_INST_BX,
    CAP_BX_CSR_INST_MAX
} cap_bx_csr_inst_id_t;

typedef enum {
    CAP_HENS_CSR_INST_MD,
    CAP_HENS_CSR_INST_MAX
} cap_hens_csr_inst_id_t;

typedef enum {
    CAP_HESE_CSR_INST_MD,
    CAP_HESE_CSR_INST_MAX
} cap_hese_csr_inst_id_t;

typedef enum {
    CAP_SEMA_CSR_INST_SEMA,
    CAP_SEMA_CSR_INST_MAX
} cap_sema_csr_inst_id_t;

typedef enum {
    CAP_MPNS_CSR_INST_MP,
    CAP_MPNS_CSR_INST_MAX
} cap_mpns_csr_inst_id_t;

typedef enum {
    CAP_MPSE_CSR_INST_MP,
    CAP_MPSE_CSR_INST_MAX
} cap_mpse_csr_inst_id_t;

typedef enum {
    CAP_APB_CSR_INST_MS,
    CAP_APB_CSR_INST_MAX
} cap_apb_csr_inst_id_t;

typedef enum {
    CAP_RBM_CSR_INST_MS,
    CAP_RBM_CSR_INST_MAX
} cap_rbm_csr_inst_id_t;

typedef enum {
    CAP_SSRAM_CSR_INST_MS,
    CAP_SSRAM_CSR_INST_MAX
} cap_ssram_csr_inst_id_t;

typedef enum {
    CAP_APX_CSR_INST_MS,
    CAP_APX_CSR_INST_MAX
} cap_apx_csr_inst_id_t;

typedef enum {
    CAP_EMMC_CSR_INST_MS,
    CAP_EMMC_CSR_INST_MAX
} cap_emmc_csr_inst_id_t;

typedef enum {
    CAP_ESECURE_CSR_INST_MS,
    CAP_ESECURE_CSR_INST_MAX
} cap_esecure_csr_inst_id_t;

typedef enum {
    CAP_AP2_CSR_INST_MS,
    CAP_AP2_CSR_INST_MAX
} cap_ap2_csr_inst_id_t;

typedef enum {
    CAP_FLASH_CSR_INST_MS,
    CAP_FLASH_CSR_INST_MAX
} cap_flash_csr_inst_id_t;

typedef enum {
    CAP_AP_CSR_INST_MS,
    CAP_AP_CSR_INST_MAX
} cap_ap_csr_inst_id_t;

typedef enum {
    CAP_MS_CSR_INST_MS,
    CAP_MS_CSR_INST_MAX
} cap_ms_csr_inst_id_t;

typedef enum {
    CAP_MSS_CSR_INST_MS,
    CAP_MSS_CSR_INST_MAX
} cap_mss_csr_inst_id_t;

typedef enum {
    CAP_EM_CSR_INST_MS,
    CAP_EM_CSR_INST_MAX
} cap_em_csr_inst_id_t;

class cap_top_csr_helper_t {
public:
    cap_top_csr_helper_t()=default;
    ~cap_top_csr_helper_t()=default;
    void init(void);

    struct {
        cap_ppa_csr_helper_t ppa[CAP_PPA_CSR_INST_MAX];
    } ppa;
    struct {
        cap_te_csr_helper_t te[CAP_TE_CSR_INST_SGI_MAX];
        cap_mpu_csr_helper_t mpu[CAP_MPU_CSR_INST_SGI_MAX];
    } sgi;
    struct {
        cap_pics_csr_helper_t pics;
    } rpc;
    struct {
        cap_intr_csr_helper_t intr;
    } intr;
    struct {
        cap_pxb_csr_helper_t pxb;
    } pxb;
    struct {
        cap_te_csr_helper_t te[CAP_TE_CSR_INST_SGE_MAX];
        cap_mpu_csr_helper_t mpu[CAP_MPU_CSR_INST_SGE_MAX];
    } sge;
    struct {
        cap_pr_csr_helper_t pr;
    } pr;
    struct {
        cap_pp_csr_helper_t pp;
    } pp;
    struct {
        cap_pr_csr_helper_t pt;
    } pt;
    struct {
        cap_pict_csr_helper_t pict;
    } tsi;
    struct {
        cap_te_csr_helper_t te[CAP_TE_CSR_INST_PCR_MAX];
        cap_mpu_csr_helper_t mpu[CAP_MPU_CSR_INST_PCR_MAX];
    } pcr;
    struct {
        cap_txs_csr_helper_t txs;
    } txs;
    struct {
        cap_pict_csr_helper_t pict;
    } tse;
    struct {
        cap_te_csr_helper_t te[CAP_TE_CSR_INST_PCT_MAX];
        cap_mpu_csr_helper_t mpu[CAP_MPU_CSR_INST_PCT_MAX];
    } pct;
    struct {
        cap_pbc_csr_helper_t pbc;
    } pb;
    struct {
        cap_pbm_csr_helper_t pbm;
    } pm;
    struct {
        cap_wa_csr_helper_t wa;
    } db;
    struct {
        cap_pics_csr_helper_t pics;
    } ssi;
    struct {
        cap_pics_csr_helper_t pics;
    } sse;
    struct {
        cap_bx_csr_helper_t bx;
    } bx;
    struct {
        cap_hens_csr_helper_t hens;
        cap_hese_csr_helper_t hese;
    } md;
    struct {
        cap_pics_csr_helper_t pics;
    } tpc;
    struct {
        cap_dpr_csr_helper_t dpr[CAP_DPR_CSR_INST_MAX];
    } dpr;
    struct {
        cap_dpr_csr_helper_t mc[CAP_MC_CSR_INST_MAX];
    } mc;
    struct {
        cap_dpp_csr_helper_t dpp[CAP_DPP_CSR_INST_MAX];
    } dpp;
    struct {
        cap_sema_csr_helper_t sema;
    } sema;
    struct {
        cap_mpns_csr_helper_t mpns;
        cap_mpse_csr_helper_t mpse;
    } mp;
    struct {
        cap_apb_csr_helper_t apb;
        cap_emmc_csr_helper_t emmc;
        cap_ap_csr_helper_t ap;
//TODO        cap_ms_csr_helper_t ms;
//TODO        cap_mss_csr_helper_t mss;
        cap_em_csr_helper_t em;
    } ms;
    struct {
        cap_mx_csr_helper_t mx[CAP_MX_CSR_INST_MAX];
    } mx;
};

extern class cap_top_csr_helper_t cap_top_csr_helper; 

#endif // CAP_TOP_CSR_H
