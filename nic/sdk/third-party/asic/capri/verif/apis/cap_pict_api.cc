#include "cap_pict_api.h"
#include "cap_pic_decoders.h"
#include "sknobs.h"
#include "LogMsg.h"

void cap_pict_soft_reset(int chip_id, int inst_id) {

  PLOG_API_MSG("PICT[" << inst_id << "]", "inside softreset\n");

  // cap_pict_csr_t & pict_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pict_csr_t, chip_id, inst_id);
  // pict_csr.cfg_tcam_reset.enable(1);
  // pict_csr.cfg_tcam_reset.vec(0xff);
  // pict_csr.cfg_tcam_reset.write();
  // pict_csr.cfg_tcam_reset.show();

  // pict_csr.cfg_tcam_reset.enable(0);
  // pict_csr.cfg_tcam_reset.vec(0x00);
  // pict_csr.cfg_tcam_reset.write();
  // pict_csr.cfg_tcam_reset.show();

  PLOG_API_MSG("PICT[" << inst_id << "]", "done with softreset\n");
}

void cap_pict_set_soft_reset(int chip_id, int inst_id, int value) {
}

void cap_pict_init_start(int chip_id, int inst_id) {
}

void cap_pict_init_done(int chip_id, int inst_id) {
}

// use sknobs base load cfg 
void cap_pict_load_from_cfg(int chip_id, int inst_id, block_type type) {

  cap_pict_csr_t & pict_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pict_csr_t, chip_id, inst_id);

  PLOG_API_MSG("PICT[" << inst_id << "]", "load_from_cfg\n");
  if (inst_id == 0) {
    PLOG_API_MSG("PICT[" << inst_id << "]", "dumping sknobs\n");
    //sknobs_dump();
  }

  string hier_path_str = pict_csr.get_hier_path();
  PLOG_MSG("PICT[" << inst_id << "]" << " current path " << hier_path_str << endl);

  pict_csr.load_from_cfg(true, true);

  cap_pict_load_tcam_def_from_cfg(chip_id, inst_id, type);

  if (PLOG_CHECK_MSG_LEVEL(LogMsg::DEBUG)) {
    pict_csr.show();
  }
}

void cap_pict_eos(int chip_id, int inst_id) {

  cap_pict_eos_cnt(chip_id, inst_id);
  cap_pict_eos_int(chip_id, inst_id);
  cap_pict_eos_sta(chip_id, inst_id);
}

void cap_pict_eos_cnt(int chip_id, int inst_id) {
  
  cap_pict_csr_t & pict_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pict_csr_t, chip_id, inst_id);
  
  for (uint32_t port = 0; port < 8; port++) {
    pict_csr.cnt_axi_pot_rdreq[port].read();
    pict_csr.cnt_axi_pot_rdrsp[port].read();
    if (pict_csr.cnt_axi_pot_rdreq[port].val() !=  pict_csr.cnt_axi_pot_rdrsp[port].val()) {
      PLOG_ERR("PICT[" << inst_id << "] port " << port << " cnt_axi_pot_rdreq " << pict_csr.cnt_axi_pot_rdreq[port].val().convert_to<uint32_t>() << " != cnt_axi_pot_rdrsp " << pict_csr.cnt_axi_pot_rdreq[port].val().convert_to<uint32_t>() << endl);
    }
    else {
      PLOG_API_MSG("PICT[" << inst_id << "]", "Serviced " << pict_csr.cnt_axi_pot_rdreq[port].val().convert_to<uint32_t>() << " tcam searches on port " << port << endl);
    }
  }
  for (uint32_t tcam = 0; tcam < 8; tcam++) {
    pict_csr.cnt_tcam_search[tcam].read();
    PLOG_API_MSG("PICT[" << inst_id << "]", "Serviced " << pict_csr.cnt_tcam_search[tcam].val().convert_to<uint32_t>() << " tcam searches on physical TCAM " << tcam << endl);
  }
}

void cap_pict_eos_int(int chip_id, int inst_id) {
}

void cap_pict_eos_sta(int chip_id, int inst_id) {
}

