/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <inttypes.h>
#include <sys/types.h>

#include "pal.h"

/*
 * ASIC C headers from $SW/nic/asic/capri/model/cap_top
 */
//#include "cap_top_csr.h"
#include "cap_top_csr_defines.h"
#include "cap_ms_c_hdr.h"
#include "cap_mpu_c_hdr.h"
#include "cap_txs_c_hdr.h"
#include "cap_wa_c_hdr.h"
#include "cap_prd_c_hdr.h"
namespace psp {
#include "cap_psp_c_hdr.h"
}
namespace ptd {
#include "cap_ptd_c_hdr.h"
}
#include "cap_te_c_hdr.h"
#include "cap_pt_c_hdr.h"
#include "cap_pics_c_hdr.h"
#include "cap_pxb_c_hdr.h"
#include "cap_ms_c_hdr.h"

// ms.csr.pp
// pic/pcis.csr.pp
// csr_axi_be_mon.csr.pp


#define TXDMA 0
#define RXDMA 1
#define P4SGI 2
#define P4SGE 3

#define TABLE_PCI 1
#define TABLE_SRAM 2
#define TABLE_HBM 3

void mpu_read_counters(int, u_int32_t, int);
void sdp_read_counters(int, u_int32_t);
void te_read_counters(int, u_int32_t);
void mpu_reset_counters(u_int32_t, int);
void mpu_reset_all_counters();
void sta_stg_poll(int, u_int32_t, int);
void mpu_read_table_addr(int, u_int32_t, int);
void sched_read_counters(int, int);    
void doorbell_read_counters(int, int);
void ptd_read_counters(int, int);
void prd_read_counters(int, int);
void psp_read_counters(int, int);
void npv_read_counters(int, int);
void axi_bw_mon(int, u_int32_t);

int
main(int argc, char *argv[])
{
  u_int32_t sta_ver, pipe_base, stage_base;
  int mpu, pipeline, stage;
  int verbose=0;
  int i=1;
  int polls=100;
  while(i < (argc)) {
    if(strcmp(argv[i], "-r")==0) {
      printf("Resetting all MPU performance counters\n");
      mpu_reset_all_counters();
    }
    else if(strcmp(argv[i], "-v")==0) {
      verbose=1;
      printf("Verbose output mode\n");
    }
    else {
      printf("usage: capmon -r[eset counters] -v[erbose] \n");
      return(0);
    }
    i++;
  }
  
    printf("****CAPMON****\n");

    sta_ver = pal_reg_rd32(CAP_ADDR_BASE_MS_MS_OFFSET +
                           CAP_MS_CSR_STA_VER_BYTE_OFFSET);

    printf("Type:    %d\n", sta_ver & 0xf);
    printf("Build:   %d\n", (sta_ver >> 16) & 0xffff);
    printf("Version: %d\n", (sta_ver >> 4) & 0xfff);

    printf("==Doorbells==\n");
    doorbell_read_counters(verbose, polls);    
    printf("==TX Scheduler==\n");
    sched_read_counters(verbose, polls);    
    
    for(pipeline=0; pipeline<4; pipeline++) {
      switch(pipeline) {
      case TXDMA: 
	printf("==TXDMA PIPELINE==\n");
	npv_read_counters(verbose, polls);    	
	break;
      case RXDMA: 
	printf("==RXDMA PIPELINE==\n");
	psp_read_counters(verbose, polls);    	
	break;
      case P4SGI: 
	printf("==P4 INGRESS PIPELINE==\n");
	break;
      case P4SGE: 
	printf("==P4 EGRESS PIPELINE==\n");
	break;
      }
      pipe_base = ((pipeline==TXDMA) ? CAP_ADDR_BASE_PCT_MPU_0_OFFSET :
		   (pipeline==P4SGI) ? CAP_ADDR_BASE_SGI_MPU_0_OFFSET :
		   (pipeline==P4SGE) ? CAP_ADDR_BASE_SGE_MPU_0_OFFSET :
		   (pipeline==RXDMA) ? CAP_ADDR_BASE_PCR_MPU_0_OFFSET : 0);
      // Visit each Stage in pipeline
      for(stage=0; stage<(((pipeline==TXDMA)|(pipeline==RXDMA)) ? 8 : 6); stage++) {
	printf(" Stage %d", stage);
	if(verbose==1) {
	  printf("\n");
	}
	stage_base = (pipe_base + 
		      (CAP_ADDR_BASE_PCR_MPU_1_OFFSET -
		       CAP_ADDR_BASE_PCR_MPU_0_OFFSET) * stage);
	sta_stg_poll(verbose, stage_base, polls);
	te_read_counters(verbose, stage_base);
	sdp_read_counters(verbose, stage_base);
	// Visit each MPU in state
	for(mpu=0; mpu<4; mpu++) {
	  mpu_read_counters(verbose, stage_base, mpu);
	  if(verbose==1) {
	    mpu_read_table_addr(verbose, stage_base, mpu);
	  }
	}
      }

      switch(pipeline) {
      case TXDMA: 
	printf(" TxDMA Engine:");
	ptd_read_counters(verbose, polls);
	break;
      case RXDMA: 
	printf(" RxDMA Engine:");
	prd_read_counters(verbose, polls);    
	break;
      case P4SGI: 
	break;
      case P4SGE: 
	break;
      }

    }
    // Bandwidth Monitors:
    printf("==AXI==\n");
    printf("RXD AXI:");
    axi_bw_mon(verbose, (CAP_ADDR_BASE_RPC_PICS_OFFSET +
			 CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));
    printf("TXD AXI:");
    axi_bw_mon(verbose, (CAP_ADDR_BASE_TPC_PICS_OFFSET +
			 CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));
    printf("SI  AXI:");
    axi_bw_mon(verbose, (CAP_ADDR_BASE_SSI_PICS_OFFSET +
			 CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));
    printf("SE  AXI:");
    axi_bw_mon(verbose, (CAP_ADDR_BASE_SSE_PICS_OFFSET +
			 CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));
    printf("MS AXI: ");
    axi_bw_mon(verbose, (CAP_ADDR_BASE_MS_MS_OFFSET +
			 CAP_MS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));
    printf("PXB AXI:");
    axi_bw_mon(verbose, (CAP_ADDR_BASE_PXB_PXB_OFFSET +
			 CAP_PXB_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));
    //    printf("SG  AXI:");
    //    axi_bw_mon(verbose, (CAP_ADDR_BASE_PXB_PXB_OFFSET +
    //			 CAP_SG_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));


    
    exit(0);
}

