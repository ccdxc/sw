/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <getopt.h>
#include <inttypes.h>
#include <sys/types.h>

#ifdef __aarch64__
#include "pal.h"
#define rd_reg          pal_reg_rd32
#define wr_reg          pal_reg_wr32
#define rd_mem          pal_mem_rd
#define wr_mem          pal_mem_wr
#define rd_reg32w       pal_reg_rd32w
#define wr_reg32w       pal_reg_wr32w
#endif

#ifdef __x86_64__
#include "lib_model_client.h"

uint32_t rd_reg(uint64_t pa)
{
    uint32_t val;
    read_reg(pa, val);
    return val;
}

#define wr_reg          write_reg

void rd_reg32w(uint64_t pa, uint32_t *w, uint32_t nw)
{
    for (uint32_t i = 0; i < nw; i++) {
        w[i] = rd_reg(pa);
        pa += 4;
    }
}

void wr_reg32w(uint64_t pa, uint32_t *w, uint32_t nw)
{
    for (uint32_t i = 0; i < nw; i++) {
        wr_reg(pa, w[i]);
        pa += 4;
    }
}

#define rd_mem          read_mem
#define wr_mem          write_mem

#endif

/*
 * ASIC C headers from $SW/nic/asic/capri/model/cap_top
 */
//#include "cap_top_csr.h"
#include "cap_top_csr_defines.h"
#include "cap_ms_c_hdr.h"
#include "cap_mpu_c_hdr.h"
#include "cap_txs_c_hdr.h"
#include "cap_wa_c_hdr.h"
namespace psp {
#include "cap_psp_c_hdr.h"
}
namespace ptd {
#include "cap_pt_c_hdr.h"
}
namespace prd {
#include "cap_pr_c_hdr.h"
}
#include "cap_te_c_hdr.h"
#include "cap_pt_c_hdr.h"
#include "cap_pics_c_hdr.h"
#include "cap_pxb_c_hdr.h"
#include "cap_ms_c_hdr.h"
#include "cap_ppa_c_hdr.h"
#include "cap_pbc_c_hdr.h"
#include "cap_hens_c_hdr.h"
#include "cap_mpns_c_hdr.h"

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

#define PORT_ETH0 0
#define PORT_ETH1 1
#define PORT_DMA 9
#define PORT_P4EG 10
#define PORT_P4IG 11

struct pb_cnt_t {
  u_int32_t eth0_cnt_in;
  u_int32_t eth0_cnt_out;
  u_int32_t eth1_cnt_in;
  u_int32_t eth1_cnt_out;
  u_int32_t p4ig_cnt;
  u_int32_t p4eg_cnt;
  u_int32_t rxdma_cnt;
  u_int32_t txdma_cnt;
  u_int32_t timestamp;
};

using namespace std;
string portname[12] = {"Eth0",
		       "Eth1",
		       "Eth2",
		       "Eth3",
		       "Eth4",
		       "Eth5",
		       "Eth6",
		       "Eth7",
		       "BMC",
		       "RXDMA",
		       "P4EG",
		       "P4IG"};

string offloadname[9] = {"GCM0",
			 "GCM1",
			 "XTS",
			 "XTS_ENC",
			 "HE",
			 "CP",
			 "DC",
			 "MP",
			 "Master"};			 
			 
void mpu_read_counters(int, int, u_int32_t, int);
void sdp_read_counters(int, int, u_int32_t);
void te_read_counters(int, int, u_int32_t);
void mpu_reset_counters(u_int32_t, int);
void mpu_reset_all_counters();
void sta_stg_poll(int, int, u_int32_t, int);
void mpu_read_table_addr(int, u_int32_t, int);
void sched_read_counters(int, int, int);    
void doorbell_read_counters(int, int, int);
void ptd_read_counters(int, int, int);
void prd_read_counters(int, int, int);
void psp_read_counters(int, int, int);
void npv_read_counters(int, int, int);
void axi_bw_mon(int, u_int32_t);
void parser_read_counters(int, u_int32_t);
void qstate_lif_dump(int, int);
void dump_qstate(int, int, int, u_int64_t);
void crypto_read_queues(int, int);
void pbx_read_state(int);
void pb_read_queues(int);
void read_all_bw_monitors(int);
void measure_pps(int);

using namespace std;

int bw_mon=0;
int crypto=0;

int
main(int argc, char *argv[])
{
  u_int32_t sta_ver, pipe_base, stage_base;
  u_int32_t te_pipe_base, te_stage_base;
  int mpu, pipeline, stage;
  int verbose=0;
  int queue_dump=0;
  int i=1;
  int interval=0;
  int polls=100;
  int exclude=0;
  int idle_check=0;
  
#ifdef __x86_64__
   lib_model_connect();
#endif
    
  while(i < (argc)) {
    if(strcmp(argv[i], "-r")==0) {
      printf("Resetting all MPU performance counters\n");
      mpu_reset_all_counters();
    }
    else if(strcmp(argv[i], "-v")==0) {
      verbose=1;
      printf("Verbose output mode\n");
    }
    else if(strcmp(argv[i], "-q")==0) {
      printf("Capmon Queue Dump\n");
      queue_dump=1;
    }
    else if(strcmp(argv[i], "-b")==0) {
      printf("Capmon BW Monitor\n");
      bw_mon=1;
    }
    else if(strcmp(argv[i], "-p")==0) {
      i++;
      polls = atoi(argv[i]);
      printf("Polls: %d\n", polls);
    }
    else if(strcmp(argv[i], "-i")==0) {
      idle_check = 1;
      printf("Idle Check, resetting all counters.\n");
      mpu_reset_all_counters();
    }
    else if(strcmp(argv[i], "-x")==0) {
      i++;
      exclude = atoi(argv[i]);
      printf("Exclude: %d\n", exclude);
    }
    else if(strcmp(argv[i], "-c")==0) {
      printf("Crypto\n");
      crypto = 1;
    }
    else if(strcmp(argv[i], "-s")==0) {
      i++;
      interval = atoi(argv[i]);
      measure_pps(interval);
      return(0);
    }
    else {      
      printf("usage: capmon -r[eset counters] -v[erbose] -p[olls] N -i[dle check] -b[w monitor] -s[PPS]\n");
      return(0);
    }
    i++;
  }

    printf("****CAPMON****\n");
    sta_ver = rd_reg(CAP_ADDR_BASE_MS_MS_OFFSET +
                           CAP_MS_CSR_STA_VER_BYTE_OFFSET);
    printf("Type:    %d\n", sta_ver & 0xf);
    printf("Build:   %d\n", (sta_ver >> 16) & 0xffff);
    printf("Version: %d\n", (sta_ver >> 4) & 0xfff);

    // Queue Scan:
    if((queue_dump == 1) | (idle_check == 1)) {
      qstate_lif_dump(idle_check, verbose);
      if(idle_check == 0) exit(0);
    }

    printf("==PCIe==\n");
    pbx_read_state(idle_check);
    printf("==Doorbells==\n");
    doorbell_read_counters(idle_check, verbose, polls);    
    printf("==TX Scheduler==\n");
    sched_read_counters(idle_check, verbose, polls);    

    for(pipeline=0; pipeline<4; pipeline++) {
      if((exclude & 1) == 1) {
	exclude = exclude >> 1;
      }
      else {
	exclude = exclude >> 1;

	switch(pipeline) {
	case TXDMA: 
	  printf("==TXDMA PIPELINE==\n");
	  npv_read_counters(idle_check, verbose, polls);      
	  break;
	case RXDMA: 
	  printf("==RXDMA PIPELINE==\n");
	  psp_read_counters(idle_check, verbose, polls);      
	  break;
	case P4SGI: 
	  printf("==P4 INGRESS PIPELINE==\n");
	  printf(" SI Parser:");
	  parser_read_counters(verbose, CAP_ADDR_BASE_PPA_PPA_1_OFFSET);
	  break;
	case P4SGE: 
	  printf("==P4 EGRESS PIPELINE==\n");
	  printf(" SE Parser:");
	  parser_read_counters(verbose, CAP_ADDR_BASE_PPA_PPA_0_OFFSET);
	  break;
	}

	pipe_base = ((pipeline==TXDMA) ? CAP_ADDR_BASE_PCT_MPU_0_OFFSET :
		     (pipeline==P4SGI) ? CAP_ADDR_BASE_SGI_MPU_0_OFFSET :
		     (pipeline==P4SGE) ? CAP_ADDR_BASE_SGE_MPU_0_OFFSET :
		     (pipeline==RXDMA) ? CAP_ADDR_BASE_PCR_MPU_0_OFFSET : 0);
	te_pipe_base = ((pipeline==TXDMA) ? CAP_ADDR_BASE_PCT_TE_0_OFFSET :
			(pipeline==P4SGI) ? CAP_ADDR_BASE_SGI_TE_0_OFFSET :
			(pipeline==P4SGE) ? CAP_ADDR_BASE_SGE_TE_0_OFFSET :
			(pipeline==RXDMA) ? CAP_ADDR_BASE_PCR_TE_0_OFFSET : 0);
	// Visit each Stage in pipeline
	for(stage=0; stage<(((pipeline==TXDMA)|(pipeline==RXDMA)) ? 8 : 6); stage++) {
	  printf(" Stage %d", stage);
	  if(verbose==1) {
	    printf("\n");
	  }
	  stage_base = (pipe_base + 
			(CAP_ADDR_BASE_PCR_MPU_1_OFFSET -
			 CAP_ADDR_BASE_PCR_MPU_0_OFFSET) * stage);
	  te_stage_base = (te_pipe_base + 
			   (CAP_ADDR_BASE_PCR_TE_1_OFFSET -
			    CAP_ADDR_BASE_PCR_TE_0_OFFSET) * stage);      
	  sta_stg_poll(idle_check, verbose, stage_base, polls);
	  te_read_counters(idle_check, verbose, te_stage_base);
	  sdp_read_counters(idle_check, verbose, stage_base);
	  // Visit each MPU in state
	  for(mpu=0; mpu<4; mpu++) {
	    mpu_read_counters(idle_check, verbose, stage_base, mpu);
	    if(verbose==1) {
	    }  mpu_read_table_addr(verbose, stage_base, mpu);
	  }
        }

	switch(pipeline) {
	case TXDMA: 
	  printf(" TxDMA Engine:");
	  ptd_read_counters(idle_check, verbose, polls);
	  break;
	case RXDMA: 
	  printf(" RxDMA Engine:");
	  prd_read_counters(idle_check, verbose, polls);    
	  break;
	case P4SGI:
	  break;
	case P4SGE: 
	  break;
	}
      }
    }
    // Packet Buffer Report:
    pb_read_queues(idle_check);

    // Offload Engines:
    if (crypto)
      crypto_read_queues(idle_check, verbose);    
    // AXI BAndwidth Monitor:
    if(bw_mon==1)
      read_all_bw_monitors(verbose);
    
#ifdef __x86_64__
    lib_model_conn_close();
#endif 

    exit(0);
}

