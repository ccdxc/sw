#include "cap_intr_api.h"
#include "msg_man.h"

void cap_intr_soft_reset(int chip_id, int inst_id) {
	// PLOG_MSG ("Dumping Sknob vars" << std::endl);
	// sknobs_dump();

	PLOG_MSG("inside softreset\n");
}

void cap_intr_set_soft_reset(int chip_id, int inst_id, int value) {
}


void cap_intr_init_start(int chip_id, int inst_id) {
	PLOG_MSG("inside init start\n");

}

void cap_intr_init_done(int chip_id, int inst_id) {
	PLOG_MSG("inside init done\n");

	cap_intr_csr_t& intr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_intr_csr_t,
	                                                    chip_id,
	                                                    inst_id);
	int count = 0;
	bool done  = SKNOBS_GET("sam_only", 0);

	while (!done) {
		intr_csr.sta_intr_init.read();


		if (intr_csr.sta_intr_init.done() == 1) {
			done = true;
		} else {
			count++;

			if (count >= 20000) {
				PLOG_ERR("cap_intr_init_done timed out waiting for sta_intr_init" <<
				         endl);
				return;
			}
			SLEEP(5); // sleep 5 ns
		}
	}

	// set function mask to model HW behavior after reset
	for (int idx = 0; idx < intr_csr.dhs_intr_state.get_depth_entry(); idx++) {
		intr_csr.dhs_intr_fwcfg.entry[idx].function_mask(1);
		intr_csr.dhs_intr_state.entry[idx].fwcfg_function_mask(1);
	}

}



void cap_intr_msixcfg_cmd_mask(int chip_id, int inst_id, int intr_index,
	                             int mask){
	 cap_intr_csr_t& intr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_intr_csr_t,
 	                                                    chip_id,
 	                                                    inst_id);

 	if (intr_index > (1 << 12) - 1) {
 		PLOG_ERR("intr_index out of range:" << intr_index << endl);
 		return;
 	}
	int cmd_offset = 3;
	cap_intr_csr_dhs_intr_msixcfg_entry_t msixcfg_entry;
	msixcfg_entry.all(intr_csr.dhs_intr_msixcfg.entry[intr_index].all());
	msixcfg_entry.vector_ctrl(mask);
	PLOG_MSG("Showing msixmsg intr_index:"<<intr_index<<endl;)
	msixcfg_entry.show();
	vector<unsigned int> data_vec = msixcfg_entry.get_write_vec();

	uint64_t start_addr;
	start_addr = intr_csr.dhs_intr_msixcfg.entry[intr_index].get_offset() +
							 (cmd_offset * 4);
	cpu::access()->write(chip_id, start_addr, data_vec[cmd_offset], 0);

}

void cap_intr_fwcfg_cmd_mask(int chip_id, int inst_id, int intr_index,
	                             int mask){
	 cap_intr_csr_t& intr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_intr_csr_t,
 	                                                    chip_id,
 	                                                    inst_id);

 	if (intr_index > (1 << 12) - 1) {
 		PLOG_ERR("intr_index out of range:" << intr_index << endl);
 		return;
 	}
	int cmd_offset = 0;
	cap_intr_csr_dhs_intr_fwcfg_entry_t fwcfg_entry;
	fwcfg_entry.all(intr_csr.dhs_intr_fwcfg.entry[intr_index].all());
	fwcfg_entry.function_mask(mask);
	PLOG_MSG("Showing msixmsg intr_index:"<<intr_index<<endl;)
	fwcfg_entry.show();
	vector<unsigned int> data_vec = fwcfg_entry.get_write_vec();

	uint64_t start_addr;
	start_addr = intr_csr.dhs_intr_fwcfg.entry[intr_index].get_offset() +
							 (cmd_offset * 4);
	cpu::access()->write(chip_id, start_addr, data_vec[cmd_offset], 0);

}

