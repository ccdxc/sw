#include "cap_pics_api.h"
#include "cap_csr_util_api.h"
#include "pic.h"
#include "sknobs.h"
#include "LogMsg.h"
#include "dpi_utils.h"

void cap_pics_soft_reset(int chip_id, int inst_id) {

  PLOG_API_MSG("PICS[" << inst_id << "]", "inside softreset\n");
  PLOG_API_MSG("PICS[" << inst_id << "]", "done with softreset\n");

}

void cap_pics_set_soft_reset(int chip_id, int inst_id, int value) {
}

void cap_pics_init_start(int chip_id, int inst_id) {

  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);

 //Disable ECC detection & correction if CPU backdoor access is enabled.
 string access_type_str = sknobs_get_string((char *)"cpu_access", (char *)"");
 if(!access_type_str.compare("back_door")) {
   PLOG_API_MSG("PICS[" << inst_id << "]", "disabling ECC for backdoor access\n");
   pics_csr.cfg_sram.ecc_disable_det(0x3ff);
   pics_csr.cfg_sram.ecc_disable_cor(0x3ff);
   pics_csr.cfg_sram.write();
 }
 PLOG_API_MSG("PICS[" << inst_id << "]", "Enabling all interrupts\n");
 enable_all_interrupts(&pics_csr);
}

void cap_pics_init_done(int chip_id, int inst_id) {
}

// use sknobs base load cfg 
void cap_pics_load_from_cfg(int chip_id, int inst_id, block_type type) {

  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);

  PLOG_API_MSG("PICS[" << inst_id << "]", "load_from_cfg\n");
  if (inst_id == 0) {
    PLOG_API_MSG("PICS[" << inst_id << "]", "dumping sknobs\n");
    //sknobs_dump();
  }

  string hier_path_str = pics_csr.get_hier_path();
  PLOG_MSG("PICS[" << inst_id << "]" << " current path " << hier_path_str << endl);
  pics_csr.load_from_cfg(true, true);

  cap_pics_load_sram_def_from_cfg(chip_id, inst_id, type);

  uint32_t pic_cache_enable  = sknobs_get_value((char*)"cap0/top/main/pic_cache_enable", 0);
  if (pic_cache_enable == 1) {
     pics_csr.picc.cfg_cache_global.bypass(0);
     pics_csr.picc.cfg_cache_global.hash_mode(0);
     pics_csr.picc.cfg_cache_global.write();

     pics_csr.picc.filter_addr_hi_s.data[0].read();
     pics_csr.picc.filter_addr_hi_s.data[0].value(0xfffffff);
     pics_csr.picc.filter_addr_hi_s.data[0].write();
     pics_csr.picc.filter_addr_ctl_s.value[0].read();
     pics_csr.picc.filter_addr_ctl_s.value[0].value(0xf);
     pics_csr.picc.filter_addr_ctl_s.value[0].write();
     pics_csr.picc.filter_addr_hi_m.data[0].read();
     pics_csr.picc.filter_addr_hi_m.data[0].value(0xfffffff);
     pics_csr.picc.filter_addr_hi_m.data[0].write();
     pics_csr.picc.filter_addr_ctl_m.value[0].read();
     pics_csr.picc.filter_addr_ctl_m.value[0].value(0xf);
     pics_csr.picc.filter_addr_ctl_m.value[0].write();
  }

  // takes too long
  if (PLOG_CHECK_MSG_LEVEL(LogMsg::DEBUG)) {
    pics_csr.show();
  }
  else if (PLOG_CHECK_MSG_LEVEL(LogMsg::INFO)) {
    for (uint32_t idx = 0; idx < 16; idx++) {
      pics_csr.cfg_bg_update_profile[idx].show();
    }
    pics_csr.cfg_bg_update_profile_enable.show();
  }
}

void cap_pics_eos(int chip_id, int inst_id) {

  cap_pics_eos_cnt(chip_id, inst_id);
  cap_pics_eos_int(chip_id, inst_id);
  //cap_pics_eos_sta(chip_id, inst_id);  // Combine eos_sta with eos_int. Only check sta if there is corresponding int

  cap_pics_print_cache_cfg_info(chip_id, inst_id);
  cap_pics_cache_cnt(chip_id, inst_id);

  // Just for debug purpose
  //cap_pics_cache_tag_tbl_report(chip_id, inst_id)

  //TODO: enable when we can do zerotime
  //cap_pics_check_mem_values(chip_id, inst_id);
}

void cap_pics_eos_cnt(int chip_id, int inst_id) {

  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);

  for (uint32_t port = 0; port < 8; port++) {
    pics_csr.cnt_axi_por_rdreq[port].read();
    pics_csr.cnt_axi_por_rdrsp[port].read();
    pics_csr.cnt_axi_por_wrreq[port].read();
    pics_csr.cnt_axi_por_wrrsp[port].read();
    if (pics_csr.cnt_axi_por_rdreq[port].val() != pics_csr.cnt_axi_por_rdrsp[port].val()) {
      PLOG_ERR("PICS[" << inst_id << "] port " << port << " cnt_axi_por_rdreq " << pics_csr.cnt_axi_por_rdreq[port].val().convert_to<uint32_t>() << " != cnt_axi_por_rdrsp " << pics_csr.cnt_axi_por_rdrsp[port].val().convert_to<uint32_t>() << endl);
    }
    else {
      PLOG_API_MSG("PICS[" << inst_id << "]", "Serviced " << pics_csr.cnt_axi_por_rdreq[port].val().convert_to<uint32_t>() << " table reads on port " << port << endl);
    }
    PLOG_API_MSG("PICS[" << inst_id << "]", "Serviced " << pics_csr.cnt_axi_por_rmwreq[port].val().convert_to<uint32_t>() << " table read-modify-write on port " << port << endl);
    if (pics_csr.cnt_axi_por_wrreq[port].val() != pics_csr.cnt_axi_por_wrrsp[port].val()) {
      PLOG_ERR("PICS[" << inst_id << "] port " << port << " cnt_axi_por_wrreq " << pics_csr.cnt_axi_por_wrreq[port].val().convert_to<uint32_t>() << " != cnt_axi_por_wrrsp " << pics_csr.cnt_axi_por_wrrsp[port].val().convert_to<uint32_t>() << endl);
    }
    else {
      PLOG_API_MSG("PICS[" << inst_id << "]", "Serviced " << pics_csr.cnt_axi_por_wrreq[port].val().convert_to<uint32_t>() << " table writes on port " << port << endl);
    }
  }

  for (uint32_t sram = 0; sram < 10; sram++) {
    pics_csr.cnt_sram_read[sram].read();
    pics_csr.cnt_sram_write[sram].read();
    PLOG_API_MSG("PICS[" << inst_id << "]", " Did " << pics_csr.cnt_sram_read[sram].val().convert_to<uint32_t>() << " reads on SRAM " << sram << endl);
    PLOG_API_MSG("PICS[" << inst_id << "]", " Did " << pics_csr.cnt_sram_write[sram].val().convert_to<uint32_t>() << " writes on SRAM " << sram << endl);
  }

  pics_csr.cnt_soc_rl_msg.read();
  PLOG_API_MSG("PICS[" << inst_id << "]", " Sent " << pics_csr.cnt_soc_rl_msg.val().convert_to<uint32_t>() << " rate limiter messages" << endl);
}

void cap_pics_eos_int(int chip_id, int inst_id) {
      
  PLOG_API_MSG("PICS[" << inst_id << "]", "Checking interrupts" << endl);

  //cpp_check_interrupts((char *)"cap_pics_csr_t", chip_id, inst_id);
  
  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);
  pics_csr.int_pics.read();
  pics_csr.int_pics.show();

  if ((pics_csr.int_pics.intreg.uncorrectable_ecc_interrupt() != 0) || (pics_csr.int_pics.intreg.correctable_ecc_interrupt() != 0)) {
     pics_csr.sta_ecc.read();
     pics_csr.sta_ecc.show();
  }

  if (pics_csr.int_badaddr.intreg.all() != 0) {
     pics_csr.sta_bg_bad_addr.read();
     pics_csr.sta_bg_bad_addr.show();
     pics_csr.sta_cpu_bad_addr.read();
     pics_csr.sta_cpu_bad_addr.show();
     pics_csr.sta_rdreq_bad_addr.read();
     pics_csr.sta_rdreq_bad_addr.show();
     pics_csr.sta_wrreq_bad_addr.read();
     pics_csr.sta_wrreq_bad_addr.show();
  }

  uint32_t ignore_oob_errors = sknobs_get_value((char *)"sc_tb/oob", 0);
  if (ignore_oob_errors) {
    // clear the bad address errors AXI response errors because we're generating those in the SC TB
    PLOG_API_MSG("PICS[" << inst_id << "]", "Clearing bad_addr interrupts" << endl);
    pics_csr.int_badaddr.intreg.all(0xffff);
    pics_csr.int_badaddr.intreg.write();
  }

  if (sknobs_exists((char *)"PEN_IP_MEM_ECC_CORRUPT_ENABLE")) {
    PLOG_API_MSG("PICS[" << inst_id << "]", "Clearing ECC interrupts" << endl);
    pics_csr.int_pics.intreg.all(0x3);
    pics_csr.int_pics.intreg.write();
  }

  check_interrupts(&pics_csr, 0);
}

