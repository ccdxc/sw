#include "model_sim/include/sknobs.h"
#include "include/sdk/types.hpp"
#include "lib/catalog/catalog.hpp"
#include "include/sdk/asic/capri/cap_mx_api.h"
#include "cap_bx_api.h"
#include "msg_man.h"

mac_profile_t bx[MAX_MAC];
#if 0
// TBD-ELBA-REBASE
void cap_bx_soft_reset(int chip_id, int inst_id) {
   cap_bx_set_soft_reset(chip_id, inst_id, 0x1);
}

int cap_bx_apb_read(int chip_id, int inst_id, int addr) {
 cap_bx_csr_t & bx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_bx_csr_t, chip_id, inst_id);
    bx_csr.dhs_apb.entry[addr].read();
    int rdata = bx_csr.dhs_apb.entry[addr].data().convert_to<int>();
    return rdata;
}
#endif

void cap_bx_apb_write(int chip_id, int inst_id, int addr, int data) {
#if 0
// TBD-ELBA-REBASE
// TBD-ELBA-REBASE
 cap_bx_csr_t & bx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_bx_csr_t, chip_id, inst_id);
    bx_csr.dhs_apb.entry[addr].data(data);
    bx_csr.dhs_apb.entry[addr].write();
    // bx_csr.dhs_apb.entry[addr].show();
#endif
}

#if 0
// TBD-ELBA-REBASE
// TBD-ELBA-REBASE
// reads addr, set/reset bit and writes back
static int
cap_bx_set_bit (int chip_id, int inst_id, int addr, int bit, bool set)
{
   int data = cap_bx_apb_read(chip_id, inst_id, addr);

   if (set) {
       data = data | (1 << bit);
   } else {
       data = data & ~(1 << bit);
   }
   cap_bx_apb_write(chip_id, inst_id, addr, data);
   return 0;
}

static inline uint32_t
set_bits(uint32_t data, int pos, int num_bits, uint32_t value)
{
    uint32_t mask = 0;

    for (int i = pos; i < pos + num_bits; ++i) {
        mask |= (1 << i);
    }

    // clear the bits
    data = data & ~mask;

    // set the bits
    data = data | (value << pos);

    return data;
}

static int
cap_bx_set_bits(int chip_id, int inst_id, uint32_t addr,
                int pos, int num_bits, uint32_t value)
{
    uint32_t data = cap_bx_apb_read(chip_id, inst_id, addr);

    data = set_bits(data, pos, num_bits, value);
    cap_bx_apb_write(chip_id, inst_id, addr, data);
    return 0;
}
#endif

void cap_bx_set_ch_enable(int chip_id, int inst_id, int value) { 
#if 0
// TBD-ELBA-REBASE
    int rdata = cap_bx_apb_read(chip_id, inst_id, 0x1f10);
    int wdata = (rdata & 0xfe) | (value & 0x1);
    cap_bx_apb_write(chip_id, inst_id, 0x1f10, wdata);
#endif
}

void cap_bx_set_glbl_mode(int chip_id, int inst_id, int value) { 
#if 0
// TBD-ELBA-REBASE
    int rdata = cap_bx_apb_read(chip_id, inst_id, 0x1f10);
    int wdata = (rdata & 0x3) | ((value & 0xf) << 2);
    cap_bx_csr_t & bx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_bx_csr_t, chip_id, inst_id);

    cap_bx_apb_write(chip_id, inst_id, 0x1f10, wdata);

    bx_csr.cfg_fixer.read();
    // set the fixer timeout to 1024
    bx_csr.cfg_fixer.timeout(1024);
    bx_csr.cfg_fixer.write();
#endif
}


#if 0
// TBD-ELBA-REBASE
void cap_bx_set_tx_rx_enable(int chip_id, int inst_id, int value) {
    uint32_t addr = 0x2100;
    int pos = 0;
    int num_bits = 2;

    cap_bx_set_bits(chip_id, inst_id, addr, pos, num_bits, value);
}
#endif

