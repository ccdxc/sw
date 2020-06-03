#ifndef ELB_PCIE_SW_API_H
#define ELB_PCIE_SW_API_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
    struct elb_pcie_sbus_bin_hdr_t {
        uint32_t magic;
        uint32_t nwords;
        uint32_t build_id;
        uint32_t rev_id;
    };


int pcie_check_sd_core_status (int chip_id, int inst_id, unsigned phy_port, unsigned lanes);
int pcie_core_interrupts_sw (int chip_id, int inst_id, int int_code, int int_data, int delay_num, int phy_port, int lanes, int check_output_data, int exp_data);
#ifndef _PCIE_BLOCK_LEVEL_    
void elb_pcie_sbus_clk(int chip_id, int sbus_clk_div);
int elb_pcie_sbus_idcode_check(int chip_id);
#endif 
void elb_pcie_init_sbus(int chip_id, int bcast_all_lanes, int px_port, int lanes, int bgroup, char * file_name);
void pcie_pcs_reset(int chip_id, int pp_inst, int phy_port, int val); 
int elb_pcie_serdes_init(int chip_id, int pp_inst, int phy_port, int lanes, int issue_core_interrupts, int poll_sd_core_status, int delay_num, int exp_build_id, int exp_rev_id) ;

void * elb_pcie_sbus_bin_info(struct elb_pcie_sbus_bin_hdr_t * bin_info, char * file);
int elb_pcie_upload_sbus_bin_file(int chip_id, int bcast_all_lanes, int px_port, int lanes, int bgroup, int nwords, void * ctx);
int elb_pcie_init_sbus_bin_download_wrapper(int chip_id, int bcast_all_lanes, int px_port, int lanes, int bgroup, int issue_core_interrupts, int poll_sd_core_status, char * file);

#ifdef __cplusplus
}
#endif

#endif
