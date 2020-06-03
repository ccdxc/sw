
#include "elb_pcie_sw_api.h"
#include "elb_sbus_api.h"
#include "elb_pp_c_hdr.h"
#include "elb_top_csr_defines.h"
#include <stdio.h>
#ifndef _DISABLE_SKNOBS_
#include "sknobs.h"
#endif
char glb_log_buf[10240*8];

#define ELB_PCIE_SBUS_IDCODE_MAX_PORT 32
#define NUM_PP_SBUS_DEV 67
#define ELB_PCIE_LANES_PER_PHY_PORT 4
#define ELB_PCIE_MAX_LANES 16

#ifdef ELBA_SW

#ifndef PCIE_MSG
#define PCIE_MSG SW_PRINT
#endif

#ifndef PCIE_INFO
#define PCIE_INFO(...) do {} while (0)
#endif

#ifndef PCIE_ERR
#define PCIE_ERR SW_PRINT
#endif


#else // ELBA_SW
#ifndef PCIE_INFO
#define PCIE_INFO(...) { \
    sprintf(glb_log_buf,##__VA_ARGS__); \
    pen_c_printf_msg(glb_log_buf); \
}
#endif

#ifndef PCIE_MSG
#define PCIE_MSG(...) { \
    sprintf(glb_log_buf,##__VA_ARGS__); \
    pen_c_printf_msg(glb_log_buf); \
}
#endif

#ifndef PCIE_ERR
#define PCIE_ERR(...) { \
    sprintf(glb_log_buf,##__VA_ARGS__); \
    pen_c_printf_err(glb_log_buf); \
}
#endif

#endif

#ifdef ELBA_SW
#define PCIE_SD_CORE_STATUS_POLL_LIMIT 1000
#define PCIE_SD_CORE_STATUS_DELAY_UNIT 1000
#define PCIE_CORE_INTERRUPTS_SW_IN_PROGRESS_POLL_LIMIT 1000
#define PCIE_CORE_INTERRUPTS_SW_IN_PROGRESS_PER_LANE_DELAY_UNIT 1000
#define PCIE_CORE_INTERRUPTS_SW_IN_PROGRESS_PER_LANE_POLL_LIMIT 1000
#define PCIE_SKIP_FIRST_POLL_DELAY 1
#else
#define PCIE_SD_CORE_STATUS_POLL_LIMIT 10000
#define PCIE_SD_CORE_STATUS_DELAY_UNIT 1000
#define PCIE_CORE_INTERRUPTS_SW_IN_PROGRESS_POLL_LIMIT 10000
#define PCIE_CORE_INTERRUPTS_SW_IN_PROGRESS_PER_LANE_DELAY_UNIT 1000
#define PCIE_CORE_INTERRUPTS_SW_IN_PROGRESS_PER_LANE_POLL_LIMIT 10000
#define PCIE_SKIP_FIRST_POLL_DELAY 0
#endif

#ifndef SBUS_ROM_MAGIC
#define SBUS_ROM_MAGIC 0x53554253
#endif


/**
 * @brief Check sd core status and Return 
 * 1 if sucessful 
 * 0 if fail
 *
 * @param chip_id   unused 
 * @param inst_id   this will select 0(pp0) or 1(pp1)
 * @param phy_port  select physical port. valid values are 0..3 
 * @param lanes     number of lanes for this phy_port. valid values are 0..15
 * @return status   1 if successful, 0 if fail
 */