void cap_intr_drvcfg_cmd_mask(int chip_id, int inst_id, int intr_index,
                              int mask) {
	cap_intr_csr_t& intr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_intr_csr_t,
	                                                    chip_id,
	                                                    inst_id);

	if (intr_index > (1 << 12) - 1) {
		PLOG_ERR("intr_index out of range:" << intr_index << endl);
		return;
	}

	int cmd_offset = 1;
	cap_intr_csr_dhs_intr_drvcfg_entry_t drvcfg_entry;
	drvcfg_entry.all(intr_csr.dhs_intr_drvcfg.entry[intr_index].all());
	drvcfg_entry.mask(mask);

	vector<unsigned int> data_vec = drvcfg_entry.get_write_vec();

	uint64_t start_addr;
	start_addr = intr_csr.dhs_intr_drvcfg.entry[intr_index].get_offset() +
	             (cmd_offset * 4);
	cpu::access()->write(chip_id, start_addr, data_vec[cmd_offset], 0);
}

void cap_intr_drvcfg_cmd_credits(int chip_id,
                                 int inst_id,
                                 int intr_index,
                                 int16_t credits,
                                 int unmask,
                                 int restart_coal) {
	cap_intr_csr_t& intr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_intr_csr_t,
	                                                    chip_id,
	                                                    inst_id);

	if (intr_index > (1 << 12) - 1) {
		PLOG_ERR("intr_index out of range:" << intr_index << endl);
		return;
	}

	int cmd_offset = 2;
	cap_intr_csr_dhs_intr_drvcfg_entry_t drvcfg_entry;
	drvcfg_entry.all(intr_csr.dhs_intr_drvcfg.entry[intr_index].all());
	drvcfg_entry.int_credits(credits);
	drvcfg_entry.unmask(unmask);
	drvcfg_entry.restart_coal(restart_coal);

	vector<unsigned int> data_vec = drvcfg_entry.get_write_vec();

	uint64_t start_addr;
	start_addr = intr_csr.dhs_intr_drvcfg.entry[intr_index].get_offset() +
	             (cmd_offset * 4);
	cpu::access()->write(chip_id, start_addr, data_vec[cmd_offset], 0);
}

void cap_intr_get_drvcfg_cmd_credits(int chip_id,
                                     int inst_id,
                                     int intr_index,
                                     int16_t & credits,
                                     int & unmask,
                                     int & restart_coal) {
	cap_intr_csr_t& intr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_intr_csr_t,
	                                                    chip_id,
	                                                    inst_id);

	if (intr_index > (1 << 12) - 1) {
		PLOG_ERR("intr_index out of range:" << intr_index << endl);
		return;
	}

	int cmd_offset = 2;
	cap_intr_csr_dhs_intr_drvcfg_entry_t drvcfg_entry;

	cpp_int_helper hlp;
	cpp_int tmp(0);


	uint64_t start_addr;
	start_addr = intr_csr.dhs_intr_drvcfg.entry[intr_index].get_offset() +
	             (cmd_offset * 4);
	uint32_t read_value;
	read_value = cpu::access()->read(chip_id, start_addr);

	hlp.set_slc(tmp, read_value,cmd_offset*32,cmd_offset*32+31);
	drvcfg_entry.all(tmp);

	credits = drvcfg_entry.int_credits().convert_to<int16_t>();
	// credits &= ((1<<intr_credit_size)-1);
	// if (credits & (1<(intr_credit_size-1))) credits = (~credits) * (-1);


	unmask  = drvcfg_entry.unmask().convert_to<int>();
	restart_coal = drvcfg_entry.restart_coal().convert_to<int>();

}

void cap_intr_get_drvcfg_cmd_X(int chip_id,
                              int inst_id,
                              int intr_index,
                              int32_t & data,
															int cmd_offset
                              ) {
	cap_intr_csr_t& intr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_intr_csr_t,
	                                                    chip_id,
	                                                    inst_id);

	if (intr_index > (1 << 12) - 1) {
		PLOG_ERR("intr_index out of range:" << intr_index << endl);
		return;
	}
	if (cmd_offset > 4) {
		PLOG_ERR("offset out of range:" << cmd_offset << endl);
		return;
	}

	uint64_t start_addr;
	start_addr = intr_csr.dhs_intr_drvcfg.entry[intr_index].get_offset() +
	             (cmd_offset * 4);
	data = cpu::access()->read(chip_id, start_addr);
}