//Configure pic axi_attr registers to be in sync with TE/MPU axi_attr regs
void cap_pics_set_axi_attrs(int chip_id, int inst_id) {

  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);

  pics_csr.picc.cfg_cache_fill_axi.arcache(0xf);
  pics_csr.picc.cfg_cache_fill_axi.arprot(0x2);
  pics_csr.picc.cfg_cache_fill_axi.write();
}

void cap_pics_eos_sta(int chip_id, int inst_id) {

  //PLOG_API_MSG("PICS[" << inst_id << "]", "Checking STAs" << endl);

  //cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);
}

cpp_int cap_pics_read_table_entry(int chip_id, int inst_id, 
                                  uint32_t table_start, 
                                  uint32_t entry_size,
                                  uint32_t entries_per_line,
                                  uint32_t entry_idx) {

  cpp_int_helper hlp;
  cpp_int entry = 0;
  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);

  // calculate physical address for the entry; table_start and entry_size in increments of 2B
  uint32_t entry_size_bytes = entry_size * 2;

  uint32_t entry_addr = table_start * 2 + 
    (entry_idx / entries_per_line) * 256 +
    (entry_idx % entries_per_line) * entry_size_bytes;

  uint32_t entry_x = entry_addr % 256;
  uint32_t entry_y = entry_addr / 256;
  uint32_t phys_table = entry_x / 16;
  uint32_t dhs_first_entry_idx = phys_table * 4096 + entry_y;
  uint32_t dhs_first_line = dhs_first_entry_idx;
  uint32_t byte_in_first_line =  entry_x % 16;

  // the entry may span multiple physical SRAMs
  uint32_t cur_byte = byte_in_first_line;
  uint32_t dhs_entry_line;
  uint32_t byte_in_dhs_line;
  for (uint32_t byte = 0; byte < entry_size_bytes; byte++) {

    dhs_entry_line = dhs_first_line + ((cur_byte + byte) / 16) * 4096;
    byte_in_dhs_line = ((cur_byte + byte) % 16);
    
    //read from hw
    if ((byte == 0) || (byte_in_dhs_line == 0)) {
      pics_csr.dhs_sram.entry[dhs_entry_line].read();
    }

    entry = entry | ( hlp.get_slc(pics_csr.dhs_sram.entry[dhs_entry_line].data(), 
                                  (16 - byte_in_dhs_line - 1) * 8, 
                                  ((16 - byte_in_dhs_line) * 8) - 1) << (byte * 8) );

    //PLOG_INFO("entry " << entry_idx << " of width " << entry_size_bytes << "B for logical table starting at " << (table_start * 2) << " beginning in physical SRAM " << phys_table << " DHS line " << dhs_first_line << " byte " << byte_in_first_line << " and ending in physical SRAM " << dhs_entry_line /  4096 << " DHS line " << dhs_entry_line << " byte " << byte_in_dhs_line << ": " << hex << entry << dec << endl);
  }
  return entry;
}

cpp_int cap_pics_read_logical_table_entry(int chip_id, int inst_id, 
                                          uint32_t table_id, 
                                          uint32_t entry_idx,
                                          bool read_from_hw) {

  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);

  if (read_from_hw) {
    pics_csr.cfg_table_profile[table_id].read();
  }

  return cap_pics_read_table_entry(chip_id, inst_id, 
                                   pics_csr.cfg_table_profile[table_id].start_addr().convert_to<uint32_t>(),
                                   pics_csr.cfg_table_profile[table_id].width().convert_to<uint32_t>(),
                                   1 << pics_csr.cfg_table_profile[table_id].log2bkts().convert_to<uint32_t>(),
                                   entry_idx);
}

void cap_pics_load_table_entry(int chip_id, int inst_id, 
                               uint32_t table_start, 
                               uint32_t entry_size,
                               uint32_t entries_per_line,
                               uint32_t entry_idx,
                               cpp_int entry,
                               block_type type) {

  cpp_int_helper hlp;
  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);

  // calculate physical address for the entry; table_start and entry_size in increments of 2B
  uint32_t entry_size_bytes = entry_size * 2;

  uint32_t entry_addr = table_start * 2 + 
    (entry_idx / entries_per_line) * 256 +
    (entry_idx % entries_per_line) * entry_size_bytes;

  uint32_t entry_x = entry_addr % 256;
  uint32_t entry_y = entry_addr / 256;
  uint32_t phys_table = entry_x / 16;
  uint32_t dhs_first_entry_idx = phys_table * 4096 + entry_y;
  uint32_t dhs_first_line = dhs_first_entry_idx;
  uint32_t byte_in_first_line =  entry_x % 16;

  // the entry may span multiple physical SRAMs
  uint32_t cur_byte = byte_in_first_line;
  uint32_t dhs_entry_line;
  uint32_t byte_in_dhs_line;
  for (uint32_t byte = 0; byte < entry_size_bytes; byte++) {
    // figure out how endianess works 
    auto entry_byte = entry_size_bytes - byte;
    dhs_entry_line = dhs_first_line + ((cur_byte + byte) / 16) * 4096;
    byte_in_dhs_line = ((cur_byte + byte) % 16);
    cpp_int temp = pics_csr.dhs_sram.entry[dhs_entry_line].data();
    temp = hlp.set_slc(temp, 
                       hlp.get_slc(entry, (entry_byte - 1) * 8, (entry_byte * 8) - 1), 
                       (16 - byte_in_dhs_line - 1) * 8, ((16 - byte_in_dhs_line) * 8) - 1);
    pics_csr.dhs_sram.entry[dhs_entry_line].data(temp);
    PLOG_DEBUG("entry_size " << entry_size_bytes << " entry_byte " << entry_byte << " dhs_entry_line " << dhs_entry_line << " byte_in_dhs_line " << byte_in_dhs_line << " current line: "<< endl);
    if (PLOG_CHECK_MSG_LEVEL(LogMsg::DEBUG)) {
      pics_csr.dhs_sram.entry[dhs_entry_line].show();
    }
    if ((byte_in_dhs_line == 16 - 1) || (byte == entry_size_bytes - 1)) {
      pics_csr.dhs_sram.entry[dhs_entry_line].write();
    }
  }
  PLOG_DEBUG("Wrote entry " << entry_idx << " of width " << entry_size_bytes << "B for logical table starting at " << (table_start * 2) << " beginning in physical SRAM " << phys_table << " DHS line " << dhs_first_line << " byte " << byte_in_first_line << " and ending in physical SRAM " << dhs_entry_line /  4096 << " DHS line " << dhs_entry_line << " byte " << byte_in_dhs_line << endl);
  if (PLOG_CHECK_MSG_LEVEL(LogMsg::DEBUG)) {
    pics_csr.dhs_sram.entry[dhs_entry_line].show();
  }
}

void cap_pics_load_logical_table_entry(int chip_id, int inst_id, 
                                       uint32_t table_id, 
                                       uint32_t entry_idx,
                                       cpp_int entry,
                                       bool read_from_hw,
                                       block_type type) {

  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);

  if (read_from_hw) {
    pics_csr.cfg_table_profile[table_id].read();
  }

  cap_pics_load_table_entry(chip_id, inst_id, 
                            pics_csr.cfg_table_profile[table_id].start_addr().convert_to<uint32_t>(),
                            pics_csr.cfg_table_profile[table_id].width().convert_to<uint32_t>(),
                            1 << pics_csr.cfg_table_profile[table_id].log2bkts().convert_to<uint32_t>(),
                            entry_idx,
                            entry);
}

void cap_pics_add_logical_table(int chip_id, int inst_id, 
                                uint32_t table_id,
                                uint32_t upper_x,
                                uint32_t upper_y,
                                uint32_t lower_x,
                                uint32_t lower_y,
                                uint32_t entry_size_bytes,
                                uint32_t hash_table,
                                uint32_t opcode,
                                uint32_t axishift,
                                uint32_t rlimit_en,
                                block_type type) {

  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);

  if ((lower_x < upper_x) || (lower_y < upper_y)) {
    PLOG_ERR("Incorrect SRAM definition lower_x: " << lower_x << " lower_y: " << lower_y << " upper_x: " << upper_x << " upper_y: " << upper_y << endl);
  }
  uint32_t table_start = upper_y * 256 + upper_x;
  uint32_t table_end = lower_y * 256 + lower_x;
  uint32_t table_width = lower_x - upper_x + 2;
  
  if (table_width % entry_size_bytes != 0) {
    PLOG_ERR("SRAM table width " << table_width << " not a multiple of the entry size " << entry_size_bytes << endl);
  }
  if (((table_width / entry_size_bytes) & (table_width / entry_size_bytes - 1)) != 0) {
    PLOG_ERR("Number of entries per line " <<  (table_width / entry_size_bytes) << " has to be a power of 2" << endl);
  }
  pics_csr.cfg_table_profile[table_id].start_addr(table_start/2);
  pics_csr.cfg_table_profile[table_id].end_addr(table_end/2);
  pics_csr.cfg_table_profile[table_id].width(entry_size_bytes/2);
  pics_csr.cfg_table_profile[table_id].hash(hash_table);
  pics_csr.cfg_table_profile[table_id].opcode(opcode);
  pics_csr.cfg_table_profile[table_id].axishift(axishift);
  pics_csr.cfg_table_profile[table_id].rlimit_en(rlimit_en);
  uint32_t log2bkts = log2(table_width / entry_size_bytes);
  pics_csr.cfg_table_profile[table_id].log2bkts(log2bkts);
  pics_csr.cfg_table_profile[table_id].write();
  
  PLOG_INFO("Added logical table with id " << table_id 
            << " for entries of 2B size " 
            << pics_csr.cfg_table_profile[table_id].width().convert_to<uint32_t>()
            << " starting at 2B address " 
            << pics_csr.cfg_table_profile[table_id].start_addr().convert_to<uint32_t>()  
            << " ending at 2B address " 
            << pics_csr.cfg_table_profile[table_id].end_addr().convert_to<uint32_t>() 
            << " entries per line " 
            << (1 << pics_csr.cfg_table_profile[table_id].log2bkts().convert_to<uint32_t>()) << endl);

}