void measure_pps(int interval)
{
  uint64_t timestamp_start, timestamp_end;
  uint32_t txd_start_cnt, txd_end_cnt;
  uint32_t rxd_start_cnt, rxd_end_cnt;
  uint64_t p4ig_start_cnt, p4ig_end_cnt;
  uint64_t p4eg_start_cnt, p4eg_end_cnt;
  uint32_t zero[4] = {0};
  uint32_t cnt[4] = {0};

  // Clear Packet Counters
  wr_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PT_CSR_PTD_CNT_PB_BYTE_ADDRESS, zero, 3);
  wr_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PR_CSR_PRD_CNT_PS_PKT_BYTE_ADDRESS, zero, 3);
  wr_reg32w(CAP_ADDR_BASE_PPA_PPA_0_OFFSET +
            CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS, zero, 4);
  wr_reg32w(CAP_ADDR_BASE_PPA_PPA_1_OFFSET +
            CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS, zero, 4);

  // Capture Timestamp
  rd_reg32w(CAP_ADDR_BASE_PB_PBC_OFFSET +
        CAP_PBC_CSR_HBM_STA_HBM_TIMESTAMP_BYTE_ADDRESS, cnt, 2);
  timestamp_start = ((uint64_t)CAP_PBCHBM_CSR_STA_HBM_TIMESTAMP_STA_HBM_TIMESTAMP_0_2_VALUE_31_0_GET(cnt[0])) |
              ((uint64_t) CAP_PBCHBM_CSR_STA_HBM_TIMESTAMP_STA_HBM_TIMESTAMP_1_2_VALUE_47_32_GET(cnt[1]) << 32);

  // TXDMA
  rd_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PT_CSR_PTD_CNT_PB_BYTE_ADDRESS, cnt, 3);
  txd_start_cnt = CAP_PTD_CSR_CNT_PB_CNT_PB_0_3_SOP_31_0_GET(cnt[0]);

  // RXDMA
  rd_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PR_CSR_PRD_CNT_PS_PKT_BYTE_ADDRESS, cnt, 3);
  rxd_start_cnt = CAP_PRD_CSR_CNT_PS_PKT_CNT_PS_PKT_0_3_SOP_31_0_GET(cnt[0]);

  // P4 EG
  rd_reg32w(CAP_ADDR_BASE_PPA_PPA_0_OFFSET + CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS, cnt, 4);
  p4eg_start_cnt = ((u_int64_t) CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_0_4_SOP_31_0_GET(cnt[0]) + 
             ((u_int64_t) CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_1_4_SOP_39_32_GET(cnt[1]) << 32));

  // P4 IG
  rd_reg32w(CAP_ADDR_BASE_PPA_PPA_1_OFFSET + CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS, cnt, 4);
  p4ig_start_cnt = ((u_int64_t) CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_0_4_SOP_31_0_GET(cnt[0]) + 
             ((u_int64_t) CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_1_4_SOP_39_32_GET(cnt[1]) << 32));

  sleep(interval);

  // Capture Timestamp
  rd_reg32w(CAP_ADDR_BASE_PB_PBC_OFFSET +
        CAP_PBC_CSR_HBM_STA_HBM_TIMESTAMP_BYTE_ADDRESS, cnt, 2);
  timestamp_end = ((uint64_t)CAP_PBCHBM_CSR_STA_HBM_TIMESTAMP_STA_HBM_TIMESTAMP_0_2_VALUE_31_0_GET(cnt[0])) |
              ((uint64_t)CAP_PBCHBM_CSR_STA_HBM_TIMESTAMP_STA_HBM_TIMESTAMP_1_2_VALUE_47_32_GET(cnt[1]) << 32);

  // TXDMA
  rd_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PT_CSR_PTD_CNT_PB_BYTE_ADDRESS, cnt, 3);
  txd_end_cnt = CAP_PTD_CSR_CNT_PB_CNT_PB_0_3_SOP_31_0_GET(cnt[0]);

  // RXDMA
  rd_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PR_CSR_PRD_CNT_PS_PKT_BYTE_ADDRESS, cnt, 3);
  rxd_end_cnt = CAP_PRD_CSR_CNT_PS_PKT_CNT_PS_PKT_0_3_SOP_31_0_GET(cnt[0]);

  // P4 EG
  rd_reg32w(CAP_ADDR_BASE_PPA_PPA_0_OFFSET + CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS, cnt, 4);
  p4eg_end_cnt = ((u_int64_t) CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_0_4_SOP_31_0_GET(cnt[0]) + 
             ((u_int64_t) CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_1_4_SOP_39_32_GET(cnt[1]) << 32));

  // P4 IG
  rd_reg32w(CAP_ADDR_BASE_PPA_PPA_1_OFFSET + CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS, cnt, 4);
  p4ig_end_cnt = ((u_int64_t) CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_0_4_SOP_31_0_GET(cnt[0]) + 
             ((u_int64_t) CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_1_4_SOP_39_32_GET(cnt[1]) << 32));

  uint64_t ts_delta = timestamp_end - timestamp_start; 
  uint64_t txd_delta = txd_end_cnt - txd_start_cnt;
  uint64_t rxd_delta = rxd_end_cnt - rxd_start_cnt;
  uint64_t p4ig_delta = p4ig_end_cnt - p4ig_start_cnt;
  uint64_t p4eg_delta = p4eg_end_cnt - p4eg_start_cnt;

  //printf("PACKETS: TXDMA=%lu RXDMA=%lu P4IG=%lu P4EG=%lu\n",
  //  txd_delta, rxd_delta, p4ig_delta, p4eg_delta);

  printf("PPS: TXDMA=%.0f RXDMA=%.0f P4IG=%.0f P4EG=%.0f\n",
    txd_delta*1e9/(ts_delta*1.2),
    rxd_delta*1e9/(ts_delta*1.2),
    p4ig_delta*1e9/(ts_delta*1.2),
    p4eg_delta*1e9/(ts_delta*1.2));
}

void idle_fail()
{
  printf("\nSystem not idle during idle check, FAIL.\n");
  //exit(-1);
}


void mpu_read_counters(int idle_check, int verbose, u_int32_t stage_base, int mpu_id)
{
  u_int32_t mpu_offset = mpu_id * 4; // word array
  u_int32_t inst_executed = rd_reg(mpu_offset + stage_base +
                                       CAP_MPU_CSR_CNT_INST_EXECUTED_BYTE_OFFSET);
  u_int32_t icache_miss = rd_reg(mpu_offset + stage_base +
                                       CAP_MPU_CSR_CNT_ICACHE_MISS_BYTE_OFFSET);
  u_int32_t icache_fill_stall = rd_reg(mpu_offset + stage_base +
                                       CAP_MPU_CSR_CNT_ICACHE_FILL_STALL_BYTE_OFFSET);
  u_int32_t cycles = rd_reg(mpu_offset + stage_base +
                                       CAP_MPU_CSR_CNT_CYCLES_BYTE_OFFSET);
  u_int32_t phv_executed = rd_reg(mpu_offset + stage_base +
                                       CAP_MPU_CSR_CNT_PHV_EXECUTED_BYTE_OFFSET);
  u_int32_t hazard_stall = rd_reg(mpu_offset + stage_base +
                                       CAP_MPU_CSR_CNT_HAZARD_STALL_BYTE_OFFSET);
  u_int32_t phvwr_stall = rd_reg(mpu_offset + stage_base +
                                       CAP_MPU_CSR_CNT_PHVWR_STALL_BYTE_OFFSET);
  u_int32_t memwr_stall = rd_reg(mpu_offset + stage_base +
                                       CAP_MPU_CSR_CNT_MEMWR_STALL_BYTE_OFFSET);
  u_int32_t tblwr_stall = rd_reg(mpu_offset + stage_base +
                                       CAP_MPU_CSR_CNT_TBLWR_STALL_BYTE_OFFSET);
  u_int32_t fence_stall = rd_reg(mpu_offset + stage_base +
                                       CAP_MPU_CSR_CNT_FENCE_STALL_BYTE_OFFSET);
  if(verbose==1) {
    printf("  mpu %d cycles=%u",mpu_id,cycles);
    printf(" inst=%u",inst_executed);
    printf(" miss=%u", icache_miss);
    printf(" istl=%u", icache_fill_stall);
    printf(" phv=%u",phv_executed);
    printf(" hzrd=%u",hazard_stall);
    printf(" phvwr_stl=%u",phvwr_stall);
    printf(" memwr_stl=%u",memwr_stall);
    printf(" tblwr_stl=%u",tblwr_stall);
    printf(" fence_stl=%u\n",fence_stall);
  }
  if(verbose==1) {
    if(cycles==0) {cycles = 1;}
    printf(" mpu %u percentages", mpu_id);
    printf(" inst=%u%%",(inst_executed * 100) / cycles);
    printf(" miss=%u%%",(icache_miss * 100) / cycles);
    printf(" istl=%u%%", (icache_fill_stall * 100) / cycles);
    printf(" phv=%u%%", (phv_executed * 100) / cycles);
    printf(" hzrd=%u%%", (hazard_stall * 100) / cycles);
    printf(" phvwr_stl=%u%%", (phvwr_stall * 100) / cycles);
    printf(" memwr_stl=%u%%", (memwr_stall * 100) / cycles);
    printf(" tblwr_stl=%u%%", (tblwr_stall * 100) / cycles);
    printf(" fence_stl=%u%%\n", (fence_stall * 100) / cycles);
  }
  if((inst_executed!=0) & idle_check) idle_fail(); 
  if((icache_miss!=0) & idle_check) idle_fail(); 
  if((phv_executed!=0) & idle_check) idle_fail(); 
  if((fence_stall!=0) & idle_check) idle_fail(); 
  if((tblwr_stall!=0) & idle_check) idle_fail(); 
  if((memwr_stall!=0) & idle_check) idle_fail(); 
  if((tblwr_stall!=0) & idle_check) idle_fail(); 
}

