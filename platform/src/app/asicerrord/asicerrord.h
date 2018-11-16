/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "logger.h"
#include "nic/asic/capri/model/cap_top/cap_top_csr_defines.h"
#include "nic/asic/capri/model/cap_top/csr_defines/cap_dpp_c_hdr.h"
#include "nic/asic/capri/model/cap_top/csr_defines/cap_dpr_c_hdr.h"
#include "nic/asic/capri/model/cap_top/csr_defines/cap_pics_c_hdr.h"
#include "nic/asic/capri/model/cap_top/csr_defines/cap_wa_c_hdr.h"
#include "nic/asic/capri/model/cap_top/csr_defines/cap_te_c_hdr.h"
#include "nic/asic/capri/model/cap_top/csr_defines/cap_mpu_c_hdr.h"
#include "nic/asic/capri/model/cap_top/csr_defines/cap_hens_c_hdr.h"
#include "nic/asic/capri/model/cap_top/csr_defines/cap_mpns_c_hdr.h"
#include "nic/asic/capri/model/cap_top/csr_defines/cap_pbc_c_hdr.h"
#include "nic/asic/capri/model/cap_top/csr_defines/cap_pbm_c_hdr.h"
#include "nic/asic/capri/model/cap_top/csr_defines/cap_mc_c_hdr.h"
#include "gen/proto/asicerrord.delphi.hpp"