void cap_pics_load_sram_def_from_cfg(int chip_id, int inst_id, block_type type) {

  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);

  stringstream tmp_ss;
  string hier_path_str = pics_csr.get_hier_path();
  string tmp_str;

  replace(hier_path_str.begin(), hier_path_str.end(), '.', '/');
  replace(hier_path_str.begin(), hier_path_str.end(), '[', '/');
  replace(hier_path_str.begin(), hier_path_str.end(), ']', '/');
  unique(hier_path_str.begin(), hier_path_str.end());
  if (hier_path_str[hier_path_str.length() - 1] == '/') {
    hier_path_str.pop_back();
  }
  PLOG_API_MSG("PICS[" << inst_id << "]", " current path " << hier_path_str << endl);

  tmp_ss.str("");
  tmp_ss << hier_path_str << "/num_tables";
  tmp_str = tmp_ss.str();
  uint32_t num_tables = sknobs_get_value((char *)tmp_str.c_str(), 0);

  PLOG_API_MSG("PICS[" << inst_id << "]", " loading " << num_tables << " logical tables" << endl);
  for (uint32_t table_idx = 0; table_idx < num_tables; table_idx++) {
    uint32_t table_id;
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/table_defs/" << table_idx << "/table_id";
    tmp_str = tmp_ss.str();
    if (!sknobs_exists((char *)tmp_str.c_str())) {
      PLOG_ERR("Table " << table_idx << " missing definition for table_id" << endl);
    }
    else {
      table_id = sknobs_get_value((char *)tmp_str.c_str(), 0);
    }
    uint32_t upper_x;
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/table_defs/" << table_idx << "/upper_x";
    tmp_str = tmp_ss.str();
    if (!sknobs_exists((char *)tmp_str.c_str())) {
      PLOG_ERR("Table " << table_idx << " missing definition for upper_x" << endl);
    }
    else {
      upper_x = sknobs_get_value((char *)tmp_str.c_str(), 0);
    }
    uint32_t upper_y;
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/table_defs/" << table_idx << "/upper_y";
    tmp_str = tmp_ss.str();
    if (!sknobs_exists((char *)tmp_str.c_str())) {
      PLOG_ERR("Table " << table_idx << " missing definition for upper_y" << endl);
    }
    else {
      upper_y = sknobs_get_value((char *)tmp_str.c_str(), 0);
    }
    uint32_t lower_x;
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/table_defs/" << table_idx << "/lower_x";
    tmp_str = tmp_ss.str();
    if (!sknobs_exists((char *)tmp_str.c_str())) {
      PLOG_ERR("Table " << table_idx << " missing definition for lower_x" << endl);
    }
    else {
      lower_x = sknobs_get_value((char *)tmp_str.c_str(), 0);
    }
    uint32_t lower_y;
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/table_defs/" << table_idx << "/lower_y";
    tmp_str = tmp_ss.str();
    if (!sknobs_exists((char *)tmp_str.c_str())) {
      PLOG_ERR("Table " << table_idx << " missing definition for lower_y" << endl);
    }
    else {
      lower_y = sknobs_get_value((char *)tmp_str.c_str(), 0);
    }
    uint32_t entry_size;
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/table_defs/" << table_idx << "/entry_size";
    tmp_str = tmp_ss.str();
    if (!sknobs_exists((char *)tmp_str.c_str())) {
      PLOG_ERR("Table " << table_idx << " missing definition for entry_size" << endl);
    }
    else {
      entry_size = sknobs_get_value((char *)tmp_str.c_str(), 0);
    }
    uint32_t hash;
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/table_defs/" << table_idx << "/hash";
    tmp_str = tmp_ss.str();
    if (!sknobs_exists((char *)tmp_str.c_str())) {
      PLOG_ERR("Table " << table_idx << " missing definition for hash" << endl);
    }
    else {
      hash = sknobs_get_value((char *)tmp_str.c_str(), 0);
    }
    uint32_t opcode;
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/table_defs/" << table_idx << "/opcode";
    tmp_str = tmp_ss.str();
    if (!sknobs_exists((char *)tmp_str.c_str())) {
      PLOG_ERR("Table " << table_idx << " missing definition for opcode" << endl);
    }
    else {
      opcode = sknobs_get_value((char *)tmp_str.c_str(), 0);
    }
    uint32_t axishift;
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/table_defs/" << table_idx << "/axishift";
    tmp_str = tmp_ss.str();
    if (!sknobs_exists((char *)tmp_str.c_str())) {
      PLOG_ERR("Table " << table_idx << " missing definition for axishift" << endl);
    }
    else {
      axishift = sknobs_get_value((char *)tmp_str.c_str(), 0);
    }
    uint32_t rlimit_en;
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/table_defs/" << table_idx << "/rlimit_en";
    tmp_str = tmp_ss.str();
    if (!sknobs_exists((char *)tmp_str.c_str())) {
      PLOG_ERR("Table " << table_idx << " missing definition for rlimit_en" << endl);
    }
    else {
      rlimit_en = sknobs_get_value((char *)tmp_str.c_str(), 0);
    }
  cap_pics_add_logical_table(chip_id, inst_id, table_id, upper_x, upper_y, lower_x, lower_y, entry_size, hash, opcode, axishift, rlimit_en, type);

    // load entries
    uint32_t max_entries = ((lower_x - upper_x + 2) / entry_size) * (lower_y - upper_y + 1);
    PLOG_INFO("Looking for max entries " << max_entries << " for logical table " << table_idx << " with table_id " << table_id << endl);
    cpp_int temp;
    for (uint32_t entry_idx = 0; entry_idx < max_entries; entry_idx++) {
      tmp_ss.str("");
      tmp_ss << hier_path_str << "/table_entries/" << table_id << "/entry_idx/" << entry_idx;
      tmp_str = tmp_ss.str();
      if (sknobs_exists((char *)tmp_str.c_str())) {
        temp.assign(sknobs_get_string((char *)tmp_str.c_str(), (char *)""));
        PLOG_INFO("PICS[" << inst_id << "] Loading entry " << entry_idx << " with value " << hex << temp << dec << " in table_id " << table_id << endl);
        cap_pics_load_logical_table_entry(chip_id, inst_id, table_id, entry_idx, temp, false);
      }
    }
  }
}