void mpu_read_counters(int verbose, u_int32_t stage_base, int mpu_id)
{
  u_int32_t mpu_offset = mpu_id * 4; // word array
  u_int32_t inst_executed = pal_reg_rd32(mpu_offset + stage_base +
				       CAP_MPU_CSR_CNT_INST_EXECUTED_BYTE_OFFSET);
  u_int32_t icache_miss = pal_reg_rd32(mpu_offset + stage_base +
				       CAP_MPU_CSR_CNT_ICACHE_MISS_BYTE_OFFSET);
  u_int32_t icache_fill_stall = pal_reg_rd32(mpu_offset + stage_base +
				       CAP_MPU_CSR_CNT_ICACHE_FILL_STALL_BYTE_OFFSET);
  u_int32_t cycles = pal_reg_rd32(mpu_offset + stage_base +
				       CAP_MPU_CSR_CNT_CYCLES_BYTE_OFFSET);
  u_int32_t phv_executed = pal_reg_rd32(mpu_offset + stage_base +
				       CAP_MPU_CSR_CNT_PHV_EXECUTED_BYTE_OFFSET);
  u_int32_t hazard_stall = pal_reg_rd32(mpu_offset + stage_base +
				       CAP_MPU_CSR_CNT_HAZARD_STALL_BYTE_OFFSET);
  u_int32_t phvwr_stall = pal_reg_rd32(mpu_offset + stage_base +
				       CAP_MPU_CSR_CNT_PHVWR_STALL_BYTE_OFFSET);
  u_int32_t memwr_stall = pal_reg_rd32(mpu_offset + stage_base +
				       CAP_MPU_CSR_CNT_MEMWR_STALL_BYTE_OFFSET);
  u_int32_t tblwr_stall = pal_reg_rd32(mpu_offset + stage_base +
				       CAP_MPU_CSR_CNT_TBLWR_STALL_BYTE_OFFSET);
  u_int32_t fence_stall = pal_reg_rd32(mpu_offset + stage_base +
				       CAP_MPU_CSR_CNT_FENCE_STALL_BYTE_OFFSET);
  if(verbose==1) {
    printf("  mpu %d cycles=%d",mpu_id,cycles);
    printf(" inst=%d",inst_executed);
    printf(" miss=%d", icache_miss);
    printf(" istl=%d", icache_fill_stall);
    printf(" phv=%d",phv_executed);
    printf(" hzrd=%d",hazard_stall);
    printf(" phvwr_stl=%d",phvwr_stall);
    printf(" memwr_stl=%d",memwr_stall);
    printf(" tblwr_stl=%d",tblwr_stall);
    printf(" fence_stl=%d\n",fence_stall);
  }
  if(verbose==1) {
    if(cycles==0) {cycles = 1;}
    printf("  mpu %d percentages", mpu_id);
    printf(" inst=%d%%",(inst_executed * 100) / cycles);
    printf(" miss=%d%%",(icache_miss * 100) / cycles);
    printf(" istl=%d%%", (icache_fill_stall * 100) / cycles);
    printf(" phv=%d%%", (phv_executed * 100) / cycles);
    printf(" hzrd=%d%%", (hazard_stall * 100) / cycles);
    printf(" phvwr_stl=%d%%", (phvwr_stall * 100) / cycles);
    printf(" memwr_stl=%d%%", (memwr_stall * 100) / cycles);
    printf(" tblwr_stl=%d%%", (tblwr_stall * 100) / cycles);
    printf(" fence_stl=%d%%\n", (fence_stall * 100) / cycles);
  }
}