int pcie_check_sd_core_status (int chip_id, int inst_id, unsigned phy_port, unsigned lanes) {
    uint64_t pp_base = ELB_ADDR_BASE_PP_PP_0_OFFSET + (inst_id *  ELB_ADDR_BASE_PP_PP_0_SIZE);

    if(lanes > ELB_PCIE_MAX_LANES) {
        PCIE_ERR("pcie_check_sd_core_status: max supported lanes %d argument has %d", ELB_PCIE_MAX_LANES, lanes);
    }


    int count= 0;
    int done = 0;
    PCIE_INFO("PCIE:SW FUNCTION 2\n");
    uint32_t local_storage[ELB_PP_CSR_STA_PP_SD_CORE_STATUS_SIZE];
    while(!done) {
        if(!done && !(PCIE_SKIP_FIRST_POLL_DELAY && (count == 0))) SLEEP(PCIE_SD_CORE_STATUS_DELAY_UNIT);
        done = 1;
        for(int i=0; i < ELB_PP_CSR_STA_PP_SD_CORE_STATUS_SIZE; i++) {
            local_storage[i] = pen_c_csr_read(pp_base + ELB_PP_CSR_STA_PP_SD_CORE_STATUS_BYTE_ADDRESS + (i*4), 1,1);
            if( (i >= (phy_port*ELB_PCIE_LANES_PER_PHY_PORT)) && (i < ((phy_port*ELB_PCIE_LANES_PER_PHY_PORT)+lanes)) ) {
                if((local_storage[i] & 0x20) == 0) {
                    if(count % 10) {
                        PCIE_INFO("sta_pp_sd_core_status not set for lane %d\n",i);
                    }
                    done = 0; break;
                }
            }
        }
        count++;
        if(count > PCIE_SD_CORE_STATUS_POLL_LIMIT) {
            PCIE_ERR("sta_pp_sd_core_status is not set\n");
            for(int i=0; i < ELB_PP_CSR_STA_PP_SD_CORE_STATUS_SIZE; i++) {
                PCIE_INFO("sta_pp_sd_core_status.lane%d : 0x%x\n",i,local_storage[i]);
            }
            done = 1;
            return 1;
        }

    }
    return 0;
}


/**
 * @brief process int_code and int_data using interrupt interface
 * @param chip_id   unused 
 * @param inst_id   this will select 0(pp0) or 1(pp1)
 * @param int_code  refer avago documents 
 * @param int_data  refer avago documents 
 * @param delay_num this is for sim only. Time to wait between polls 
 * @param phy_port  select physical port. valid values are 0..3 
 * @param lanes     number of lanes for this phy_port. valid values are 0..15
 * @param check_output_data pass 1 if we need to check data, 0 otherwise
 * @param exp_data this data will be compared with HW if check_output_data is set
 */