void mpu_reset_all_counters()
{
  u_int32_t pipe_base, stage_base;
  int mpu, pipeline, stage, cos;
  u_int32_t zero[4] = {0};

  // Crypto count reset
  if (crypto) {
  wr_reg32w(CAP_ADDR_BASE_MD_HENS_OFFSET +
	    CAP_HENS_CSR_CNT_DOORBELL_XTS_BYTE_ADDRESS, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_MD_HENS_OFFSET +
	    CAP_HENS_CSR_CNT_DOORBELL_XTS_ENC_BYTE_ADDRESS, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_MD_HENS_OFFSET +
	    CAP_HENS_CSR_CNT_DOORBELL_GCM0_BYTE_ADDRESS, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_MD_HENS_OFFSET +
	    CAP_HENS_CSR_CNT_DOORBELL_GCM1_BYTE_ADDRESS, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_MD_HENS_OFFSET +
	     CAP_HENS_CSR_CNT_DOORBELL_PK_BYTE_ADDRESS, zero, 2);
  }
  // PCIe count reset:
  printf("PCIe count reset\n");
  wr_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_CNT_TGT_TOT_AXI_WR_BYTE_ADDRESS, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_CNT_TGT_TOT_AXI_RD_BYTE_ADDRESS, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_CNT_ITR_TOT_AXI_WR_BYTE_OFFSET, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_CNT_ITR_TOT_AXI_WR_BYTE_OFFSET, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_CNT_ITR_TOT_AXI_RD_BYTE_OFFSET, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_CNT_ITR_TOT_AXI_RD_BYTE_OFFSET, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_SAT_ITR_RDLAT0_BYTE_OFFSET, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_SAT_ITR_RDLAT1_BYTE_OFFSET, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_SAT_ITR_RDLAT2_BYTE_OFFSET, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_SAT_ITR_RDLAT3_BYTE_OFFSET, zero, 2);
  // Doorbell count reset:
  printf("DB reset\n");
  wr_reg32w(CAP_ADDR_BASE_TXS_TXS_OFFSET +
            CAP_TXS_CSR_CNT_SCH_DOORBELL_SET_BYTE_ADDRESS, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_TXS_TXS_OFFSET +
            CAP_TXS_CSR_CNT_SCH_DOORBELL_CLR_BYTE_ADDRESS, zero, 2);
  // Doorbell Host
  wr_reg32w( CAP_ADDR_BASE_DB_WA_OFFSET +
             CAP_WA_CSR_CNT_WA_HOST_DOORBELLS_BYTE_ADDRESS, zero, 2);
  // Doorbell Local
  wr_reg32w( CAP_ADDR_BASE_DB_WA_OFFSET +
             CAP_WA_CSR_CNT_WA_LOCAL_DOORBELLS_BYTE_ADDRESS, zero, 2);
  // Doorbell to Sched
  wr_reg32w( CAP_ADDR_BASE_DB_WA_OFFSET +
             CAP_WA_CSR_CNT_WA_SCHED_OUT_BYTE_ADDRESS, zero, 2);
  // NPV count reset:
  printf("NPV reset\n");
  wr_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PSP_CSR_CNT_MA_SOP_BYTE_ADDRESS, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PSP_CSR_CNT_SW_SOP_BYTE_ADDRESS, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PSP_CSR_CNT_PB_PBUS_SOP_BYTE_ADDRESS, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PSP_CSR_CNT_PR_PBUS_SOP_BYTE_ADDRESS, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PSP_CSR_CNT_MA_DROP_BYTE_ADDRESS, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PSP_CSR_CNT_MA_RECIRC_BYTE_ADDRESS, zero, 2);
  // TxDMA
  printf("TXD reset\n");
  for(cos=0; cos<16; cos++) {  
    wr_reg(CAP_ADDR_BASE_TXS_TXS_OFFSET + (cos * 8) +
           CAP_TXS_CSR_CNT_SCH_TXDMA_COS0_ADDRESS, 0);
  }
  wr_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
      CAP_PT_CSR_PTD_BYTE_ADDRESS +
	    CAP_PTD_CSR_CNT_PHV_BYTE_OFFSET, zero, 4);
  wr_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PT_CSR_PTD_CNT_MA_BYTE_ADDRESS, zero, 3);
  wr_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PT_CSR_PTD_CNT_NPV_RESUB_BYTE_ADDRESS, zero, 3);
  wr_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PT_CSR_PTD_CNT_PB_BYTE_ADDRESS, zero, 3);
  // RxDMA
  printf("RXD reset\n");
  wr_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PR_CSR_PRD_CNT_MA_BYTE_ADDRESS, zero, 3);
  wr_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PR_CSR_PRD_CNT_PS_RESUB_PKT_BYTE_ADDRESS, zero, 3);
  wr_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PR_CSR_PRD_CNT_PS_PKT_BYTE_ADDRESS, zero, 3);
  // PSP
  printf("PSP reset\n");
  wr_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PSP_CSR_CNT_MA_SOP_BYTE_ADDRESS, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PSP_CSR_CNT_SW_SOP_BYTE_ADDRESS, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PSP_CSR_CNT_PB_PBUS_SOP_BYTE_ADDRESS, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PSP_CSR_CNT_PR_PBUS_SOP_BYTE_ADDRESS, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PSP_CSR_CNT_MA_DROP_BYTE_ADDRESS, zero, 2);
  wr_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PSP_CSR_CNT_MA_RECIRC_BYTE_ADDRESS, zero, 2);
  // Parser
  printf("Parser reset\n");
  wr_reg32w(CAP_ADDR_BASE_PPA_PPA_0_OFFSET + CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS, zero, 4);
  wr_reg32w(CAP_ADDR_BASE_PPA_PPA_0_OFFSET + CAP_PPA_CSR_CNT_PPA_DP_BYTE_ADDRESS, zero, 4);
  wr_reg32w(CAP_ADDR_BASE_PPA_PPA_0_OFFSET + CAP_PPA_CSR_CNT_PPA_MA_BYTE_ADDRESS, zero, 4);
  wr_reg32w(CAP_ADDR_BASE_PPA_PPA_1_OFFSET + CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS, zero, 4);
  wr_reg32w(CAP_ADDR_BASE_PPA_PPA_1_OFFSET + CAP_PPA_CSR_CNT_PPA_DP_BYTE_ADDRESS, zero, 4);
  wr_reg32w(CAP_ADDR_BASE_PPA_PPA_1_OFFSET + CAP_PPA_CSR_CNT_PPA_MA_BYTE_ADDRESS, zero, 4);
  // Stage:
  printf("Stage resets\n");
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
      // SDP:
      wr_reg32w(stage_base + CAP_MPU_CSR_CNT_SDP_BYTE_OFFSET, zero, 3);
      // TE
      wr_reg32w(stage_base + CAP_TE_CSR_CNT_PHV_IN_SOP_BYTE_ADDRESS, zero, 1);
      wr_reg32w(stage_base + CAP_TE_CSR_CNT_AXI_RDREQ_BYTE_ADDRESS, zero, 1);
      wr_reg32w(stage_base + CAP_TE_CSR_CNT_TCAM_REQ_BYTE_ADDRESS, zero, 1);
      wr_reg32w(stage_base + CAP_TE_CSR_CNT_MPU_OUT_BYTE_ADDRESS, zero, 1);
      // Visit each MPU in stage
      for(mpu=0; mpu<4; mpu++) {
        mpu_reset_counters(stage_base, mpu);
      }
    }
  }
}


void mpu_reset_counters(u_int32_t stage_base, int mpu_id)
{
  u_int32_t mpu_offset = mpu_id * 4; // word array
  wr_reg(mpu_offset + stage_base +
               CAP_MPU_CSR_CNT_INST_EXECUTED_BYTE_OFFSET, 0);
  wr_reg(mpu_offset + stage_base +
               CAP_MPU_CSR_CNT_ICACHE_MISS_BYTE_OFFSET, 0);
  wr_reg(mpu_offset + stage_base +
               CAP_MPU_CSR_CNT_ICACHE_FILL_STALL_BYTE_OFFSET, 0);
  wr_reg(mpu_offset + stage_base +
               CAP_MPU_CSR_CNT_CYCLES_BYTE_OFFSET, 0);
  wr_reg(mpu_offset + stage_base +
               CAP_MPU_CSR_CNT_PHV_EXECUTED_BYTE_OFFSET, 0);
  wr_reg(mpu_offset + stage_base +
               CAP_MPU_CSR_CNT_HAZARD_STALL_BYTE_OFFSET, 0);
  wr_reg(mpu_offset + stage_base +
               CAP_MPU_CSR_CNT_PHVWR_STALL_BYTE_OFFSET, 0);
  wr_reg(mpu_offset + stage_base +
               CAP_MPU_CSR_CNT_MEMWR_STALL_BYTE_OFFSET, 0);
  wr_reg(mpu_offset + stage_base +
               CAP_MPU_CSR_CNT_TBLWR_STALL_BYTE_OFFSET, 0);
  wr_reg(mpu_offset + stage_base +
               CAP_MPU_CSR_CNT_FENCE_STALL_BYTE_OFFSET, 0);
}

void sdp_read_counters(int idle_check, int verbose, u_int32_t stage_base)
{
  u_int32_t sdp[3]; /* only 3, CAP_MCPU_CSR_CNT_SDP_SIZE=4 looks wrong? */
  int sop_out, phv_fifo_depth;

  rd_reg32w(stage_base + CAP_MPU_CSR_CNT_SDP_BYTE_OFFSET, sdp, 3);

  /* extract from sdp[1] */
  sop_out = CAP_MPU_CSR_CNT_SDP_CNT_SDP_1_3_SOP_OUT_GET(sdp[1]);
  /* extract from sdp[0] */
  phv_fifo_depth = CAP_MPU_CSR_CNT_SDP_CNT_SDP_0_3_PHV_FIFO_DEPTH_GET(sdp[0]);

  if(verbose==0) {
    //
  } else {
    printf("  sdp PHV FIFO depth=%u\n", phv_fifo_depth);
    printf("  sdp PHV processed count=%u\n", sop_out);
  }
  if((phv_fifo_depth!=0) & idle_check) idle_fail();      
  if((sop_out!=0) & idle_check) idle_fail();      
}

void te_read_counters(int idle_check, int verbose, u_int32_t stage_base)
{
  u_int32_t te_phv_cnt = rd_reg(stage_base + CAP_TE_CSR_CNT_PHV_IN_SOP_BYTE_ADDRESS);
  u_int32_t te_axi_cnt = rd_reg(stage_base + CAP_TE_CSR_CNT_AXI_RDREQ_BYTE_ADDRESS);
  u_int32_t te_tcam_cnt = rd_reg(stage_base + CAP_TE_CSR_CNT_TCAM_REQ_BYTE_ADDRESS);
  u_int32_t te_mpu_cnt  = rd_reg(stage_base + CAP_TE_CSR_CNT_MPU_OUT_BYTE_ADDRESS);
  if(verbose==0) {
    //
  } else {
    printf("  te phv=%u, axi_rd=%u, tcam=%u, mpu_out=%u\n",
           te_phv_cnt, te_axi_cnt, te_tcam_cnt, te_mpu_cnt);
  }
  if((te_phv_cnt!=0) & idle_check) idle_fail();      
  if((te_axi_cnt!=0) & idle_check) idle_fail();      
  if((te_tcam_cnt!=0) & idle_check) idle_fail();      
  if((te_mpu_cnt!=0) & idle_check) idle_fail();      
}