void cap_bx_set_soft_reset(int chip_id, int inst_id, int value) { 
#if 0
// TBD-ELBA-REBASE
    // Software reset for stats
    cap_bx_apb_write(chip_id, inst_id, 0x2006, value);

    // umac software reset
    int rdata = cap_bx_apb_read(chip_id, inst_id, 0x1f10);
    int wdata = (rdata & 0xfd) | ((value & 0x1) << 1);
    cap_bx_apb_write(chip_id, inst_id, 0x1f10, wdata);
#endif
}

void cap_bx_set_mtu(int chip_id , int inst_id, int max_value, int jabber_value) {
#if 0
// TBD-ELBA-REBASE
   cap_bx_apb_write(chip_id, inst_id, 0x2103, max_value);
   cap_bx_apb_write(chip_id, inst_id, 0x2104, jabber_value);
   cap_bx_apb_write(chip_id, inst_id, 0x2105, jabber_value);
#endif
}

#if 0
// TBD-ELBA-REBASE
void cap_bx_set_pause(int chip_id , int inst_id, int pri_vec, int legacy) {
 cap_bx_csr_t & bx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_bx_csr_t, chip_id, inst_id);

   int txfcxoff_enable = 0;
   int txpfcxoff_enable = 0;
   int addr1 = 0x2101;
   int addr2 = 0x2102;
   int addr3 = 0x2106;

   if (pri_vec == 0) {
      cap_bx_apb_write(chip_id, inst_id, addr1, 0x030);
      cap_bx_apb_write(chip_id, inst_id, addr2, 0x010);  // bit4 Promiscuous Mode (1: disable MAC address check)
      cap_bx_apb_write(chip_id, inst_id, addr3, 0x00);   // MAC Tx Priority Pause Vector
   } else {
      if (legacy) {
         txfcxoff_enable = 1;
         cap_bx_apb_write(chip_id, inst_id, addr1, 0x130);   // Bit8: Control Frame Generation Enable
         cap_bx_apb_write(chip_id, inst_id, addr2, 0x130);   // bit5: Enable Rx Flow Control Decode, bit8 filter pause frame
         cap_bx_apb_write(chip_id, inst_id, addr3, 0x00);    // MAC Tx Priority Pause Vector
      } else {
         txpfcxoff_enable = 0xff;
         cap_bx_apb_write(chip_id, inst_id, addr1, 0x230);   // bit9: Priority Flow Control Generation Enable
         cap_bx_apb_write(chip_id, inst_id, addr2, 0x130);   // bit5: Enable Rx Flow Control Decode, bit8 filter pause frame
         cap_bx_apb_write(chip_id, inst_id, addr3, pri_vec); // MAC Tx Priority Pause Vector
      }
   }

   bx_csr.cfg_mac_xoff.ff_txfcxoff_i(txfcxoff_enable);
   bx_csr.cfg_mac_xoff.ff_txpfcxoff_i(txpfcxoff_enable);
   bx_csr.cfg_mac_xoff.write();
   // bx_csr.cfg_mac_xoff.show();
}

void cap_bx_init_start(int chip_id, int inst_id) {
 PLOG_MSG("inside init start\n");

 cap_bx_csr_t & bx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_bx_csr_t, chip_id, inst_id);

 cap_bx_set_ch_enable(chip_id, inst_id, 0x0);

 PLOG_MSG("Set BX fixer timeout to 1024\n");
 bx_csr.cfg_fixer.timeout(1024);
 bx_csr.cfg_fixer.write();
}

void cap_bx_init_done(int chip_id, int inst_id) {
 PLOG_MSG("inside init done\n");
}

void cap_bx_show_mac_mode(int chip_id, int inst_id) {
   string mode_path = "cap" + to_string(chip_id) + "/top/mac_cfg/bx" + to_string(inst_id) + "/mode";
   string mode = sknobs_get_string((char *)mode_path.c_str(), (char *)"mode_rsvd");
   PLOG_MSG( "cap_bx_show_mac_mode: " << mode_path << " = " << mode << "\n");
}