void mpu_reset_all_counters()
{
  u_int32_t pipe_base, stage_base;
  int mpu, pipeline, stage;
  
  for(pipeline=0; pipeline<4; pipeline++) {
    pipe_base = ((pipeline==TXDMA) ? CAP_ADDR_BASE_PCT_MPU_0_OFFSET :
		 (pipeline==P4SGI) ? CAP_ADDR_BASE_SGI_MPU_0_OFFSET :
		 (pipeline==P4SGE) ? CAP_ADDR_BASE_SGE_MPU_0_OFFSET :
		 (pipeline==RXDMA) ? CAP_ADDR_BASE_PCR_MPU_0_OFFSET : 0);
    // Visit each Stage in pipeline
    for(stage=0; stage<(((pipeline==TXDMA)|(pipeline==RXDMA)) ? 8 : 6); stage++) {
      stage_base = (pipe_base + 
		    (CAP_ADDR_BASE_PCR_MPU_1_OFFSET -
		     CAP_ADDR_BASE_PCR_MPU_0_OFFSET) * stage);
      // Visit each MPU in state
      for(mpu=0; mpu<4; mpu++) {
	mpu_reset_counters(stage_base, mpu);
      }
    }
  }
}


void mpu_reset_counters(u_int32_t stage_base, int mpu_id)
{
  u_int32_t mpu_offset = mpu_id * 4; // word array
  pal_reg_wr32(mpu_offset + stage_base +
	       CAP_MPU_CSR_CNT_INST_EXECUTED_BYTE_OFFSET, 0);
  pal_reg_wr32(mpu_offset + stage_base +
	       CAP_MPU_CSR_CNT_ICACHE_MISS_BYTE_OFFSET, 0);
  pal_reg_wr32(mpu_offset + stage_base +
	       CAP_MPU_CSR_CNT_ICACHE_FILL_STALL_BYTE_OFFSET, 0);
  pal_reg_wr32(mpu_offset + stage_base +
	       CAP_MPU_CSR_CNT_CYCLES_BYTE_OFFSET, 0);
  pal_reg_wr32(mpu_offset + stage_base +
	       CAP_MPU_CSR_CNT_PHV_EXECUTED_BYTE_OFFSET, 0);
  pal_reg_wr32(mpu_offset + stage_base +
	       CAP_MPU_CSR_CNT_HAZARD_STALL_BYTE_OFFSET, 0);
  pal_reg_wr32(mpu_offset + stage_base +
	       CAP_MPU_CSR_CNT_PHVWR_STALL_BYTE_OFFSET, 0);
  pal_reg_wr32(mpu_offset + stage_base +
	       CAP_MPU_CSR_CNT_MEMWR_STALL_BYTE_OFFSET, 0);
  pal_reg_wr32(mpu_offset + stage_base +
	       CAP_MPU_CSR_CNT_TBLWR_STALL_BYTE_OFFSET, 0);
  pal_reg_wr32(mpu_offset + stage_base +
	       CAP_MPU_CSR_CNT_FENCE_STALL_BYTE_OFFSET, 0);
}

void sdp_read_counters(int verbose, u_int32_t stage_base)
{
  u_int32_t sdp[3]; /* only 3, CAP_MCPU_CSR_CNT_SDP_SIZE=4 looks wrong? */
  int sop_out, phv_fifo_depth;

  pal_reg_rd32w(stage_base + CAP_MPU_CSR_CNT_SDP_BYTE_OFFSET, sdp, 3);

  /* extract from sdp[1] */
  sop_out = CAP_MPU_CSR_CNT_SDP_CNT_SDP_1_3_SOP_OUT_GET(sdp[1]);
  /* extract from sdp[0] */
  phv_fifo_depth = CAP_MPU_CSR_CNT_SDP_CNT_SDP_0_3_PHV_FIFO_DEPTH_GET(sdp[0]);

  if(verbose==0) {
    //
  } else {
    printf("  sdp PHV FIFO depth=%d\n", phv_fifo_depth);
    printf("  sdp PHV processed count=%d\n", sop_out);
  }
}

void te_read_counters(int verbose, u_int32_t stage_base)
{
  u_int32_t te_phv_cnt = pal_reg_rd32(stage_base +
				      CAP_TE_CSR_CNT_PHV_IN_SOP_BYTE_ADDRESS);
  u_int32_t te_axi_cnt = pal_reg_rd32(stage_base +
				      CAP_TE_CSR_CNT_AXI_RDREQ_ADDRESS);
  u_int32_t te_tcam_cnt = pal_reg_rd32(stage_base +
				      CAP_TE_CSR_CNT_TCAM_REQ_ADDRESS);
  u_int32_t te_mpu_cnt = pal_reg_rd32(stage_base +
				      CAP_TE_CSR_CNT_MPU_OUT_BYTE_ADDRESS);
  if(verbose==0) {
    //
  } else {
    printf("  te phv=%d, axi_rd=%d, tcam=%d, mpu_out=%d\n",
	   te_phv_cnt, te_axi_cnt, te_tcam_cnt, te_mpu_cnt);
  }
}