int pcie_core_interrupts_sw (int chip_id, int inst_id, int int_code, int int_data, int delay_num, int phy_port, int lanes, int check_output_data, int exp_data) { 

    uint64_t pp_base = ELB_ADDR_BASE_PP_PP_0_OFFSET + (inst_id *  ELB_ADDR_BASE_PP_PP_0_SIZE);
    unsigned ret_val = 0;

    if(lanes > ELB_PCIE_MAX_LANES) {
        PCIE_ERR("pcie_core_interrupts_sw: max supported lanes %d argument has %d", ELB_PCIE_MAX_LANES, lanes);
    }
    int pp_pcsd_intr_beg  = phy_port * 4;
    int pp_pcsd_intr_end  = (phy_port * 4) + lanes;
    int pp_pcsd_intr_lane = ((1 << lanes) -1) << (phy_port * 4);


    PCIE_INFO("PCIE:SW FUNCTION 4\n");
    // *** issue core_interrupt to serdes ***
    PCIE_INFO("inside serdes interrupts\n");

    uint32_t local_storage = 0;
    // set up interrupt code and data per lane
    for(int j = pp_pcsd_intr_beg; j < pp_pcsd_intr_end; j++) {
        local_storage = ELB_PP_CSR_CFG_PP_PCSD_INTERRUPT_CODE_SET(int_code);
        local_storage = ELB_PP_CSR_CFG_PP_PCSD_INTERRUPT_DATA_MODIFY(local_storage, int_data);
        pen_c_csr_write(pp_base + ELB_PP_CSR_CFG_PP_PCSD_INTERRUPT_BYTE_ADDRESS + (j* 4), local_storage, 1, 1);
    }

    // issue interrupt request, with lane mask for all  configured lanes
    local_storage = ELB_PP_CSR_CFG_PP_PCSD_INTERRUPT_REQUEST_LANEMASK_SET(pp_pcsd_intr_lane);
    pen_c_csr_write(pp_base + ELB_PP_CSR_CFG_PP_PCSD_INTERRUPT_REQUEST_BYTE_OFFSET, local_storage, 1, 1);

    // poll for interrupt_in_progress to go high, for all lanes
    int done=0;
    int count= 0;
    int sleep_number = ((PCIE_CORE_INTERRUPTS_SW_IN_PROGRESS_PER_LANE_DELAY_UNIT+delay_num-1)/PCIE_CORE_INTERRUPTS_SW_IN_PROGRESS_PER_LANE_DELAY_UNIT);
    while(!done) {
        if(!done && !(PCIE_SKIP_FIRST_POLL_DELAY && (count == 0))) SLEEP(sleep_number);
        local_storage = pen_c_csr_read(pp_base + ELB_PP_CSR_STA_PP_PCSD_INTERRUPT_IN_PROGRESS_BYTE_OFFSET, 1,1);
        done = ((ELB_PP_CSR_STA_PP_PCSD_INTERRUPT_IN_PROGRESS_PER_LANE_GET(local_storage) & pp_pcsd_intr_lane) == pp_pcsd_intr_lane);
        count++;
        if(count > PCIE_CORE_INTERRUPTS_SW_IN_PROGRESS_POLL_LIMIT) {
            PCIE_ERR("sta_pp_pcsd_interrupt_in_progress is not set int_code : int_code 0x%x int_data 0x%x\n",  int_code, int_data);
            PCIE_MSG("sta_pp_pcsd_interrupt_in_progress : per_lane 0x%x\n",  local_storage);
            ret_val++;
            done = 1;
        }
    }

    // deassert the interrupt request 
    local_storage = ELB_PP_CSR_CFG_PP_PCSD_INTERRUPT_REQUEST_LANEMASK_SET(0);
    pen_c_csr_write(pp_base + ELB_PP_CSR_CFG_PP_PCSD_INTERRUPT_REQUEST_BYTE_OFFSET, local_storage, 1, 1);


    // poll for interrupt_in_progress to go LOW, for all lanes
    done=0;
    count= 0;
    while(!done) {
        if(!done && !(PCIE_SKIP_FIRST_POLL_DELAY && (count == 0)) ) SLEEP(PCIE_CORE_INTERRUPTS_SW_IN_PROGRESS_PER_LANE_DELAY_UNIT);
        local_storage = pen_c_csr_read(pp_base + ELB_PP_CSR_STA_PP_PCSD_INTERRUPT_IN_PROGRESS_BYTE_OFFSET, 1,1);
        done = ((ELB_PP_CSR_STA_PP_PCSD_INTERRUPT_IN_PROGRESS_PER_LANE_GET(local_storage) & pp_pcsd_intr_lane) == 0);

        count++;
        if(count > PCIE_CORE_INTERRUPTS_SW_IN_PROGRESS_PER_LANE_POLL_LIMIT) {
            PCIE_ERR("sta_pp_pcsd_interrupt_in_progress is not set int_code : int_code 0x%x int_data 0x%x\n",  int_code, int_data);
            PCIE_MSG("sta_pp_pcsd_interrupt_in_progress : per_lane 0x%x\n",  local_storage);
            done = 1;
        }
    }

    // read interrupt response data 
    for(int i=0; i < ELB_PP_CSR_STA_PP_PCSD_INTERRUPT_DATA_OUT_SIZE; i++) {
        local_storage = pen_c_csr_read(pp_base + ELB_PP_CSR_STA_PP_PCSD_INTERRUPT_DATA_OUT_BYTE_ADDRESS + (i*4), 1,1);
        PCIE_MSG("PCSD interrupt data_out lane %d : 0x%x\n", (i*2),  ELB_PP_CSR_STA_PP_PCSD_INTERRUPT_DATA_OUT_STA_PP_PCSD_INTERRUPT_DATA_OUT_0_8_LANE0_GET(local_storage));
        PCIE_MSG("PCSD interrupt data_out lane %d : 0x%x\n", (i*2)+1,  ELB_PP_CSR_STA_PP_PCSD_INTERRUPT_DATA_OUT_STA_PP_PCSD_INTERRUPT_DATA_OUT_0_8_LANE1_GET(local_storage));
        if(check_output_data && ((i*2) >= pp_pcsd_intr_beg) && ((i*2) <= pp_pcsd_intr_end) && ((local_storage & 0xffff) != (exp_data & 0xffff))) {
            PCIE_ERR("PCSD_INTERRUPT_DATA_OUT Mismatch on %d: Expected data_out 0x%x received 0x%x\n", (i*2), (exp_data & 0xffff), local_storage & 0xffff);
            ret_val++;
        }
        if(check_output_data && (((i*2)+1) >= pp_pcsd_intr_beg) && (((i*2)+1) <= pp_pcsd_intr_end) && (((local_storage >> 16) & 0xffff) != (exp_data & 0xffff))) {
            PCIE_ERR("PCSD_INTERRUPT_DATA_OUT Mismatch on %d: Expected data_out 0x%x received 0x%x\n", (i*2)+1, exp_data & 0xffff, (local_storage >> 16) & 0xffff);
            ret_val++;
        }


    }
    PCIE_INFO("serdes core_interrupt complete\n");
    return ret_val;
}