void cap_bx_set_mac_mode(int chip_id, int inst_id, string new_mode) {
   string mode_path = "cap" + to_string(chip_id) + "/top/mac_cfg/bx" + to_string(inst_id) + "/mode";
   sknobs_set_string((char *)mode_path.c_str(), (char *)new_mode.c_str());

   string mode = sknobs_get_string((char *)mode_path.c_str(), (char *)"mode_rsvd");
   PLOG_MSG( "cap_bx_set_mac_mode: Change " << mode_path << " to " << mode << "\n");
}

int cap_bx_get_port_enable_state(int chip_id, int inst_id, int port) {
   string enable_path = "cap" + to_string(chip_id) + "/top/mac_cfg/bx" + to_string(inst_id) + "/" + to_string(port) + "/enable" ;
   if (sknobs_exists((char *) enable_path.c_str())) {
      int enable = sknobs_get_value((char *)enable_path.c_str(), 0);
      return enable;
   } else {
      PLOG_MSG( "cap_bx_get_port_enable_state: cannot find sknobs path " << enable_path << "\n");
      return 0;
   }
}

int cap_bx_get_mac_detail(int chip_id, int inst_id, string field) {
   string mode_path = "cap" + to_string(chip_id) + "/top/mac_cfg/bx" + to_string(inst_id) + "/mode";
   if (sknobs_exists((char *) mode_path.c_str())) {
      string mode = sknobs_get_string((char *)mode_path.c_str(), (char *)"mode_rsvd");
      string field_path = "bx_prog/" + mode + "/" + field;
      int field_value = SKNOBS_GET((char *)field_path.c_str(), 0);
      PLOG_MSG( "cap_bx_get_mac_detail: " << field_path << " = " << field_value << "\n");
      return field_value;
   } else {
      PLOG_MSG( "cap_bx_get_mac_detail: cannot find sknobs path " << mode_path << "\n");
      return 0;
   }
}

void cap_bx_load_from_cfg(int chip_id, int inst_id) {
   struct mac_profile {
      string mode_name;
      int glbl_mode;
      int ch_mode[1];
      int speed[1];
      int enable[1];
   };
   mac_profile mac;

   PLOG_MSG( "cap_bx_load_from_cfg: cap" << chip_id << "/bx" << inst_id << ":\n");

   string mode_path = "cap" + to_string(chip_id) + "/top/mac_cfg/bx" + to_string(inst_id) + "/mode";

   mac.mode_name = sknobs_get_string((char *)mode_path.c_str(), (char *)"mode_rsvd");
   PLOG_MSG( "cap" << chip_id << "/top/mac_cfg/bx" << inst_id << "/mode = " << mac.mode_name << "\n");

   mac.glbl_mode = cap_bx_get_mac_detail(chip_id, inst_id, "glbl_mode");
   PLOG_MSG( "cap" << chip_id << "/top/mac_cfg/bx" << inst_id << "/glbl_mode = " << mac.glbl_mode << "\n");

   int enable_vec = 0;
   for (int i = 0; i >= 0; i--) {
      mac.ch_mode[i] = cap_bx_get_mac_detail(chip_id, inst_id, to_string(i) + "/ch_mode");
      mac.speed[i] = cap_bx_get_mac_detail(chip_id, inst_id, to_string(i) + "/speed");
      mac.enable[i] = cap_bx_get_port_enable_state(chip_id, inst_id, i) & 0x1;

      enable_vec = enable_vec << 1;
      enable_vec = enable_vec | mac.enable[i];
   }

   // 
   // Comira MAC Configurations
   // 
   // global mode
   cap_bx_set_glbl_mode(chip_id, inst_id, mac.glbl_mode);

   // MAC Rx Configuration: bit4: Promiscuous Mode (1: disable MAC address check)
   cap_bx_apb_write(chip_id, inst_id, 0x2102, 0x10);

   // FIFO Control 1: 16'b0_000010_01000_0100;
   cap_bx_apb_write(chip_id, inst_id, 0x3f01, 0x484);

   // Rx and Tx Enable, Channel Mode, MTU, mask rx len error
   if (mac.enable[0] == 1) {
      cap_bx_set_tx_rx_enable(chip_id, inst_id, 0x3);
      cap_bx_apb_write(chip_id, inst_id, 0x4010, mac.ch_mode[0]);
      cap_bx_set_mtu(chip_id, inst_id, 9216, 9217);
   } else {
      cap_bx_set_tx_rx_enable(chip_id, inst_id, 0x0);
   }

   cap_bx_set_ch_enable(chip_id, inst_id, enable_vec);

   int reset_vec = (enable_vec ^ 0x1) & 0x1;
   cap_bx_set_soft_reset(chip_id, inst_id, reset_vec);

   if (sknobs_exists((char*)"enable_legacy_pause")) {
       cap_bx_set_pause(chip_id, inst_id, 0x1, 1);
   }
   else if (sknobs_exists((char*)"enable_pause")) {
       cap_bx_set_pause(chip_id, inst_id, 0xff, 0);
   }
}