void mpu_read_table_addr(int verbose, u_int32_t stage_base, int mpu)
{
  u_int32_t table_addr[2];
  pal_reg_rd32w(stage_base + CAP_MPU_CSR_STA_TBL_ADDR_BYTE_OFFSET +
		( CAP_MPU_CSR_STA_TBL_ADDR_ARRAY_ELEMENT_SIZE * 4 * mpu), table_addr, 2);
  if(verbose==0) {
  } else {
    printf("  mpu %d table address = 0x%x%8x\n", mpu, (unsigned int) table_addr[1], (unsigned int) table_addr[0]);
  }
}

void sta_stg_poll(int verbose, u_int32_t stage_base, int polls)
{
  u_int32_t sta_stg;
  u_int32_t sta_ctl[4][2];
  u_int32_t latency[4];
  u_int32_t latency_val[4];
  u_int32_t table_type[4];
  u_int32_t phv_data_depth[4], phv_cmd_depth[4];
  u_int32_t latency_min = 10000;
  u_int32_t latency_max = 0;
  int latency_total = 0;
  int last_table_type = 0;
  int mpu_processing[4], stall[4][4], stg_srdy, stg_drdy, i, mpu, te_valid, j;
  for(mpu=0; mpu<4; mpu++) {
    mpu_processing[mpu] = 0;
    stall[mpu][0] = 0;
    stall[mpu][1] = 0;
    stall[mpu][2] = 0;
    stall[mpu][3] = 0;
    latency[mpu] = 0;
    latency_val[mpu] = 0;
    table_type[mpu] = 0;
    phv_data_depth[mpu] = 0;
    phv_cmd_depth[mpu] = 0;
  }
  stg_srdy = 0;
  stg_drdy = 0;
  te_valid = 0;
  for(i=0; i<polls; i++) {
    sta_stg = pal_reg_rd32(stage_base + CAP_MPU_CSR_STA_STG_BYTE_OFFSET);
    for(mpu=0; mpu<4; mpu++) {
      pal_reg_rd32w(stage_base + CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_0_2_BYTE_OFFSET +
		    mpu * CAP_MPU_CSR_STA_CTL_MPU_BYTE_SIZE, sta_ctl[mpu], 2);
      latency[mpu] = pal_reg_rd32(stage_base +
				  (CAP_MPU_CSR_STA_TABLE_ARRAY_ELEMENT_SIZE * 4 * mpu) +
				  CAP_MPU_CSR_STA_TABLE_BYTE_OFFSET);
    }
                
    for(mpu=0; mpu<4; mpu++) {
      //	CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_EX_PC_30_29_GET(sta_ctl[mpu][1])
      //	CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_0_2_EX_PC_28_0_GET(sta_ctl[mpu][0])
      phv_data_depth[mpu] += CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_PHV_DATA_FIFO_DEPTH_GET(sta_ctl[mpu][1]);
      phv_cmd_depth[mpu] += CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_PHV_CMD_FIFO_DEPTH_GET(sta_ctl[mpu][1]);
      stall[mpu][0] += CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_STALL_VECTOR_GET(sta_ctl[mpu][1]) & 0x1;
      stall[mpu][1] += (CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_STALL_VECTOR_GET(sta_ctl[mpu][1]) >> 1) & 0x1;
      stall[mpu][2] += (CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_STALL_VECTOR_GET(sta_ctl[mpu][1]) >> 2) & 0x1;
      stall[mpu][3] += (CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_STALL_VECTOR_GET(sta_ctl[mpu][1]) >> 3) & 0x1;
      mpu_processing[mpu] += (CAP_MPU_CSR_STA_STG_MPU_PROCESSING_GET(sta_stg) >> mpu) & 0x1;
      latency_val[mpu] +=CAP_MPU_CSR_STA_TABLE_MPU_PROCESSING_TABLE_LATENCY_GET(latency[mpu]);
      table_type[mpu] = CAP_MPU_CSR_STA_TABLE_MPU_PROCESSING_TABLE_PCIE_GET(latency[mpu]) ? TABLE_PCI :
   	                CAP_MPU_CSR_STA_TABLE_MPU_PROCESSING_TABLE_SRAM_GET(latency[mpu]) ? TABLE_SRAM : TABLE_HBM;
      latency_total += latency_val[mpu];
      last_table_type = table_type[mpu];
      if(latency_val[mpu] > latency_max) latency_max = latency_val[mpu];
      if(latency_val[mpu] < latency_min) latency_min = latency_val[mpu];
    }
    stg_srdy += CAP_MPU_CSR_STA_STG_SRDY_GET(sta_stg);
    stg_drdy += CAP_MPU_CSR_STA_STG_DRDY_GET(sta_stg);
    // count all te_valid pending: 
   for(j=0; j<16; j++) {
      te_valid += (((CAP_MPU_CSR_STA_STG_TE_VALID_GET(sta_stg) >> j) & 1) == 1) ? 1 : 0;
    }
  }
  printf(" srdy=%3d%% drdy=%3d%% te=%d",
	 stg_srdy * 100 / polls,
	 stg_drdy * 100 / polls,
	 te_valid / polls
	 );
  if(last_table_type==TABLE_PCI)       printf(" PCI_lat=");
  else if(last_table_type==TABLE_SRAM) printf(" SRAM_lat=");
  else                                 printf(" HBM_lat=");
  printf("%dclk", latency_total * 400 / polls);
  if(verbose==1) {
    printf(" min=%dclk, max=%dclk", (int) latency_min, (int) latency_max);
    printf(" phvwr depths");
    for(mpu=0; mpu<4; mpu++) {
      printf(" m%d=%d,%d", mpu,
	     (int) (phv_cmd_depth[mpu] / polls),
	     (int) (phv_data_depth[mpu] / polls));
    }
    printf("\n");
  }
  
  for(mpu=0; mpu<4; mpu++) {
    if(verbose==1) {
      printf("  mpu %d  processing %2d%%, stalls: hazard %2d%% phvwr %2d%% tblwr %2d%% memwr %2d%%\n",
	     mpu,
	     mpu_processing[mpu] * 100 / polls,	   
	     stall[mpu][3] * 100 / polls,
	     stall[mpu][2] * 100 / polls,
	     stall[mpu][1] * 100 / polls,
	     stall[mpu][0] * 100 / polls);
    } else {
      printf(" m%0d %0d%%",
	     mpu,
	     mpu_processing[mpu] * 100 / polls);   
    }
  }
  if(verbose==0) {
    printf("\n");
  }
}