void cap_pict_load_tcam_entry(int chip_id, int inst_id, 
                              uint32_t table_start, 
                              uint32_t table_end, 
                              uint32_t entry_size,
                              uint32_t entries_per_line,
                              uint32_t entry_idx,
                              cpp_int key,
                              cpp_int mask,
                              block_type type) {

  cpp_int_helper hlp;
  cap_pict_csr_t & pict_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pict_csr_t, chip_id, inst_id);
  uint32_t num_tcams = type == SI ? 8 : 4;
  // calculate coordinates for the entry; table_start and entry_size in increments of 2B
  uint32_t entry_size_bytes = entry_size * 2;

  // if we have multiple entries per line, we first search a full column before going to the next
  uint32_t table_start_x = (table_start * 2) % (16 * num_tcams);
  uint32_t table_start_y = (table_start * 2) / (16 * num_tcams);
  uint32_t table_end_x = (table_end * 2) % (16 * num_tcams) + 2;
  uint32_t table_end_y = (table_end * 2) / (16 * num_tcams);

  uint32_t macros_spanned_by_entry = (((table_start_x % 16) + entry_size_bytes) / 16) + (((((table_start_x % 16) + entry_size_bytes) % 16) == 0) ? 0 : 1);

  // uint32_t entry_x_coord = table_start_x + (entry_idx / (table_end_y - table_start_y + 1)) * 
  //   ((entry_size_bytes % 16 == 0) ? entry_size_bytes / 16 : (entry_size_bytes / 16) + 1) * 16;
  
  uint32_t entry_x_coord = table_start_x + (entry_idx / (table_end_y - table_start_y + 1)) * macros_spanned_by_entry * 16;
  uint32_t entry_y_coord = table_start_y + entry_idx % (table_end_y - table_start_y + 1);

  if ((entry_x_coord < table_start_x) ||
      (entry_x_coord > table_end_x) ||
      (entry_y_coord < table_start_y) ||
      (entry_y_coord > table_end_y) ||
      ((entry_x_coord  - table_start_x) % 16) > entries_per_line) {
    PLOG_ERR("Entry coordinates are wrong table_start_x: " << table_start_x << " table_start_y: " << table_start_y << " table_end_x: " << table_end_x << " table_end_y: " << table_end_y << " entry_x_coord: " << entry_x_coord << " entry_y_coord: " << entry_y_coord << " macros_spanned_by_entry: " << macros_spanned_by_entry << endl);
  }

  uint32_t entry_x_coord_within_macro = entry_x_coord % 16;

  PLOG_INFO("table_start_x: " << table_start_x << " table_start_y: " << table_start_y << " table_end_x: " << table_end_x << " table_end_y: " << table_end_y << " entry_x_coord: " << entry_x_coord << " entry_y_coord: " << entry_y_coord  << " x_within_macro: " << entry_x_coord_within_macro << " entry_size " << entry_size_bytes << "B"  << " macros_spanned_by_entry: " << macros_spanned_by_entry << endl);

  // entry may span multiple physical TCAMs
  uint32_t first_tcam_macro = entry_x_coord / 16;
  uint32_t dhs_first_tcam_entry_idx = first_tcam_macro * 1024 + entry_y_coord;
  uint32_t dhs_first_line = dhs_first_tcam_entry_idx;
  uint32_t byte_in_first_line = entry_x_coord % 16;
  uint32_t cur_byte = byte_in_first_line;
  uint32_t cur_dhs_line;
  uint32_t cur_byte_in_dhs_line;
  uint8_t all_ones_byte = -1;

  for (uint32_t byte = 0; byte < entry_size_bytes; byte++) {
    auto entry_byte = entry_size_bytes - byte;
    cur_dhs_line = dhs_first_line + ((cur_byte + byte) / 16) *1024;
    cur_byte_in_dhs_line = ((cur_byte + byte) % 16);
    cpp_int line_x = pict_csr.dhs_tcam_xy.entry[cur_dhs_line].x();
    cpp_int line_y = pict_csr.dhs_tcam_xy.entry[cur_dhs_line].y();
    cpp_int entry_x = all_ones_byte & hlp.get_slc(mask, (entry_byte - 1) * 8, (entry_byte * 8) - 1);
    entry_x = entry_x & hlp.get_slc(key, (entry_byte - 1) * 8, (entry_byte * 8) - 1);
    cpp_int entry_y = all_ones_byte & hlp.get_slc(mask, (entry_byte - 1) * 8, (entry_byte * 8) - 1);
    entry_y = entry_y & ~(hlp.get_slc(key, (entry_byte - 1) * 8, (entry_byte * 8) - 1));
    line_x = hlp.set_slc(line_x,
                         entry_x,
                         (16 - cur_byte_in_dhs_line - 1) * 8, (16 - cur_byte_in_dhs_line) * 8 - 1);
    line_y = hlp.set_slc(line_y,
                         entry_y,
                         (16 - cur_byte_in_dhs_line - 1) * 8, (16 - cur_byte_in_dhs_line) * 8 - 1);
    pict_csr.dhs_tcam_xy.entry[cur_dhs_line].x(line_x);
    pict_csr.dhs_tcam_xy.entry[cur_dhs_line].y(line_y);
    if (PLOG_CHECK_MSG_LEVEL(LogMsg::INFO)) {
      pict_csr.dhs_tcam_xy.entry[cur_dhs_line].show();
    }
    if ((cur_byte_in_dhs_line == 16 - 1) || (byte == entry_size_bytes - 1)) {
      pict_csr.dhs_tcam_xy.entry[cur_dhs_line].valid(1);
      pict_csr.dhs_tcam_xy.entry[cur_dhs_line].write();
      if (PLOG_CHECK_MSG_LEVEL(LogMsg::INFO)) {
        pict_csr.dhs_tcam_xy.entry[cur_dhs_line].show();
        PLOG_INFO("partial mask " << hex << (pict_csr.dhs_tcam_xy.entry[cur_dhs_line].x() | pict_csr.dhs_tcam_xy.entry[cur_dhs_line].y()) << dec << endl);
        PLOG_INFO("partial key " << hex << (pict_csr.dhs_tcam_xy.entry[cur_dhs_line].x() & (pict_csr.dhs_tcam_xy.entry[cur_dhs_line].x() | pict_csr.dhs_tcam_xy.entry[cur_dhs_line].y())) << dec << endl);
      }

      //PLOG_MSG("tcam read");
      //pict_csr.dhs_tcam_xy.entry[cur_dhs_line].read();
      //pict_csr.dhs_tcam_xy.entry[cur_dhs_line].show();
      //pict_csr.dhs_tcam_xy.entry[cur_dhs_line+1].read();
      //pict_csr.dhs_tcam_xy.entry[cur_dhs_line+1].show();

      //PLOG_MSG("cpu tcam search key and mask, line = " << cur_dhs_line << endl);
      //pict_csr.cfg_tcam_srch.key(line_x);
      //pict_csr.cfg_tcam_srch.mask(line_y);
      //pict_csr.cfg_tcam_srch.tbl_mask(0xffff);
      //pict_csr.cfg_tcam_srch.write();
      //pict_csr.cfg_tcam_srch.show();

      //pict_csr.dhs_tcam_srch.entry[0].read();

      //PLOG_MSG("cpu tcam search result");
      //pict_csr.sta_tcam_srch[0].read();
      //pict_csr.sta_tcam_srch[0].show();

      //pict_csr.dhs_tcam_srch.entry[first_tcam_macro].read();
    }
  }
  /*
  cpp_int line_x = pict_csr.dhs_tcam_xy.entry[dhs_tcam_entry_idx].x();
  cpp_int line_y = pict_csr.dhs_tcam_xy.entry[dhs_tcam_entry_idx].y();
  uint128_t all_ones = -1;
  cpp_int entry_x = hlp.get_slc(all_ones, 0, entry_size_bytes * 8 - 1);
  entry_x = entry_x & mask;
  entry_x = entry_x & key;
  cpp_int entry_y = hlp.get_slc(all_ones, 0, entry_size_bytes * 8 - 1);
  entry_y = entry_y & mask;
  entry_y = entry_y & ~key;

  int32_t entry_low_bound = 16 - (entry_x_coord_within_macro + entry_size_bytes);
  if (entry_low_bound < 0) {
    PLOG_ERR("Incorrect entry coordinate or size entry_x_coord_within_macro: " << entry_x_coord_within_macro << " entry_size_bytes: " << entry_size_bytes << endl);
  }
  int32_t entry_high_bound = 16 - entry_x_coord_within_macro;
  line_x = hlp.set_slc(line_x,
                       entry_x,
                       entry_low_bound * 8, entry_high_bound * 8 - 1);
  line_y = hlp.set_slc(line_y,
                       entry_y,
                       entry_low_bound * 8, entry_high_bound * 8 - 1);

  pict_csr.dhs_tcam_xy.entry[dhs_tcam_entry_idx].valid(1);
  pict_csr.dhs_tcam_xy.entry[dhs_tcam_entry_idx].x(line_x);
  pict_csr.dhs_tcam_xy.entry[dhs_tcam_entry_idx].y(line_y);
  pict_csr.dhs_tcam_xy.entry[dhs_tcam_entry_idx].write();
  */
  PLOG_INFO("Wrote entry " << entry_idx << " of width " << entry_size_bytes << "B for logical TCAM starting at " << table_start * 2 << " beginning in TCAM macro " << first_tcam_macro << " DHS line " << dhs_first_line << " byte " << byte_in_first_line << " and ending in TCAM macro " << cur_dhs_line / 1024 << " DHS line " << cur_dhs_line << " byte " << cur_byte_in_dhs_line << " entries_per_line " << entries_per_line << endl);
}