void cap_bx_eos(int chip_id, int inst_id) {
   cap_bx_eos_cnt(chip_id,inst_id);
   cap_bx_eos_int(chip_id,inst_id);
   cap_bx_eos_sta(chip_id,inst_id);
}
#endif

void cap_bx_mac_stat(int chip_id, int inst_id, int port, int short_report,
                     uint64_t *stats_data) {
#if 0
// TBD-ELBA-REBASE
 cap_bx_csr_t & bx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_bx_csr_t, chip_id, inst_id);

   string stats[64] = {
   "A", "Frames Received OK",
   "A", "Frames Received All (Good/Bad Frames)",
   "A", "Frames Received with Bad FCS",
   "A", "Frames with any bad (CRC, Length, Align)",
   "A", "Octets Received in Good Frames",
   "A", "Octets Received (Good/Bad Frames)",
   "L", "Frames Received with Unicast Address",
   "L", "Frames Received with Multicast Address",
   "L", "Frames Received with Broadcast Address",
   "L", "Frames Received of type PAUSE",
   "L", "Frames Received with Bad Length",
   "L", "Frames Received Undersized",
   "L", "Frames Received Oversized",
   "L", "Fragments Received",
   "L", "Jabber Received",
   "L", "Frames Received Length=64",
   "L", "Frames Received Length=65~127",
   "L", "Frames Received Length=128~255",
   "L", "Frames Received Length=256~511",
   "L", "Frames Received Length=512~1023",
   "L", "Frames Received Length=1024~1518",
   "L", "Frames Received Length>=1518",
   "A", "Frames RX FIFO Full",
   "A", "Frames Transmitted OK",
   "A", "Frames Transmitted All (Good/Bad Frames)",
   "A", "Frames Transmitted Bad",
   "A", "Octets Transmitted Good",
   "A", "Octets Transmitted Total (Good/Bad)",
   "L", "Frames Transmitted with Unicast Address",
   "L", "Frames Transmitted with Multicast Address",
   "L", "Frames Transmitted with Broadcast Address",
   "L", "Frames Transmitted of type PAUSE"
   };
  
   for (int i = 0; i < 32; i += 1) {
      string report_type = stats[i*2];
      if (short_report == 1 && (report_type.compare("L") == 0)) continue;

      int addr = ((port&0x3) << 7) | i;
      bx_csr.dhs_mac_stats.entry[addr].read();
      cpp_int rdata = bx_csr.dhs_mac_stats.entry[addr].value();
      if (stats_data != NULL) {
          stats_data[i] = bx_csr.dhs_mac_stats.entry[addr].value().convert_to<uint64_t>();
      }
      string counter_name = stats[i*2+1];
      PLOG_MSG( "BX: " << counter_name.append(50-counter_name.length(),' ') << " : " << rdata << "\n" );
   }
#endif
}

#if 0
// TBD-ELBA-REBASE
void cap_bx_eos_cnt(int chip_id, int inst_id) {
   cap_bx_mac_stat(chip_id,inst_id, 0, 1, NULL);  // port 0
}

void cap_bx_eos_int(int chip_id, int inst_id) {
}

void cap_bx_eos_sta(int chip_id, int inst_id) {
}