void cap_pics_check_progr_sanity(int chip_id, int inst_id, block_type type) {

  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);
  cap_pic_opcode_decoder_t tbl_opcode_dec;
  
  PLOG_API_MSG("PICS[" << inst_id << "]", "running programming sanity\n");
  for (auto idx = 0; idx < 96; idx++) {
    pics_csr.cfg_table_profile[idx].read();
  }
  for (auto idx = 0; idx < 96; idx++) {
    if (pics_csr.cfg_table_profile[idx].end_addr().convert_to<uint32_t>() != 0) {
      uint32_t start_addr =  pics_csr.cfg_table_profile[idx].start_addr().convert_to<uint32_t>() * 2;
      uint32_t end_addr =  pics_csr.cfg_table_profile[idx].end_addr().convert_to<uint32_t>() * 2;
      uint32_t upper_x = start_addr % 256;
      uint32_t upper_y = start_addr / 256;
      uint32_t lower_x = end_addr % 256;
      uint32_t lower_y = end_addr / 256;

      if ((lower_x < upper_x) || (lower_y < upper_y)) {
        PLOG_ERR("Incorrect logic table " << idx << " definition lower_x: " << lower_x << " lower_y: " << lower_y << " upper_x: " << upper_x << " upper_y: " << upper_y << endl);
      }

      for (auto idx1 = idx + 1; idx1 < 96; idx1++) {
        if (pics_csr.cfg_table_profile[idx1].end_addr().convert_to<uint32_t>() != 0) {
          uint32_t start_addr1 =  pics_csr.cfg_table_profile[idx1].start_addr().convert_to<uint32_t>() * 2;
          uint32_t end_addr1 =  pics_csr.cfg_table_profile[idx1].end_addr().convert_to<uint32_t>() * 2;
          uint32_t upper1_x = start_addr1 % 256;
          uint32_t upper1_y = start_addr1 / 256;
          uint32_t lower1_x = end_addr1 % 256;
          uint32_t lower1_y = end_addr1 / 256;

          if ((lower1_x < upper1_x) || (lower1_y < upper1_y)) {
            PLOG_ERR("Incorrect logic table " << idx1 << " definition lower1_x: " << lower1_x << " lower1_y: " << lower1_y << " upper1_x: " << upper1_x << " upper1_y: " << upper1_y << endl);
          }

          // we can have different stages share the same logical table
          if ((upper_y == upper1_y) && (lower_y == lower1_y) && (upper_x == upper1_x) && (lower_x == lower1_x) &&
              ((idx >> 4) != (idx1 >> 4))) {
            PLOG_INFO("Logical tables " << idx << " and " << idx1 << " are common to stages " << (idx >> 4) << " and " << (idx1 >> 4) << endl);
            continue;
          }
          if (((upper_y < lower1_y) && (lower_y > upper1_y) && (upper_x < lower1_x) && (lower_x > upper1_x)) ||
              ((upper1_y < lower_y) && (lower1_y > upper_y) && (upper1_x < lower_x) && (lower1_x > upper_x))) {
            PLOG_ERR("Overlapping logic tables " << idx << " and " << idx1
                     << " start_addr: " << start_addr << " end_addr: " << end_addr
                     << " upper_x: " << upper_x << " upper_y: " << upper_y
                     << " lower_x: " << lower_x << " lower_y: " << lower_y
                     << " start_addr1: " << start_addr1 << " end_addr1: " << end_addr1
                     << " upper1_x: " << upper1_x << " upper1_y: " << upper1_y
                     << " lower1_x: " << lower1_x << " lower1_y: " << lower1_y << endl);
          }
        }
      }
      uint32_t entry_size = pics_csr.cfg_table_profile[idx].width().convert_to<uint32_t>() * 2;
      // entries should not span more than 4 SRAM macros
      uint32_t entries_per_line = 1 << pics_csr.cfg_table_profile[idx].log2bkts().convert_to<uint32_t>();
      
      if ((upper_x % 16 + entry_size > 4 * 16) ||
          ((entries_per_line == 2) && (upper_x % 16 + 2 * entry_size > 8 * 16))) {
        PLOG_ERR("Table " << idx << " entry cannot span more than 4 physical SRAM macros upper_x: " << upper_x << " entry_size: " << entry_size << endl);
      }

      tbl_opcode_dec.all(pics_csr.cfg_table_profile[idx].opcode());
      uint32_t operation = tbl_opcode_dec.operation().convert_to<uint32_t>();
      uint32_t oprd1_sel = tbl_opcode_dec.oprd1_sel().convert_to<uint32_t>();
      uint32_t oprd2_sel = tbl_opcode_dec.oprd2_sel().convert_to<uint32_t>();
      uint32_t saturate = tbl_opcode_dec.saturate().convert_to<uint32_t>();
      uint32_t policer = tbl_opcode_dec.policer().convert_to<uint32_t>();
      if (operation != PIC_TBL_OPCODE_OPERATION_NONE) {
        // rmw entries should not span SRAM macros
        if (policer) {
          if ((upper_x % entry_size != 0) && (entry_size != 16) && (entry_size != 32))   {
            PLOG_ERR("Table " << idx << " configured as policer needs to have entries that are SRAM macro aligned and either 16 or 32 bytes upper_x: " << upper_x << " lower_x: " << lower_x << " entry_size: " << entry_size << endl);
          }
        }
        else {
          if ((upper_x % entry_size != 0) &&
            (upper_x >> 4) != (lower_x >> 4))  {
            PLOG_ERR("Table " << idx << " configured as counter needs to have entries that do not span physical macros upper_x: " << upper_x << " lower_x: " << lower_x << " entry_size: " << entry_size << endl);
          }
        }
        // legal programming
        bool legal = true;
        if (policer == 0) {
          switch (oprd1_sel) {
          case PIC_TBL_OPCODE_OPRD1_SEL_COUNTER: 
            switch (oprd2_sel) {
            case PIC_TBL_OPCODE_OPRD2_SEL_ONE: 
              switch (operation) {
              case PIC_TBL_OPCODE_OPERATION_ADD: 
                switch (saturate) {
                case PIC_TBL_OPCODE_SATURATE_NONE:
                case PIC_TBL_OPCODE_SATURATE_MINMAX:
                  break;
                default:
                  legal = false;
                  break;
                }
                break;
              case PIC_TBL_OPCODE_OPERATION_CLEAR: 
                switch (saturate) {
                case PIC_TBL_OPCODE_SATURATE_NONE:
                  break;
                default:
                  legal = false;
                  break;
                }
                break;
              default: 
                legal = false;
                break;
              }
              break;
            case PIC_TBL_OPCODE_OPRD2_SEL_PKTSIZE:
              switch (operation) {
              case PIC_TBL_OPCODE_OPERATION_ADD: 
                switch (saturate) {
                case PIC_TBL_OPCODE_SATURATE_NONE:
                case PIC_TBL_OPCODE_SATURATE_MINMAX:
                  break;
                default:
                  legal = false;
                  break;
                }
                break;
              case PIC_TBL_OPCODE_OPERATION_BITSET:
                switch (saturate) {
                case PIC_TBL_OPCODE_SATURATE_NONE:
                  break;
                default:
                  legal = false;
                  break;
                }
                break;
              default: 
                legal = false;
                break;
              }
              break;
            default: 
              legal = false;
              break;
            }
            break;
          case PIC_TBL_OPCODE_OPRD1_SEL_RATE: 
            switch (oprd2_sel) {
            case PIC_TBL_OPCODE_OPRD2_SEL_ONE: 
              switch (operation) {
              case PIC_TBL_OPCODE_OPERATION_ADD: 
                switch (saturate) {
                case PIC_TBL_OPCODE_SATURATE_MINMAX:
                case PIC_TBL_OPCODE_SATURATE_NONE:
                  break;
                default:
                  legal = false;
                  break;
                }
                break;
              default: 
                legal = false;
                break;
              }
              break;
            case PIC_TBL_OPCODE_OPRD2_SEL_PKTSIZE:
              switch (operation) {
              case PIC_TBL_OPCODE_OPERATION_ADD: 
                switch (saturate) {
                case PIC_TBL_OPCODE_SATURATE_MINMAX:
                case PIC_TBL_OPCODE_SATURATE_NONE:
                  break;
                default:
                  legal = false;
                  break;
                }
                break;
              default: 
                legal = false;
                break;
              }
              break;
            default: 
              legal = false;
              break;
            }
            break;
          case PIC_TBL_OPCODE_OPRD1_SEL_TBKT: 
            switch (oprd2_sel) {
            case PIC_TBL_OPCODE_OPRD2_SEL_RATE: 
              switch (operation) {
              case PIC_TBL_OPCODE_OPERATION_MIN: 
                switch (saturate) {
                case PIC_TBL_OPCODE_SATURATE_NONE:
                  break;
                default:
                  legal = false;
                  break;
                }
                break;
              case PIC_TBL_OPCODE_OPERATION_MAX: 
                switch (saturate) {
                case PIC_TBL_OPCODE_SATURATE_NONE:
                  break;
                default:
                  legal = false;
                  break;
                }
                break;
              case PIC_TBL_OPCODE_OPERATION_AVG: 
                switch (saturate) {
                case PIC_TBL_OPCODE_SATURATE_NONE:
                  break;
                default:
                  legal = false;
                  break;
                }
                break;
              case PIC_TBL_OPCODE_OPERATION_CPY: 
                switch (saturate) {
                case PIC_TBL_OPCODE_SATURATE_NONE:
                  break;
                default:
                  legal = false;
                  break;
                }
                break;
              default: 
                legal = false;
                break;
              }
              break;
            default: 
              legal = false;
              break;
            }
            break;
          }
        }
        else {
          if (oprd1_sel == PIC_TBL_OPCODE_OPRD1_SEL_TBKT) {
            // Kevin & SW this is appropiate for rate-lim 
            if (oprd2_sel == PIC_TBL_OPCODE_OPRD2_SEL_PKTSIZE) {
              if (operation == PIC_TBL_OPCODE_OPERATION_SUB) {
                if ((saturate == PIC_TBL_OPCODE_SATURATE_NONE) || (saturate == PIC_TBL_OPCODE_SATURATE_NEG)) {
                }
                else {
                  legal = false;
                }
              }
              else {
                legal = false;
              }
            }
            else if (oprd2_sel == PIC_TBL_OPCODE_OPRD2_SEL_RATE) {
              if (operation == PIC_TBL_OPCODE_OPERATION_ADD) {
                if (saturate == PIC_TBL_OPCODE_SATURATE_OPRD3) {
                }
                else {
                  legal = false;
                }
              }
              else {
                legal = false;
              }
            }
            else {
              if (operation == PIC_TBL_OPCODE_OPERATION_SUB) {
                if (saturate == PIC_TBL_OPCODE_SATURATE_NEG) {
                }
                else {
                  legal = false;
                }
              }
              else {
                legal = false;
              }
            }
          }
        }
        if (!legal) {
          PLOG_ERR("Read-modify-write table " << idx << " has incorrect programming" << endl);
          tbl_opcode_dec.show();
        }
      }
    }
  }

  pics_csr.cfg_bg_update_profile_enable.read();
  for (auto idx = 0; idx < 16; idx++) {
    if ((pics_csr.cfg_bg_update_profile_enable.vector().convert_to<uint32_t>() >> idx) & 0x1) {
      pics_csr.cfg_bg_update_profile[idx].read();
      tbl_opcode_dec.all(pics_csr.cfg_bg_update_profile[idx].opcode());
      uint32_t operation = tbl_opcode_dec.operation().convert_to<uint32_t>();
      uint32_t oprd1_sel = tbl_opcode_dec.oprd1_sel().convert_to<uint32_t>();
      uint32_t oprd2_sel = tbl_opcode_dec.oprd2_sel().convert_to<uint32_t>();
      uint32_t saturate = tbl_opcode_dec.saturate().convert_to<uint32_t>();
      uint32_t policer = tbl_opcode_dec.policer().convert_to<uint32_t>();
      if ((operation == PIC_TBL_OPCODE_OPERATION_ADD) &&
          (oprd1_sel == PIC_TBL_OPCODE_OPRD1_SEL_TBKT) &&
          (oprd2_sel == PIC_TBL_OPCODE_OPRD2_SEL_RATE) &&
          (saturate == PIC_TBL_OPCODE_SATURATE_OPRD3) &&
          (policer == 1)) {
        // only legal combination
      }
      else {
          PLOG_ERR("Background update profile " << idx << " has incorrect programming" << endl);
          tbl_opcode_dec.show();
      }
    }
  }   
  //Cache filters consistency check
  string inst_name[4] = {
   "RD",
   "SI",
   "SE",
   "TD"
   };

 pics_csr.picc.cfg_filter_m.read();
 if(pics_csr.picc.cfg_filter_m.arcache_match() != pics_csr.picc.cfg_filter_m.awcache_match()) {
    PLOG_ERR(inst_name[inst_id] << " : " << " cfg_filter_m.arcache_match : 0x" << hex << pics_csr.picc.cfg_filter_m.arcache_match() 
              << " not equal to cfg_filter_m.awcache_match : 0x" << pics_csr.picc.cfg_filter_m.awcache_match() << dec << endl);
 }
 if(pics_csr.picc.cfg_filter_m.arcache_mask() != pics_csr.picc.cfg_filter_m.awcache_mask()) {
    PLOG_ERR(inst_name[inst_id] << " : " << " cfg_filter_m.arcache_mask : 0x" << hex << pics_csr.picc.cfg_filter_m.arcache_mask() 
              << " not equal to cfg_filter_m.awcache_mask : 0x" << pics_csr.picc.cfg_filter_m.awcache_mask() << dec << endl);
 }

 pics_csr.picc.cfg_filter_s.read();
 if(pics_csr.picc.cfg_filter_s.all() != pics_csr.picc.cfg_filter_m.all()) {
    PLOG_ERR(inst_name[inst_id] << " : " << " cfg_filter_s : 0x" << hex << pics_csr.picc.cfg_filter_s.all()
              << " not equal to cfg_filter_m : 0x" << pics_csr.picc.cfg_filter_m.all() << dec << endl);
 }

 for(int idx = 0; idx < 8; idx++) {
     pics_csr.picc.filter_addr_ctl_m.value[idx].read();
     if( (pics_csr.picc.filter_addr_ctl_m.value[idx].all() & 0x8) == 0x8) { //bit3 (vld)

       pics_csr.picc.filter_addr_hi_s.data[idx].read();
       pics_csr.picc.filter_addr_hi_m.data[idx].read();
       if( pics_csr.picc.filter_addr_hi_s.data[idx].all() != pics_csr.picc.filter_addr_hi_m.data[idx].all() ) {
         PLOG_ERR(inst_name[inst_id] << " : idx : " << idx << " filter_addr_hi_s : 0x" << hex << pics_csr.picc.filter_addr_hi_s.data[idx].all()
              << " not equal to filter_addr_hi_m : 0x" << pics_csr.picc.filter_addr_hi_m.data[idx].all() << dec << endl);
       }

       pics_csr.picc.filter_addr_lo_s.data[idx].read();
       pics_csr.picc.filter_addr_lo_m.data[idx].read();
       if( pics_csr.picc.filter_addr_lo_s.data[idx].all() != pics_csr.picc.filter_addr_lo_m.data[idx].all() ) {
         PLOG_ERR(inst_name[inst_id] << " : idx : " << idx << " filter_addr_lo_s : 0x" << hex << pics_csr.picc.filter_addr_lo_s.data[idx].all()
              << " not equal to filter_addr_lo_m : 0x" << pics_csr.picc.filter_addr_lo_m.data[idx].all() << dec << endl);
       }
     } //if filter enabled
 }

} //cap_pics_check_progr_sanity