void mpu_read_table_addr(int verbose, u_int32_t stage_base, int mpu)
{
  u_int32_t table_addr[2];
  u_int64_t addr;
  rd_reg32w(stage_base + CAP_MPU_CSR_STA_TBL_ADDR_BYTE_OFFSET +
            ( CAP_MPU_CSR_STA_TBL_ADDR_ARRAY_ELEMENT_SIZE * 4 * mpu), table_addr, 2);
  addr = ((u_int64_t) CAP_MPU_CSR_STA_TBL_ADDR_STA_TBL_ADDR_0_2_IN_MPU_31_0_GET(table_addr[0]) +           
          ((u_int64_t) CAP_MPU_CSR_STA_TBL_ADDR_STA_TBL_ADDR_1_2_IN_MPU_63_32_GET(table_addr[1]) << 32));
  
  if(verbose==0) {
  } else {
    printf("  mpu %u table address = 0x%lx\n", mpu, addr);
  }
}

void sta_stg_poll(int idle_check, int verbose, u_int32_t stage_base, int polls)
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
    sta_stg = rd_reg(stage_base + CAP_MPU_CSR_STA_STG_BYTE_OFFSET);
    for(mpu=0; mpu<4; mpu++) {
      rd_reg32w(stage_base + CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_0_2_BYTE_OFFSET +
                    mpu * CAP_MPU_CSR_STA_CTL_MPU_BYTE_SIZE, sta_ctl[mpu], 2);
      latency[mpu] = rd_reg(stage_base +
                                  (CAP_MPU_CSR_STA_TABLE_ARRAY_ELEMENT_SIZE * 4 * mpu) +
                                  CAP_MPU_CSR_STA_TABLE_BYTE_OFFSET);
    }
                
    for(mpu=0; mpu<4; mpu++) {
      //        CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_EX_PC_30_29_GET(sta_ctl[mpu][1])
      //        CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_0_2_EX_PC_28_0_GET(sta_ctl[mpu][0])
      phv_data_depth[mpu] += CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_PHV_DATA_FIFO_DEPTH_GET(sta_ctl[mpu][1]);
      phv_cmd_depth[mpu] += CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_PHV_CMD_FIFO_DEPTH_GET(sta_ctl[mpu][1]);
      stall[mpu][0] += CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_STALL_VECTOR_GET(sta_ctl[mpu][1]) & 0x1;
      stall[mpu][1] += (CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_STALL_VECTOR_GET(sta_ctl[mpu][1]) >> 1) & 0x1;
      stall[mpu][2] += (CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_STALL_VECTOR_GET(sta_ctl[mpu][1]) >> 2) & 0x1;
      stall[mpu][3] += (CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_STALL_VECTOR_GET(sta_ctl[mpu][1]) >> 3) & 0x1;
      mpu_processing[mpu] += (CAP_MPU_CSR_STA_STG_MPU_PROCESSING_GET(sta_stg) >> mpu) & 0x1;
      latency_val[mpu] = CAP_MPU_CSR_STA_TABLE_MPU_PROCESSING_TABLE_LATENCY_GET(latency[mpu]);
      table_type[mpu] = CAP_MPU_CSR_STA_TABLE_MPU_PROCESSING_TABLE_PCIE_GET(latency[mpu]) ? TABLE_PCI :
                        CAP_MPU_CSR_STA_TABLE_MPU_PROCESSING_TABLE_SRAM_GET(latency[mpu]) ? TABLE_SRAM : TABLE_HBM;
    }
    // Only use MPU[3] for latency as it is used first in distribution
    latency_total += latency_val[3];
    last_table_type = table_type[3];
    if(latency_val[3] > latency_max) latency_max = latency_val[3];
    if(latency_val[3] < latency_min) latency_min = latency_val[3];
    // Stage srdy/drdy
    stg_srdy += CAP_MPU_CSR_STA_STG_SRDY_GET(sta_stg);
    stg_drdy += CAP_MPU_CSR_STA_STG_DRDY_GET(sta_stg);
    // count all te_valid pending: 
   for(j=0; j<16; j++) {
      te_valid += (((CAP_MPU_CSR_STA_STG_TE_VALID_GET(sta_stg) >> j) & 1) == 1) ? 1 : 0;
    }
  }
  printf(" srdy=%3d%% drdy=%3d%% te=%02u",
         stg_srdy * 100 / polls,
         stg_drdy * 100 / polls,
         te_valid / polls
         );
  if((stg_srdy!=0) & idle_check) idle_fail();    
  if((stg_drdy!=polls) & idle_check) idle_fail();    
  if((te_valid!=0) & idle_check) idle_fail();    
  if(last_table_type==TABLE_PCI)       printf("  PCI_lat=");
  else if(last_table_type==TABLE_SRAM) printf(" SRAM_lat=");
  else                                 printf("  HBM_lat=");
  printf("%05u", latency_total / polls);
  if(verbose==1) {
    printf(" min=%u, max=%u", (int) latency_min, (int) latency_max);
    printf(" phvwr depths");
    for(mpu=0; mpu<4; mpu++) {
      printf(" m%u=%u,%u", mpu,
             (int) (phv_cmd_depth[mpu] / polls),
             (int) (phv_data_depth[mpu] / polls));
      if((phv_cmd_depth[mpu]!=0) & idle_check) idle_fail();    
      if((phv_data_depth[mpu]!=0) & idle_check) idle_fail();          
    }
    printf("\n");
  }
  
  for(mpu=0; mpu<4; mpu++) {
    if(verbose==1) {
      printf("  mpu %u  processing %2d%%, stalls: hazard %2d%% phvwr %2d%% tblwr %2d%% memwr %2d%%\n",
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

void sched_read_counters(int idle_check, int verbose, int polls)
{
  u_int32_t cnt[2];
  u_int32_t xoff_vector, cnt_txdma;
  int xoff[16];
  int cos, i;
  // Doorbell sets/clears:
  rd_reg32w(CAP_ADDR_BASE_TXS_TXS_OFFSET +
                CAP_TXS_CSR_CNT_SCH_DOORBELL_SET_BYTE_ADDRESS, cnt, 2);
  printf(" Doorbell sets=%u", cnt[0]);
  if((cnt[0]!=0) & idle_check) idle_fail();  
  rd_reg32w(CAP_ADDR_BASE_TXS_TXS_OFFSET +
                CAP_TXS_CSR_CNT_SCH_DOORBELL_CLR_BYTE_ADDRESS, cnt, 2);
  printf(" clears=%u", cnt[0]);
  if((cnt[0]!=0) & idle_check) idle_fail();  
  // Packet Buffer XOFF:
  for(cos=0; cos<16; cos++) {
    xoff[cos]=0;
  }
  for(i=0; i<polls; i++) {
    xoff_vector = rd_reg(CAP_ADDR_BASE_TXS_TXS_OFFSET +
                               CAP_TXS_CSR_STA_GLB_BYTE_OFFSET);
    for(cos=0; cos<16; cos++) {
      xoff[cos] += (((xoff_vector >> cos) & 1)==1) ? 1 : 0;
    }
  }
  printf(" PB XOFF:");
  for(cos=0; cos<16; cos++) {
    printf(" %x%%", (xoff[cos] * 100) / polls);
    if((xoff[cos]!=0) & idle_check) idle_fail();  
  }
  // TxDMA PHVs:
  printf("\n PHVs to Stage0:");
  for(cos=0; cos<16; cos++) {  
    cnt_txdma = rd_reg(CAP_ADDR_BASE_TXS_TXS_OFFSET + (cos * 8) +
                             CAP_TXS_CSR_CNT_SCH_TXDMA_COS0_ADDRESS);
    if(cnt_txdma > 0) {
      printf(" COS%u=%u",cos, cnt_txdma);
      if((cnt_txdma > 0) & idle_check) idle_fail();  
    }
  }
  printf("\n");
}  

void doorbell_read_counters(int idle_check, int verbose, int polls)
{
  u_int64_t cnt;
  // Doorbell Host
  rd_reg32w( CAP_ADDR_BASE_DB_WA_OFFSET +
             CAP_WA_CSR_CNT_WA_HOST_DOORBELLS_BYTE_ADDRESS, (uint32_t *)&cnt, 2);
  printf(" Host_DBs=%ld", cnt);
  if((cnt!=0) & idle_check) idle_fail();
  // Doorbell Local
  rd_reg32w( CAP_ADDR_BASE_DB_WA_OFFSET +
             CAP_WA_CSR_CNT_WA_LOCAL_DOORBELLS_BYTE_ADDRESS, (uint32_t *)&cnt, 2);
  printf(" Local_DBs=%ld", cnt);
  if((cnt!=0) & idle_check) idle_fail();
  // Doorbell to Sched
  rd_reg32w( CAP_ADDR_BASE_DB_WA_OFFSET +
             CAP_WA_CSR_CNT_WA_SCHED_OUT_BYTE_ADDRESS, (uint32_t *)&cnt, 2);
  printf(" DBtoSchedReq=%ld", cnt);
  if((cnt!=0) & idle_check) idle_fail();
  printf("\n");
}  

void ptd_read_counters(int idle_check, int verbose, int polls)
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
  u_int32_t cnt[4];

  rd_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
      CAP_PT_CSR_PTD_BYTE_ADDRESS +
	    CAP_PTD_CSR_CNT_PHV_BYTE_OFFSET, cnt, 4);
  u_int32_t phv_drop = CAP_PTD_CSR_CNT_PHV_CNT_PHV_1_4_DROP_GET(cnt[1]);
  u_int32_t phv_err = CAP_PTD_CSR_CNT_PHV_CNT_PHV_2_4_ERR_GET(cnt[2]);
  u_int32_t phv_recirc = CAP_PTD_CSR_CNT_PHV_CNT_PHV_3_4_RECIRC_GET(cnt[3]);
  printf(" PHV_drop=%u  PHV_err=%u  PHV_recirc=%u\n",
	 phv_drop, phv_err, phv_recirc);

  // Get PHV counts:
  rd_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PT_CSR_PTD_CNT_MA_BYTE_ADDRESS, cnt, 3);
  int ma_cnt = CAP_PTD_CSR_CNT_MA_CNT_MA_0_3_SOP_31_0_GET(cnt[0]);
  rd_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PT_CSR_PTD_CNT_NPV_RESUB_BYTE_ADDRESS, cnt, 3);
  int resub_cnt = CAP_PTD_CSR_CNT_NPV_RESUB_CNT_NPV_RESUB_0_3_SOP_31_0_GET(cnt[0]);

  rd_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PT_CSR_PTD_CNT_PB_BYTE_ADDRESS, cnt, 3);
  int pb_cnt = CAP_PTD_CSR_CNT_PB_CNT_PB_0_3_SOP_31_0_GET(cnt[0]);
  // Pending Reads/Writes, # PHVs
  for(i=0; i<polls; i++) {
    // FIFO Status
    sta_fifo = rd_reg(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PT_CSR_PTD_STA_FIFO_BYTE_ADDRESS);
    rd_ff_full   += CAP_PTD_CSR_STA_FIFO_LAT_FF_FULL_GET(sta_fifo);
    rd_ff_empty  += CAP_PTD_CSR_STA_FIFO_LAT_FF_EMPTY_GET(sta_fifo);
    wr_ff_full   += CAP_PTD_CSR_STA_FIFO_WR_MEM_FF_FULL_GET(sta_fifo);
    wr_ff_empty  += CAP_PTD_CSR_STA_FIFO_WR_MEM_FF_EMPTY_GET(sta_fifo);
    pkt_ff_full  += CAP_PTD_CSR_STA_FIFO_PKT_FF_FULL_GET(sta_fifo);
    pkt_ff_empty += CAP_PTD_CSR_STA_FIFO_PKT_FF_EMPTY_GET(sta_fifo);
    // Pending:
    cnt_pend = rd_reg(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PT_CSR_PTD_STA_ID_BYTE_ADDRESS);
    sta_xoff = rd_reg(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PT_CSR_PTD_STA_XOFF_BYTE_ADDRESS);
    pend_rd += CAP_PTD_CSR_STA_ID_RD_PEND_CNT_GET(cnt_pend);
    pend_wr += CAP_PTD_CSR_STA_ID_WR_PEND_CNT_GET(cnt_pend);
    num_phv += CAP_PTD_CSR_STA_XOFF_NUMPHV_COUNTER_GET(sta_xoff);
  }
  printf("       phv_cnt=%u pb_cnt=%u resub=%u AXI_Reads=%u AXI_Writes=%u PHVs_in_P4+=%u\n",
         ma_cnt, pb_cnt, resub_cnt,
         (pend_rd) / polls,
         (pend_wr) / polls,
         (num_phv) / polls);
  if((pend_rd!=0) & idle_check) idle_fail();  
  if((pend_wr!=0) & idle_check) idle_fail();  
  if((num_phv!=0) & idle_check) idle_fail();  
  printf("       (FIFOs empty/full) rd=%u%%/%u%% wr=%u%%/%u%% pkt=%u%%/%u%%\n",
         rd_ff_empty / polls, rd_ff_full / polls,
         wr_ff_empty / polls, wr_ff_full / polls,
         pkt_ff_empty / polls, pkt_ff_full / polls);
  if((rd_ff_empty!=polls) & idle_check) idle_fail();  
  if((wr_ff_empty!=polls) & idle_check) idle_fail();  
  if((pkt_ff_empty!=polls) & idle_check) idle_fail();  
  if((rd_ff_full!=0) & idle_check) idle_fail();  
  if((wr_ff_full!=0) & idle_check) idle_fail();  
  if((pkt_ff_full!=0) & idle_check) idle_fail();  
}  