void cap_bx_set_debug_ctl(int chip_id, int inst_id, int enable, int select) {

 PLOG_MSG("Set BX debug enable/select\n");
 cap_bx_csr_t & bx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_bx_csr_t, chip_id, inst_id);

 bx_csr.cfg_debug_port.read();
 bx_csr.cfg_debug_port.enable(enable);
 bx_csr.cfg_debug_port.select(select);
 bx_csr.cfg_debug_port.write();
}

void cap_bx_set_an_ability(int chip_id, int inst_id, int value) {
 PLOG_MSG ("BX: set an ability" << std::endl);
   cap_bx_apb_write(chip_id, inst_id, 0x4c01, value);
}

int cap_bx_rd_rx_an_ability(int chip_id, int inst_id) {
   PLOG_MSG("BX: read rx an_ability " << endl);
   int addr = 0x4c02;
   int value = cap_bx_apb_read(chip_id, inst_id, addr);
   return value;
}

void cap_bx_start_an(int chip_id, int inst_id) {
   PLOG_MSG ("BX: start an" << std::endl);
   cap_bx_apb_write(chip_id, inst_id, 0x4c00, 0x3);
}

void cap_bx_stop_an(int chip_id, int inst_id) {
   PLOG_MSG ("BX: stop an" << std::endl);
   cap_bx_apb_write(chip_id, inst_id, 0x4c00, 0x0);
}

void cap_bx_wait_an_done(int chip_id, int inst_id) {
   PLOG_MSG ("BX: wait an done" << std::endl);
   int data;
   do {
     data = cap_bx_apb_read(chip_id, inst_id, 0x4c06);
   } while ((data & 0x1) == 0);
}

void cap_bx_set_an_link_timer(int chip_id, int inst_id, int value) {
   PLOG_MSG("BX: set an_ability " << value << endl);
   int addr = 0x4c03;
   cap_bx_apb_write(chip_id, inst_id, addr, value & 0xffff);
   addr = 0x4c13;
   cap_bx_apb_write(chip_id, inst_id, addr, (value >> 16));
}

int cap_bx_check_tx_idle(int chip_id, int inst_id) {
 cap_bx_csr_t & bx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_bx_csr_t, chip_id, inst_id);
   bx_csr.sta_mac.read();
   bool tx_idle;
   tx_idle = (bx_csr.sta_mac.ff_txidle_o().convert_to<int>() == 1);

   return tx_idle ? 1 : 0;
}
#endif

int cap_bx_check_sync(int chip_id, int inst_id) {
#if 0
// TBD-ELBA-REBASE
 cap_bx_csr_t & bx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_bx_csr_t, chip_id, inst_id);
   bx_csr.sta_mac.read();
   bool rx_sync;
   rx_sync = (bx_csr.sta_mac.ff_rxsync_o().convert_to<int>() == 1);
   return rx_sync ? 1 : 0;
#else
   return 1;
#endif
}

#if 0
// TBD-ELBA-REBASE
void cap_bx_set_tx_rx_enable(int chip_id, int inst_id, int tx_enable, int rx_enable) {

   PLOG_MSG("bx" << inst_id << "Set tx_enable = " << tx_enable << ", rx_enable = " << rx_enable << endl);
   int addr = 0x2100;
   int wdata = (rx_enable & 0x1) << 1 | (tx_enable & 0x1);
   cap_bx_apb_write(chip_id, inst_id, addr, wdata);
}

void cap_bx_bist_run(int chip_id, int inst_id, int enable) {
  cap_bx_csr_t & bx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_bx_csr_t, chip_id, inst_id);

  PLOG_API_MSG("BX[" << inst_id << "]", ": cap_bx_bist_run, enable = " << enable << "\n");

  bx_csr.cfg_ff_txfifo.read();
  bx_csr.cfg_ff_txfifo.bist_run(enable);
  bx_csr.cfg_ff_txfifo.write();
  bx_csr.cfg_ff_rxfifo.read();
  bx_csr.cfg_ff_rxfifo.bist_run(enable);
  bx_csr.cfg_ff_rxfifo.write();
  bx_csr.cfg_stats_mem.read();
  bx_csr.cfg_stats_mem.bist_run(enable);
  bx_csr.cfg_stats_mem.write();
}