void cap_pics_check_mem_values(int chip_id, int inst_id, int num_srams) {

  
  PLOG_API_MSG("PICS[" << inst_id << "]", "running dhs_sram mem check\n");

#ifdef PEN_CSR_ZERO_TIME_ENABLE
  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);
  uint32_t skip_cpu_rmw_loc = sknobs_get_value((char *)"sc_tb/skip_cpu_rmw_loc", 0);

  for (auto idx = 0; idx < 4096 * num_srams; idx++) {
    // can't use this directly because of ECC
    // pics_csr.dhs_sram.entry[idx].read_compare();
    cpp_int shadow_all = pics_csr.dhs_sram.entry[idx].all();
    if ((cpu::access()->get_access_type() == back_door_e) && (pics_csr.dhs_sram.entry[idx].get_access_no_zero_time() == false)) {
      bool zerotime_success = pics_csr.dhs_sram.entry[idx].read_all_fields_zero_time();
      if (zerotime_success) {
        if (pics_csr.dhs_sram.entry[idx].all()) {
          PLOG_MSG("csr read zerotime: " << pics_csr.dhs_sram.entry[idx].get_hier_path() << " val: 0x" << hex << pics_csr.dhs_sram.entry[idx].all() << endl << dec);
        }
        // Mask off 9 bits of ECC
        uint256_t ecc_mask = 0x1ff;
        ecc_mask <<= 128;
        ecc_mask = ~ecc_mask;
        cpp_int shdw_val_no_ecc = shadow_all & ecc_mask;
        cpp_int hw_val_no_ecc = pics_csr.dhs_sram.entry[idx].all() & ecc_mask;
        if (shdw_val_no_ecc != hw_val_no_ecc) {
          if (skip_cpu_rmw_loc && (idx >= 4076)) {
            PLOG_WARN("read_compare : " << pics_csr.dhs_sram.entry[idx].get_hier_path() << " exp: 0x" << hex << shdw_val_no_ecc << " actual: 0x" << hw_val_no_ecc << " ecc_mask: 0x" << ecc_mask << dec << " but check disabled because we're testing cpu_rmw" << endl);
          }
          else {
            PLOG_ERR("read_compare : " << pics_csr.dhs_sram.entry[idx].get_hier_path() << " exp: 0x" << hex << shdw_val_no_ecc << " actual: 0x" << hw_val_no_ecc << " ecc_mask: 0x" << ecc_mask << dec << endl);
          }
        }
      }
      else {
        PLOG_ERR("zerotime read failure: " << pics_csr.dhs_sram.entry[idx].get_hier_path() << endl);
      }
    }
    else {
      PLOG_ERR("PICS[" << inst_id << "] refusing to go through all the memories when zero time is not enabled" << endl);
    }
  }
#else
  PLOG_ERR("PICS[" << inst_id << "] refusing to go through all the memories when zero time is not enabled" << endl);
#endif    

  PLOG_API_MSG("PICS[" << inst_id << "]", " done running mem check\n");
}

void cap_pics_zero_init_sram(int chip_id, int inst_id, int num_srams) {

  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);
  
  PLOG_API_MSG("PICS[" << inst_id << "]", "initializing SRAMs to zero\n");

  for (auto idx = 0; idx < 4096 * num_srams; idx++) {
    pics_csr.dhs_sram.entry[idx].data(0);
    pics_csr.dhs_sram.entry[idx].write();
  }

  PLOG_API_MSG("PICS[" << inst_id << "]", " done initializing SRAMs to zero\n");
}