void prd_read_counters(int idle_check, int verbose, int polls)
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
  rd_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PR_CSR_PRD_CNT_MA_BYTE_ADDRESS, cnt, 3);
  int ma_cnt = CAP_PRD_CSR_CNT_MA_CNT_MA_0_3_SOP_31_0_GET(cnt[0]);
  rd_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PR_CSR_PRD_CNT_PS_RESUB_PKT_BYTE_ADDRESS, cnt, 3);
  int resub_cnt = CAP_PRD_CSR_CNT_PS_RESUB_PKT_CNT_PS_RESUB_PKT_0_3_SOP_31_0_GET(cnt[0]);
  rd_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PR_CSR_PRD_CNT_PS_PKT_BYTE_ADDRESS, cnt, 3);
  int ps_cnt = CAP_PRD_CSR_CNT_PS_PKT_CNT_PS_PKT_0_3_SOP_31_0_GET(cnt[0]);
  // Pending Reads/Writes, # PHVs
  for(i=0; i<polls; i++) {
    // FIFO Status
    sta_fifo = rd_reg(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PR_CSR_PRD_STA_FIFO_BYTE_ADDRESS);
    rd_ff_full   += CAP_PRD_CSR_STA_FIFO_RD_LAT_FF_FULL_GET(sta_fifo);
    rd_ff_empty  += CAP_PRD_CSR_STA_FIFO_RD_LAT_FF_EMPTY_GET(sta_fifo);
    wr_ff_full   += CAP_PRD_CSR_STA_FIFO_WR_LAT_FF_FULL_GET(sta_fifo);
    wr_ff_empty  += CAP_PRD_CSR_STA_FIFO_WR_LAT_FF_EMPTY_GET(sta_fifo);
    pkt_ff_full  += CAP_PRD_CSR_STA_FIFO_PKT_FF_FULL_GET(sta_fifo);
    pkt_ff_empty += CAP_PRD_CSR_STA_FIFO_PKT_FF_EMPTY_GET(sta_fifo);
    // Pending reads/writes:
    sta_id =  rd_reg(CAP_ADDR_BASE_PR_PR_OFFSET +
                     CAP_PR_CSR_PRD_STA_ID_BYTE_ADDRESS);
    pend_rd += CAP_PRD_CSR_STA_ID_RD_PEND_CNT_GET(sta_id);
    pend_wr += CAP_PRD_CSR_STA_ID_WR_PEND_CNT_GET(sta_id);
    // num PHVs, xoff:
    rd_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
              CAP_PR_CSR_PRD_STA_XOFF_BYTE_ADDRESS, cnt, 3);               
    num_phv += CAP_PRD_CSR_STA_XOFF_STA_XOFF_0_3_NUMPHV_XOFF_GET(cnt[0]);
    pb_xoff = (CAP_PRD_CSR_STA_XOFF_STA_XOFF_1_3_PBPR_P15_PBUS_XOFF_27_0_GET(cnt[1]) << 4) |
              CAP_PRD_CSR_STA_XOFF_STA_XOFF_2_3_PBPR_P15_PBUS_XOFF_31_28_GET(cnt[2]);
  }
  printf(" phv_cnt=%u pkt_cnt=%u resub=%u AXI_Reads=%u AXI_Writes=%u PHVs_in_P4+=%u\n",
         ma_cnt, ps_cnt, resub_cnt,
         (pend_rd) / polls,
         (pend_wr) / polls,
         (num_phv) / polls);
  if((pend_rd!=0) & idle_check) idle_fail();  
  if((pend_wr!=0) & idle_check) idle_fail();  
  if((num_phv!=0) & idle_check) idle_fail();  
  printf("       (FIFOs empty/full) rd=%u%%/%u%% wr=%u%%/%u%% pkt=%u%%/%u%% XOFF=%x\n",
         rd_ff_empty / polls, rd_ff_full / polls,
         wr_ff_empty / polls, wr_ff_full / polls,
         pkt_ff_empty / polls, pkt_ff_full / polls,
         pb_xoff);
  if((rd_ff_empty!=polls) & idle_check) idle_fail();  
  if((wr_ff_empty!=polls) & idle_check) idle_fail();  
  if((pkt_ff_empty!=polls) & idle_check) idle_fail();  
  if((rd_ff_full!=0) & idle_check) idle_fail();  
  if((wr_ff_full!=0) & idle_check) idle_fail();  
  if((pkt_ff_full!=0) & idle_check) idle_fail();  
  if((pb_xoff!=0) & idle_check) idle_fail();  
}  

void psp_read_counters(int idle_check, int verbose, int polls)
{
  uint64_t ma_cnt, sw_cnt, pb_pbus_cnt, pr_pbus_cnt, ma_drop_cnt, ma_recirc_cnt;

  rd_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PSP_CSR_CNT_MA_SOP_BYTE_ADDRESS, (uint32_t *)&ma_cnt, 2);
  rd_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PSP_CSR_CNT_SW_SOP_BYTE_ADDRESS, (uint32_t *)&sw_cnt, 2);
  rd_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PSP_CSR_CNT_PB_PBUS_SOP_BYTE_ADDRESS, (uint32_t *)&pb_pbus_cnt, 2);
  rd_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PSP_CSR_CNT_PR_PBUS_SOP_BYTE_ADDRESS, (uint32_t *)&pr_pbus_cnt, 2);
  rd_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PSP_CSR_CNT_MA_DROP_BYTE_ADDRESS, (uint32_t *)&ma_drop_cnt, 2);
  rd_reg32w(CAP_ADDR_BASE_PR_PR_OFFSET +
            CAP_PSP_CSR_CNT_MA_RECIRC_BYTE_ADDRESS, (uint32_t *)&ma_recirc_cnt, 2);

  printf(" PSP phv=%ld pb_pbus=%ld pr_pbus=%ld sw=%ld phv_drop=%ld recirc=%ld\n",
         ma_cnt,sw_cnt,pb_pbus_cnt,pr_pbus_cnt,ma_drop_cnt,ma_recirc_cnt);
  if((ma_cnt!=0) & idle_check) idle_fail();  
  if((sw_cnt!=0) & idle_check) idle_fail();  
  if((pb_pbus_cnt!=0) & idle_check) idle_fail();  
  if((pr_pbus_cnt!=0) & idle_check) idle_fail();  
  if((ma_drop_cnt!=0) & idle_check) idle_fail();  
  if((ma_recirc_cnt!=0) & idle_check) idle_fail();  
}  

void npv_read_counters(int idle_check, int verbose, int polls)
{
  uint64_t ma_cnt, sw_cnt, pb_pbus_cnt, pr_pbus_cnt, ma_drop_cnt, ma_recirc_cnt;
  
  rd_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PSP_CSR_CNT_MA_SOP_BYTE_ADDRESS, (uint32_t *)&ma_cnt, 2);
  rd_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PSP_CSR_CNT_SW_SOP_BYTE_ADDRESS, (uint32_t *)&sw_cnt, 2);
  rd_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PSP_CSR_CNT_PB_PBUS_SOP_BYTE_ADDRESS, (uint32_t *)&pb_pbus_cnt, 2);
  rd_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PSP_CSR_CNT_PR_PBUS_SOP_BYTE_ADDRESS, (uint32_t *)&pr_pbus_cnt, 2);
  rd_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PSP_CSR_CNT_MA_DROP_BYTE_ADDRESS, (uint32_t *)&ma_drop_cnt, 2);
  rd_reg32w(CAP_ADDR_BASE_PT_PT_OFFSET +
            CAP_PSP_CSR_CNT_MA_RECIRC_BYTE_ADDRESS, (uint32_t *)&ma_recirc_cnt, 2);
  printf(" NPV phv=%ld pb_pbus=%ld pr_pbus=%ld sw=%ld phv_drop=%ld recirc=%ld\n",
         ma_cnt,pb_pbus_cnt,pr_pbus_cnt,sw_cnt,ma_drop_cnt,ma_recirc_cnt);
  if((ma_cnt!=0) & idle_check) idle_fail();  
  if((sw_cnt!=0) & idle_check) idle_fail();  
  if((pb_pbus_cnt!=0) & idle_check) idle_fail();  
  if((pr_pbus_cnt!=0) & idle_check) idle_fail();  
  if((ma_drop_cnt!=0) & idle_check) idle_fail();  
  if((ma_recirc_cnt!=0) & idle_check) idle_fail();  
}  