void cap_bx_bist_chk(int chip_id, int inst_id) {
  cap_bx_csr_t & bx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_bx_csr_t, chip_id, inst_id);

  PLOG_API_MSG("BX[" << inst_id << "]", ": cap_bx_bist_chk\n");

  int loop_cnt = 0;
  do {
     bx_csr.sta_ff_txfifo.read();
     bx_csr.sta_ff_rxfifo.read();
     bx_csr.sta_stats_mem.read();
  } while((loop_cnt < 10000) &&
          ((bx_csr.sta_ff_txfifo.all() == 0) ||
           (bx_csr.sta_ff_rxfifo.all() == 0) ||
           (bx_csr.sta_stats_mem.all() == 0)));

  bool err = false;
  if (bx_csr.sta_ff_txfifo.bist_done_pass() != 0x1) {
    PLOG_ERR("bx_csr.sta_ff_txfifo.bist_done_pass(): exp = 0x1, rcv = 0x" << bx_csr.sta_ff_txfifo.bist_done_pass() << dec << endl);
    err = true;
  }
  if (bx_csr.sta_ff_txfifo.bist_done_fail() != 0x0) {
    PLOG_ERR("bx_csr.sta_ff_txfifo.bist_done_fail(): exp = 0x0, rcv = 0x" << bx_csr.sta_ff_txfifo.bist_done_fail() << dec << endl);
    err = true;
  }
  if (bx_csr.sta_ff_rxfifo.bist_done_pass() != 0x1) {
    PLOG_ERR("bx_csr.sta_ff_rxfifo.bist_done_pass(): exp = 0x1, rcv = 0x" << bx_csr.sta_ff_rxfifo.bist_done_pass() << dec << endl);
    err = true;
  }
  if (bx_csr.sta_ff_rxfifo.bist_done_fail() != 0x0) {
    PLOG_ERR("bx_csr.sta_ff_rxfifo.bist_done_fail(): exp = 0x0, rcv = 0x" << bx_csr.sta_ff_rxfifo.bist_done_fail() << dec << endl);
    err = true;
  }
  if (bx_csr.sta_stats_mem.bist_done_pass() != 0x1) {
    PLOG_ERR("bx_csr.sta_stats_mem.bist_done_pass(): exp = 0x1, rcv = 0x" << bx_csr.sta_stats_mem.bist_done_pass() << dec << endl);
    err = true;
  }
  if (bx_csr.sta_stats_mem.bist_done_fail() != 0x0) {
    PLOG_ERR("bx_csr.sta_stats_mem.bist_done_fail(): exp = 0x0, rcv = 0x" << bx_csr.sta_stats_mem.bist_done_fail() << dec << endl);
    err = true;
  }
  if (!err) {
     PLOG_API_MSG("PASSED: BX[" << inst_id << "]", ": cap_bx_bist_chk\n");
  }
}
#endif

int
cap_bx_tx_drain (int chip_id, int inst_id, int mac_ch, bool drain)
{
#if 0
// TBD-ELBA-REBASE
    int addr = 0;

    // MAC control bit 5
    addr = 0x2100;
    cap_bx_set_bit(chip_id, inst_id, addr, 5, drain);
    cap_bx_apb_read(chip_id, inst_id, addr);

    cap_bx_set_bit(chip_id, inst_id, addr, 0, drain == true? false : true);
    cap_bx_apb_read(chip_id, inst_id, addr);

    cap_bx_set_bit(chip_id, inst_id, addr, 1, drain == true? false : true);
    cap_bx_apb_read(chip_id, inst_id, addr);

    // MAC transmit config  bit 15
    addr = 0x2101;
    cap_bx_set_bit(chip_id, inst_id, addr, 15, drain);
    cap_bx_apb_read(chip_id, inst_id, addr);

    // MAC txdebug config  bit 6
    addr = 0x2191;
    cap_bx_set_bit(chip_id, inst_id, addr, 6, drain);
    cap_bx_apb_read(chip_id, inst_id, addr);
#endif
    return 0;
}