#define DPP0_INT_CREDIT CAP_ADDR_BASE_DPP_DPP_0_OFFSET + CAP_DPP_CSR_INT_CREDIT_INTREG_BYTE_ADDRESS
#define DPP1_INT_CREDIT CAP_ADDR_BASE_DPP_DPP_1_OFFSET + CAP_DPP_CSR_INT_CREDIT_INTREG_BYTE_ADDRESS
#define DPP0_INT_FIFO CAP_ADDR_BASE_DPP_DPP_0_OFFSET + CAP_DPP_CSR_INT_FIFO_INTREG_BYTE_ADDRESS
#define DPP1_INT_FIFO CAP_ADDR_BASE_DPP_DPP_1_OFFSET + CAP_DPP_CSR_INT_FIFO_INTREG_BYTE_ADDRESS
#define DPP0_INT_REG1 CAP_ADDR_BASE_DPP_DPP_0_OFFSET + CAP_DPP_CSR_INT_REG1_INTREG_BYTE_ADDRESS
#define DPP0_INT_REG2 CAP_ADDR_BASE_DPP_DPP_0_OFFSET + CAP_DPP_CSR_INT_REG2_INTREG_BYTE_ADDRESS
#define DPP1_INT_REG1 CAP_ADDR_BASE_DPP_DPP_1_OFFSET + CAP_DPP_CSR_INT_REG1_INTREG_BYTE_ADDRESS
#define DPP1_INT_REG2 CAP_ADDR_BASE_DPP_DPP_1_OFFSET + CAP_DPP_CSR_INT_REG2_INTREG_BYTE_ADDRESS
#define DPP0_INT_SPARE CAP_ADDR_BASE_DPP_DPP_0_OFFSET + CAP_DPP_CSR_INT_SPARE_INTREG_BYTE_ADDRESS
#define DPP1_INT_SPARE CAP_ADDR_BASE_DPP_DPP_1_OFFSET + CAP_DPP_CSR_INT_SPARE_INTREG_BYTE_ADDRESS
#define DPP0_INT_SRAMS_ECC CAP_ADDR_BASE_DPP_DPP_0_OFFSET + CAP_DPP_CSR_INT_SRAMS_ECC_INTREG_BYTE_ADDRESS
#define DPP1_INT_SRAMS_ECC CAP_ADDR_BASE_DPP_DPP_1_OFFSET + CAP_DPP_CSR_INT_SRAMS_ECC_INTREG_BYTE_ADDRESS
#define DPR0_INT_CREDIT CAP_ADDR_BASE_DPR_DPR_0_OFFSET + CAP_DPR_CSR_INT_CREDIT_INTREG_BYTE_ADDRESS
#define DPR1_INT_CREDIT CAP_ADDR_BASE_DPR_DPR_1_OFFSET + CAP_DPR_CSR_INT_CREDIT_INTREG_BYTE_ADDRESS
#define DPR0_INT_FIFO CAP_ADDR_BASE_DPR_DPR_0_OFFSET + CAP_DPR_CSR_INT_FIFO_INTREG_BYTE_ADDRESS
#define DPR1_INT_FIFO CAP_ADDR_BASE_DPR_DPR_1_OFFSET + CAP_DPR_CSR_INT_FIFO_INTREG_BYTE_ADDRESS
#define DPR0_INT_FLOP_FIFO CAP_ADDR_BASE_DPR_DPR_0_OFFSET + CAP_DPR_CSR_INT_FLOP_FIFO_0_INTREG_BYTE_ADDRESS
#define DPR1_INT_FLOP_FIFO CAP_ADDR_BASE_DPR_DPR_1_OFFSET + CAP_DPR_CSR_INT_FLOP_FIFO_1_INTREG_BYTE_ADDRESS
#define DPR0_INT_REG1 CAP_ADDR_BASE_DPR_DPR_0_OFFSET + CAP_DPR_CSR_INT_REG1_INTREG_BYTE_ADDRESS
#define DPR0_INT_REG2 CAP_ADDR_BASE_DPR_DPR_0_OFFSET + CAP_DPR_CSR_INT_REG2_INTREG_BYTE_ADDRESS
#define DPR1_INT_REG1 CAP_ADDR_BASE_DPR_DPR_1_OFFSET + CAP_DPR_CSR_INT_REG1_INTREG_BYTE_ADDRESS
#define DPR1_INT_REG2 CAP_ADDR_BASE_DPR_DPR_1_OFFSET + CAP_DPR_CSR_INT_REG2_INTREG_BYTE_ADDRESS
#define DPR0_INT_SPARE CAP_ADDR_BASE_DPR_DPR_0_OFFSET + CAP_DPR_CSR_INT_SPARE_INTREG_BYTE_ADDRESS
#define DPR1_INT_SPARE CAP_ADDR_BASE_DPR_DPR_1_OFFSET + CAP_DPR_CSR_INT_SPARE_INTREG_BYTE_ADDRESS
#define DPR0_INT_SRAMS_ECC CAP_ADDR_BASE_DPR_DPR_0_OFFSET + CAP_DPR_CSR_INT_SRAMS_ECC_INTREG_BYTE_ADDRESS
#define DPR1_INT_SRAMS_ECC CAP_ADDR_BASE_DPR_DPR_1_OFFSET + CAP_DPR_CSR_INT_SRAMS_ECC_INTREG_BYTE_ADDRESS
#define SSE_PICS_INT_BADADDR CAP_ADDR_BASE_SSE_PICS_OFFSET + CAP_PICS_CSR_INT_BADADDR_INTREG_BYTE_ADDRESS
#define SSE_PICS_INT_BG CAP_ADDR_BASE_SSE_PICS_OFFSET + CAP_PICS_CSR_INT_BG_INTREG_BYTE_ADDRESS
#define SSE_PICS_INT_PICS CAP_ADDR_BASE_SSE_PICS_OFFSET + CAP_PICS_CSR_INT_PICS_INTREG_BYTE_ADDRESS
#define DB_WA_INT_DB CAP_ADDR_BASE_DB_WA_OFFSET + CAP_WA_CSR_INT_DB_INTREG_BYTE_ADDRESS
#define DB_WA_INT_LIF_QSTATE_MAP CAP_ADDR_BASE_DB_WA_OFFSET + CAP_WA_CSR_INT_LIF_QSTATE_MAP_INTREG_BYTE_ADDRESS
#define SGE_TE0_INT_ERR CAP_ADDR_BASE_SGE_TE_0_OFFSET + CAP_TE_CSR_INT_ERR_INTREG_BYTE_ADDRESS
#define SGE_TE1_INT_ERR CAP_ADDR_BASE_SGE_TE_1_OFFSET + CAP_TE_CSR_INT_ERR_INTREG_BYTE_ADDRESS
#define SGE_TE2_INT_ERR CAP_ADDR_BASE_SGE_TE_2_OFFSET + CAP_TE_CSR_INT_ERR_INTREG_BYTE_ADDRESS
#define SGE_TE3_INT_ERR CAP_ADDR_BASE_SGE_TE_3_OFFSET + CAP_TE_CSR_INT_ERR_INTREG_BYTE_ADDRESS
#define SGE_TE4_INT_ERR CAP_ADDR_BASE_SGE_TE_4_OFFSET + CAP_TE_CSR_INT_ERR_INTREG_BYTE_ADDRESS
#define SGE_TE5_INT_ERR CAP_ADDR_BASE_SGE_TE_5_OFFSET + CAP_TE_CSR_INT_ERR_INTREG_BYTE_ADDRESS
#define SGE_TE0_INT_INFO CAP_ADDR_BASE_SGE_TE_0_OFFSET + CAP_TE_CSR_INT_INFO_INTREG_BYTE_ADDRESS
#define SGE_TE1_INT_INFO CAP_ADDR_BASE_SGE_TE_1_OFFSET + CAP_TE_CSR_INT_INFO_INTREG_BYTE_ADDRESS
#define SGE_TE2_INT_INFO CAP_ADDR_BASE_SGE_TE_2_OFFSET + CAP_TE_CSR_INT_INFO_INTREG_BYTE_ADDRESS
#define SGE_TE3_INT_INFO CAP_ADDR_BASE_SGE_TE_3_OFFSET + CAP_TE_CSR_INT_INFO_INTREG_BYTE_ADDRESS
#define SGE_TE4_INT_INFO CAP_ADDR_BASE_SGE_TE_4_OFFSET + CAP_TE_CSR_INT_INFO_INTREG_BYTE_ADDRESS
#define SGE_TE5_INT_INFO CAP_ADDR_BASE_SGE_TE_5_OFFSET + CAP_TE_CSR_INT_INFO_INTREG_BYTE_ADDRESS
#define SGE_MPU0_INT_ERR CAP_ADDR_BASE_SGE_MPU_0_OFFSET + CAP_MPU_CSR_INT_ERR_INTREG_BYTE_ADDRESS
#define SGE_MPU1_INT_ERR CAP_ADDR_BASE_SGE_MPU_1_OFFSET + CAP_MPU_CSR_INT_ERR_INTREG_BYTE_ADDRESS
#define SGE_MPU2_INT_ERR CAP_ADDR_BASE_SGE_MPU_2_OFFSET + CAP_MPU_CSR_INT_ERR_INTREG_BYTE_ADDRESS
#define SGE_MPU3_INT_ERR CAP_ADDR_BASE_SGE_MPU_3_OFFSET + CAP_MPU_CSR_INT_ERR_INTREG_BYTE_ADDRESS
#define SGE_MPU4_INT_ERR CAP_ADDR_BASE_SGE_MPU_4_OFFSET + CAP_MPU_CSR_INT_ERR_INTREG_BYTE_ADDRESS
#define SGE_MPU5_INT_ERR CAP_ADDR_BASE_SGE_MPU_5_OFFSET + CAP_MPU_CSR_INT_ERR_INTREG_BYTE_ADDRESS
#define SGE_MPU0_INT_INFO CAP_ADDR_BASE_SGE_MPU_0_OFFSET + CAP_MPU_CSR_INT_INFO_INTREG_BYTE_ADDRESS
#define SGE_MPU1_INT_INFO CAP_ADDR_BASE_SGE_MPU_1_OFFSET + CAP_MPU_CSR_INT_INFO_INTREG_BYTE_ADDRESS
#define SGE_MPU2_INT_INFO CAP_ADDR_BASE_SGE_MPU_2_OFFSET + CAP_MPU_CSR_INT_INFO_INTREG_BYTE_ADDRESS
#define SGE_MPU3_INT_INFO CAP_ADDR_BASE_SGE_MPU_3_OFFSET + CAP_MPU_CSR_INT_INFO_INTREG_BYTE_ADDRESS
#define SGE_MPU4_INT_INFO CAP_ADDR_BASE_SGE_MPU_4_OFFSET + CAP_MPU_CSR_INT_INFO_INTREG_BYTE_ADDRESS
#define SGE_MPU5_INT_INFO CAP_ADDR_BASE_SGE_MPU_5_OFFSET + CAP_MPU_CSR_INT_INFO_INTREG_BYTE_ADDRESS
#define MD_HENS_INT_AXI_ERR CAP_ADDR_BASE_MD_HENS_OFFSET + CAP_HENS_CSR_INT_AXI_ERR_INTREG_BYTE_ADDRESS
#define MD_HENS_INT_DRBG_CRYPTORAM_ECC CAP_ADDR_BASE_MD_HENS_OFFSET + CAP_HENS_CSR_INT_DRBG_CRYPTORAM_ECC_INTREG_BYTE_ADDRESS
#define MD_HENS_INT_DRBG_INTRAM_ECC CAP_ADDR_BASE_MD_HENS_OFFSET + CAP_HENS_CSR_INT_DRBG_INTRAM_ECC_INTREG_BYTE_ADDRESS
#define MD_HENS_INT_IPCORE CAP_ADDR_BASE_MD_HENS_OFFSET + CAP_HENS_CSR_INT_IPCORE_INTREG_BYTE_ADDRESS
#define MD_HENS_INT_PK_ECC CAP_ADDR_BASE_MD_HENS_OFFSET + CAP_HENS_CSR_INT_PK_ECC_INTREG_BYTE_ADDRESS
#define MP_MPNS_INT_CRYPTO CAP_ADDR_BASE_MP_MPNS_OFFSET + CAP_MPNS_CSR_INT_CRYPTO_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_CREDIT_UNDERFLOW CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_CREDIT_UNDERFLOW_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_DESC_0 CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_ECC_DESC_0_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_DESC_1 CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_ECC_DESC_1_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_FC_0 CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_ECC_FC_0_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_FC_1 CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_ECC_FC_1_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_LL_0 CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_ECC_LL_0_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_LL_1 CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_ECC_LL_1_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_NC CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_ECC_NC_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_PACK CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_ECC_PACK_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_PORT_MON_IN CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_ECC_PORT_MON_IN_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_PORT_MON_OUT CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_ECC_PORT_MON_OUT_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_RC CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_ECC_RC_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_RWR CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_ECC_RWR_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_SCHED CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_ECC_SCHED_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_SIDEBAND CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_ECC_SIDEBAND_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_UC CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_ECC_UC_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_PBUS_VIOLATION_IN CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_PBUS_VIOLATION_IN_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_PBUS_VIOLATION_OUT CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_PBUS_VIOLATION_OUT_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_RPL CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_RPL_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_WRITE_0 CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_WRITE_0_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_WRITE_1 CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_WRITE_1_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_WRITE_2 CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_WRITE_2_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_WRITE_3 CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_WRITE_3_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_WRITE_4 CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_WRITE_4_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_WRITE_5 CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_WRITE_5_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_WRITE_6 CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_WRITE_6_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_WRITE_7 CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_WRITE_7_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_WRITE_8 CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_WRITE_8_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_WRITE_9 CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_WRITE_9_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_WRITE_10 CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_WRITE_10_INTREG_BYTE_ADDRESS
#define PB_PBC_INT_ECC_WRITE_11 CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_INT_WRITE_11_INTREG_BYTE_ADDRESS
#define PB_PBC_HBM_INT_ECC_HBM_HT CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_HBM_INT_ECC_HBM_HT_INTREG_BYTE_ADDRESS
#define PB_PBC_HBM_INT_ECC_HBM_MTU CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_HBM_INT_ECC_HBM_MTU_INTREG_BYTE_ADDRESS
#define PB_PBC_HBM_INT_ECC_HBM_RB CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_HBM_INT_ECC_HBM_RB_INTREG_BYTE_ADDRESS
#define PB_PBC_HBM_INT_ECC_HBM_WB CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_HBM_INT_ECC_HBM_WB_INTREG_BYTE_ADDRESS
#define PB_PBC_HBM_INT_ECC_HBM_AXI_ERR_RSP CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_HBM_INT_HBM_AXI_ERR_RSP_INTREG_BYTE_ADDRESS
#define PB_PBC_HBM_INT_ECC_HBM_DROP CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_HBM_INT_HBM_DROP_INTREG_BYTE_ADDRESS
#define PB_PBC_HBM_INT_ECC_HBM_PBUS_VIOLATION_IN CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_HBM_INT_HBM_PBUS_VIOLATION_IN_INTREG_BYTE_ADDRESS
#define PB_PBC_HBM_INT_ECC_HBM_PBUS_VIOLATION_OUT CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_HBM_INT_HBM_PBUS_VIOLATION_OUT_INTREG_BYTE_ADDRESS
#define PB_PBC_HBM_INT_ECC_HBM_XOFF CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_HBM_INT_HBM_XOFF_INTREG_BYTE_ADDRESS
#define PM_PBM_INT_ECC_COL0 CAP_ADDR_BASE_PM_PBM_OFFSET + CAP_PBM_CSR_INT_ECC_COL0_INTREG_BYTE_ADDRESS
#define PM_PBM_INT_ECC_COL1 CAP_ADDR_BASE_PM_PBM_OFFSET + CAP_PBM_CSR_INT_ECC_COL1_INTREG_BYTE_ADDRESS
#define PM_PBM_INT_ECC_COL2 CAP_ADDR_BASE_PM_PBM_OFFSET + CAP_PBM_CSR_INT_ECC_COL2_INTREG_BYTE_ADDRESS
#define PM_PBM_INT_ECC_COL3 CAP_ADDR_BASE_PM_PBM_OFFSET + CAP_PBM_CSR_INT_ECC_COL3_INTREG_BYTE_ADDRESS
#define PM_PBM_INT_ECC_COL4 CAP_ADDR_BASE_PM_PBM_OFFSET + CAP_PBM_CSR_INT_ECC_COL4_INTREG_BYTE_ADDRESS
#define MC0_MCH_INT_MC_INTREG CAP_ADDR_BASE_MC_MC_0_OFFSET + CAP_MC_CSR_MCH_INT_MC_INTREG_BYTE_ADDRESS
#define MC1_MCH_INT_MC_INTREG CAP_ADDR_BASE_MC_MC_1_OFFSET + CAP_MC_CSR_MCH_INT_MC_INTREG_BYTE_ADDRESS
#define MC2_MCH_INT_MC_INTREG CAP_ADDR_BASE_MC_MC_2_OFFSET + CAP_MC_CSR_MCH_INT_MC_INTREG_BYTE_ADDRESS
#define MC3_MCH_INT_MC_INTREG CAP_ADDR_BASE_MC_MC_3_OFFSET + CAP_MC_CSR_MCH_INT_MC_INTREG_BYTE_ADDRESS
#define MC4_MCH_INT_MC_INTREG CAP_ADDR_BASE_MC_MC_4_OFFSET + CAP_MC_CSR_MCH_INT_MC_INTREG_BYTE_ADDRESS
#define MC5_MCH_INT_MC_INTREG CAP_ADDR_BASE_MC_MC_5_OFFSET + CAP_MC_CSR_MCH_INT_MC_INTREG_BYTE_ADDRESS
#define MC6_MCH_INT_MC_INTREG CAP_ADDR_BASE_MC_MC_6_OFFSET + CAP_MC_CSR_MCH_INT_MC_INTREG_BYTE_ADDRESS
#define MC7_MCH_INT_MC_INTREG CAP_ADDR_BASE_MC_MC_7_OFFSET + CAP_MC_CSR_MCH_INT_MC_INTREG_BYTE_ADDRESS