void cap_pics_extract_logical_table_def_from_cfg(int chip_id, int inst_id, 
                                                 bool read_from_hw,
                                                 block_type type) {

  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);
  string hier_path_str = pics_csr.get_hier_path();
  stringstream tmp_ss;
  string tmp_str;
  
  PLOG_API_MSG("PICS[" << inst_id << "]", "extracting logical table defs from config with path " << hier_path_str << endl);

  auto logic_table_idx = 0;

  for (auto idx = 0; idx < 128; idx++) {
    if (read_from_hw) {
      pics_csr.cfg_table_profile[idx].read();
    }

    if (pics_csr.cfg_table_profile[idx].all() != 0) {
      uint32_t upper_x = 2 * (pics_csr.cfg_table_profile[idx].start_addr().convert_to<uint32_t>() % 256);
      uint32_t upper_y = 2 * (pics_csr.cfg_table_profile[idx].start_addr().convert_to<uint32_t>() / 256);
      uint32_t lower_x = 2 * (pics_csr.cfg_table_profile[idx].end_addr().convert_to<uint32_t>() % 256);
      uint32_t lower_y = 2 * (pics_csr.cfg_table_profile[idx].end_addr().convert_to<uint32_t>() / 256);
      uint32_t entry_size = 2 * pics_csr.cfg_table_profile[idx].width().convert_to<uint32_t>();
      PLOG_INFO("Saving logical table " << idx << ": upper_x = " << upper_x << " lower_x = " << lower_x << " upper_y = " << upper_y << " lower_y = " << lower_y << " entry_size = " << entry_size << endl);
      tmp_ss.str("");
      tmp_ss << hier_path_str << "table_defs/" << logic_table_idx << "/table_id";
      tmp_str = tmp_ss.str();
      PLOG_INFO(tmp_str.c_str() << " = " << idx << endl);
      sknobs_set_value((char *)tmp_str.c_str(), idx);
      tmp_ss.str("");
      tmp_ss << hier_path_str << "table_defs/" << logic_table_idx << "/upper_x";
      tmp_str = tmp_ss.str();
      PLOG_INFO(tmp_str.c_str() << " = " << upper_x << endl);
      sknobs_set_value((char *)tmp_str.c_str(), upper_x);
      tmp_ss.str("");
      tmp_ss << hier_path_str << "table_defs/" << logic_table_idx << "/upper_y";
      tmp_str = tmp_ss.str();
      PLOG_INFO(tmp_str.c_str() << " = " << upper_y << endl);
      sknobs_set_value((char *)tmp_str.c_str(), upper_y);
      tmp_ss.str("");
      tmp_ss << hier_path_str << "table_defs/" << logic_table_idx << "/lower_x";
      tmp_str = tmp_ss.str();
      PLOG_INFO(tmp_str.c_str() << " = " << lower_x << endl);
      sknobs_set_value((char *)tmp_str.c_str(), lower_x);
      tmp_ss.str("");
      tmp_ss << hier_path_str << "table_defs/" << logic_table_idx << "/lower_y";
      tmp_str = tmp_ss.str();
      PLOG_INFO(tmp_str.c_str() << " = " << lower_y << endl);
      sknobs_set_value((char *)tmp_str.c_str(), lower_y);
      tmp_ss.str("");
      tmp_ss << hier_path_str << "table_defs/" << logic_table_idx << "/entry_size";
      tmp_str = tmp_ss.str();
      PLOG_INFO(tmp_str.c_str() << " = " << entry_size << endl);
      sknobs_set_value((char *)tmp_str.c_str(), entry_size);
      tmp_ss.str("");
      tmp_ss << hier_path_str << "table_defs/" << logic_table_idx << "/hash";
      tmp_str = tmp_ss.str();
      PLOG_INFO("saving " << tmp_str.c_str() << " = " << (pics_csr.cfg_table_profile[idx].hash().convert_to<uint32_t>()) << endl);
      sknobs_set_value((char *)tmp_str.c_str(), pics_csr.cfg_table_profile[idx].hash().convert_to<uint32_t>());
      tmp_ss.str("");
      tmp_ss << hier_path_str << "table_defs/" << logic_table_idx << "/opcode";
      tmp_str = tmp_ss.str();
      PLOG_INFO("saving " << tmp_str.c_str() << " = " << (pics_csr.cfg_table_profile[idx].opcode().convert_to<uint32_t>()) << endl);
      sknobs_set_value((char *)tmp_str.c_str(), pics_csr.cfg_table_profile[idx].opcode().convert_to<uint32_t>());
      tmp_ss.str("");
      tmp_ss << hier_path_str << "table_defs/" << logic_table_idx << "/axishift";
      tmp_str = tmp_ss.str();
      PLOG_INFO("saving " << tmp_str.c_str() << " = " << (pics_csr.cfg_table_profile[idx].axishift().convert_to<uint32_t>()) << endl);
      sknobs_set_value((char *)tmp_str.c_str(), pics_csr.cfg_table_profile[idx].axishift().convert_to<uint32_t>());
      tmp_ss.str("");
      tmp_ss << hier_path_str << "table_defs/" << logic_table_idx << "/rlimit_en";
      tmp_str = tmp_ss.str();
      PLOG_INFO("saving " << tmp_str.c_str() << " = " << (pics_csr.cfg_table_profile[idx].rlimit_en().convert_to<uint32_t>()) << endl);
      sknobs_set_value((char *)tmp_str.c_str(), pics_csr.cfg_table_profile[idx].rlimit_en().convert_to<uint32_t>());
      logic_table_idx++;
    }
  }
  tmp_ss.str("");
  tmp_ss << hier_path_str << "num_tables";
  sknobs_set_value((char *)tmp_str.c_str(), logic_table_idx);
}

//void cap_pics_bgnd_prof_on_off(int chip_id, int inst_id, uint32_t bg_idx, uint32_t on_off) {
void cap_pics_bgnd_prof_on_off(int chip_id, int inst_id, uint32_t vector) {

  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);

  //PLOG_MSG((on_off ? "Enabling " : " Disabling ") << " policer background update profile " << bg_idx << endl);
  //pics_csr.cfg_bg_update_profile_enable.read();
  //uint32_t data = pics_csr.cfg_bg_update_profile_enable.vector().convert_to<uint32_t>();
  //uint32_t mask = 0xffff ^ (1 << bg_idx);
  //uint32_t new_data = (data & mask) | ((on_off & 0x1) << bg_idx);
  //pics_csr.cfg_bg_update_profile_enable.vector(new_data);
  PLOG_MSG(" policer background update profile enable vector = " << hex << vector << endl);
  pics_csr.cfg_bg_update_profile_enable.vector(vector);
  pics_csr.cfg_bg_update_profile_enable.write();

}

void cap_pics_enable_cache(int chip_id, int inst_id) {

  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);

  PLOG_MSG("Enabling cache" << endl);
  pics_csr.picc.cfg_cache_global.bypass(0);
  pics_csr.picc.cfg_cache_global.hash_mode(0);
  pics_csr.picc.cfg_cache_global.write();

  // Don't do RMW because we will get Xs and error out
  //pics_csr.picc.filter_addr_hi_s.data[0].read();
  pics_csr.picc.filter_addr_hi_s.data[0].value(0xfffffff);
  pics_csr.picc.filter_addr_hi_s.data[0].write();
  //pics_csr.picc.filter_addr_ctl_s.value[0].read();
  pics_csr.picc.filter_addr_ctl_s.value[0].value(0xf);
  pics_csr.picc.filter_addr_ctl_s.value[0].write();
  //pics_csr.picc.filter_addr_hi_m.data[0].read();
  pics_csr.picc.filter_addr_hi_m.data[0].value(0xfffffff);
  pics_csr.picc.filter_addr_hi_m.data[0].write();
  //pics_csr.picc.filter_addr_ctl_m.value[0].read();
  pics_csr.picc.filter_addr_ctl_m.value[0].value(0xf);
  pics_csr.picc.filter_addr_ctl_m.value[0].write();
}

void cap_pics_print_cache_cfg_info(int chip_id, int inst_id) {
  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);

   string inst_name[4] = {
   "RD",
   "SI",
   "SE",
   "TD"
   };

  PLOG_INFO(inst_name[inst_id] << " cache config info : " << endl)
  pics_csr.picc.cfg_cache_global.read();
  if(pics_csr.picc.cfg_cache_global.bypass() == 0) {
     PLOG_INFO(inst_name[inst_id] << " : cache enabled with hash_mode : " << pics_csr.picc.cfg_cache_global.hash_mode() << endl );
  } //if cache enabled

  for(int idx = 0; idx < 8; idx++) {
      pics_csr.picc.filter_addr_ctl_m.value[idx].read();
      if( (pics_csr.picc.filter_addr_ctl_m.value[idx].all() & 0x8) == 0x8) { //bit3 (vld)
        pics_csr.picc.filter_addr_hi_m.data[idx].read();
        pics_csr.picc.filter_addr_lo_m.data[idx].read();
        if( ((pics_csr.picc.filter_addr_ctl_m.value[idx].all()>>2) & 0x1) == 1) {
          PLOG_INFO(inst_name[inst_id] << " : filter idx : " << idx << " enabled with include addr range"
               << " filter_addr_lo_m : 0x" << hex << pics_csr.picc.filter_addr_lo_m.data[idx].all()
               << " filter_addr_hi_m : 0x" << pics_csr.picc.filter_addr_hi_m.data[idx].all() 
               << " inval_send : 0x" << hex << (pics_csr.picc.filter_addr_ctl_m.value[idx].all() & 0x1)
               << " inval_fill : 0x" << hex << ((pics_csr.picc.filter_addr_ctl_m.value[idx].all()>>1) & 0x1)
               << dec << endl)
        } else {
          PLOG_INFO(inst_name[inst_id] << " : filter idx : " << idx << " enabled with exlcude addr range"
               << " filter_addr_lo_m : 0x" << hex << pics_csr.picc.filter_addr_lo_m.data[idx].all()
               << " filter_addr_hi_m : 0x" << pics_csr.picc.filter_addr_hi_m.data[idx].all() << dec << endl)
        }
      } //if filter enabled
  }

} //cap_pics_cache_print_info