void parser_read_counters(int verbose, u_int32_t base_addr)
{
  u_int32_t cnt[4];
  u_int64_t from_pb, to_dp, to_ma;
  rd_reg32w(base_addr + CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS, cnt, 4);
  from_pb = ((u_int64_t) CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_0_4_SOP_31_0_GET(cnt[0]) + 
             ((u_int64_t) CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_1_4_SOP_39_32_GET(cnt[1]) << 32));
  rd_reg32w(base_addr + CAP_PPA_CSR_CNT_PPA_DP_BYTE_ADDRESS, cnt, 4);
  to_dp = ((u_int64_t) CAP_PPA_CSR_CNT_PPA_DP_CNT_PPA_DP_0_4_SOP_31_0_GET(cnt[0]) + 
           ((u_int64_t) CAP_PPA_CSR_CNT_PPA_DP_CNT_PPA_DP_1_4_SOP_39_32_GET(cnt[1]) << 32));

  rd_reg32w(base_addr + CAP_PPA_CSR_CNT_PPA_MA_BYTE_ADDRESS, cnt, 4);
  to_ma = ((u_int64_t) CAP_PPA_CSR_CNT_PPA_MA_CNT_PPA_MA_0_4_SOP_31_0_GET(cnt[0]) + 
           ((u_int64_t) CAP_PPA_CSR_CNT_PPA_MA_CNT_PPA_MA_1_4_SOP_39_32_GET(cnt[1]) << 32));

  printf(" pkt_from_pb=%ld phv_to_s0=%ld pkt_to_dp=%ld\n", from_pb, to_ma, to_dp);
}  


void axi_bw_mon(int verbose, u_int32_t base_addr)
{
  // Use PICS defines to get relative spacing between monitor regs
  // base_addr is the address of CAP_*_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS =
  u_int32_t rd_latency = rd_reg(base_addr +
                                CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS -
                                CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS);
  u_int32_t rd_bandwidth = rd_reg(base_addr +
                                  CAP_PICS_CSR_STA_AXI_BW_MON_RD_BANDWIDTH_BYTE_ADDRESS -
                                  CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS);
  u_int32_t rd_cnt = rd_reg(base_addr +
                            CAP_PICS_CSR_CNT_AXI_BW_MON_RD_BYTE_ADDRESS -
                            CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS);
  u_int32_t rd_trans = rd_reg(base_addr +
                              CAP_PICS_CSR_STA_AXI_BW_MON_RD_TRANSACTIONS_BYTE_ADDRESS -
                              CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS);
  
  u_int32_t wr_latency = rd_reg(base_addr +
                                CAP_PICS_CSR_STA_AXI_BW_MON_WR_LATENCY_BYTE_ADDRESS -
                                CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS);
  u_int32_t wr_bandwidth = rd_reg(base_addr +
                                  CAP_PICS_CSR_STA_AXI_BW_MON_WR_BANDWIDTH_BYTE_ADDRESS -
                                  CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS);
  u_int32_t wr_cnt = rd_reg(base_addr +
                            CAP_PICS_CSR_CNT_AXI_BW_MON_WR_BYTE_ADDRESS -
                            CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS);
  u_int32_t wr_trans = rd_reg(base_addr +
                              CAP_PICS_CSR_STA_AXI_BW_MON_WR_TRANSACTIONS_BYTE_ADDRESS -
                              CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS);
  printf(" rd_latency=%u/%u",
         CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_AVRG_GET(rd_latency),
         CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_MAXV_GET(rd_latency));
  printf(" rd_bandwidth=%u/%u",
         CAP_PICS_CSR_STA_AXI_BW_MON_RD_BANDWIDTH_AVRG_GET(rd_bandwidth),
         CAP_PICS_CSR_STA_AXI_BW_MON_RD_BANDWIDTH_MAXV_GET(rd_bandwidth));
  printf(" rd_cnt=%u", rd_cnt);
  printf(" rd_pend=%u, no_drdy=%u",
         CAP_PICS_CSR_STA_AXI_BW_MON_RD_TRANSACTIONS_OUTSTANDING_GET(rd_trans),
         CAP_PICS_CSR_STA_AXI_BW_MON_RD_TRANSACTIONS_DESS_RDY_GET(rd_trans));
  // Writes:
  printf("\n         wr_latency=%u/%u",
         CAP_PICS_CSR_STA_AXI_BW_MON_WR_LATENCY_AVRG_GET(wr_latency),
         CAP_PICS_CSR_STA_AXI_BW_MON_WR_LATENCY_MAXV_GET(wr_latency));
  printf(" wr_bandwidth=%u/%u",
         CAP_PICS_CSR_STA_AXI_BW_MON_WR_BANDWIDTH_AVRG_GET(wr_bandwidth),
         CAP_PICS_CSR_STA_AXI_BW_MON_WR_BANDWIDTH_MAXV_GET(wr_bandwidth));
  printf(" wr_cnt=%u", wr_cnt);
  printf(" wr_pend=%u, no_drdy=%u",
         CAP_PICS_CSR_STA_AXI_BW_MON_WR_TRANSACTIONS_OUTSTANDING_GET(wr_trans),
         CAP_PICS_CSR_STA_AXI_BW_MON_WR_TRANSACTIONS_DESS_RDY_GET(wr_trans));
  
  printf("\n");
}

void read_all_bw_monitors(int verbose)
{
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
    /*
    printf("MS AXI: ");
    axi_bw_mon(verbose, (CAP_ADDR_BASE_MS_MS_OFFSET +
                         CAP_MS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));
    printf("PXB AXI:");
    axi_bw_mon(verbose, (CAP_ADDR_BASE_PXB_PXB_OFFSET +
                         CAP_PXB_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));
    */

    //    printf("SG  AXI:");
    //    axi_bw_mon(verbose, (CAP_ADDR_BASE_PXB_PXB_OFFSET +
    //                   CAP_SG_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));
 
}

void qstate_lif_dump(int idle_check, int verbose)
{
  u_int32_t cnt[4], size[8], length[8];
  u_int32_t valid, hint, hint_cos;
  u_int64_t base;
  int lif, type, q, max_type;
  int this_size, this_len;

  for(lif=0; lif < 128; lif++) {
    rd_reg32w(CAP_ADDR_BASE_DB_WA_OFFSET +
	      CAP_WA_CSR_DHS_LIF_QSTATE_MAP_BYTE_ADDRESS + (16 * lif), cnt, 4);
    // decode lif qstate table:
    base     = (u_int64_t) CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_0_4_QSTATE_BASE_GET(cnt[0]);
    valid    = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_0_4_VLD_GET(cnt[0]);
    hint     = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_SCHED_HINT_EN_GET(cnt[2]);
    hint_cos = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_SCHED_HINT_COS_GET(cnt[2]);
    // 3 bit size is qstate size: 32B/64B/128B...
    size[0] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_0_4_SIZE0_GET(cnt[0]);
    size[1] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_SIZE1_GET(cnt[1]);
    size[2] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_SIZE2_GET(cnt[1]);
    size[3] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_SIZE3_GET(cnt[1]);
    size[4] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_SIZE4_GET(cnt[1]);
    size[5] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_SIZE5_GET(cnt[2]);
    size[6] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_SIZE6_GET(cnt[2]);
    size[7] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_SIZE7_GET(cnt[2]);
    // 5 bit length is lg2 # entries:
    length[0] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_0_4_LENGTH0_GET(cnt[0]);
    length[1] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_0_4_LENGTH1_0_0_GET(cnt[0]) |
               (CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_LENGTH1_4_1_GET(cnt[1]) << 1);
    length[2] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_LENGTH2_GET(cnt[1]);
    length[3] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_LENGTH3_GET(cnt[1]);
    length[4] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_LENGTH4_GET(cnt[1]);
    length[5] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_LENGTH5_0_0_GET(cnt[1]) |
               (CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_LENGTH5_4_1_GET(cnt[2]) << 1);
    length[6] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_LENGTH6_GET(cnt[2]);
    length[7] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_LENGTH7_GET(cnt[2]);
    
    for (max_type = 0; max_type < 8; max_type++) {
      if (size[max_type] == 0) {
        break;
      }
    }

    if(valid) {
      base = base << 12; // base is 4KB aligned
      printf("LIF %u valid, qstate_base=0x%lx, hint=%u, hint_cos=%u\n",
	     lif, base, hint, hint_cos);
      for(type=0; type < max_type; type++) {
      	this_len = 1 << length[type];
      	this_size = 32 * (1 << size[type]);
      	printf(" type %u type_base = 0x%lx, length=%u entries, qstate_size=%u bytes\n",
      	       type, base, this_len, this_size);
      	for(q=0; q < this_len; q++) {
      	  dump_qstate(idle_check, verbose, q, base + (u_int64_t) (q * this_size));
      	}
      	base += this_size * this_len;
      }
    }
  }
}

void dump_qstate(int idle_check, int verbose, int qid, u_int64_t qaddr)
{
  u_int8_t buf[64];

  rd_mem(qaddr, buf, 64);
  
  typedef struct {
      uint8_t     pc_offset;
      uint8_t     rsvd0;
      uint8_t     cosA : 4;
      uint8_t     cosB : 4;
      uint8_t     cos_sel;
      uint8_t     eval_last;
      uint8_t     host : 4;
      uint8_t     total : 4;
      uint16_t    pid;

      struct {
        uint16_t    pi;
        uint16_t    ci;
      } rings[8];

  } qstate_t;

  qstate_t *q = (qstate_t *)buf;
  
  printf("QID %u: ", qid);
  if(verbose) {
    printf(" total_rings=%02d cosA=%02d cosB=%02d cos_sel=0x%02x pc=0x%02x",
	   q->total, q->cosA, q->cosB, q->cos_sel, q->pc_offset);
  }
  for(uint8_t ring=0; ring < q->total; ring++) {
    printf(" ring %02d: PI=%06d CI=%06d", ring, q->rings[ring].pi, q->rings[ring].ci);
    if((q->rings[ring].pi != q->rings[ring].ci) & idle_check) idle_fail();
  }
  printf("\n");
}

int read_num_entries (u_int32_t base, u_int32_t pi_addr, u_int32_t ci_addr, u_int32_t ring_size_addr)
{
  u_int32_t pi = rd_reg(base+pi_addr);
  u_int32_t ci = rd_reg(base+pi_addr);
  u_int32_t ring_size = rd_reg(base+pi_addr);
  if(pi >= ci)
    return(pi-ci);
  else
    return((ring_size - ci) + pi);
}