extern delphi::objects::dppintcreditmetrics_t                  dppintcreditmetrics[2];
extern delphi::objects::dppintfifometrics_t                    dppintfifometrics[2];
extern delphi::objects::dppintreg1metrics_t                    dppintreg1metrics[2];
extern delphi::objects::dppintreg2metrics_t                    dppintreg2metrics[2];
extern delphi::objects::intsparemetrics_t                      intsparemetrics[4];
extern delphi::objects::dppintsramseccmetrics_t                dppintsramseccmetrics[2];
extern delphi::objects::dprintcreditmetrics_t                  dprintcreditmetrics[2];
extern delphi::objects::dprintfifometrics_t                    dprintfifometrics[2];
extern delphi::objects::dprintflopfifometrics_t                dprintflopfifometrics[2];
extern delphi::objects::dprintreg1metrics_t                    dprintreg1metrics[2];
extern delphi::objects::dprintreg2metrics_t                    dprintreg2metrics[2];
extern delphi::objects::dprintsramseccmetrics_t                dprintsramseccmetrics[2];
extern delphi::objects::ssepicsintbadaddrmetrics_t             ssepicsintbadaddrmetrics[1];
extern delphi::objects::ssepicsintbgmetrics_t                  ssepicsintbgmetrics[1];
extern delphi::objects::ssepicsintpicsmetrics_t                ssepicsintpicsmetrics[1];
extern delphi::objects::dbwaintdbmetrics_t                     dbwaintdbmetrics[1];
extern delphi::objects::dbwaintlifqstatemapmetrics_t           dbwaintlifqstatemapmetrics[1];
extern delphi::objects::sgeteinterrmetrics_t                   sgeteinterrmetrics[6];
extern delphi::objects::sgeteintinfometrics_t                  sgeteintinfometrics[6];
extern delphi::objects::sgempuinterrmetrics_t                  sgempuinterrmetrics[6];
extern delphi::objects::sgempuintinfometrics_t                 sgempuintinfometrics[6];
extern delphi::objects::mdhensintaxierrmetrics_t               mdhensintaxierrmetrics[1];
extern delphi::objects::mdhensinteccmetrics_t                  mdhensinteccmetrics[3];
extern delphi::objects::mdhensintipcoremetrics_t               mdhensintipcoremetrics[1];
extern delphi::objects::mpmpnsintcryptometrics_t               mpmpnsintcryptometrics[1];
extern delphi::objects::pbpbcintcreditunderflowmetrics_t       pbpbcintcreditunderflowmetrics[1];
extern delphi::objects::inteccdescmetrics_t                    inteccdescmetrics[24];
extern delphi::objects::pbpbcintpbusviolationmetrics_t         pbpbcintpbusviolationmetrics[2];
extern delphi::objects::pbpbcintrplmetrics_t                   pbpbcintrplmetrics[1];
extern delphi::objects::pbpbcintwritemetrics_t                 pbpbcintwritemetrics[12];
extern delphi::objects::pbpbchbmintecchbmrbmetrics_t           pbpbchbmintecchbmrbmetrics[1];
extern delphi::objects::pbpbchbminthbmaxierrrspmetrics_t       pbpbchbminthbmaxierrrspmetrics[1];
extern delphi::objects::pbpbchbminthbmdropmetrics_t            pbpbchbminthbmdropmetrics[1];
extern delphi::objects::pbpbchbminthbmpbusviolationmetrics_t   pbpbchbminthbmpbusviolationmetrics[2];
extern delphi::objects::pbpbchbminthbmxoffmetrics_t            pbpbchbminthbmxoffmetrics[1];
extern delphi::objects::mcmchintmcmetrics_t                    mcmchintmcmetrics[8];