void cap_pics_cache_cnt(int chip_id, int inst_id) {
  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);

   string inst_name[4] = {
   "RD",
   "SI",
   "SE",
   "TD"
   };

   string cnts[5] = {
   "cnt_cache_rd_lookup",
   "cnt_cache_rd_hit",
   "cnt_cache_wr_lookup",
   "cnt_cache_wr_hit",
   "cnt_cache_inval"
   };

   for (int i = 0; i < 5; i++) {
      pics_csr.picc.dhs_cache_cnt.entry[i].read();
      cpp_int rdata = pics_csr.picc.dhs_cache_cnt.entry[i].value();
      PLOG_API_MSG("PICS[" << inst_id << "]", cnts[i].append(25-cnts[i].length(),' ') << " = " << rdata << "\n" );
   }
}

int cap_pics_cache_get_bit(int s, int sel) {
   return (s >> sel) & 0x1;
}
int cap_pics_cache_set_bit(int s, int sel, int val) {
   int mask = 1 << sel;
   return (s & ~mask) | ((val << sel) & mask);
}

int cap_pics_cache_hash_func1(int chip_id, int inst_id, int key) {
   int ADDR_SZ = 28;
   int HASH_POLY = (inst_id == 1 || inst_id == 2) ? 0x70 : 0x184;
   int HASH_ADDR_SZ = (inst_id == 1 || inst_id == 2) ? 7 : 9;
   int s = 0;
   for (int j = ADDR_SZ-1; j >= 0; j=j-1) {
      int feedback = cap_pics_cache_get_bit(s, HASH_ADDR_SZ-1) ^ cap_pics_cache_get_bit(key, j);
      for (int i = HASH_ADDR_SZ-1; i > 0; i=i-1) {
         if (cap_pics_cache_get_bit(HASH_POLY, i)) {
            s = cap_pics_cache_set_bit(s, i, cap_pics_cache_get_bit(s, i-1) ^ feedback);
         } else {
            s = cap_pics_cache_set_bit(s, i, cap_pics_cache_get_bit(s, i-1));
         }
      }
      s = cap_pics_cache_set_bit(s, 0, feedback);
   }
   return s;
}

int cap_pics_cache_hash_func2(int chip_id, int inst_id, int hash, int key) {
   //int ADDR_SZ = 28;
   int HASH_POLY = (inst_id == 1 || inst_id == 2) ? 0x70 : 0x184;
   int HASH_ADDR_SZ = (inst_id == 1 || inst_id == 2) ? 7 : 9;
   int s1 = cap_pics_cache_hash_func1(chip_id, inst_id, key);
   int s2 = hash;

   for (int j = 0; j < HASH_ADDR_SZ; j=j+1) {
      int feedback = cap_pics_cache_get_bit(s2, 0);
      for (int i = 0; i < HASH_ADDR_SZ; i=i+1) {
         if (i < HASH_ADDR_SZ-1) {
            if (cap_pics_cache_get_bit(HASH_POLY, i+1)) {
               s2 = cap_pics_cache_set_bit(s2, i, cap_pics_cache_get_bit(s2, i+1) ^ feedback);
            } else {
               s2 = cap_pics_cache_set_bit(s2, i, cap_pics_cache_get_bit(s2, i+1));
            }
         } else {
            s2 = cap_pics_cache_set_bit(s2, i, cap_pics_cache_get_bit(s1, j) ^ feedback);
         }
      }
   }
   return (key << HASH_ADDR_SZ) |  s2;
}

void cap_pics_cache_tag_tbl_report(int chip_id, int inst_id, uint32_t addr_lo, uint32_t addr_hi, bool show_addr) {
   cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);

   string inst_name[4] = {
   "RD",
   "SI",
   "SE",
   "TD"
   };

   int tag_tbl_depth;
   if (inst_id == 1 || inst_id == 2) tag_tbl_depth = 128;
   else tag_tbl_depth = 512;

   int key_size;
   if (inst_id == 1 || inst_id == 2) key_size = 21;
   else key_size = 19;

   int key_mask;
   if (inst_id == 1 || inst_id == 2) key_mask = 0x1fffff;
   else key_mask = 0x7ffff;

   cpp_int vld[8];
   cpp_int lru[8];
   cpp_int key[8];
   cpp_int sum = 0;
   cpp_int line_used = 0;
   for (int i = 0; i < tag_tbl_depth; i++) {
      cpp_int temp = sum;
      pics_csr.picc.dhs_cache_tag_sram0.entry[i].read();
      cpp_int rdata0 = pics_csr.picc.dhs_cache_tag_sram0.entry[i].ecc_data();
      for (int j = 0; j < 4; j++) {
         key[j] = rdata0 & key_mask; rdata0 = rdata0 >> key_size;
         lru[j] = rdata0 & 0x7; rdata0 = rdata0 >> 3;
         vld[j] = rdata0 & 0x1; rdata0 = rdata0 >> 1;
         uint32_t orig_addr = cap_pics_cache_hash_func2(chip_id, inst_id, i, (int) key[j]);
         if (vld[j] == 1 && (orig_addr >= addr_lo) && (orig_addr <= addr_hi)) {
            if (show_addr) {
               PLOG_API_MSG("PICS[" << inst_id << "]", "key = " << hex << key[j] << ", hash line = " << i << ", bucket = " << j << ", orig addr = " << orig_addr << dec << endl);
            }
            sum += vld[j];
         }
      }
      pics_csr.picc.dhs_cache_tag_sram1.entry[i].read();
      cpp_int rdata1 = pics_csr.picc.dhs_cache_tag_sram1.entry[i].ecc_data();
      for (int j = 4; j < 8; j++) {
         key[j] = rdata1 & key_mask; rdata1 = rdata1 >> key_size;
         lru[j] = rdata1 & 0x7; rdata1 = rdata1 >> 3;
         vld[j] = rdata1 & 0x1; rdata1 = rdata1 >> 1;
         uint32_t orig_addr = cap_pics_cache_hash_func2(chip_id, inst_id, i, (int) key[j]);
         if (vld[j] == 1 && (orig_addr >= addr_lo) && (orig_addr <= addr_hi)) {
            if (show_addr) {
               PLOG_API_MSG("PICS[" << inst_id << "]", "key = " << hex << key[j] << ", hash line = " << i << ", bucket = " << j << ", orig addr = " << orig_addr << dec << endl);
            }
            sum += vld[j];
         }
      }
      if (sum > temp) line_used++;
   }
   PLOG_API_MSG("PICS[" << inst_id << "]", "cap_pics_cache_tag_tbl_report: between addresses " << hex << addr_lo << " and " << addr_hi << dec << ", number of cache entries = " << sum << ", and number of cache lines used = " << line_used << endl);
}

class pics_bkdr_callback : public cap_csr_callback {

public  :

  bool is_pc;

  pics_bkdr_callback (bool _is_pc) : is_pc (_is_pc) {};

  void pre_csr_mem_zerotime_hdl_write(pen_csr_base * ptr, string & path, cpp_int & data, unsigned & bits);
  void pre_csr_mem_zerotime_hdl_read(pen_csr_base * ptr, string & path, cpp_int & data, unsigned & bits);
  void fix_dhs_sram_path(pen_csr_base * ptr, string & path);

};

void cap_pics_csr_set_hdl_path(int chip_id, int inst_id, string path, bool is_pc) {

  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);

  // id : 0 is given to csr instance.
  // id : 1 is given to memories
  pics_csr.set_csr_inst_path(0, (path + ".scsr")); 
  PLOG_API_MSG("PICS[" << inst_id << "]", "Added backdoor path " << (path + ".scsr") << endl);
    
  std::shared_ptr<cap_csr_callback> pics_bkdr_cbk = std::make_shared<pics_bkdr_callback>(is_pc);
  pics_csr.dhs_sram.append_callback(pics_bkdr_cbk);

  // disable backdoor to these
  PLOG_API_MSG("PICS[" << inst_id << "]", "Disabling backdoor path for " << pics_csr.dhs_sram_update_addr.entry.get_name() << endl);
  pics_csr.dhs_sram_update_addr.entry.set_access_no_zero_time(1);
  PLOG_API_MSG("PICS[" << inst_id << "]", "Disabling backdoor path for " << pics_csr.dhs_sram_update_data.entry.get_name() << endl);
  pics_csr.dhs_sram_update_data.entry.set_access_no_zero_time(1);
  
}

void cap_picc_csr_set_hdl_path(int chip_id, int inst_id, string path, bool is_pc) {

  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);

  if (is_pc) {
    if (path.empty()) {
      pics_csr.picc.set_csr_inst_path(1, "cap_pic_cache4p.ccsr");
    }
    else {
      pics_csr.picc.set_csr_inst_path(1, (path + ".cap_pic_cache4p_inst0.ccsr"));
    }
  }
  else {
    if (path.empty()) {
      pics_csr.picc.set_csr_inst_path(1, "cap_pic_cache4_inst0.ccsr");
    }
    else {
      pics_csr.picc.set_csr_inst_path(1, (path + ".cap_pic_cache4_inst0.ccsr"));
    }
  }
  // disable backdoor paths for the DHS because our infra croaks
  PLOG_API_MSG("PICS[" << inst_id << "]", "Disabling backdoor path for " << pics_csr.picc.dhs_cache_invalidate.get_name() << endl);
  pics_csr.picc.dhs_cache_invalidate.set_access_no_zero_time(1);
  PLOG_API_MSG("PICS[" << inst_id << "]", "Disabling backdoor path for " << pics_csr.picc.dhs_cache_cnt.get_name() << endl);
  for (uint32_t idx=0; idx < 5; idx++) {
    pics_csr.picc.dhs_cache_cnt.entry[idx].set_access_no_zero_time(1);
  }
}