void crypto_read_queues(int idle_check, int verbose)
{
  u_int32_t cnt;
  u_int64_t wcnt, aw, dw, wrsp, ar, dr, wrsp_err, rrsp_err;

  printf("==Crypto Queues==\n");
  // Doorbells
  rd_reg32w(CAP_ADDR_BASE_MD_HENS_OFFSET +
	    CAP_HENS_CSR_CNT_DOORBELL_XTS_BYTE_ADDRESS, (uint32_t *)&wcnt, 2);
  if(wcnt!=0) printf(" XTS Doorbells=%ld", wcnt);
  rd_reg32w(CAP_ADDR_BASE_MD_HENS_OFFSET +
	    CAP_HENS_CSR_CNT_DOORBELL_XTS_ENC_BYTE_ADDRESS, (uint32_t *)&wcnt, 2);
  if(wcnt!=0) printf(" XTS ENC Doorbells=%ld", wcnt);
  rd_reg32w(CAP_ADDR_BASE_MD_HENS_OFFSET +
	    CAP_HENS_CSR_CNT_DOORBELL_GCM0_BYTE_ADDRESS, (uint32_t *)&wcnt, 2);
  if(wcnt!=0) printf(" GCM0 Doorbells=%ld", wcnt);
  rd_reg32w(CAP_ADDR_BASE_MD_HENS_OFFSET +
	    CAP_HENS_CSR_CNT_DOORBELL_GCM1_BYTE_ADDRESS, (uint32_t *)&wcnt, 2);
  if(wcnt!=0) printf(" GCM1 Doorbells=%ld", wcnt);
  rd_reg32w(CAP_ADDR_BASE_MD_HENS_OFFSET +
	     CAP_HENS_CSR_CNT_DOORBELL_PK_BYTE_ADDRESS, (uint32_t *)&wcnt, 2);
  if(wcnt!=0) printf(" PK Doorbells=%ld", wcnt);
  // XTS ENC
  cnt = read_num_entries(CAP_ADDR_BASE_MD_HENS_OFFSET,
			 CAP_HENS_CSR_DHS_CRYPTO_CTL_XTS_ENC_PRODUCER_IDX_BYTE_ADDRESS,
			 CAP_HENS_CSR_DHS_CRYPTO_CTL_XTS_ENC_CONSUMER_IDX_BYTE_ADDRESS,
			 CAP_HENS_CSR_DHS_CRYPTO_CTL_XTS_ENC_RING_SIZE_BYTE_ADDRESS);
  if(cnt!=0) printf(" XTS ENC has %d ring entries\n", cnt);
  if((cnt!=0) & idle_check) idle_fail();    
  //. XTS
  cnt = read_num_entries(CAP_ADDR_BASE_MD_HENS_OFFSET,
			 CAP_HENS_CSR_DHS_CRYPTO_CTL_XTS_PRODUCER_IDX_BYTE_ADDRESS,
			 CAP_HENS_CSR_DHS_CRYPTO_CTL_XTS_CONSUMER_IDX_BYTE_ADDRESS,
			 CAP_HENS_CSR_DHS_CRYPTO_CTL_XTS_RING_SIZE_BYTE_ADDRESS);
  if(cnt!=0) printf(" XTS has %d ring entries\n", cnt);
  if((cnt!=0) & idle_check) idle_fail();    
  // GCM0
  cnt = read_num_entries(CAP_ADDR_BASE_MD_HENS_OFFSET,
			 CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM0_PRODUCER_IDX_BYTE_ADDRESS,
			 CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM0_CONSUMER_IDX_BYTE_ADDRESS,
			 CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM0_RING_SIZE_BYTE_ADDRESS);
  if(cnt!=0) printf(" GCM0 has %d ring entries\n", cnt);
  if((cnt!=0) & idle_check) idle_fail();    
  // GCM1
  cnt = read_num_entries(CAP_ADDR_BASE_MD_HENS_OFFSET,
			 CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM1_PRODUCER_IDX_BYTE_ADDRESS,
			 CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM1_CONSUMER_IDX_BYTE_ADDRESS,
			 CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM1_RING_SIZE_BYTE_ADDRESS);
  if(cnt!=0) printf(" GCM1 has %d ring entries\n", cnt);
  if((cnt!=0) & idle_check) idle_fail();    
  // CP hotq
  u_int32_t pi = rd_reg(CAP_ADDR_BASE_MD_HENS_OFFSET +
			CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_HOTQ_PD_IDX_BYTE_ADDRESS);
  u_int32_t ci = rd_reg(CAP_ADDR_BASE_MD_HENS_OFFSET +
			CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_STA_HOTQ_CP_IDX_BYTE_ADDRESS);
  u_int32_t cp_cfg = rd_reg(CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_DIST_BYTE_ADDRESS +
			    CAP_ADDR_BASE_MD_HENS_OFFSET);
  u_int32_t hotq_size = (cp_cfg >> 6) & 0xfff;
  u_int32_t descq_size = (cp_cfg >> 18) & 0xfff;
  if(pi>=ci)
    cnt = pi-ci;
  else
    cnt = (hotq_size - ci) + pi;
  if(cnt!=0) printf(" CP HOTQ has %d ring entries\n", cnt);
  if((cnt!=0) & idle_check) idle_fail();    
  // CP
  pi = rd_reg(CAP_ADDR_BASE_MD_HENS_OFFSET +
	      CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_Q_PD_IDX_BYTE_ADDRESS);
  ci = rd_reg(CAP_ADDR_BASE_MD_HENS_OFFSET +
	      CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_STA_Q_CP_IDX_BYTE_ADDRESS);
  if(pi>=ci)
    cnt = pi-ci;
  else
    cnt = (descq_size - ci) + pi;
  if(cnt!=0) printf(" CP has %d ring entries\n", cnt);
  if((cnt!=0) & idle_check) idle_fail();    
  // DC hotq
  pi = rd_reg(CAP_ADDR_BASE_MD_HENS_OFFSET +
	      CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_HOTQ_PD_IDX_BYTE_ADDRESS);
  ci = rd_reg(CAP_ADDR_BASE_MD_HENS_OFFSET +
	      CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_STA_HOTQ_CP_IDX_BYTE_ADDRESS);
  cp_cfg = rd_reg(CAP_ADDR_BASE_MD_HENS_OFFSET +
		  CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_DIST_BYTE_ADDRESS);
  hotq_size = (cp_cfg >> 6) & 0xfff;
  descq_size = (cp_cfg >> 18) & 0xfff;
  if(pi>=ci)
    cnt = pi-ci;
  else
    cnt = (hotq_size - ci) + pi;
  if(cnt!=0) printf(" DC HOTQ has %d ring entries\n", cnt);
  if((cnt!=0) & idle_check) idle_fail();    
  // DC
  pi = rd_reg(CAP_ADDR_BASE_MD_HENS_OFFSET +
	      CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_Q_PD_IDX_BYTE_ADDRESS);
  ci = rd_reg(CAP_ADDR_BASE_MD_HENS_OFFSET +
	      CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_STA_Q_CP_IDX_BYTE_ADDRESS);
  if(pi>=ci)
    cnt = pi-ci;
  else
    cnt = (descq_size - ci) + pi;
  if(cnt!=0) printf(" DC has %d ring entries\n", cnt);  
  if((cnt!=0) & idle_check) idle_fail();
  // MPP0
  cnt = read_num_entries(CAP_ADDR_BASE_MP_MPNS_OFFSET,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP0_PRODUCER_IDX_BYTE_ADDRESS,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP0_CONSUMER_IDX_BYTE_ADDRESS,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP0_RING_SIZE_BYTE_ADDRESS);
  if(cnt!=0) printf(" MPP0 has %d ring entries\n", cnt);
  if((cnt!=0) & idle_check) idle_fail();
  // MPP1
  cnt = read_num_entries(CAP_ADDR_BASE_MP_MPNS_OFFSET,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP1_PRODUCER_IDX_BYTE_ADDRESS,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP1_CONSUMER_IDX_BYTE_ADDRESS,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP1_RING_SIZE_BYTE_ADDRESS);
  if(cnt!=0) printf(" MPP1 has %d ring entries\n", cnt);
  if((cnt!=0) & idle_check) idle_fail();
  // MPP2
  cnt = read_num_entries(CAP_ADDR_BASE_MP_MPNS_OFFSET,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP2_PRODUCER_IDX_BYTE_ADDRESS,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP2_CONSUMER_IDX_BYTE_ADDRESS,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP2_RING_SIZE_BYTE_ADDRESS);
  if(cnt!=0) printf(" MPP2 has %d ring entries\n", cnt);
  if((cnt!=0) & idle_check) idle_fail();
  // MPP3
  cnt = read_num_entries(CAP_ADDR_BASE_MP_MPNS_OFFSET,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP3_PRODUCER_IDX_BYTE_ADDRESS,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP3_CONSUMER_IDX_BYTE_ADDRESS,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP3_RING_SIZE_BYTE_ADDRESS);
  if(cnt!=0) printf(" MPP3 has %d ring entries\n", cnt);
  if((cnt!=0) & idle_check) idle_fail();
  // MPP4
  cnt = read_num_entries(CAP_ADDR_BASE_MP_MPNS_OFFSET,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP4_PRODUCER_IDX_BYTE_ADDRESS,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP4_CONSUMER_IDX_BYTE_ADDRESS,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP4_RING_SIZE_BYTE_ADDRESS);
  if(cnt!=0) printf(" MPP4 has %d ring entries\n", cnt);
  if((cnt!=0) & idle_check) idle_fail();
  // MPP5
  cnt = read_num_entries(CAP_ADDR_BASE_MP_MPNS_OFFSET,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP5_PRODUCER_IDX_BYTE_ADDRESS,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP5_CONSUMER_IDX_BYTE_ADDRESS,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP5_RING_SIZE_BYTE_ADDRESS);
  if(cnt!=0) printf("MPP5 has %d ring entries\n", cnt);
  if((cnt!=0) & idle_check) idle_fail();
  // MPP6
  cnt = read_num_entries(CAP_ADDR_BASE_MP_MPNS_OFFSET,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP6_PRODUCER_IDX_BYTE_ADDRESS,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP6_CONSUMER_IDX_BYTE_ADDRESS,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP6_RING_SIZE_BYTE_ADDRESS);
  if(cnt!=0) printf(" MPP6 has %d ring entries\n", cnt);
  if((cnt!=0) & idle_check) idle_fail();
  // MPP7
  cnt = read_num_entries(CAP_ADDR_BASE_MP_MPNS_OFFSET,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP7_PRODUCER_IDX_BYTE_ADDRESS,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP7_CONSUMER_IDX_BYTE_ADDRESS,
			 CAP_MPNS_CSR_DHS_CRYPTO_CTL_MPP7_RING_SIZE_BYTE_ADDRESS);
  if(cnt!=0) printf(" MPP7 has %d ring entries\n", cnt);
  if((cnt!=0) & idle_check) idle_fail();

  if(verbose==1) {
    u_int32_t stride = (CAP_HENS_CSR_CNT_AXI_AW_GCM1_BYTE_ADDRESS -
			CAP_HENS_CSR_CNT_AXI_AW_GCM0_BYTE_ADDRESS);
    for(int i=0; i<9; i++) {
      rd_reg32w(CAP_ADDR_BASE_MD_HENS_OFFSET +
		CAP_HENS_CSR_CNT_AXI_AW_GCM0_BYTE_ADDRESS + (i * stride), (uint32_t *)&aw, 1);
      rd_reg32w(CAP_ADDR_BASE_MD_HENS_OFFSET +
		CAP_HENS_CSR_CNT_AXI_DW_GCM0_BYTE_ADDRESS + (i * stride), (uint32_t *)&dw, 1);
      rd_reg32w(CAP_ADDR_BASE_MD_HENS_OFFSET +
		CAP_HENS_CSR_CNT_AXI_WRSP_GCM0_BYTE_ADDRESS + (i * stride), (uint32_t *)&wrsp, 1);
      rd_reg32w(CAP_ADDR_BASE_MD_HENS_OFFSET +
		CAP_HENS_CSR_CNT_AXI_AR_GCM0_BYTE_ADDRESS + (i * stride), (uint32_t *)&ar, 1);
      rd_reg32w(CAP_ADDR_BASE_MD_HENS_OFFSET +
		CAP_HENS_CSR_CNT_AXI_DR_GCM0_BYTE_ADDRESS + (i * stride), (uint32_t *)&dr, 1);
      rd_reg32w(CAP_ADDR_BASE_MD_HENS_OFFSET +
		CAP_HENS_CSR_CNT_AXI_WRSP_ERR_GCM0_BYTE_ADDRESS + (i * stride), (uint32_t *)&wrsp_err, 1);
      rd_reg32w(CAP_ADDR_BASE_MD_HENS_OFFSET +
		CAP_HENS_CSR_CNT_AXI_RRSP_ERR_GCM0_BYTE_ADDRESS + (i * stride), (uint32_t *)&rrsp_err, 1);
      cout << offloadname[i];
      printf(" AW=%ld DW=%ld WRSP=%ld AR=%ld DR=%ld WR_ERR=%ld RR_ERR=%ld\n",
	     aw, dw, wrsp, ar, dr, wrsp_err, rrsp_err);
    }
  }

}  

