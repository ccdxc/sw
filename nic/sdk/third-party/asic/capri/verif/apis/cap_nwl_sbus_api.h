#ifndef CAP_NWL_SBUS_API_H
#define CAP_NWL_SBUS_API_H

void avago_top_sbus_command(uint32_t cmd, uint32_t address, uint32_t wr_data, uint32_t* rd_data);
void cap_nwl_sbus_phy_sbus_start(void);
#ifndef CAPRI_SW
void cap_nwl_sbus_phy_sbus_start_raw(uint32_t hbmsbm, const void *rom_info);
#else
void cap_nwl_sbus_phy_sbus_start_raw(uint32_t hbmsbm, void *rom_info);
#endif
void cap_nwl_sbus_phy_hbm_reset(void);
void cap_nwl_sbus_customize_avago_phy_settings(void);
void cap_nwl_sbus_test_PHY_soft_lane_repair(void);
void cap_nwl_sbus_wdr_read(uint32_t length,
	      uint32_t shift_only);
void cap_nwl_sbus_wir_write_channel(uint32_t  device_sel,
		       uint32_t  channel,
		       uint32_t  instruction,
		       uint32_t instr_length);
void cap_nwl_sbus_wdr_read_sbus_data(uint32_t   channel,
			uint32_t  length,
			uint32_t  *result_32);
void cap_nwl_sbus_wdr_write_data(uint32_t *data,
		    uint32_t length,
		    uint32_t shift_only);
void cap_nwl_sbus_wait_for_1500_done(uint32_t expected, uint32_t* error);
void cap_nwl_sbus_apc_1500_busy_done_handshake(uint32_t* error);
void cap_nwl_sbus_test_1500_device_id(uint32_t *data_32, uint32_t *is_8g8hi, uint32_t *is_samsung);
uint32_t cap_nwl_sbus_get_1500_temperature(void);
void cap_nwl_sbus_test_1500_temperature(void);
void cap_nwl_sbus_test_1500_HBM_soft_lane_repair(void);
void cap_nwl_scrub_expmt_ctc_start(int chip_id, int ctc_id, unsigned int addr_low) ;
void cap_nwl_scrub_ecc_ctc_start(int chip_id, int ctc_id);
int cap_nwl_scrub_ecc_ctc_wait_for_done(int chip_id, int ctc_id);
void cap_nwl_scrub_8G_ecc_ctc_start(int chip_id, int ctc_id);
int cap_nwl_scrub_8G_ecc_ctc_wait_for_done(int chip_id, int ctc_id);
void cap_nwl_release_ctc(int chip_id, int ctc_id) ;
void cap_nwl_set_max_addr(int chip_id, int ctc_id, int max_addr) ;
void cap_nwl_ctc_check_prbs(int chip_id, int ctc_id, unsigned int ctc_pkt_count, unsigned int ctc_prbs_type, unsigned int ctc_prbs_value, unsigned int ctc_addr_lo, unsigned int ctc_addr_hi, unsigned int ctc_test_mode, unsigned int ctc_rd_wr_mix, unsigned int ctc_rd_wr_inv) ;
int cap_nwl_ctc_wait_for_complete(int chip_id, int ctc_id) ;
void cap_nwl_test_prbs(int chip_id, int ctc_id, unsigned int ctc_pkt_count, unsigned int ctc_prbs_type, unsigned int ctc_prbs_value, unsigned int ctc_addr_lo, unsigned int ctc_addr_hi, unsigned int ctc_test_mode, unsigned int ctc_rd_wr_mix, unsigned int ctc_rd_wr_inv) ;
int cap_nwl_hbm_firmware_operation(int operation);
int cap_nwl_set_hbm_parameter(int offset, int value);
int test_firmware_aerr(void) ;
int test_firmware_derr(void) ;
int test_firmware_ctc(void) ;

#endif