#ifndef _PCIE_BLOCK_LEVEL_    
/**
 * @brief program sbus clock 
 * @param chip_id       unused 
 * @param sbus_clk_div  valid values are 6 and 1. All other values will fall back to 1
 */
void elb_pcie_sbus_clk(int chip_id, int sbus_clk_div) { 
    if (sbus_clk_div == 6) {
        PCIE_MSG("TEST:SBUS elb_top_pp_sbus_test clk divider =6\n");
        elb_pp_sbus_write(chip_id, 0xfe, 0xa, 0x3);
        elb_pp_sbus_write(chip_id, 0xfe, 0xb, 0x5);
        elb_pp_sbus_write(chip_id, 0xfe, 0xa, 0x83);
    } else { 
        PCIE_MSG("TEST:SBUS elb_top_pp_sbus_test clk divider =1 (div2)\n");
        elb_pp_sbus_write(chip_id, 0xfe, 0xa, 0x1);
    } 
}

/**
 * @brief do idcode check on pcie sbus ring 
 * @param chip_id       unused 
 */
int elb_pcie_sbus_idcode_check(int chip_id) {
    int dev_idcode[NUM_PP_SBUS_DEV] = { 0x2,
        0x1, 0x9, 0x1, 0x9, 0x1, 0x9, 0x1, 0x9,       // port0: pcie serdes, PCS
        0x1, 0x9, 0x1, 0x9, 0x1, 0x9, 0x1, 0x9,       // port1: pcie serdes, PCS
        0x1, 0x9, 0x1, 0x9, 0x1, 0x9, 0x1, 0x9,       // port2: pcie serdes, PCS
        0x1, 0x9, 0x1, 0x9, 0x1, 0x9, 0x1, 0x9,       // port3: pcie serdes, PCS
        0x1, 0x9, 0x1, 0x9, 0x1, 0x9, 0x1, 0x9,       // port4: pcie serdes, PCS
        0x1, 0x9, 0x1, 0x9, 0x1, 0x9, 0x1, 0x9,       // port5: pcie serdes, PCS
        0x1, 0x9, 0x1, 0x9, 0x1, 0x9, 0x1, 0x9,       // port6: pcie serdes, PCS
        0x1, 0x9, 0x1, 0x9, 0x1, 0x9, 0x1, 0x9,       // port7: pcie serdes, PCS
        0xa, 0xa                  // pll_pcie_0,1
    };                     


    int rd_data;
    int last_sbus_address;
    //if(pcie_serdes_idcode_check) {
    rd_data = elb_pp_sbus_read(chip_id, 0xfe, 0xff);
    if (rd_data == dev_idcode[0]) {
        PCIE_MSG("TEST:SBUS PASS elb_top_pp_sbus_test sbus controller idcode:0x%x\n", rd_data);
    } else {
        PCIE_ERR("TEST:SBUS elb_top_pp_sbus_test sbus controller idcode:0x%x exp:0x%x \n",rd_data, dev_idcode[0]);
        return -1;
    }

    last_sbus_address = elb_pp_sbus_read(0, 0xfe, 0x2);
    if (last_sbus_address == NUM_PP_SBUS_DEV) {
        PCIE_MSG("TEST:SBUS PASS elb_top_pp_sbus_test last_sbus_address:0x%x\n",last_sbus_address);
    } else {
        PCIE_ERR("TEST:SBUS elb_top_pp_sbus_test last_sbus_address:0x%x exp:0x%x\n", last_sbus_address, NUM_PP_SBUS_DEV);
        return -1;
    }
    for (int aa=1; aa<last_sbus_address; aa++) {
        rd_data = elb_pp_sbus_read(chip_id, aa, 0xff);
        if (rd_data == dev_idcode[aa]) {
            PCIE_MSG("TEST:SBUS PASS elb_top_pp_sbus_test sbus controller idcode:0x%x\n", rd_data);
        } else {
            PCIE_ERR("TEST:SBUS elb_top_pp_sbus_test sbus controller idcode:0x%x exp:0x%x\n", rd_data, dev_idcode[aa]);
            return -1;
        }
    }
    //}

    return 0;
}
#endif