void pbx_read_state(int idle_check)
{
  u_int64_t cnt;
  u_int32_t cnt32[2];
  
  printf("PCIe target");
  rd_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_CNT_TGT_TOT_AXI_WR_BYTE_ADDRESS, (uint32_t *)&cnt, 2);
  printf(" writes=%ld", cnt);
  if((cnt!=0) & idle_check) idle_fail();
  rd_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_CNT_TGT_TOT_AXI_RD_BYTE_ADDRESS, (uint32_t *)&cnt, 2);
  printf(" reads=%ld", cnt);
  if((cnt!=0) & idle_check) idle_fail();
  rd_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_STA_TGT_AXI_PENDING_BYTE_ADDRESS, cnt32, 2);
  printf(" pending=%d", 
	 CAP_PXB_CSR_STA_TGT_AXI_PENDING_STA_TGT_AXI_PENDING_0_2_IDS_P0_GET(cnt32[0]));
  if((CAP_PXB_CSR_STA_TGT_AXI_PENDING_STA_TGT_AXI_PENDING_0_2_IDS_P0_GET(cnt32[0])!=0) &
     idle_check) idle_fail();
  printf("\n");
  printf("PCIe Initiator");
  rd_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_CNT_ITR_TOT_AXI_WR_BYTE_OFFSET, (uint32_t *)&cnt, 2);
  printf(" writes=%ld", cnt);
  if((cnt!=0) & idle_check) idle_fail();
  rd_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_CNT_ITR_TOT_AXI_WR_BYTE_OFFSET, (uint32_t *)&cnt, 2);
  printf(" bytes=%ld", cnt);
  if((cnt!=0) & idle_check) idle_fail();
  printf(" pending=%d", rd_reg(CAP_ADDR_BASE_PXB_PXB_OFFSET +
			       CAP_PXB_CSR_STA_ITR_AXI_WR_NUM_IDS_BYTE_ADDRESS));
  printf("\n");
  printf("PCIe Initiator");
  rd_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_CNT_ITR_TOT_AXI_RD_BYTE_OFFSET, (uint32_t *)&cnt, 2);
  printf(" reads=%ld", cnt);
  if((cnt!=0) & idle_check) idle_fail();
  rd_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_CNT_ITR_TOT_AXI_RD_BYTE_OFFSET, (uint32_t *)&cnt, 2);
  printf(" bytes=%ld", cnt);
  if((cnt!=0) & idle_check) idle_fail();
  printf(" pending=%d", rd_reg(CAP_ADDR_BASE_PXB_PXB_OFFSET +
			       CAP_PXB_CSR_STA_ITR_AXI_RD_NUM_IDS_BYTE_ADDRESS));
  printf("\n");
  rd_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_SAT_ITR_RDLAT0_BYTE_OFFSET, (uint32_t *)&cnt, 2);
  printf("     read latency <250 clks=%ld;", cnt);
  rd_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_SAT_ITR_RDLAT1_BYTE_OFFSET, (uint32_t *)&cnt, 2);
  printf(" <500=%ld;", cnt);
  rd_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_SAT_ITR_RDLAT2_BYTE_OFFSET, (uint32_t *)&cnt, 2);
  printf(" <1000=%ld;", cnt);
  rd_reg32w(CAP_ADDR_BASE_PXB_PXB_OFFSET +
	    CAP_PXB_CSR_SAT_ITR_RDLAT3_BYTE_OFFSET, (uint32_t *)&cnt, 2);
  printf(" >1000=%ld", cnt);
  printf("\n");
}


struct pb_sta_oq_t {
  u_int32_t q0 : 14;
  u_int32_t q1 : 14;
  u_int32_t q2 : 14;
  u_int32_t q3 : 14;
  u_int32_t q4 : 14;
  u_int32_t q5 : 14;
  u_int32_t q6 : 14;
  u_int32_t q7 : 14;
  u_int32_t q8 : 14;
  u_int32_t q9 : 14;
  u_int32_t q10 : 14;
  u_int32_t q11 : 14;
  u_int32_t q12 : 14;
  u_int32_t q13 : 14;
  u_int32_t q14 : 14;
  u_int32_t q15 : 14;
  u_int32_t q16 : 14;
  u_int32_t q17 : 14;
  u_int32_t q18 : 14;
  u_int32_t q19 : 14;
  u_int32_t q20 : 14;
  u_int32_t q21 : 14;
  u_int32_t q22 : 14;
  u_int32_t q23 : 14;
  u_int32_t q24 : 14;
  u_int32_t q25 : 14;
  u_int32_t q26 : 14;
  u_int32_t q27 : 14;
  u_int32_t q28 : 14;
  u_int32_t q29 : 14;
  u_int32_t q30 : 14;
  u_int32_t q31 : 14;
};

void pb_read_queues(int idle_check)
{
  u_int32_t oq_cnt[14];
  u_int32_t depth, index, shift;
  bool portempty;
  int port;

  for(port=0; port<12; port++) {
    portempty=true;
    // Output Queues:
    rd_reg32w(CAP_ADDR_BASE_PB_PBC_OFFSET +
	      CAP_PBC_CSR_STA_OQ_BYTE_ADDRESS +
	      (port * 4 * CAP_PBC_CSR_STA_OQ_ARRAY_ELEMENT_SIZE), oq_cnt, 14);

    for(int q=0; q<32; q++) {
      index =(q*14)/32;
      shift =(q*14)%32;
      depth = oq_cnt[index] >> shift;
      if(shift > 18) depth |= oq_cnt[index+1] << (32 - shift);
      depth &= 0x3fff;
      if(depth!=0) {
	if(portempty==true) {
	  cout << "PB OQ " << portname[port];
	  portempty = false;
	}
	printf(" Q[%d]=%u entries", q, depth);
	if((depth!=0) & idle_check) idle_fail();  
      }
    }
    if(portempty==false) printf("\n");
  }
  // Input Buffers:
  u_int32_t iq_port_addr[12] = {CAP_PBC_CSR_PORT_0_STA_ACCOUNT_BYTE_ADDRESS,
				CAP_PBC_CSR_PORT_1_STA_ACCOUNT_BYTE_ADDRESS,
				CAP_PBC_CSR_PORT_2_STA_ACCOUNT_BYTE_ADDRESS,
				CAP_PBC_CSR_PORT_3_STA_ACCOUNT_BYTE_ADDRESS,
				CAP_PBC_CSR_PORT_4_STA_ACCOUNT_BYTE_ADDRESS,
				CAP_PBC_CSR_PORT_5_STA_ACCOUNT_BYTE_ADDRESS,
				CAP_PBC_CSR_PORT_6_STA_ACCOUNT_BYTE_ADDRESS,
				CAP_PBC_CSR_PORT_7_STA_ACCOUNT_BYTE_ADDRESS,
				CAP_PBC_CSR_PORT_8_STA_ACCOUNT_BYTE_ADDRESS,
				CAP_PBC_CSR_PORT_9_STA_ACCOUNT_BYTE_ADDRESS,
				CAP_PBC_CSR_PORT_10_STA_ACCOUNT_BYTE_ADDRESS,
				CAP_PBC_CSR_PORT_11_STA_ACCOUNT_BYTE_ADDRESS};
  int iq_port_queues[12] = {8,
			    8,
			    8,
			    8,
			    8,
			    8,
			    8,
			    8,
			    8,
			    16,
			    32,
			    32};
  u_int32_t iq_cnt[28];
  for(port=0; port<9; port++) {
    portempty=true;
    rd_reg32w(CAP_ADDR_BASE_PB_PBC_OFFSET + iq_port_addr[port], iq_cnt, (28 * iq_port_queues[port]) / 32);
    for(int q=0; q<iq_port_queues[port]; q++) {
      index =(q*28)/32;
      shift =(q*28)%32;
      depth = iq_cnt[index] >> shift;
      if(shift > 4) depth |= iq_cnt[index+1] << (32 - shift);
      depth &= 0x3fff; // {watermark[14], occupancy[14]}
      if(depth!=0) {
	if(portempty==true) {
	  cout << "PB IQ " << portname[port];
	  portempty = false;
	}
	printf(" Q[%d]=%u entries", q, depth);
	if((depth!=0) & idle_check) idle_fail();  
      }
    }
    if(portempty==false) printf("\n");
  }
}