void cap_pict_load_logical_tcam_entry(int chip_id, int inst_id, 
                                      uint32_t table_id, 
                                      uint32_t entry_idx,
                                      cpp_int key,
                                      cpp_int mask,
                                      bool read_from_hw,
                                      block_type type) {

  cap_pict_csr_t & pict_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pict_csr_t, chip_id, inst_id);

  if (read_from_hw) {
    pict_csr.cfg_tcam_table_profile[table_id].read();
  }

  cap_pict_load_tcam_entry(chip_id, inst_id, 
                           pict_csr.cfg_tcam_table_profile[table_id].start_addr().convert_to<uint32_t>(),
                           pict_csr.cfg_tcam_table_profile[table_id].end_addr().convert_to<uint32_t>(),
                           pict_csr.cfg_tcam_table_profile[table_id].width().convert_to<uint32_t>(),
                           pict_csr.cfg_tcam_table_profile[table_id].bkts().convert_to<uint32_t>(),
                           entry_idx,
                           key,
                           mask,
                           type);
}

void cap_pict_add_logical_tcam(int chip_id, int inst_id, 
                               uint32_t tcam_id,
                               uint32_t upper_x,
                               uint32_t upper_y,
                               uint32_t lower_x,
                               uint32_t lower_y,
                               uint32_t entry_size_bytes,
                               uint32_t entries_per_line,
                               uint32_t en_tbid,
                               uint32_t tbid,
                               uint32_t keyshift,
                               block_type type) {

  cap_pict_csr_t & pict_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pict_csr_t, chip_id, inst_id);
  uint32_t num_tcams = type == SI ? 8 : 4;

  if ((lower_x < upper_x) || (lower_y <= upper_y) || ((lower_y - upper_y + 1) % 64 != 0)) {
    PLOG_ERR("Incorrect TCAM definition lower_x: " << lower_x << " lower_y: " << lower_y << " upper_x: " << upper_x << " upper_y: " << upper_y << endl);
  }
  uint32_t tcam_start = upper_y * 16 * num_tcams + upper_x;
  uint32_t tcam_end = lower_y * 16 * num_tcams + lower_x;

  // relationship between entry size, num_entries and coordinates
  if ((((upper_x % 16) + entry_size_bytes) % 16) != ((lower_x + 2) % 16)) {
    PLOG_ERR("TCAM entry width " << entry_size_bytes << " + upper_x offset in macro " << (upper_x % 16) << " should have the same offset in macro as lower_x " << (lower_x % 16) << endl);
  }

  uint32_t macros_spanned_by_table = ((upper_x % 16) + (lower_x - upper_x + 2)) / 16 + (((upper_x % 16) + (lower_x - upper_x + 2)) % 16 == 0 ? 0 : 1);
  uint32_t macros_spanned_by_entry = ((upper_x % 16) + entry_size_bytes) / 16 + (((upper_x % 16) + entry_size_bytes) % 16 == 0 ? 0 : 1);
  if (entries_per_line *  macros_spanned_by_entry != macros_spanned_by_table) {
    PLOG_ERR("Incorrect TCAM definition lower_x: " << lower_x << " lower_y: " << lower_y << " upper_x: " << upper_x << " upper_y: " << upper_y << " macros_spanned_by_table: " << macros_spanned_by_table << " macros_spanned_by_entry: " << macros_spanned_by_entry << " entries_per_line: " << entries_per_line << " entry_size_bytes: " << entry_size_bytes << endl);
  }

  pict_csr.cfg_tcam_table_profile[tcam_id].start_addr(tcam_start/2);
  pict_csr.cfg_tcam_table_profile[tcam_id].end_addr(tcam_end/2);
  pict_csr.cfg_tcam_table_profile[tcam_id].width(entry_size_bytes/2);
  pict_csr.cfg_tcam_table_profile[tcam_id].en_tbid(en_tbid);
  pict_csr.cfg_tcam_table_profile[tcam_id].tbid(tbid);
  pict_csr.cfg_tcam_table_profile[tcam_id].bkts(entries_per_line);
  pict_csr.cfg_tcam_table_profile[tcam_id].keyshift(keyshift);
  pict_csr.cfg_tcam_table_profile[tcam_id].write();
  
  PLOG_INFO("Added logical TCAM with id " << tcam_id 
            << " for entries of 2B size " 
            << pict_csr.cfg_tcam_table_profile[tcam_id].width().convert_to<uint32_t>()
            << " starting at 2B address " 
            << pict_csr.cfg_tcam_table_profile[tcam_id].start_addr().convert_to<uint32_t>()  
            << " ending at 2B address " 
            << pict_csr.cfg_tcam_table_profile[tcam_id].end_addr().convert_to<uint32_t>() 
            << " entries per line " 
            << pict_csr.cfg_tcam_table_profile[tcam_id].bkts().convert_to<uint32_t>() << endl);

}