void sched_read_counters(int verbose, int polls)
{
  u_int32_t cnt[2];
  u_int32_t xoff_vector, cnt_txdma;
  int xoff[16];
  int cos, i;
  // Doorbell sets/clears:
  pal_reg_rd32w(CAP_ADDR_BASE_TXS_TXS_OFFSET +
		CAP_TXS_CSR_CNT_SCH_DOORBELL_SET_BYTE_ADDRESS, cnt, 2);
  printf(" Doorbell sets=%d", cnt[0]);
  pal_reg_rd32w(CAP_ADDR_BASE_TXS_TXS_OFFSET +
		CAP_TXS_CSR_CNT_SCH_DOORBELL_CLR_BYTE_ADDRESS, cnt, 2);
  printf(" clears=%d", cnt[0]);
  // Packet Buffer XOFF:
  for(cos=0; cos<16; cos++) {
    xoff[cos]=0;
  }
  for(i=0; i<polls; i++) {
    xoff_vector = pal_reg_rd32(CAP_ADDR_BASE_TXS_TXS_OFFSET +
			       CAP_TXS_CSR_STA_GLB_BYTE_OFFSET);
    for(cos=0; cos<16; cos++) {
      xoff[cos] += (((xoff_vector >> cos) & 1)==1) ? 1 : 0;
    }
  }
  printf(" PB XOFF:");
  for(cos=0; cos<16; cos++) {
    printf(" %x%%", (xoff[cos] * 100) / polls);
  }
  // TxDMA PHVs:
  printf("\n PHVs to Stage0:");
  for(cos=0; cos<16; cos++) {  
    cnt_txdma = pal_reg_rd32(CAP_ADDR_BASE_TXS_TXS_OFFSET + (cos * 8) +
			     CAP_TXS_CSR_CNT_SCH_TXDMA_COS0_ADDRESS);
    if(cnt_txdma > 0)
      printf(" COS%d=%d",cos, cnt_txdma);
  }
  printf("\n");
}  

void doorbell_read_counters(int verbose, int polls)
{
  u_int32_t cnt[2];
  // Doorbell Host
  pal_reg_rd32w( CAP_ADDR_BASE_DB_WA_OFFSET +
		 CAP_WA_CSR_CNT_WA_HOST_DOORBELLS_BYTE_ADDRESS, cnt, 2);
  printf(" Host_DBs=%d", cnt[0]);
  // Doorbell Local
  pal_reg_rd32w( CAP_ADDR_BASE_DB_WA_OFFSET +
		 CAP_WA_CSR_CNT_WA_LOCAL_DOORBELLS_BYTE_ADDRESS, cnt, 2);
  printf(" Local_DBs=%d", cnt[0]);
  // Doorbell to Sched
  pal_reg_rd32w( CAP_ADDR_BASE_DB_WA_OFFSET +
		 CAP_WA_CSR_CNT_WA_SCHED_OUT_BYTE_ADDRESS, cnt, 2);
  printf(" DBtoSchedReq=%d", cnt[0]);

  printf("\n");
}  