/**
 * @brief create sbus group for a given px_port.
 * After creating group, download firmware if needed
 * @param chip_id   unused 
 * @param bcast_all_lanes if this is 1, then broadcast firmware operation to all lanes 
 * @param px_port select physical port. valid values are 0..7
 * @param lanes     number of lanes for this px_port. valid values are 0..15
 * @param bgroup    group number for broadcast. valid values 0..1 
 */
void elb_pcie_init_sbus(int chip_id, int bcast_all_lanes, int px_port, int lanes, int bgroup, char * file_name) {
#ifndef _PCIE_BLOCK_LEVEL_    
#ifndef _ZEBU_ // exclude from zebu 
        for (int aa=1; aa<NUM_PP_SBUS_DEV; aa++) {
            // there are ELB_PCIE_LANES_PER_PHY_PORT per port and there are N lanes 
            // each lane reuqires 2 IDs
            if( (aa < NUM_PP_SBUS_DEV-2) && ((aa % 2) == 1)) { // even id
                if(bcast_all_lanes || 
                   ((aa >= (px_port* ELB_PCIE_LANES_PER_PHY_PORT * 2)) && (aa < ((px_port* ELB_PCIE_LANES_PER_PHY_PORT * 2) + (lanes * 2)) ) ) ) {
                    set_sbus_broadcast_grp(0, 0, (2*ELB_PCIE_SBUS_IDCODE_MAX_PORT) - aa , 0xe1 + px_port, bgroup % 2);
                }
            }
        }
       PCIE_INFO("TEST:SBUS upload_sbus_master_firmware grp%d\n", bgroup);
       upload_sbus_master_firmware(0, 0, 0xe1 + px_port, file_name);
#endif //  _ZEBU_
#endif  
}


/**
 * @brief
 *  Perform PCS reset OR unreset
 * @param chip_id   unused 
 * @param pp_inst   0 (pp0) OR 1 (pp1) 
 * @param phy_port  port number within pp_inst, valid values 0..3 
 * @param val       valid values: 0 or 1 
 */

void pcie_pcs_reset(int chip_id, int pp_inst, int phy_port, int val) { 
    uint64_t pp_base = ELB_ADDR_BASE_PP_PP_0_OFFSET + (pp_inst *  ELB_ADDR_BASE_PP_PP_0_SIZE);

    uint32_t local_storage = 0;
    // *** deassert PCS RESET_N  ***
    local_storage = pen_c_csr_read(pp_base + ELB_PP_CSR_CFG_PP_PCS_RESET_N_BYTE_ADDRESS, 1,1);
    if(val == 1) {
        local_storage = local_storage | (1 << phy_port);
    } else {
        local_storage = local_storage & ~(1 << phy_port);
    }
    pen_c_csr_write(pp_base + ELB_PP_CSR_CFG_PP_PCS_RESET_N_BYTE_ADDRESS, local_storage, 1,1);
}