void pics_bkdr_callback::fix_dhs_sram_path(pen_csr_base * ptr, string & path) {

  uint32_t dhs_idx = get_idx_from_field_name(path);
  uint32_t sram_bank = dhs_idx / 4096;
  uint32_t sram_idx = dhs_idx % 4096;

  stringstream tmp_ss;
  tmp_ss << (is_pc ? ".ppor.ram_" : ".por.ram_") << sram_bank << ".mem.mem[" << sram_idx << "]";

  size_t last_dot_pos =  path.find_last_of('.');
  // need to get rid of the index, the scsr which was added and the pic instance name
  for (auto level = 0; level < 2; level++) { 
    last_dot_pos = path.find_last_of('.', last_dot_pos - 1);
  }
  path.erase(last_dot_pos);
  path.append(tmp_ss.str());
  
}

void pics_bkdr_callback::pre_csr_mem_zerotime_hdl_write(pen_csr_base * ptr, 
                                                      string & path, 
                                                      cpp_int & data, 
                                                      unsigned & bits) {
  if (ptr->get_name().compare(0, 5, "entry") == 0) {
    fix_dhs_sram_path(ptr, path);
    PLOG_INFO("zerotime callback memory entry write to " << path << endl;)
  }
  else {
    PLOG_ERR("zerotime callback called with the wrong object: " << ptr->get_name() << endl);
  }
}

void pics_bkdr_callback::pre_csr_mem_zerotime_hdl_read(pen_csr_base * ptr, 
                                                      string & path, 
                                                      cpp_int & data, 
                                                      unsigned & bits) {
  if (ptr->get_name().compare(0, 5, "entry") == 0) {
    fix_dhs_sram_path(ptr, path);
    PLOG_INFO("zerotime callback memory entry read from " << path << endl;)
  }
  else {
    PLOG_ERR("zerotime callback called with the wrong object: " << ptr->get_name() << endl);
  }
}

void cap_pics_invalidate_cache(int chip_id, int inst_id) {
  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);

  pics_csr.picc.dhs_cache_invalidate.entry.inval_all(1);
  pics_csr.picc.dhs_cache_invalidate.entry.write();
}

void cap_pics_bist_run(int chip_id, int inst_id, int enable) {
  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);

  PLOG_API_MSG("PICS[" << inst_id << "]", ": cap_pics_bist_run, enable = " << enable << "\n");

  int mem_vec = (inst_id == 1 || inst_id == 2) ? 0xff : 0x7;
  pics_csr.cfg_sram.read();
  pics_csr.cfg_sram.bist_run(mem_vec);
  pics_csr.cfg_sram.write();

  pics_csr.picc.cfg_cache_bist.tag_sram0_run(enable);
  pics_csr.picc.cfg_cache_bist.tag_sram1_run(enable);
  pics_csr.picc.cfg_cache_bist.data_sram0_run(enable);
  pics_csr.picc.cfg_cache_bist.data_sram1_run(enable);
  pics_csr.picc.cfg_cache_bist.data_sram2_run(enable);
  pics_csr.picc.cfg_cache_bist.data_sram3_run(enable);
  pics_csr.picc.cfg_cache_bist.write();
}

void cap_pics_bist_chk(int chip_id, int inst_id) {
  cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, chip_id, inst_id);

  PLOG_API_MSG("PICS[" << inst_id << "]", ": cap_pics_bist_chk\n");

  int loop_cnt = 0;
  do {
     pics_csr.sta_sram.read();
     pics_csr.picc.sta_cache_bist.read();
  } while((loop_cnt < 10000) &&
          ((pics_csr.sta_sram.all() == 0) ||
           (pics_csr.picc.sta_cache_bist.all() == 0)));

  bool err = false;
  int mem_vec = (inst_id == 1 || inst_id == 2) ? 0xff : 0x7;
  if (pics_csr.sta_sram.bist_done_pass() != mem_vec) {
     PLOG_ERR("pics_csr.sta_sram.bist_done_pass(): exp = 0x" << hex << mem_vec << ", rcv = 0x" << pics_csr.sta_sram.bist_done_pass() << dec << endl);
     err = true;
  }
  if (pics_csr.sta_sram.bist_done_fail() != 0) {
     PLOG_ERR("pics_csr.sta_sram.bist_done_fail(): exp = 0x0, rcv = 0x" << pics_csr.sta_sram.bist_done_fail() << dec << endl);
     err = true;
  }

  if (pics_csr.picc.sta_cache_bist.tag_sram0_pass() != 0x1) {
     PLOG_ERR("pics_csr.picc.sta_cache_bist.tag_sram0_pass: exp = 0x1, rcv = 0x" << pics_csr.picc.sta_cache_bist.tag_sram0_pass() << dec << endl);
     err = true;
  }
  if (pics_csr.picc.sta_cache_bist.tag_sram1_pass() != 0x1) {
     PLOG_ERR("pics_csr.picc.sta_cache_bist.tag_sram1_pass: exp = 0x1, rcv = 0x" << pics_csr.picc.sta_cache_bist.tag_sram1_pass() << dec << endl);
     err = true;
  }
  if (pics_csr.picc.sta_cache_bist.data_sram0_pass() != 0x1) {
     PLOG_ERR("pics_csr.picc.sta_cache_bist.data_sram0_pass: exp = 0x1, rcv = 0x" << pics_csr.picc.sta_cache_bist.data_sram0_pass() << dec << endl);
     err = true;
  }
  if (pics_csr.picc.sta_cache_bist.data_sram1_pass() != 0x1) {
     PLOG_ERR("pics_csr.picc.sta_cache_bist.data_sram1_pass: exp = 0x1, rcv = 0x" << pics_csr.picc.sta_cache_bist.data_sram1_pass() << dec << endl);
     err = true;
  }
  if (pics_csr.picc.sta_cache_bist.data_sram2_pass() != 0x1) {
     PLOG_ERR("pics_csr.picc.sta_cache_bist.data_sram2_pass: exp = 0x1, rcv = 0x" << pics_csr.picc.sta_cache_bist.data_sram2_pass() << dec << endl);
     err = true;
  }
  if (pics_csr.picc.sta_cache_bist.data_sram3_pass() != 0x1) {
     PLOG_ERR("pics_csr.picc.sta_cache_bist.data_sram3_pass: exp = 0x1, rcv = 0x" << pics_csr.picc.sta_cache_bist.data_sram3_pass() << dec << endl);
     err = true;
  }
  if (pics_csr.picc.sta_cache_bist.tag_sram0_fail() != 0x0) {
     PLOG_ERR("pics_csr.picc.sta_cache_bist.tag_sram0_fail: exp = 0x0, rcv = 0x" << pics_csr.picc.sta_cache_bist.tag_sram0_fail() << dec << endl);
     err = true;
  }
  if (pics_csr.picc.sta_cache_bist.tag_sram1_fail() != 0x0) {
     PLOG_ERR("pics_csr.picc.sta_cache_bist.tag_sram1_fail: exp = 0x0, rcv = 0x" << pics_csr.picc.sta_cache_bist.tag_sram1_fail() << dec << endl);
     err = true;
  }
  if (pics_csr.picc.sta_cache_bist.data_sram0_fail() != 0x0) {
     PLOG_ERR("pics_csr.picc.sta_cache_bist.data_sram0_fail: exp = 0x0, rcv = 0x" << pics_csr.picc.sta_cache_bist.data_sram0_fail() << dec << endl);
     err = true;
  }
  if (pics_csr.picc.sta_cache_bist.data_sram1_fail() != 0x0) {
     PLOG_ERR("pics_csr.picc.sta_cache_bist.data_sram1_fail: exp = 0x0, rcv = 0x" << pics_csr.picc.sta_cache_bist.data_sram1_fail() << dec << endl);
     err = true;
  }
  if (pics_csr.picc.sta_cache_bist.data_sram2_fail() != 0x0) {
     PLOG_ERR("pics_csr.picc.sta_cache_bist.data_sram2_fail: exp = 0x0, rcv = 0x" << pics_csr.picc.sta_cache_bist.data_sram2_fail() << dec << endl);
     err = true;
  }
  if (pics_csr.picc.sta_cache_bist.data_sram3_fail() != 0x0) {
     PLOG_ERR("pics_csr.picc.sta_cache_bist.data_sram3_fail: exp = 0x0, rcv = 0x" << pics_csr.picc.sta_cache_bist.data_sram3_fail() << dec << endl);
     err = true;
  }

  if (!err) {
     PLOG_API_MSG("PASSED: PICS[" << inst_id << "] including PICC", ": cap_pics_bist_chk\n");
  }

}