void cap_intr_get_fwcfg_cmd_X(int chip_id, int inst_id, int intr_index,
	                            uint32_t & data, int cmd_offset){
	 cap_intr_csr_t& intr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_intr_csr_t,
 	                                                    chip_id,
 	                                                    inst_id);

 	if (intr_index > (1 << 12) - 1) {
 		PLOG_ERR("intr_index out of range:" << intr_index << endl);
 		return;
 	}
	if (cmd_offset > 1) {
		PLOG_ERR("offset out of range:" << cmd_offset << endl);
		return;
	}

	uint64_t start_addr;
	start_addr = intr_csr.dhs_intr_fwcfg.entry[intr_index].get_offset() +
	             (cmd_offset * 4);
	data = cpu::access()->read(chip_id, start_addr);
}

void cap_intr_coal_timer_init(int chip_id,
                         int inst_id,
                         int intr_index,
                         uint32_t timer_value) {
	cap_intr_csr_t& intr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_intr_csr_t,
	                                                    chip_id,
	                                                    inst_id);


	if (intr_index > (1 << 12) - 1) {
		PLOG_ERR("intr_index out of range:" << intr_index << endl);
		return;
	}

	if (intr_index > (1 << 12) - 1) {
		PLOG_ERR("intr_index out of range:" << intr_index << endl);
		return;
	}

	int cmd_offset = 0;
	cap_intr_csr_dhs_intr_drvcfg_entry_t drvcfg_entry;
	drvcfg_entry.all(intr_csr.dhs_intr_drvcfg.entry[intr_index].all());
	drvcfg_entry.coal_init(timer_value);

	vector<unsigned int> data_vec = drvcfg_entry.get_write_vec();

	uint64_t start_addr;
	start_addr = intr_csr.dhs_intr_drvcfg.entry[intr_index].get_offset() +
	             (cmd_offset * 4);
	cpu::access()->write(chip_id, start_addr, data_vec[cmd_offset], 0);

	intr_csr.dhs_intr_drvcfg.entry[intr_index].show();

}

void cap_intr_coal_timer_resolution(int chip_id, int inst_id, int resolution_us){

	int clocks = ((resolution_us*1000)/1.2);
	cap_intr_csr_t& intr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_intr_csr_t,
	                                                    chip_id,
	                                                    inst_id);
	intr_csr.cfg_intr_coalesce.resolution(clocks);
	intr_csr.cfg_intr_coalesce.write();

}