enum etype {
    ERROR = 0,
    INFO = 1,
    FATAL = 2,
    UNKNOWN = 3,
};

void poll_capri_intr();
const char* errortostring(etype errortype);

#define CAPRI_INTR_KIND_BEGIN(kind, len, classkind) static inline void clear_##kind##metrics(uint32_t key, uint32_t addr) { \
    uint32_t size = 0; \
    uint32_t data = 0; \
    if (len % 32 == 0) { \
        size = len / 32; \
    } else { \
        size =  len / 32 + 1; \
    } \
    sdk::lib::pal_ret_t rc = sdk::lib::pal_reg_read(addr, &data, size); \
    if (rc == sdk::lib::PAL_RET_NOK) { \
        INFO("unable to read the interrupt failed"); \
        return; \
    } \
    rc = sdk::lib::pal_reg_write(addr, &data, size); \
    if (rc == sdk::lib::PAL_RET_NOK) { \
        INFO("clearing the interrupt failed"); \
    } \
    memset(&kind##metrics[key], 0, sizeof(delphi::objects::kind##metrics_t)); \
    delphi::objects::classkind::Publish(key, &kind##metrics[key]); \
} \
static inline void poll_##kind##metrics(uint32_t key, uint32_t addr) { \
    uint32_t size = 0; \
    uint32_t data = 0; \
    uint32_t regkey = key; \
    uint32_t regaddr = addr; \
    delphi::objects::kind##metrics_t *reg = &kind##metrics[key]; \
    char regname[50] = #kind; \
    if (len % 32 == 0) { \
        size = len / 32; \
    } else { \
        size =  len / 32 + 1; \
    } \
    if(addr == 0) { \
        return; \
    } \
    sdk::lib::pal_ret_t rc = sdk::lib::pal_reg_read(addr, &data, size); \
    if (rc == sdk::lib::PAL_RET_NOK) { \
        return; \
    } \

#define CAPRI_INTR_KIND_FIELD(fld, offset, type) { \
    if (data & (1 << offset)) { \
        reg->fld++; \
        INFO("Register {} key {} at address {:x} interrupt {} type {} times {}", \
             regname, regkey, regaddr, #fld, errortostring(type),reg->fld); \
    } \
}

#define CAPRI_INTR_KIND_END(classkind) \
    rc = sdk::lib::pal_reg_write(addr, &data, size); \
    if (rc == sdk::lib::PAL_RET_NOK) { \
        INFO("clearing the interrupt failed"); \
    } \
    delphi::objects::classkind::Publish(regkey, reg); \
}

#define CAPRI_INTR_READ(kind, key, addr) poll_##kind(key, addr);

#define CAPRI_INTR_CLEAR(kind, key, addr) clear_##kind(key, addr);