void cap_pict_load_tcam_def_from_cfg(int chip_id, int inst_id, block_type type) {

  cap_pict_csr_t & pict_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pict_csr_t, chip_id, inst_id);

  stringstream tmp_ss;
  string hier_path_str = pict_csr.get_hier_path();
  string tmp_str;

  replace(hier_path_str.begin(), hier_path_str.end(), '.', '/');
  replace(hier_path_str.begin(), hier_path_str.end(), '[', '/');
  replace(hier_path_str.begin(), hier_path_str.end(), ']', '/');
  unique(hier_path_str.begin(), hier_path_str.end());
  if (hier_path_str[hier_path_str.length() - 1] == '/') {
    hier_path_str.pop_back();
  }
  PLOG_API_MSG("PICT[" << inst_id << "]", " current path " << hier_path_str << endl);

  tmp_ss.str("");
  tmp_ss << hier_path_str << "/num_tcams";
  tmp_str = tmp_ss.str();
  uint32_t num_tcams = sknobs_get_value((char *)tmp_str.c_str(), 0);

  for (uint32_t tcam_idx = 0; tcam_idx < num_tcams; tcam_idx++) {
    uint32_t tcam_id;
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/tcam_defs/" << tcam_idx << "/tcam_id";
    tmp_str = tmp_ss.str();
    if (!sknobs_exists((char *)tmp_str.c_str())) {
      PLOG_ERR("Tcam " << tcam_idx << " missing definition for tcam_id" << endl);
    }
    else {
      tcam_id = sknobs_get_value((char *)tmp_str.c_str(), 0);
    }
    uint32_t upper_x;
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/tcam_defs/" << tcam_idx << "/upper_x";
    tmp_str = tmp_ss.str();
    if (!sknobs_exists((char *)tmp_str.c_str())) {
      PLOG_ERR("Tcam " << tcam_idx << " missing definition for upper_x" << endl);
    }
    else {
      upper_x = sknobs_get_value((char *)tmp_str.c_str(), 0);
    }
    uint32_t upper_y;
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/tcam_defs/" << tcam_idx << "/upper_y";
    tmp_str = tmp_ss.str();
    if (!sknobs_exists((char *)tmp_str.c_str())) {
      PLOG_ERR("Tcam " << tcam_idx << " missing definition for upper_y" << endl);
    }
    else {
      upper_y = sknobs_get_value((char *)tmp_str.c_str(), 0);
    }
    uint32_t lower_x;
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/tcam_defs/" << tcam_idx << "/lower_x";
    tmp_str = tmp_ss.str();
    if (!sknobs_exists((char *)tmp_str.c_str())) {
      PLOG_ERR("Tcam " << tcam_idx << " missing definition for lower_x" << endl);
    }
    else {
      lower_x = sknobs_get_value((char *)tmp_str.c_str(), 0);
    }
    uint32_t lower_y;
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/tcam_defs/" << tcam_idx << "/lower_y";
    tmp_str = tmp_ss.str();
    if (!sknobs_exists((char *)tmp_str.c_str())) {
      PLOG_ERR("Tcam " << tcam_idx << " missing definition for lower_y" << endl);
    }
    else {
      lower_y = sknobs_get_value((char *)tmp_str.c_str(), 0);
    }
    uint32_t entry_size;
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/tcam_defs/" << tcam_idx << "/entry_size";
    tmp_str = tmp_ss.str();
    if (!sknobs_exists((char *)tmp_str.c_str())) {
      PLOG_ERR("Tcam " << tcam_idx << " missing definition for entry_size" << endl);
    }
    else {
      entry_size = sknobs_get_value((char *)tmp_str.c_str(), 0);
    }
    uint32_t entries_per_line;
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/tcam_defs/" << tcam_idx << "/entries_per_line";
    tmp_str = tmp_ss.str();
    if (!sknobs_exists((char *)tmp_str.c_str())) {
      PLOG_ERR("Tcam " << tcam_idx << " missing definition for entries_per_line" << endl);
    }
    else {
      entries_per_line = sknobs_get_value((char *)tmp_str.c_str(), 0);
    }
    uint32_t en_tbid;
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/tcam_defs/" << tcam_idx << "/en_tbid";
    tmp_str = tmp_ss.str();
    if (!sknobs_exists((char *)tmp_str.c_str())) {
      PLOG_ERR("Tcam " << tcam_idx << " missing definition for en_tbid" << endl);
    }
    else {
      en_tbid = sknobs_get_value((char *)tmp_str.c_str(), 0);
    }
    uint32_t tbid;
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/tcam_defs/" << tcam_idx << "/tbid";
    tmp_str = tmp_ss.str();
    if (!sknobs_exists((char *)tmp_str.c_str())) {
      PLOG_ERR("Tcam " << tcam_idx << " missing definition for tbid" << endl);
    }
    else {
      tbid = sknobs_get_value((char *)tmp_str.c_str(), 0);
    }
    uint32_t keyshift;
    tmp_ss.str("");
    tmp_ss << hier_path_str << "/tcam_defs/" << tcam_idx << "/keyshift";
    tmp_str = tmp_ss.str();
    if (!sknobs_exists((char *)tmp_str.c_str())) {
      PLOG_ERR("Tcam " << tcam_idx << " missing definition for keyshift" << endl);
    }
    else {
      keyshift = sknobs_get_value((char *)tmp_str.c_str(), 0);
    }
  cap_pict_add_logical_tcam(chip_id, inst_id, tcam_id, upper_x, upper_y, lower_x, lower_y, entry_size, entries_per_line, en_tbid, tbid, keyshift);

    // load entries
    uint32_t max_entries = entries_per_line * (lower_y - upper_y + 1);
    cpp_int key;
    cpp_int mask;
    for (uint32_t entry_idx = 0; entry_idx < max_entries; entry_idx++) {
      tmp_ss.str("");
      tmp_ss << hier_path_str << "/tcam_entries/" << tcam_id << "/entry_idx/" << entry_idx << "/key";
      tmp_str = tmp_ss.str();
      if (sknobs_exists((char *)tmp_str.c_str())) {
        key.assign(sknobs_get_string((char *)tmp_str.c_str(), (char *)""));
        tmp_ss.str("");
        tmp_ss << hier_path_str << "/tcam_entries/" << tcam_id << "/entry_idx/" << entry_idx << "/mask";
        tmp_str = tmp_ss.str();
        if (sknobs_exists((char *)tmp_str.c_str())) {
          mask.assign(sknobs_get_string((char *)tmp_str.c_str(), (char *)""));
          PLOG_INFO("PICT[" << inst_id << "] Loading entry " << entry_idx << " with key " << hex << key << " mask " << mask << dec << " in tcam_id " << tcam_id << endl);
          cap_pict_load_logical_tcam_entry(chip_id, inst_id, tcam_id, entry_idx, key, mask, false, type);
        }
        else {
          PLOG_ERR("Missing mask value for entry " << entry_idx << endl);
        }
      }
    }
  }
}