void cap_intr_load_from_cfg(int chip_id, int inst_id) {
#if 0    
	PLOG_MSG("inside cap_intr_load_from_cfg \n");

	cap_intr_csr_t& intr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_intr_csr_t,
	                                                    chip_id,
	                                                    inst_id);

	cpu_access_type_e cur_cpu_access_type = cpu::access()->get_access_type();

	bool load_rtl        = true;
	bool look_for_fields = true;

	// if (cur_cpu_access_type == back_door_e) {
	load_rtl = false;

	// }

	intr_csr.dhs_intr_fwcfg.load_from_cfg(load_rtl, look_for_fields);
	intr_csr.dhs_intr_msixcfg.load_from_cfg(load_rtl, look_for_fields);
	intr_csr.dhs_intr_drvcfg.load_from_cfg(load_rtl, look_for_fields);

	// if (cur_cpu_access_type == back_door_e) {
	for (int idx = 0; idx < intr_csr.dhs_intr_state.get_depth_entry(); idx++) {
		intr_csr.dhs_intr_state.entry[idx].msixcfg_msg_addr_51_2(
			intr_csr.dhs_intr_msixcfg.entry[idx].msg_addr()>>2);
		intr_csr.dhs_intr_state.entry[idx].msixcfg_msg_data(
			intr_csr.dhs_intr_msixcfg.entry[idx].msg_data());
		intr_csr.dhs_intr_state.entry[idx].msixcfg_vector_ctrl(
			intr_csr.dhs_intr_msixcfg.entry[idx].vector_ctrl());
		intr_csr.dhs_intr_state.entry[idx].fwcfg_function_mask(
			intr_csr.dhs_intr_fwcfg.entry[idx].function_mask());
		intr_csr.dhs_intr_state.entry[idx].fwcfg_lif(
			intr_csr.dhs_intr_fwcfg.entry[idx].lif());
		intr_csr.dhs_intr_state.entry[idx].fwcfg_port_id(
			intr_csr.dhs_intr_fwcfg.entry[idx].port_id());
		intr_csr.dhs_intr_state.entry[idx].fwcfg_local_int(
			intr_csr.dhs_intr_fwcfg.entry[idx].local_int());
		intr_csr.dhs_intr_state.entry[idx].fwcfg_legacy_int(
			intr_csr.dhs_intr_fwcfg.entry[idx].legacy());
		intr_csr.dhs_intr_state.entry[idx].fwcfg_legacy_pin(
			intr_csr.dhs_intr_fwcfg.entry[idx].intpin());
		intr_csr.dhs_intr_state.entry[idx].drvcfg_mask(
			intr_csr.dhs_intr_drvcfg.entry[idx].mask());
		intr_csr.dhs_intr_state.entry[idx].drvcfg_int_credits(
			intr_csr.dhs_intr_drvcfg.entry[idx].int_credits());
		intr_csr.dhs_intr_state.entry[idx].drvcfg_mask_on_assert(
			intr_csr.dhs_intr_drvcfg.entry[idx].mask_on_assert());
		intr_csr.dhs_intr_state.entry[idx].write();
	}

	// }
	for (int idx = 0; idx < intr_csr.dhs_intr_coalesce.get_depth_entry(); idx++) {
		intr_csr.dhs_intr_coalesce.entry[idx].coal_init_value0(
			intr_csr.dhs_intr_drvcfg.entry[idx * 4 + 0].coal_init());
		intr_csr.dhs_intr_coalesce.entry[idx].coal_init_value1(
			intr_csr.dhs_intr_drvcfg.entry[idx * 4 + 1].coal_init());
		intr_csr.dhs_intr_coalesce.entry[idx].coal_init_value2(
			intr_csr.dhs_intr_drvcfg.entry[idx * 4 + 2].coal_init());
		intr_csr.dhs_intr_coalesce.entry[idx].coal_init_value3(
			intr_csr.dhs_intr_drvcfg.entry[idx * 4 + 3].coal_init());
		intr_csr.dhs_intr_coalesce.entry[idx].write();
	}

	intr_csr.cfg_intr_coalesce.read();
	intr_csr.cfg_intr_coalesce.show();

    // kinjal(03/03): no need to call set_access_type here, fullchip test will take very long
	//if (cur_cpu_access_type == back_door_e) {
	//	cpu::access()->set_access_type(front_door_e);
	//}


	PLOG_MSG("done cap_intr_load_from_cfg \n");
#endif    
}

void cap_intr_eos(int chip_id, int inst_id) {
	//cpu_access_type_e cur_cpu_access_type = cpu::access()->get_access_type();
	cap_intr_csr_t  & intr_csr            = CAP_BLK_REG_MODEL_ACCESS(cap_intr_csr_t,
	                                                                 chip_id,
	                                                                 inst_id);

	intr_csr.cfg_intr_coalesce.read();

	cap_intr_eos_cnt(chip_id, inst_id);
	cap_intr_eos_int(chip_id, inst_id);
	cap_intr_eos_sta(chip_id, inst_id);
}

void cap_intr_eos_cnt(int chip_id, int inst_id) {
}

void cap_intr_eos_int(int chip_id, int inst_id) {
}

void cap_intr_eos_sta(int chip_id, int inst_id) {
	PLOG_MSG("inside cap_intr_eos_sta \n");

	// cap_intr_csr_t& intr_csr = CAP_BLK_REG_MODEL_ACCESS(cap_intr_csr_t,
	//                                                     chip_id,
	//                                                     inst_id);
}

void cap_intr_csr_set_hdl_path(int chip_id, int inst_id, string path) {
    cap_intr_csr_t & intr = CAP_BLK_REG_MODEL_ACCESS(cap_intr_csr_t, chip_id, inst_id);

    intr.set_csr_inst_path(0, (path + ".intr_csr"), 0);
    //intr.dhs_intr_drvcfg.set_csr_inst_path(1, (path + ".u_intr_drvcfg.mem"));
}