void ptd_read_counters(int verbose, int polls)
{
  u_int32_t cnt_pend;
  u_int32_t sta_xoff;
  int i;
  int pend_rd=0;
  int pend_wr=0;
  int num_phv=0;
  int rd_ff_empty=0;
  int rd_ff_full=0;
  int wr_ff_empty=0;
  int wr_ff_full=0;
  int pkt_ff_empty=0;
  int pkt_ff_full=0;
  int sta_fifo=0;
  u_int32_t cnt[3];
  // Get PHV counts:
  pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
		CAP_PTD_CSR_CNT_MA_BYTE_ADDRESS, cnt, 3);
  int ma_cnt = CAP_PTD_CSR_CNT_MA_CNT_MA_0_3_SOP_31_0_GET(cnt[0]);
  pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
		CAP_PTD_CSR_CNT_NPV_RESUB_BYTE_ADDRESS, cnt, 3);
  int resub_cnt = CAP_PTD_CSR_CNT_NPV_RESUB_CNT_NPV_RESUB_0_3_SOP_31_0_GET(cnt[0]);

  pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
		CAP_PTD_CSR_CNT_PB_BYTE_ADDRESS, cnt, 3);
  int pb_cnt = CAP_PTD_CSR_CNT_PB_CNT_PB_0_3_SOP_31_0_GET(cnt[0]);
  // Pending Reads/Writes, # PHVs
  for(i=0; i<polls; i++) {
    // FIFO Status
    sta_fifo = pal_reg_rd32(CAP_ADDR_BASE_PR_PR_OFFSET +
			    CAP_PRD_CSR_STA_FIFO_BYTE_ADDRESS);
    rd_ff_full   += CAP_PRD_CSR_STA_FIFO_RD_LAT_FF_FULL_GET(sta_fifo);
    rd_ff_empty  += CAP_PRD_CSR_STA_FIFO_RD_LAT_FF_EMPTY_GET(sta_fifo);
    wr_ff_full   += CAP_PRD_CSR_STA_FIFO_WR_LAT_FF_FULL_GET(sta_fifo);
    wr_ff_empty  += CAP_PRD_CSR_STA_FIFO_WR_LAT_FF_EMPTY_GET(sta_fifo);
    pkt_ff_full  += CAP_PRD_CSR_STA_FIFO_PKT_FF_FULL_GET(sta_fifo);
    pkt_ff_empty += CAP_PRD_CSR_STA_FIFO_PKT_FF_EMPTY_GET(sta_fifo);
    // Pending:
    cnt_pend = pal_reg_rd32(CAP_ADDR_BASE_PT_PT_OFFSET +
			    CAP_PTD_CSR_STA_ID_BYTE_ADDRESS);
    sta_xoff = pal_reg_rd32(CAP_ADDR_BASE_PT_PT_OFFSET +
			    CAP_PT_CSR_PTD_STA_XOFF_BYTE_ADDRESS);
    pend_rd += CAP_PTD_CSR_STA_ID_RD_PEND_CNT_GET(cnt_pend);
    pend_wr += CAP_PTD_CSR_STA_ID_WR_PEND_CNT_GET(cnt_pend);
    num_phv += CAP_PTD_CSR_STA_XOFF_NUMPHV_COUNTER_GET(sta_xoff);
  }
  printf(" phv_cnt=%d, pb_cnt=%d, resub=%d, AXI_Reads=%d, AXI_Writes=%d PHVs_in_P4+=%d\n",
	 ma_cnt, pb_cnt, resub_cnt,
	 (pend_rd) / polls,
	 (pend_wr) / polls,
	 (num_phv) / polls);
  printf("       (FIFOs empty/full) rd=%d%%/%d%% wr=%d%%/%d%% pkt=%d%%/%d%%\n",
	 rd_ff_empty / polls, rd_ff_full / polls,
	 wr_ff_empty / polls, wr_ff_full / polls,
	 pkt_ff_empty / polls, pkt_ff_full / polls);
}  