/**
 * @brief
 *      TODO: PLL and link width are not part of this
 *      pcs reset
 *      perform sd core status
 *      issue core interrupts
 *      enable pcs interrupts 
 * @param chip_id   unused 
 * @param pp_inst   pp0/pp1 
 * @param phy_port  port number within pp_inst, valid values 0..3 
 * @param lanes     lanes associated with this phy_port. Valid values 0..15
 * @param issue_core_interrupts option to issue core interrupts - for sim, this can be 0 to speed up
 * @param poll_sd_core_status   option to poll for sd core status - for sim, this can be 0 to speed up 
 * @param delay_num             delay between reading 2 sta values - for sim
 */
int elb_pcie_serdes_init(int chip_id, int pp_inst, int phy_port, int lanes, int issue_core_interrupts, int poll_sd_core_status, int delay_num, int exp_build_id, int exp_rev_id) { 
    uint64_t pp_base = ELB_ADDR_BASE_PP_PP_0_OFFSET + (pp_inst *  ELB_ADDR_BASE_PP_PP_0_SIZE);
    unsigned ret_val = 0;

    uint32_t local_storage = 0;
    pcie_pcs_reset(chip_id, pp_inst, phy_port, 1);

    if(lanes > ELB_PCIE_MAX_LANES) {
        PCIE_ERR("pcie_core_interrupts_sw: max supported lanes %d argument has %d", ELB_PCIE_MAX_LANES, lanes);
    }

    if(!poll_sd_core_status) {
        SLEEP(1000);
    } else {
        if(pcie_check_sd_core_status(chip_id, pp_inst, phy_port, lanes)) {
            ret_val++;
            PCIE_ERR("pcie_check_sd_core_status failed\n"); return ret_val;
        }
    }

    if(issue_core_interrupts) {
        // *** issue core interrupts to Serdes ***
        // build ID 
        ret_val += pcie_core_interrupts_sw(chip_id, pp_inst, 0, 0, delay_num, phy_port, lanes, (exp_build_id != 0 ? 1 : 0), exp_build_id );
        // Rev ID 
        ret_val += pcie_core_interrupts_sw(chip_id, pp_inst, 0x3f, 0, delay_num, phy_port, lanes, (exp_rev_id != 0 ? 1 : 0), exp_rev_id);
        // Int_run_iCal_on_Eq_Eval for Equalization : 
        ret_val += pcie_core_interrupts_sw(chip_id, pp_inst, 0x26, 0x5201, delay_num, phy_port, lanes, 0,0);
        // DFE_0, DFE_1 and DFE_2 params 
        ret_val += pcie_core_interrupts_sw(chip_id, pp_inst, 0x26, 0x000E, delay_num, phy_port, lanes, 0,0);
        ret_val += pcie_core_interrupts_sw(chip_id, pp_inst, 0x26, 0x0102, delay_num, phy_port, lanes, 0,0);
        ret_val += pcie_core_interrupts_sw(chip_id, pp_inst, 0x26, 0x0238, delay_num, phy_port, lanes, 0,0);

#ifndef _CSV_INCLUDED_    
        // Check FW CRC : Caution: interrupt 0x3c takes long time for response in simulations -- ~2ms 
        ret_val += pcie_core_interrupts_sw(chip_id, pp_inst, 0x3c, 0, delay_num, phy_port, lanes, 1,0);
#endif         
    }

    // *** enable PCS interrupts ***
    local_storage = pen_c_csr_read(pp_base + ELB_PP_CSR_CFG_PP_PCS_INTERRUPT_DISABLE_BYTE_ADDRESS, 1,1);
    local_storage =  local_storage & ~(((1 << lanes)-1) << ((4*phy_port))); // zero out lanes
    pen_c_csr_write(pp_base + ELB_PP_CSR_CFG_PP_PCS_INTERRUPT_DISABLE_BYTE_ADDRESS, local_storage, 1,1);
    return ret_val; 
}