void cap_pict_check_progr_sanity(int chip_id, int inst_id, block_type type) {

  cap_pict_csr_t & pict_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pict_csr_t, chip_id, inst_id);
  
  PLOG_API_MSG("PICT[" << inst_id << "]", "running programming sanity\n");
  for (auto idx = 0; idx < 96; idx++) {
    pict_csr.cfg_tcam_table_profile[idx].read();
  }
  for (auto idx = 0; idx < 96; idx++) {
    if (pict_csr.cfg_tcam_table_profile[idx].end_addr().convert_to<uint32_t>() != 0) {
      uint32_t start_addr =  pict_csr.cfg_tcam_table_profile[idx].start_addr().convert_to<uint32_t>() * 2;
      uint32_t end_addr =  pict_csr.cfg_tcam_table_profile[idx].end_addr().convert_to<uint32_t>() * 2;
      uint32_t upper_x = start_addr % 128;
      uint32_t upper_y = start_addr / 128;
      uint32_t lower_x = end_addr % 128;
      uint32_t lower_y = end_addr / 128;
      if ((lower_x < upper_x) || (lower_y < upper_y)) {
        PLOG_ERR("Incorrect logic table " << idx << " definition lower_x: " << lower_x << " lower_y: " << lower_y << " upper_x: " << upper_x << " upper_y: " << upper_y << endl);
      }
      uint32_t entry_size_bytes = pict_csr.cfg_tcam_table_profile[idx].width().convert_to<uint32_t>() * 2;
      uint32_t entries_per_line = pict_csr.cfg_tcam_table_profile[idx].bkts().convert_to<uint32_t>();
      if ((lower_y - upper_y + 1) % 64 != 0) {
        PLOG_ERR("TCAM depth has to be multiple of 64, TCAM " << idx << " depth = " << (lower_y - upper_y + 1) << endl);
      }
      if (entry_size_bytes > 4 * 16) {
        PLOG_ERR("TCAM key cannot be wider than 4 TCAM macros, TCAM " << idx << " width = " << entry_size_bytes << "B" << endl);
      }

      // relationship between entry size, num_entries and coordinates
      uint32_t entry_end_offset_in_macro = (upper_x + entry_size_bytes) % 16;
      uint32_t lower_x_offset_in_macro = (lower_x + 2) % 16;
      if (entry_end_offset_in_macro != lower_x_offset_in_macro) {
        PLOG_ERR("TCAM entry width " << entry_size_bytes << " + upper_x offset in macro " << entry_end_offset_in_macro << " should have the same offset in macro as lower_x " << lower_x_offset_in_macro << endl);
      }
      uint32_t macros_spanned_by_table = ((upper_x % 16) + (lower_x - upper_x + 2)) / 16 + (((upper_x % 16) + (lower_x - upper_x + 2)) % 16 == 0 ? 0 : 1);
      uint32_t macros_spanned_by_entry = (((upper_x % 16) + entry_size_bytes) / 16) + (((((upper_x % 16) + entry_size_bytes) % 16) == 0) ? 0 : 1);
      if (macros_spanned_by_entry > 4) {
        PLOG_ERR("TCAM entry cannot span more than 4 physical TCAM macros, TCAM " << idx << " width = " << entry_size_bytes << "B" << endl);
      }

      if (entries_per_line *  macros_spanned_by_entry != macros_spanned_by_table) {
        PLOG_ERR("Incorrect TCAM definition lower_x: " << lower_x << " lower_y: " << lower_y << " upper_x: " << upper_x << " upper_y: " << upper_y << " macros_spanned_by_table: " << macros_spanned_by_table << " macros_spanned_by_entry: " << macros_spanned_by_entry << " entries_per_line: " << entries_per_line << " entry_size_bytes: " << entry_size_bytes << endl);
      }

      for (auto idx1 = idx + 1; idx1 < 96; idx1++) {
        if (pict_csr.cfg_tcam_table_profile[idx1].end_addr().convert_to<uint32_t>() != 0) {
          uint32_t start_addr1 =  pict_csr.cfg_tcam_table_profile[idx1].start_addr().convert_to<uint32_t>() * 2;
          uint32_t end_addr1 =  pict_csr.cfg_tcam_table_profile[idx1].end_addr().convert_to<uint32_t>() * 2;
          uint32_t upper1_x = start_addr1 % 128;
          uint32_t upper1_y = start_addr1 / 128;
          uint32_t lower1_x = end_addr1 % 128;
          uint32_t lower1_y = end_addr1 / 128;
          
          if ((lower1_x < upper1_x) || (lower1_y < upper1_y)) {
            PLOG_ERR("Incorrect logic table " << idx1 << " definition lower1_x: " << lower1_x << " lower1_y: " << lower1_y << " upper1_x: " << upper1_x << " upper1_y: " << upper1_y << endl);
          }

          if (((upper_y < lower1_y) && (lower_y > upper1_y) && (upper_x < lower1_x) && (lower_x > upper1_x)) ||
              ((upper1_y < lower_y) && (lower1_y > upper_y) && (upper1_x < lower_x) && (lower1_x > upper_x))) {
            // there is an overlap of the rectangles
            uint32_t x_start_offset_in_macro = upper_x % 16;
            uint32_t x_end_offset_in_macro = (lower_x + 2) % 16;
            if (x_end_offset_in_macro == 0) {
              x_end_offset_in_macro = 16;
            }
            uint32_t x1_start_offset_in_macro = upper1_x % 16;
            uint32_t x1_end_offset_in_macro = (lower1_x + 2)% 16;
            if (x1_end_offset_in_macro == 0) {
              x1_end_offset_in_macro = 16;
            }
            uint32_t entry_size_bytes1 = pict_csr.cfg_tcam_table_profile[idx1].width().convert_to<uint32_t>() * 2;
            uint32_t macros_spanned_by_entry1 = (((upper1_x % 16) + entry_size_bytes1) / 16) + (((((upper1_x % 16) + entry_size_bytes1) % 16) == 0) ? 0 : 1);

            // for each macro in which they overlap do the check
            uint32_t start_macro = upper_x / 16;
            uint32_t end_macro = (lower_x + 2) / 16;
            uint32_t start1_macro = upper1_x / 16;
            uint32_t end1_macro = (lower1_x + 2) / 16;
            uint32_t entry_start_macro = start_macro;
            for (uint32_t m = start_macro; m <= end_macro; m++) {
              if (m >= entry_start_macro + macros_spanned_by_entry) {
                entry_start_macro += macros_spanned_by_entry;
              }
              bool covers_entire_macro = (x_start_offset_in_macro + entry_size_bytes >= (m - entry_start_macro + 1) * 16) && (m != entry_start_macro);
              uint32_t entry_start_macro1 = start1_macro;
              for (uint32_t m1 = start1_macro; m1 <= end1_macro; m1++) {
                if (m1 >= entry_start_macro1 + macros_spanned_by_entry1) {
                  entry_start_macro1 += macros_spanned_by_entry1;
                }
                bool covers_entire_macro1 = (x1_start_offset_in_macro + entry_size_bytes1 >= (m1 - entry_start_macro1 + 1) * 16) && (m1 != entry_start_macro1);
                if (m == m1) {
                  if (((x_start_offset_in_macro < x1_start_offset_in_macro) && (x_end_offset_in_macro > x1_start_offset_in_macro)) ||
                      ((x1_start_offset_in_macro < x_start_offset_in_macro) && (x1_end_offset_in_macro > x_start_offset_in_macro)) ||
                      covers_entire_macro || covers_entire_macro1) {
                    
                    PLOG_ERR("Overlapping logic TCAMs " << idx << " and " << idx1
                             << " start_addr: " << start_addr << " end_addr: " << end_addr
                             << " upper_x: " << upper_x << " upper_y: " << upper_y
                             << " lower_x: " << lower_x << " lower_y: " << lower_y
                             << " start_macro: " << start_macro << " end_macro: " << end_macro
                             << " start offset in macro: " << x_start_offset_in_macro
                             << " end offset in macro: " << x_end_offset_in_macro
                             << " entry_size: " << entry_size_bytes
                             << " upper1_x: " << upper1_x << " upper1_y: " << upper1_y
                             << " lower1_x: " << lower1_x << " lower1_y: " << lower1_y
                             << " start1_macro: " << start1_macro << " end1_macro: " << end1_macro
                             << " start1 offset in macro: " << x1_start_offset_in_macro
                             << " end1 offset in macro: " << x1_end_offset_in_macro
                             << " entry_size1: " << entry_size_bytes1
                             << " in macro " << m  
                             << " covers_entire_macro " << covers_entire_macro  
                             << " covers_entire_macro1 " << covers_entire_macro1 << endl);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

void cap_pict_csr_set_hdl_path(int chip_id, int inst_id, string path) {

    cap_pict_csr_t & pict_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pict_csr_t, chip_id, inst_id);

    // id : 0 is given to csr instance.
    // id : 1 is given to memories
    pict_csr.set_csr_inst_path(0, (path + ".tcsr")); 
    PLOG_API_MSG("PICT[" << inst_id << "]", "Added backdoor path " << (path + ".tcsr") << endl);
    
    // disable zerotime access to TCAM for now
    for (auto idx = 0; idx < pict_csr.dhs_tcam_xy.get_depth_entry(); idx++) {
      pict_csr.dhs_tcam_xy.entry[idx].set_access_no_zero_time(1);
    }
}

void cap_pict_zero_init_tcam(int chip_id, int inst_id, int num_tcams) {

  cap_pict_csr_t & pict_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pict_csr_t, chip_id, inst_id);
  
  PLOG_API_MSG("PICT[" << inst_id << "]", "initializing " << num_tcams << " TCAMs to zero\n");

  for (auto idx = 0; idx < 1024 * num_tcams; idx++) {
    uint128_t all_ones = -1;
    pict_csr.dhs_tcam_xy.entry[idx].x(0);
    pict_csr.dhs_tcam_xy.entry[idx].y(all_ones);
    pict_csr.dhs_tcam_xy.entry[idx].valid(1);
    pict_csr.dhs_tcam_xy.entry[idx].write();
    pict_csr.dhs_tcam_xy.entry[idx].valid(0);
    pict_csr.dhs_tcam_xy.entry[idx].write();
  }

  if (PLOG_CHECK_MSG_LEVEL(LogMsg::INFO)) {
    PLOG_INFO("Dumping TCAMs" << endl);
    pict_csr.dhs_tcam_xy.show();
  }

  PLOG_API_MSG("PICT[" << inst_id << "]", " done initializing TCAMs to zero\n");
}

void cap_pict_bist_run(int chip_id, int inst_id, int enable) {
  cap_pict_csr_t & pict_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pict_csr_t, chip_id, inst_id);

  PLOG_API_MSG("PICT[" << inst_id << "]", ": cap_pict_bist_run, enable = " << enable << "\n");

  int mem_vec = (inst_id == 0) ? 0xff : 0xf;
  pict_csr.cfg_tcam.read();
  pict_csr.cfg_tcam.bist_run(mem_vec);
  pict_csr.cfg_tcam.write();
}

void cap_pict_bist_chk(int chip_id, int inst_id) {
  cap_pict_csr_t & pict_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pict_csr_t, chip_id, inst_id);

  PLOG_API_MSG("PICT[" << inst_id << "]", ": cap_pict_bist_chk\n");

  int loop_cnt = 0;
  do {
     pict_csr.sta_tcam.read();
  } while((loop_cnt < 10000) &&
          ((pict_csr.sta_tcam.all() == 0)));

  bool err = false;
  int mem_vec = (inst_id == 0) ? 0xff : 0xf;
  if (pict_csr.sta_tcam.bist_done_pass() != mem_vec) {
     PLOG_ERR("pict_csr.sta_tcam.bist_done_pass(): exp = 0x" << hex << mem_vec << ", rcv = 0x" << pict_csr.sta_tcam.bist_done_pass() << dec << endl);
     err = true;
  }  
  if (pict_csr.sta_tcam.bist_done_fail() != 0) {
     PLOG_ERR("pict_csr.sta_tcam.bist_done_fail(): exp = 0x0, rcv = 0x" << pict_csr.sta_tcam.bist_done_fail() << dec << endl);
     err = true;
  }

  if (!err) {
     PLOG_API_MSG("PASSED: PICT[" << inst_id << "]", ": cap_pict_bist_chk\n");
  }
}


//
// pict tcam test
//
// TCAM test for pict
//    - tsi, tse
//    - 4/8 banks
//
// fast_mode: 0 read back and check all writes
// fast_mode: 1 read back and check only one write
//            both modes check all searches
//
// test:
//    - write key/mask to banks, lines with valid=1
//    - read x/y and check
//    - read search and check hit/hit_addr
//    - reset tcam
//    - read search and check hit/hit_addr (expect no hit)
//   run throught key bus: 1<<n
//   run throught line bus: 1<<n
//
int cap_run_pict_tcam_rdwr_test(int chip_id, int pict, int fast_mode, int max_err_cnt, int verbosity) {

   cap_pict_csr_t & pict_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pict_csr_t, chip_id, pict);

   int num_line = 128;
   int fail     = 0;
   cpp_int valid = 1;
   cpp_int set_one   = 1;
   //uint128_t all_ones = -1;

   cpp_int_helper hlp;
   cpp_int mask128b("0xffffffffffffffffffffffffffffffff");

   cpp_int key;
   cpp_int line;
   cpp_int hit;
   cpp_int hit_ret;
   cpp_int exp_hit;
   int hit_addr;
   int exp_hit_addr;

   vector<int> hit_entry_q;

   int err_cnt;

   int max_bank;
   if (pict == 1) {
      max_bank = 3; // tse
   } else {
      max_bank = 7; // tsi
   }

   // addr_lp:0 increment addr line by 1
   // addr_lp:1 increment addr line by 2x
   for (int addr_lp=0; addr_lp<2; addr_lp++) {
      int MYnum_line;
      int MYnum_line_rd;
      if (addr_lp == 1) {
         PLOG_MSG("pict:" << pict << " tcam walk bits in line" << endl);
         MYnum_line = 10;
      } else {
         PLOG_MSG("pict:" << pict << " tcam walk bits in key" << endl);
         MYnum_line = num_line;
      }

      // reduce test time mode - only check 1 line
      if (fast_mode == 1) {
         MYnum_line_rd = 1;
      } else {
         MYnum_line_rd = MYnum_line;
      }
      for (int bank=0; bank<=max_bank; bank++) {
         PLOG_MSG("testing tcam pict:" << pict << " bank:" << bank << endl);
         err_cnt = 0;
         for (int kk=0; kk<MYnum_line; kk++) {
            key = 0;
            key = hlp.set_slc( key , set_one , kk % MYnum_line , kk % MYnum_line );
            line  = kk;
            if (addr_lp == 1) {
               line  = 0;
               line  = hlp.set_slc( line , set_one , kk % MYnum_line , kk % MYnum_line );
            }
            cap_pict_tcam_write(chip_id, pict, key, mask128b, bank, line, valid);
         }

         for (int kk=0; kk<MYnum_line_rd; kk++) {
            key = 0;
            key = hlp.set_slc( key , set_one , kk % MYnum_line_rd , kk % MYnum_line_rd );
            line  = kk;
            if (addr_lp == 1) {
               line  = 0;
               line  = hlp.set_slc( line , set_one , kk % MYnum_line_rd , kk % MYnum_line_rd );
            }
            int chk_fail = cap_pict_tcam_read_chk(chip_id, pict, key, mask128b, bank, line, verbosity);

            if (chk_fail) {
               err_cnt++;
               fail = 1;
               if (err_cnt >= max_err_cnt) {
                  PLOG_MSG("pict:" << pict << " bank:" << bank << " test terminates after max_err:" << err_cnt << endl);
                  break;
               }
            }
         }
   
         err_cnt = 0;
         for (int kk=0; kk<MYnum_line; kk++) {
            key  = 0;
            key  = hlp.set_slc( key , set_one , kk % MYnum_line , kk % MYnum_line );
            hit_ret  = cap_pict_tcam_search(chip_id, pict, key, mask128b, bank);
            hit      = hit_ret & 0x1;
            hit_addr = cap_pict_tcam_addr(bank, (hit_ret>>1));
            hit_entry_q.push_back(hit_addr);

            cpp_int exp_hit = 1;
            cpp_int line = kk;
            if (addr_lp == 1) {
               line  = 0;
               line  = hlp.set_slc( line , set_one , kk % MYnum_line , kk % MYnum_line );
            }
            exp_hit_addr = cap_pict_tcam_addr(bank, line);

            if (hit == exp_hit) {
               if (verbosity == 1) PLOG_MSG("pict:" << pict << " tcam hit pass key:0x" << hex << key  << " line:0x" << line << dec  << " bank:" << bank << endl);
            } else {
               PLOG_ERR("pict:" << pict << " tcam hit fail key:0x" << hex << key  << " line:0x" << line << dec  << " bank:" << bank << endl);
               err_cnt++;
               fail = 1;
               if (err_cnt >= max_err_cnt) {
                  PLOG_MSG("pict:" << pict << " bank:" << bank << " test terminates after max_err:" << err_cnt << endl);
                  break;
               }
            }

            if (hit_addr == exp_hit_addr) {
               if (verbosity == 1) PLOG_MSG("pict:" << pict << " tcam hit addr pass key:0x" << hex << key  << " line:0x" << line << dec  << " bank:" << bank << " hit_addr:0x" << hit_addr << endl);
            } else {
               PLOG_ERR("pict:" << pict << " tcam hit addr fail key:0x" << hex << key  << " line:0x" << line << dec  << " bank:" << bank << " hit_addr:0x" << hit_addr << " exp:0x" << exp_hit_addr << endl);
               err_cnt++;
               fail = 1;
               if (err_cnt >= max_err_cnt) {
                  PLOG_MSG("pict:" << pict << " bank:" << bank << " test terminates after max_err:" << err_cnt << endl);
                  break;
               }
            }
         }

         PLOG_MSG("reset tcam pict:" << pict << " bank:" << bank << endl);
         //cap_pict_tcam_reset(chip_id, pict, bank);
         for (auto aa: hit_entry_q) {
            int rst_entry = aa; 
            pict_csr.dhs_tcam_xy.entry[rst_entry].valid(0);
            pict_csr.dhs_tcam_xy.entry[rst_entry].write();
         }
         hit_entry_q.erase(hit_entry_q.begin(), hit_entry_q.end());

      
         err_cnt = 0;
         for (int kk=0; kk<MYnum_line; kk++) {
            key  = 0;
            key  = hlp.set_slc( key , set_one , kk % MYnum_line , kk % MYnum_line );
            cpp_int hit_ret = cap_pict_tcam_search(chip_id, pict, key, mask128b, bank);
            cpp_int exp_hit = 0;

            if (hit_ret == exp_hit) {
               if (verbosity == 1) PLOG_MSG("pict:" << pict << " tcam no hit after reset pass key:0x" << hex << key  << dec  << " bank:" << bank << endl);
            } else {
               PLOG_ERR("pict:" << pict << " expect tcam no hit after reset fail key:0x" << hex << key  << dec  << " bank:" << bank << endl);
               err_cnt++;
               fail = 1;
               if (err_cnt >= max_err_cnt) {
                  PLOG_MSG("pict:" << pict << " bank:" << bank << " test terminates after max_err:" << err_cnt << endl);
                  break;
               }
            }
         }
      }
   }

   if (fail == 0) {
      PLOG_MSG("=== pict:" << pict << " tcam pass" << endl);
   } else {
      PLOG_ERR("=== pict:" << pict << " tcam fail" << endl);
   }

   return fail;

}

cpp_int cap_pict_key_xy(cpp_int key, cpp_int mask) {
   cpp_int ret_val =  key & mask;
   //LOG_MSG("key_xy: 0x" << ret_val << endl);
   return ret_val;
}

cpp_int cap_pict_mask_xy(cpp_int key, cpp_int mask) {
   
   //uint128_t all_ones = -1;
   cpp_int mask128b("0xffffffffffffffffffffffffffffffff");
   cpp_int ret_val = (mask128b ^ key) & mask;
   //LOG_MSG("mask_xy: 0x" << ret_val << endl);
   return ret_val;
}

int cap_pict_tcam_addr( int bank, cpp_int line) {
   return (1024*bank + line.convert_to<uint32_t>());
}

// pict=0 tsi
// pict=1 tse
void cap_pict_tcam_reset(int chip_id, int pict, int bank) {

   cap_pict_csr_t & pict_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pict_csr_t, chip_id, pict);

   for (int line=0; line<1024; line++) {
      int addr = cap_pict_tcam_addr(bank, line);
      pict_csr.dhs_tcam_xy.entry[addr].valid(0);
      pict_csr.dhs_tcam_xy.entry[addr].write();
   }

   //bug pict_csr.cfg_tcam_reset.enable(0);
   //bug pict_csr.cfg_tcam_reset.vec(0xff);
   //bug pict_csr.cfg_tcam_reset.write();
   //bug //pict_csr.cfg_tcam_reset.show();
   //bug SLEEP(10);

   //bug pict_csr.cfg_tcam_reset.enable(1);
   //bug pict_csr.cfg_tcam_reset.vec(0xff);
   //bug pict_csr.cfg_tcam_reset.write();
   //bug //pict_csr.cfg_tcam_reset.show();
   //bug SLEEP(10);

   //bug pict_csr.cfg_tcam_reset.enable(0);
   //bug pict_csr.cfg_tcam_reset.vec(0xff);
   //bug pict_csr.cfg_tcam_reset.write();
   //bug //pict_csr.cfg_tcam_reset.show();
}

void cap_pict_tcam_write(int chip_id, int pict, cpp_int key, cpp_int mask, int bank, cpp_int line, cpp_int valid)  {

   cap_pict_csr_t & pict_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pict_csr_t, chip_id, pict);

   int addr = cap_pict_tcam_addr(bank, line);

   pict_csr.dhs_tcam_xy.entry[addr].x( cap_pict_key_xy(key, mask) );    
   pict_csr.dhs_tcam_xy.entry[addr].y( cap_pict_mask_xy(key, mask) );
   pict_csr.dhs_tcam_xy.entry[addr].valid(valid);
   pict_csr.dhs_tcam_xy.entry[addr].write();
}

int cap_pict_tcam_read_chk(int chip_id, int pict, cpp_int key, cpp_int mask, int bank, cpp_int line, int verbosity)  {

   cap_pict_csr_t & pict_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pict_csr_t, chip_id, pict);

   int addr = cap_pict_tcam_addr(bank, line);
   int fail = 0;

    
   pict_csr.dhs_tcam_xy.entry[addr].read();
   cpp_int rd_x  = pict_csr.dhs_tcam_xy.entry[addr].x();
   cpp_int rd_y  = pict_csr.dhs_tcam_xy.entry[addr].y();
   cpp_int exp_x = cap_pict_key_xy(key, mask);   
   cpp_int exp_y = cap_pict_mask_xy(key, mask);

   if (rd_x == exp_x) {
      if (verbosity == 1) PLOG_MSG("pict:" << pict << " tcam pass bank:" << bank << " line:0x" << hex << line << " x:0x" << rd_x << dec << endl);
   } else {
      PLOG_ERR("pict:" << pict << " tcam fail bank:" << bank << " line:0x" << hex << line << " x rd:0x" << rd_x << " exp:0x" << exp_x << dec << endl);
      fail = 1;
   }

   if (rd_y == exp_y) {
      if (verbosity == 1) PLOG_MSG("pict:" << pict << " tcam pass bank:" << bank << " line:0x" << hex << line << " y:0x" << rd_y << dec << endl);
   } else {
      PLOG_ERR("pict:" << pict << " tcam fail bank:" << bank << " line:0x" << hex << line << " y rd:0x" << rd_y << " exp:0x" << exp_y << dec << endl);
      fail = 1;
   }
   return(fail);
}

void cap_pict_tcam_read(int chip_id, int pict, int bank, cpp_int line)  {

   cap_pict_csr_t & pict_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pict_csr_t, chip_id, pict);

   int addr = cap_pict_tcam_addr(bank, line);

   pict_csr.dhs_tcam_xy.entry[addr].read();
}

cpp_int cap_pict_tcam_search(int chip_id, int pict, cpp_int key, cpp_int mask, int bank)  {

   cap_pict_csr_t & pict_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pict_csr_t, chip_id, pict);

   // search
   cpp_int tbl_mask = 0xffff;
   cpp_int hit      = 0;
   cpp_int hit_addr = 0;

   pict_csr.cfg_tcam_srch.key( cap_pict_key_xy(key, mask) );
   pict_csr.cfg_tcam_srch.mask( cap_pict_mask_xy(key, mask) );
   pict_csr.cfg_tcam_srch.tbl_mask( tbl_mask );
   pict_csr.cfg_tcam_srch.write();

   // Trigger cpu tcam search
   pict_csr.dhs_tcam_srch.entry[bank].read();
   
   // Show tcam search result
   pict_csr.sta_tcam_srch[bank].read();
   hit       = pict_csr.sta_tcam_srch[bank].hit();
   hit_addr  = pict_csr.sta_tcam_srch[bank].hit_addr();

   return (hit_addr << 1) | hit; 

}   