void prd_read_counters(int verbose, int polls)
{
  u_int32_t sta_id;
  u_int32_t cnt[3];
  int i;
  int pend_rd=0;
  int pend_wr=0;
  int num_phv=0;
  int rd_ff_empty=0;
  int rd_ff_full=0;
  int wr_ff_empty=0;
  int wr_ff_full=0;
  int pkt_ff_empty=0;
  int pkt_ff_full=0;
  int pb_xoff=0;
  int sta_fifo=0;
  // Get PHV counts:
  pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
		CAP_PRD_CSR_CNT_MA_BYTE_ADDRESS, cnt, 3);
  int ma_cnt = CAP_PRD_CSR_CNT_MA_CNT_MA_0_3_SOP_31_0_GET(cnt[0]);
  pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
		CAP_PRD_CSR_CNT_PS_RESUB_PKT_BYTE_ADDRESS, cnt, 3);
  int resub_cnt = CAP_PRD_CSR_CNT_PS_RESUB_PKT_CNT_PS_RESUB_PKT_0_3_SOP_31_0_GET(cnt[0]);
  pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
		CAP_PRD_CSR_CNT_PS_PKT_BYTE_ADDRESS, cnt, 3);
  int ps_cnt = CAP_PRD_CSR_CNT_PS_PKT_CNT_PS_PKT_0_3_SOP_31_0_GET(cnt[0]);
  // Pending Reads/Writes, # PHVs
  for(i=0; i<polls; i++) {
    // FIFO Status
    sta_fifo = pal_reg_rd32(CAP_ADDR_BASE_PR_PR_OFFSET +
			    CAP_PRD_CSR_STA_FIFO_BYTE_ADDRESS);
    rd_ff_full   += CAP_PRD_CSR_STA_FIFO_RD_LAT_FF_FULL_GET(sta_fifo);
    rd_ff_empty  += CAP_PRD_CSR_STA_FIFO_RD_LAT_FF_EMPTY_GET(sta_fifo);
    wr_ff_full   += CAP_PRD_CSR_STA_FIFO_WR_LAT_FF_FULL_GET(sta_fifo);
    wr_ff_empty  += CAP_PRD_CSR_STA_FIFO_WR_LAT_FF_EMPTY_GET(sta_fifo);
    pkt_ff_full  += CAP_PRD_CSR_STA_FIFO_PKT_FF_FULL_GET(sta_fifo);
    pkt_ff_empty += CAP_PRD_CSR_STA_FIFO_PKT_FF_EMPTY_GET(sta_fifo);
    // Pending reads/writes:
    sta_id =  pal_reg_rd32(CAP_ADDR_BASE_PR_PR_OFFSET +
			   CAP_PRD_CSR_STA_ID_BYTE_ADDRESS);
    pend_rd += CAP_PRD_CSR_STA_ID_RD_PEND_CNT_GET(sta_id);
    pend_wr += CAP_PRD_CSR_STA_ID_WR_PEND_CNT_GET(sta_id);
    // num PHVs, xoff:
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
		  CAP_PRD_CSR_STA_XOFF_BYTE_ADDRESS, cnt, 3);
    num_phv += CAP_PRD_CSR_STA_XOFF_STA_XOFF_0_3_NUMPHV_COUNTER_GET(cnt[0]);
    pb_xoff = (CAP_PRD_CSR_STA_XOFF_STA_XOFF_1_3_PBPR_P15_PBUS_XOFF_27_0_GET(cnt[1]) << 4) |
              CAP_PRD_CSR_STA_XOFF_STA_XOFF_2_3_PBPR_P15_PBUS_XOFF_31_28_GET(cnt[2]);
  }
  printf(" phv_cnt=%d, pkt_cnt=%d, resub=%d, AXI_Reads=%d, AXI_Writes=%d PHVs_in_P4+=%d\n",
	 ma_cnt, ps_cnt, resub_cnt,
	 (pend_rd) / polls,
	 (pend_wr) / polls,
	 (num_phv) / polls);
  printf("       (FIFOs empty/full) rd=%d%%/%d%% wr=%d%%/%d%% pkt=%d%%/%d%% XOFF=%x\n",
	 rd_ff_empty / polls, rd_ff_full / polls,
	 wr_ff_empty / polls, wr_ff_full / polls,
	 pkt_ff_empty / polls, pkt_ff_full / polls,
	 pb_xoff);
}  

void psp_read_counters(int verbose, int polls)
{
  u_int32_t cnt[2];
  pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
		CAP_PSP_CSR_CNT_MA_SOP_BYTE_ADDRESS, cnt, 2);
  int ma_cnt = cnt[0];
  pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
		CAP_PSP_CSR_CNT_SW_SOP_BYTE_ADDRESS, cnt, 2);
  int sw_cnt = cnt[0];
  pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
		CAP_PSP_CSR_CNT_PB_PBUS_SOP_BYTE_ADDRESS, cnt, 2);
  int pb_pbus_cnt = cnt[0];
  pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
		CAP_PSP_CSR_CNT_PR_PBUS_SOP_BYTE_ADDRESS, cnt, 2);
  int pr_pbus_cnt = cnt[0];
  pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
		CAP_PSP_CSR_CNT_MA_DROP_BYTE_ADDRESS, cnt, 2);
  int ma_drop_cnt = cnt[0];
  pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
		CAP_PSP_CSR_CNT_MA_RECIRC_BYTE_ADDRESS, cnt, 2);
  int ma_recirc_cnt = cnt[0];
  printf(" PSP phv=%d, pb_pbus=%d, pr_pbus=%d, sw=%d, phv_drop=%d, recirc=%d\n",
	 ma_cnt,sw_cnt,pb_pbus_cnt,pr_pbus_cnt,ma_drop_cnt,ma_recirc_cnt);
}  