int elb_pcie_upload_sbus_bin_file(int chip_id, int bcast_all_lanes, int px_port, int lanes, int bgroup, int nwords, void * ctx) {
#ifndef _PCIE_BLOCK_LEVEL_    
#ifndef _ZEBU_ // exclude from zebu 
    for (int aa=1; aa<NUM_PP_SBUS_DEV; aa++) {
        // each lane reuqires 2 IDs
        if( (aa < NUM_PP_SBUS_DEV-2) && ((aa % 2) == 1)) { // even id
            if(bcast_all_lanes || 
                    ((aa >= (px_port* ELB_PCIE_LANES_PER_PHY_PORT * 2)) && (aa < ((px_port* ELB_PCIE_LANES_PER_PHY_PORT * 2) + (lanes * 2)) ) ) ) {
                set_sbus_broadcast_grp(0, 0, (2*ELB_PCIE_SBUS_IDCODE_MAX_PORT) - aa , 0xe1 + px_port, bgroup % 2);
            }
        }
    }
    PCIE_INFO("TEST:SBUS upload_sbus_master_firmware grp%d\n", bgroup);
#endif //  _ZEBU_

    elb_pp_sbus_write(chip_id, 0xe1 + px_port, 0x7, 0x11);
    elb_pp_sbus_write(chip_id, 0xe1 + px_port, 0x7, 0x10);
    elb_pp_sbus_write(chip_id, 0xe1 + px_port, 0x0, 0x40000000);

    unsigned int data;

    unsigned word_cnt = 0;
#ifdef ELBA_SW   // FIXME: Review 
    while(word_cnt < nwords) {
        data = *((uint32_t *) ctx + word_cnt);
        elb_pp_sbus_write(chip_id, 0xe1 + px_port, 0xa, data);
        word_cnt++;
    }

#else
    while(fread(&data, 4, 1, (FILE *) ctx) > 0) {
        elb_pp_sbus_write(chip_id, 0xe1 + px_port, 0xa, data);
        word_cnt++;
    }
    if(word_cnt != nwords) {
        PCIE_ERR("expected SBUS words 0x%x received 0x%x\n", nwords, word_cnt);
    }
    fclose(ctx);
#endif

    elb_pp_sbus_write(chip_id, 0xe1 + px_port, 0x0, 0); // IMEM override off
    elb_pp_sbus_write(chip_id, 0xe1 + px_port, 0xb, 0xc0000); // Turn ECC on
    elb_pp_sbus_write(chip_id, 0xe1 + px_port, 0x7, 2); // Turn SPICO Enable on
    elb_pp_sbus_write(chip_id, 0xe1 + px_port, 0x8, 0); // Enable core and hw interrupts

    elb_pp_sbus_read(chip_id, 1, 0xff);

#endif  
    return 0;
}



void * elb_pcie_sbus_bin_info(struct elb_pcie_sbus_bin_hdr_t * bin_info, char * file) {

    void * ctx = fopen((char *) file, "rb");
    if(ctx == NULL) {
        PCIE_ERR("can not open file %s", file);
        return NULL;
    }

    int ret = fread( bin_info, 4, 4 , (FILE *) ctx);
    if(ret <= 0) {
        PCIE_ERR("Can not read enough bytes, exiting\n");
        fclose(ctx);
        return NULL;
    }

    if (bin_info->magic != SBUS_ROM_MAGIC) {
        PCIE_ERR("bad magic got 0x%x want 0x%x\n", bin_info->magic, SBUS_ROM_MAGIC);
        fclose(ctx);
        return NULL;
    }

    return ctx;


}
int elb_pcie_init_sbus_bin_download_wrapper(int chip_id, int bcast_all_lanes, int px_port, int lanes, int bgroup, int issue_core_interrupts, int poll_sd_core_status, char * file) {

    //void * ctx = fopen((char *) file, "rb");

    struct elb_pcie_sbus_bin_hdr_t hdr;
    void * ctx = elb_pcie_sbus_bin_info(&hdr, file);
    if(ctx == NULL) return -1;

    int pp_inst = px_port >>2;
    int phy_port = px_port & 0x3;


    int ret =0;

    ret = elb_pcie_upload_sbus_bin_file(chip_id, bcast_all_lanes, px_port, lanes, bgroup, hdr.nwords, ctx);


    if(issue_core_interrupts || poll_sd_core_status) {
        elb_pcie_serdes_init(chip_id, pp_inst, phy_port, lanes, issue_core_interrupts, poll_sd_core_status, 10000, hdr.build_id, hdr.rev_id);
    }
    return ret;
}