void npv_read_counters(int verbose, int polls)
{
  u_int32_t cnt[2];
  pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
		CAP_PSP_CSR_CNT_MA_SOP_BYTE_ADDRESS, cnt, 2);
  int ma_cnt = cnt[0];
  pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
		CAP_PSP_CSR_CNT_SW_SOP_BYTE_ADDRESS, cnt, 2);
  int sw_cnt = cnt[0];
  pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
		CAP_PSP_CSR_CNT_PB_PBUS_SOP_BYTE_ADDRESS, cnt, 2);
  int pb_pbus_cnt = cnt[0];
  pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
		CAP_PSP_CSR_CNT_PR_PBUS_SOP_BYTE_ADDRESS, cnt, 2);
  int pr_pbus_cnt = cnt[0];
  pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
		CAP_PSP_CSR_CNT_MA_DROP_BYTE_ADDRESS, cnt, 2);
  int ma_drop_cnt = cnt[0];
  pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
		CAP_PSP_CSR_CNT_MA_RECIRC_BYTE_ADDRESS, cnt, 2);
  int ma_recirc_cnt = cnt[0];
  printf(" NPV phv=%d, pb_pbus=%d, pr_pbus=%d, sw=%d, phv_drop=%d, recirc=%d\n",
	 ma_cnt,sw_cnt,pb_pbus_cnt,pr_pbus_cnt,ma_drop_cnt,ma_recirc_cnt);
}  


void axi_bw_mon(int verbose, u_int32_t base_addr)
{
  // Use PICS defines to get relative spacing between monitor regs
  // base_addr is the address of CAP_*_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS =
  u_int32_t rd_latency = pal_reg_rd32(base_addr +
				      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS -
				      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS);
  u_int32_t rd_bandwidth = pal_reg_rd32(base_addr +
				      CAP_PICS_CSR_STA_AXI_BW_MON_RD_BANDWIDTH_BYTE_ADDRESS -
				      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS);
  u_int32_t rd_cnt = pal_reg_rd32(base_addr +
				  CAP_PICS_CSR_CNT_AXI_BW_MON_RD_ADDRESS -
				  CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS);
  u_int32_t rd_trans = pal_reg_rd32(base_addr +
				    CAP_PICS_CSR_STA_AXI_BW_MON_RD_TRANSACTIONS_BYTE_ADDRESS +
				    CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS);

  u_int32_t wr_latency = pal_reg_rd32(base_addr +
				      CAP_PICS_CSR_STA_AXI_BW_MON_WR_LATENCY_BYTE_ADDRESS -
				      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS);
  u_int32_t wr_bandwidth = pal_reg_rd32(base_addr +
				      CAP_PICS_CSR_STA_AXI_BW_MON_WR_BANDWIDTH_BYTE_ADDRESS -
				      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS);
  u_int32_t wr_cnt = pal_reg_rd32(base_addr +
				  CAP_PICS_CSR_CNT_AXI_BW_MON_WR_ADDRESS -
				  CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS);
  u_int32_t wr_trans = pal_reg_rd32(base_addr +
				    CAP_PICS_CSR_STA_AXI_BW_MON_WR_TRANSACTIONS_BYTE_ADDRESS +
				    CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS);
  printf(" rd_latency=%d/%d",
	 CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_AVRG_GET(rd_latency),
	 CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_MAXV_GET(rd_latency));
  printf(" rd_bandwidth=%d/%d",
	 CAP_PICS_CSR_STA_AXI_BW_MON_RD_BANDWIDTH_AVRG_GET(rd_bandwidth),
	 CAP_PICS_CSR_STA_AXI_BW_MON_RD_BANDWIDTH_MAXV_GET(rd_bandwidth));
  printf(" rd_cnt=%d", rd_cnt);
  printf(" rd_pend=%d, no_drdy=%d",
	 CAP_PICS_CSR_STA_AXI_BW_MON_RD_TRANSACTIONS_OUTSTANDING_GET(rd_trans),
	 CAP_PICS_CSR_STA_AXI_BW_MON_RD_TRANSACTIONS_DESS_RDY_GET(rd_trans));
  // Writes:
  printf("\n         wr_latency=%d/%d",
	 CAP_PICS_CSR_STA_AXI_BW_MON_WR_LATENCY_AVRG_GET(wr_latency),
	 CAP_PICS_CSR_STA_AXI_BW_MON_WR_LATENCY_MAXV_GET(wr_latency));
  printf(" wr_bandwidth=%d/%d",
	 CAP_PICS_CSR_STA_AXI_BW_MON_WR_BANDWIDTH_AVRG_GET(wr_bandwidth),
	 CAP_PICS_CSR_STA_AXI_BW_MON_WR_BANDWIDTH_MAXV_GET(wr_bandwidth));
  printf(" wr_cnt=%d", wr_cnt);
  printf(" wr_pend=%d, no_drdy=%d",
	 CAP_PICS_CSR_STA_AXI_BW_MON_WR_TRANSACTIONS_OUTSTANDING_GET(wr_trans),
	 CAP_PICS_CSR_STA_AXI_BW_MON_WR_TRANSACTIONS_DESS_RDY_GET(wr_trans));
  
  printf("\n");
}  
