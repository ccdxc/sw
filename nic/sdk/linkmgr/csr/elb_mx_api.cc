// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include <cstdint>
#include "include/sdk/types.hpp"
#include "lib/catalog/catalog.hpp"
#include "include/sdk/asic/capri/cap_mx_api.h"
#include "third-party/asic/capri/model/cap_mx/cap_mx_csr.h"
#include "third-party/asic/capri/model/cap_bx/cap_bx_csr.h"
#include "cap_blk_reg_model.h"

mac_profile_t mx[MAX_MAC];

#if 0
// TBD-ELBA-REBASE

// reads addr, set/reset bit and writes back
static int
cap_mx_set_bit (int chip_id, int inst_id, int addr, int bit, bool set)
{
   int data = cap_mx_apb_read(chip_id, inst_id, addr);

   if (set) {
       data = data | (1 << bit);
   } else {
       data = data & ~(1 << bit);
   }
   cap_mx_apb_write(chip_id, inst_id, addr, data);
   return 0;
}

int
cap_mx_serdes_lpbk_get (int chip_id, int inst_id, int ch)
{
    int addr = 0x1901;
    int data = cap_mx_apb_read(chip_id, inst_id, addr);
    return (data & (1 << ch)) >> ch;
}

void cap_mx_set_ch_enable(int chip_id, int inst_id, int value) { 
    // channel enable: {ch3, ch2, ch1, ch0}
    cap_mx_apb_write(chip_id, inst_id, 0x4, value);
}

// MX Port# to MX TDM Slot Mapping
void cap_mx_set_cfg_mac_tdm(int chip_id, int inst_id, int slot0, int slot1, int slot2, int slot3) { 
 cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
   mx_csr.cfg_mac_tdm.slot0(slot0);
   mx_csr.cfg_mac_tdm.slot1(slot1);
   mx_csr.cfg_mac_tdm.slot2(slot2);
   mx_csr.cfg_mac_tdm.slot3(slot3);
   if (mx[inst_id].low_power_mode) {
      mx_csr.cfg_mac_tdm.slot0(0);
      mx_csr.cfg_mac_tdm.slot1(0);
      mx_csr.cfg_mac_tdm.slot2(0);
      mx_csr.cfg_mac_tdm.slot3(0);
   }
   mx_csr.cfg_mac_tdm.write();
   // mx_csr.cfg_mac_tdm.show();
}

void cap_mx_set_mtu_jabber(int chip_id , int inst_id, int ch, int max_value, int jabber_value) {
   if (ch == 0) {
      cap_mx_apb_write(chip_id, inst_id, 0x403, max_value);
      cap_mx_apb_write(chip_id, inst_id, 0x404, jabber_value);
      cap_mx_apb_write(chip_id, inst_id, 0x405, jabber_value);
   } else if (ch == 1) {
      cap_mx_apb_write(chip_id, inst_id, 0x503, max_value);
      cap_mx_apb_write(chip_id, inst_id, 0x504, jabber_value);
      cap_mx_apb_write(chip_id, inst_id, 0x505, jabber_value);
   } else if (ch == 2) {
      cap_mx_apb_write(chip_id, inst_id, 0x603, max_value);
      cap_mx_apb_write(chip_id, inst_id, 0x604, jabber_value);
      cap_mx_apb_write(chip_id, inst_id, 0x605, jabber_value);
   } else if (ch == 3) {
      cap_mx_apb_write(chip_id, inst_id, 0x703, max_value);
      cap_mx_apb_write(chip_id, inst_id, 0x704, jabber_value);
      cap_mx_apb_write(chip_id, inst_id, 0x705, jabber_value);
   }
}

int cap_mx_port_to_ch_mapping(int chip_id, int inst_id, int port) {
   mac_mode_t mode = mx[inst_id].mac_mode;

   if (mode == MAC_MODE_1x100g) {
      return 0;
   } else if (mode == MAC_MODE_1x40g || mode == MAC_MODE_1x50g) {
      return 0;
   } else if (mode == MAC_MODE_2x40g || mode == MAC_MODE_2x50g) {
      return (port == 0) ? 0 : 2;
   } else if (mode == MAC_MODE_4x25g || mode == MAC_MODE_4x10g || mode == MAC_MODE_4x1g) {
      return port;
   } else {
      PLOG_ERR("cap_mx_port_to_ch_mapping:" << " No support for " << mode << endl);
   }

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
cap_mx_set_bits(int chip_id, int inst_id, uint32_t addr,
                int pos, int num_bits, uint32_t value)
{
    uint32_t data = cap_mx_apb_read(chip_id, inst_id, addr);

    data = set_bits(data, pos, num_bits, value);
    cap_mx_apb_write(chip_id, inst_id, addr, data);
    return 0;
}
static inline uint64_t
set_bit (uint64_t data, uint8_t index) {
    return (data | ((uint64_t)1 << index));
}

static inline uint64_t
reset_bit (uint64_t data, uint8_t index) {
    return (data & ~((uint64_t)1 << index));
}

void cap_mx_set_pause(int chip_id , int inst_id, int ch0_pri_vec, int ch1_pri_vec, int ch2_pri_vec, int ch3_pri_vec, int legacy) {
 cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);

   int txfcxoff_enable[4];
   int txpfcxoff_enable[4];

   for (int ch = 0; ch < 4; ch++) {
      int pri_vec = (ch == 1) ? ch1_pri_vec : (ch == 2) ? ch2_pri_vec : (ch == 3) ? ch3_pri_vec : ch0_pri_vec;
      int addr1 = (ch == 1) ? 0x501 : (ch == 2) ? 0x601 : (ch == 3) ? 0x701 : 0x401;
      int addr2 = (ch == 1) ? 0x502 : (ch == 2) ? 0x602 : (ch == 3) ? 0x702 : 0x402;
      int addr3 = (ch == 1) ? 0x506 : (ch == 2) ? 0x606 : (ch == 3) ? 0x706 : 0x406;
      txfcxoff_enable[ch] = 0;
      txpfcxoff_enable[ch] = 0;

      if (pri_vec == 0) {
         cap_mx_apb_write(chip_id, inst_id, addr1, 0x030);
         cap_mx_apb_write(chip_id, inst_id, addr2, 0x010);  // bit4 Promiscuous Mode (1: disable MAC address check)
         cap_mx_apb_write(chip_id, inst_id, addr3, 0x00);   // MAC Tx Priority Pause Vector
      } else {
         if (legacy) {
            txfcxoff_enable[ch] = 1;
            cap_mx_apb_write(chip_id, inst_id, addr1, 0x130);   // Bit8: Control Frame Generation Enable
            cap_mx_apb_write(chip_id, inst_id, addr2, 0x130);   // bit5: Enable Rx Flow Control Decode, bit8 filter pause frame
            cap_mx_apb_write(chip_id, inst_id, addr3, 0x00);    // MAC Tx Priority Pause Vector
         } else {
            txpfcxoff_enable[ch] = 0xff;
            cap_mx_apb_write(chip_id, inst_id, addr1, 0x230);   // bit9: Priority Flow Control Generation Enable
            cap_mx_apb_write(chip_id, inst_id, addr2, 0x130);   // bit5: Enable Rx Flow Control Decode, bit8 filter pause frame
            cap_mx_apb_write(chip_id, inst_id, addr3, pri_vec); // MAC Tx Priority Pause Vector
         }
      }
   }

   mx_csr.cfg_mac_xoff.ff_tx0fcxoff_i(txfcxoff_enable[cap_mx_port_to_ch_mapping(chip_id, inst_id, 0)]);
   mx_csr.cfg_mac_xoff.ff_tx0pfcxoff_i(txpfcxoff_enable[cap_mx_port_to_ch_mapping(chip_id, inst_id, 0)]);
   mx_csr.cfg_mac_xoff.ff_tx1fcxoff_i(txfcxoff_enable[cap_mx_port_to_ch_mapping(chip_id, inst_id, 1)]);
   mx_csr.cfg_mac_xoff.ff_tx1pfcxoff_i(txpfcxoff_enable[cap_mx_port_to_ch_mapping(chip_id, inst_id, 1)]);
   mx_csr.cfg_mac_xoff.ff_tx2fcxoff_i(txfcxoff_enable[cap_mx_port_to_ch_mapping(chip_id, inst_id, 2)]);
   mx_csr.cfg_mac_xoff.ff_tx2pfcxoff_i(txpfcxoff_enable[cap_mx_port_to_ch_mapping(chip_id, inst_id, 2)]);
   mx_csr.cfg_mac_xoff.ff_tx3fcxoff_i(txfcxoff_enable[cap_mx_port_to_ch_mapping(chip_id, inst_id, 3)]);
   mx_csr.cfg_mac_xoff.ff_tx3pfcxoff_i(txpfcxoff_enable[cap_mx_port_to_ch_mapping(chip_id, inst_id, 3)]);
   mx_csr.cfg_mac_xoff.write();
   // mx_csr.cfg_mac_xoff.show();
}

int cap_mx_check_tx_idle(int chip_id, int inst_id, int port) {
 cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
   mx_csr.sta_mac.read();
   bool tx_idle[4];
   tx_idle[0] = (mx_csr.sta_mac.ff_tx0idle_o().convert_to<int>() == 1);
   tx_idle[1] = (mx_csr.sta_mac.ff_tx1idle_o().convert_to<int>() == 1);
   tx_idle[2] = (mx_csr.sta_mac.ff_tx2idle_o().convert_to<int>() == 1);
   tx_idle[3] = (mx_csr.sta_mac.ff_tx3idle_o().convert_to<int>() == 1);

   return (tx_idle[port]) ? 1 : 0;
}

int cap_mx_check_rx_idle(int chip_id, int inst_id, int port) {
 cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
   mx_csr.sta_mac.read();
   bool rx_idle[4];
   rx_idle[0] = (mx_csr.sta_mac.ff_rx0idle_o().convert_to<int>() == 1) ||
                (mx_csr.sta_mac.ff_rx0sync_o().convert_to<int>() == 0);
   rx_idle[1] = (mx_csr.sta_mac.ff_rx1idle_o().convert_to<int>() == 1) ||
                (mx_csr.sta_mac.ff_rx1sync_o().convert_to<int>() == 0);
   rx_idle[2] = (mx_csr.sta_mac.ff_rx2idle_o().convert_to<int>() == 1) ||
                (mx_csr.sta_mac.ff_rx2sync_o().convert_to<int>() == 0);
   rx_idle[3] = (mx_csr.sta_mac.ff_rx3idle_o().convert_to<int>() == 1) ||
                (mx_csr.sta_mac.ff_rx3sync_o().convert_to<int>() == 0);

   return (rx_idle[port]) ? 1 : 0;
}

void cap_mx_wait_mac_sync(int chip_id, int mx_inst, int ch) {
   int num_try = 0, max_attempts=500000;
   while (cap_mx_check_ch_sync(chip_id, mx_inst, ch) == 0) {
     SLEEP(100);
     num_try++;
     if (num_try > max_attempts) {
       PLOG_ERR("cap_mx_wait_mac_sync : mx " << mx_inst <<" ch " << ch << " is not sync" << endl);
       break;
     }
     if (!(num_try%1000)) {
       PLOG_MSG("cap_mx_wait_mac_sync : mx " << mx_inst <<" ch " << ch << " waiting for sync" << endl);
     }
   }
}

int cap_mx_check_sync(int chip_id, int inst_id, int port) {
 cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
   mx_csr.sta_mac.read();
   bool rx_sync[4];
   rx_sync[0] = (mx_csr.sta_mac.ff_rx0sync_o().convert_to<int>() == 1);
   rx_sync[1] = (mx_csr.sta_mac.ff_rx1sync_o().convert_to<int>() == 1);
   rx_sync[2] = (mx_csr.sta_mac.ff_rx2sync_o().convert_to<int>() == 1);
   rx_sync[3] = (mx_csr.sta_mac.ff_rx3sync_o().convert_to<int>() == 1);

   return (rx_sync[port]) ? 1 : 0;
}

// MX Port# to MAC Channel# Mapping
void cap_mx_set_appfifoportmap(int chip_id, int inst_id, int enable_vec, int port_map3, int port_map2, int port_map1, int port_map0) {

   int wdata = ((enable_vec & 0xf) << 12) |
               ((port_map3  & 0x7) << 9) |
               ((port_map2  & 0x7) << 6) |
               ((port_map1  & 0x7) << 3) |
               ((port_map0  & 0x7) << 0);
   cap_mx_apb_write(chip_id, inst_id, 0x205, wdata);
}

// Channel# to internal MAC Slot Mapping
void cap_mx_set_chmapping(int chip_id, int inst_id, int slot3_chmap, int slot2_chmap, int slot1_chmap, int slot0_chmap) {

   int wdata = ((slot3_chmap  & 0x7) << 9) |
               ((slot2_chmap  & 0x7) << 6) |
               ((slot1_chmap  & 0x7) << 3) |
               ((slot0_chmap  & 0x7) << 0);
   cap_mx_apb_write(chip_id, inst_id, 0x217, wdata);
   cap_mx_apb_write(chip_id, inst_id, 0x218, wdata);
}

void cap_mx_set_pcs_lane_remap(int chip_id, int inst_id, int pcs_lane7, int pcs_lane6, int pcs_lane5, int pcs_lane4) {

   int wdata = ((pcs_lane7  & 0x7) << 9) |
               ((pcs_lane6  & 0x7) << 6) |
               ((pcs_lane5  & 0x7) << 3) |
               ((pcs_lane4  & 0x7) << 0);
   cap_mx_apb_write(chip_id, inst_id, 0x1903, wdata);  // Lane Remap RX
   cap_mx_apb_write(chip_id, inst_id, 0x1983, wdata);  // Lane Remap TX
}

void cap_mx_set_tx_rx_enable(int chip_id, int inst_id, int ch, int tx_enable, int rx_enable) {

   PLOG_MSG("mx" << inst_id << " channel " << ch << ": set tx_enable = " << tx_enable << ", rx_enable = " << rx_enable << endl);
   int addr = (ch == 1) ? 0x500 : (ch == 2) ? 0x600 : (ch == 3) ? 0x700 : 0x400;
   int wdata = (rx_enable & 0x1) << 1 | (tx_enable & 0x1);
   cap_mx_apb_write(chip_id, inst_id, addr, wdata);
}

void cap_mx_set_ch_mode(int chip_id, int inst_id, int ch, int mode) {

   int addr = (ch == 1) ? 0x910 : (ch == 2) ? 0xA10 : (ch == 3) ? 0xB10 : 0x810;
   cap_mx_apb_write(chip_id, inst_id, addr, mode);
}

// If the value of the length/type field in MAC frame >= 1536, then value
// indicates type of the frame.
// If the value of the length/type field in MAC frame < 1500, then value
// indicates length of the frame.
// All other values are undefined.
void cap_mx_disable_eth_len_err(int chip_id, int inst_id, int ch, int value) {

   int addr = (ch == 1) ? 0x525 : (ch == 2) ? 0x625 : (ch == 3) ? 0x725 : 0x425;
   if (value == 1) {
      cap_mx_apb_write(chip_id, inst_id, addr, 0x20);
   } else {
      cap_mx_apb_write(chip_id, inst_id, addr, 0x00);
   }
}

void cap_mx_init_start(int chip_id, int inst_id) {
    return;
}

void cap_mx_init_done(int chip_id, int inst_id) {
    PLOG_MSG("inside init done\n");
}

int cap_mx_get_port_enable_state(int chip_id, int inst_id, int port) {
   return mx[inst_id].port_enable[port];
}


void cap_mx_load_from_cfg_glbl2(int chip_id, int inst_id, int ch_enable_vec) {
   cap_mx_set_ch_enable(chip_id, inst_id, ch_enable_vec);

   int reset_vec = (ch_enable_vec ^ 0xf) & 0xf;
   cap_mx_set_soft_reset(chip_id, inst_id, reset_vec);

   if (mx[inst_id].enable_legacy_pause) {
       cap_mx_set_pause(chip_id, inst_id, 0x1, 0x1, 0x1, 0x1, 1);
   }
   else if (mx[inst_id].enable_pause) {
       cap_mx_set_pause(chip_id, inst_id, 0xff, 0xff, 0xff, 0xff, 0);
   }
   if(mx[inst_id].tx_pad_disable) {
     cap_mx_tx_pad_disable(0,inst_id);
   }  

   cap_mx_apb_write(chip_id, inst_id, 0x1901, mx[inst_id].serdes_lp);
}

void cap_mx_load_from_cfg(int chip_id, int inst_id, int rst) {
   if (rst == 1) {
     cap_mx_set_soft_reset(chip_id, inst_id, 0xf);
   }
   PLOG_MSG( "cap_mx_load_from_cfg: cap" << chip_id << "/mx" << inst_id << ":\n");

   int ch_enable_vec = 0;

   cap_mx_load_from_cfg_glbl1(chip_id, inst_id, &ch_enable_vec);

   // Rx and Tx Enable, Channel Mode, MTU, mask rx len error
   for (int ch = 0; ch < 4; ch++) {
        cap_mx_cfg_ch(chip_id, inst_id, ch);
        cap_mx_cfg_ch_en(chip_id, inst_id, ch, (ch_enable_vec >> ch) & 0x1);
   }

   cap_mx_load_from_cfg_glbl2(chip_id, inst_id, ch_enable_vec);
}

void cap_mx_tx_pad_disable(int chip_id, int inst_id) {
   cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
   PLOG_MSG( "MX inst_id:" << inst_id << " Entered cap_mx_tx_pad_disable " << endl)
   mx_csr.cfg_mac_gbl.read();
   mx_csr.cfg_mac_gbl.ff_txdispad_i(1);
   mx_csr.cfg_mac_gbl.write();
}

void cap_mx_eos(int chip_id, int inst_id) {
   cap_mx_eos_cnt(chip_id,inst_id);
   cap_mx_eos_int(chip_id,inst_id);
   cap_mx_eos_sta(chip_id,inst_id);
}


void cap_mx_eos_cnt(int chip_id, int inst_id) {
 //cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
   CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
   cap_mx_mac_stat(chip_id,inst_id, 0, 1, NULL);  // channel 0
   cap_mx_mac_stat(chip_id,inst_id, 1, 1, NULL);  // channel 1
   cap_mx_mac_stat(chip_id,inst_id, 2, 1, NULL);  // channel 2
   cap_mx_mac_stat(chip_id,inst_id, 3, 1, NULL);  // channel 3
}

void cap_mx_eos_int(int chip_id, int inst_id) {
}

void cap_mx_eos_sta(int chip_id, int inst_id) {
}

void cap_mx_set_debug_ctl(int chip_id, int inst_id, int enable, int select) {

 PLOG_MSG("Set MX debug enable/select\n");
 cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);

 mx_csr.cfg_debug_port.read();
 mx_csr.cfg_debug_port.enable(enable);
 mx_csr.cfg_debug_port.select(select);
 mx_csr.cfg_debug_port.write();
}

void cap_mx_dump_mibs(int chip_id) {
#if 0
// TBD-ELBA-REBASE
 cap_mx_csr_t & mx_csr0 = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, 0);
 cap_mx_csr_t & mx_csr1 = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, 1);

   string stats[90] = {
   "Frames RX OK",
   "Frames RX All",
   "Frames RX Bad FCS",
   "Frames RX Bad All",
   "Octets RX OK",
   "Octets RX All",
   "Frames RX Unicast",
   "Frames RX Multicast",
   "Frames RX Broadcast",
   "Frames RX Pause",
   "Frames RX Bad Length",
   "Frames RX Undersized",
   "Frames RX Oversized",
   "Frames RX Fragments",
   "Frames RX Jabber",
   "Frames RX PriPause",
   "Frames RX Stomped CRC",
   "Frames RX Too Long",
   "Frames RX VLAN (Good)",
   "Frames RX Dropped",
   "Frames RX <64B",
   "Frames RX 64B",
   "Frames RX 65B~127B",
   "Frames RX 128B~255B",
   "Frames RX 256B~511B",
   "Frames RX 512B~1023B",
   "Frames RX 1024B~1518B",
   "Frames RX 1519B~2047B",
   "Frames RX 2048B~4095B",
   "Frames RX 4096B~8191B",
   "Frames RX 8192B~9215B",
   "Frames RX >=9216B",
   "Frames TX OK",
   "Frames TX All",
   "Frames TX Bad",
   "Octets TX OK",
   "Octets TX Total",
   "Frames TX Unicast",
   "Frames TX Multicast",
   "Frames TX Broadcast",
   "Frames TX Pause",
   "Frames TX PriPause",
   "Frames TX VLAN",
   "Frames TX <64B",
   "Frames TX 64B",
   "Frames TX 65B~127B",
   "Frames TX 128B~255B",
   "Frames TX 256B~511B",
   "Frames TX 512B~1023B",
   "Frames TX 1024B~1518B",
   "Frames TX 1519B~2047B",
   "Frames TX 2048B~4095B",
   "Frames TX 4096B~8191B",
   "Frames TX 8192B~9215B",
   "Frames TX >=9216B",
   "Frames TX Pri#0",
   "Frames TX Pri#1",
   "Frames TX Pri#2",
   "Frames TX Pri#3",
   "Frames TX Pri#4",
   "Frames TX Pri#5",
   "Frames TX Pri#6",
   "Frames TX Pri#7",
   "Frames RX Pri#0",
   "Frames RX Pri#1",
   "Frames RX Pri#2",
   "Frames RX Pri#3",
   "Frames RX Pri#4",
   "Frames RX Pri#5",
   "Frames RX Pri#6",
   "Frames RX Pri#7",
   "TX PriPause#0 1us Count",
   "TX PriPause#1 1us Count",
   "TX PriPause#2 1us Count",
   "TX PriPause#3 1us Count",
   "TX PriPause#4 1us Count",
   "TX PriPause#5 1us Count",
   "TX PriPause#6 1us Count",
   "TX PriPause#7 1us Count",
   "RX PriPause#0 1us Count",
   "RX PriPause#1 1us Count",
   "RX PriPause#2 1us Count",
   "RX PriPause#3 1us Count",
   "RX PriPause#4 1us Count",
   "RX PriPause#5 1us Count",
   "RX PriPause#6 1us Count",
   "RX PriPause#7 1us Count",
   "RX Pause 1us Count",
   "Frames TX Truncated",
   "Reserved"
   };

   PRN_MSG(setw(125) << "====================================================== MAC MIBS ============================================================\n");
   for (int inst = 0; inst < 2; inst++) {
     if (inst != 0) {
       PRN_MSG(setw(125) << "----------------------------------------------------------------------------------------------------------------------------\n");
     }
     PRN_MSG(setw(41) << "mx" << inst << ".p0" << setw(16) << "mx" << inst << ".p1" << setw(16) << "mx" << inst << ".p2" << setw(16) << "mx" << inst << ".p3\n\n");
     for (int i = 0; i < 90; i++) {
       string counter_name = stats[i];
       counter_name.append(25-counter_name.length(),' ');
       stringstream sdata;
       sdata.str("");
       for (int ch = 0; ch < 4; ch++) {
         int addr = ((ch&0x3) << 7) | i;
         cpp_int rdata;
         if (inst == 0) {
           mx_csr0.dhs_mac_stats.entry[addr].read();
           rdata = mx_csr0.dhs_mac_stats.entry[addr].value();
         } else {
           mx_csr1.dhs_mac_stats.entry[addr].read();
           rdata = mx_csr1.dhs_mac_stats.entry[addr].value();
         }
         stringstream rval;
         if (rdata == 0) {
	   rval << "-";
         } else {
           rval << "0x" << hex << rdata;
         }
         sdata << setw(20) << rval.str();
       }
       PRN_MSG(counter_name << sdata.str() << "\n");
     }
   }
   PRN_MSG(setw(125) << "======================================================== END MAC MIBS ======================================================\n");
#endif
}

void print_msg(string msg) {
#ifdef _CSV_INCLUDED_
    vpi_printf((PLI_BYTE8*) "%s", msg.c_str());
#else 
    std::cout << msg;
#endif    
}

void cap_mx_set_tx_drain(int chip_id, int inst_id, int ch, int value) {
   PLOG_MSG("mx" << inst_id << " channel " << ch << ": set tx_drain " << value << endl);
   int addr = (ch == 1) ? 0x500 : (ch == 2) ? 0x600 : (ch == 3) ? 0x700 : 0x400;
   int data = cap_mx_apb_read(chip_id, inst_id, addr);
   data = value ? (data | (0x1 << 5)) : (data & ~(0x1 << 5));
   cap_mx_apb_write(chip_id, inst_id, addr, data);
}

void cap_mx_set_tx_autodrain(int chip_id, int inst_id, int ch, int value) {
   PLOG_MSG("mx" << inst_id << " channel " << ch << ": set tx_drain " << value << endl);
   int addr = (ch == 1) ? 0x591 : (ch == 2) ? 0x691 : (ch == 3) ? 0x791 : 0x491;
   int data = cap_mx_apb_read(chip_id, inst_id, addr);
   data = value ? (data | (0x1 << 6)) : (data & ~(0x1 << 6));
   cap_mx_apb_write(chip_id, inst_id, addr, data);
}

void cap_mx_wait_fec_sync(int chip_id, int inst_id, int ch) {
  PLOG_MSG("cap_mx_wait_fec_sync mx" << inst_id << " channel:" << ch << ": wait fec sync" << endl);
  int addr = (ch == 1) ? 0x30c9 : (ch == 2) ? 0x31c9 : (ch == 3) ? 0x32c9 : 0x10c9;
  int data;
  int num_try = 0, max_attempts=500000;
  do {
    num_try++;
    if (num_try > max_attempts) {
      PLOG_ERR("cap_mx_wait_fec_sync : mx " << inst_id << " channel:" << ch << ": wait fec sync failed" << endl);
      break;
    }
    if (!(num_try%1000)) {
      PLOG_MSG("cap_mx_wait_fec_sync : mx " << inst_id << " channel: " << ch << " waiting for sync" << endl);
    }
    data = cap_mx_apb_read(chip_id, inst_id, addr);
    data = data & 0x4000;
  } while (data == 0);
}

void cap_mx_set_an_ability(int chip_id, int inst_id, int ch, int value) {
   PLOG_MSG("mx" << inst_id << " channel " << ch << ": set an_ability " << value << endl);
   int addr = (ch == 1) ? 0xd01 : (ch == 2) ? 0xe01 : (ch == 3) ? 0xf01 : 0xc01;
   cap_mx_apb_write(chip_id, inst_id, addr, value);
}

int cap_mx_rd_rx_an_ability(int chip_id, int inst_id, int ch) {
   PLOG_MSG("mx" << inst_id << " channel " << ch << ": read rx an_ability " << endl);
   int addr = (ch == 1) ? 0xd02 : (ch == 2) ? 0xe02 : (ch == 3) ? 0xf02 : 0xc02;
   int value = cap_mx_apb_read(chip_id, inst_id, addr);
   return value;
}

void cap_mx_start_an(int chip_id, int inst_id, int ch) {
   PLOG_MSG("mx" << inst_id << " channel " << ch << ": start an " << endl);
   int addr = (ch == 1) ? 0xd00 : (ch == 2) ? 0xe00 : (ch == 3) ? 0xf00 : 0xc00;
   int wdata = 0x803;
   cap_mx_apb_write(chip_id, inst_id, addr, wdata);
}

void cap_mx_stop_an(int chip_id, int inst_id, int ch) {
   PLOG_MSG("mx" << inst_id << " channel " << ch << ": start an " << endl);
   int addr = (ch == 1) ? 0xd00 : (ch == 2) ? 0xe00 : (ch == 3) ? 0xf00 : 0xc00;
   int wdata = 0x0;
   cap_mx_apb_write(chip_id, inst_id, addr, wdata);
}

void cap_mx_wait_an_done(int chip_id, int inst_id, int ch) {
   PLOG_MSG("mx" << inst_id << " channel " << ch << ": wait an_done " << endl);
   int addr = (ch == 1) ? 0xd06 : (ch == 2) ? 0xe06 : (ch == 3) ? 0xf06 : 0xc06;
   int data;
   do {
     data = cap_mx_apb_read(chip_id, inst_id, addr);
   } while ((data & 0x1) == 0);
}

void cap_mx_set_an_link_timer(int chip_id, int inst_id, int ch, int value) {
   PLOG_MSG("mx" << inst_id << " channel " << ch << ": set an_link_timer " << value << endl);
   int addr = (ch == 1) ? 0xd03 : (ch == 2) ? 0xe03 : (ch == 3) ? 0xf03 : 0xc03;
   cap_mx_apb_write(chip_id, inst_id, addr, value & 0xffff);
   addr = (ch == 1) ? 0xd13 : (ch == 2) ? 0xe13 : (ch == 3) ? 0xf13 : 0xc13;
   cap_mx_apb_write(chip_id, inst_id, addr, (value >> 16));
}

int cap_mx_get_fec_cor_blK_cnt(int chip_id, int inst_id, int ch) {
 //cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
 CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
 int addr = (ch == 1) ? 0x30ca : (ch == 2) ? 0x31ca : (ch == 3) ? 0x32ca : 0x10ca;
 int data = cap_mx_apb_read(chip_id, inst_id, addr);
 PLOG_MSG("cap_mx_get_fec_cor_blK_cnt :: mx" << inst_id << " channel " << ch << ": corrected blocks counter low:" << data << endl);
 return(data);
}

int cap_mx_tx_bad_cnt(int chip_id, int inst_id, int ch) {
 cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
 int cnt_offset = 34;
 int addr = ((ch&0x3) << 7) | cnt_offset;
 int rdata = 0;
 mx_csr.dhs_mac_stats.entry[addr].read();
 rdata = mx_csr.dhs_mac_stats.entry[addr].value().convert_to<int>();
 return rdata;
}

int cap_mx_rx_bad_fcs_cnt(int chip_id, int inst_id, int ch) {
 cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
 int cnt_offset = 2;
 int addr = ((ch&0x3) << 7) | cnt_offset;
 int rdata = 0;
 mx_csr.dhs_mac_stats.entry[addr].read();
 rdata = mx_csr.dhs_mac_stats.entry[addr].value().convert_to<int>();
 return rdata;
}

int cap_mx_rx_good_cnt(int chip_id, int inst_id, int ch) {
 cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
 int cnt_offset = 0;
 int addr = ((ch&0x3) << 7) | cnt_offset;
 int rdata = 0;
 mx_csr.dhs_mac_stats.entry[addr].read();
 rdata = mx_csr.dhs_mac_stats.entry[addr].value().convert_to<int>();
 return rdata;
}

int cap_mx_rx_all_cnt(int chip_id, int inst_id, int ch) {
 cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
 int cnt_offset = 1;
 int addr = ((ch&0x3) << 7) | cnt_offset;
 int rdata = 0;
 mx_csr.dhs_mac_stats.entry[addr].read();
 rdata = mx_csr.dhs_mac_stats.entry[addr].value().convert_to<int>();
 return rdata;
}

int cap_mx_rx_crc_stomp_cnt(int chip_id, int inst_id, int ch) {
 cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
 int cnt_offset = 16;
 int addr = ((ch&0x3) << 7) | cnt_offset;
 int rdata = 0;
 mx_csr.dhs_mac_stats.entry[addr].read();
 rdata = mx_csr.dhs_mac_stats.entry[addr].value().convert_to<int>();
 return rdata;
}

int cap_mx_rx_jabber_cnt(int chip_id, int inst_id, int ch) {
 cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
 int cnt_offset = 14;
 int addr = ((ch&0x3) << 7) | cnt_offset;
 int rdata = 0;
 mx_csr.dhs_mac_stats.entry[addr].read();
 rdata = mx_csr.dhs_mac_stats.entry[addr].value().convert_to<int>();
 return rdata;
}

int cap_mx_tx_truncated_cnt(int chip_id, int inst_id, int ch) {
 cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
 int cnt_offset = 17;
 int addr = ((ch&0x3) << 7) | cnt_offset;
 int rdata = 0;
 mx_csr.dhs_mac_stats.entry[addr].read();
 rdata = mx_csr.dhs_mac_stats.entry[addr].value().convert_to<int>();
 return rdata;
}

int cap_mx_tx_good_cnt(int chip_id, int inst_id, int ch) {
 cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
 int cnt_offset = 32;
 int addr = ((ch&0x3) << 7) | cnt_offset;
 int rdata = 0;
 mx_csr.dhs_mac_stats.entry[addr].read();
 rdata = mx_csr.dhs_mac_stats.entry[addr].value().convert_to<int>();
 return rdata;
}

int cap_mx_tx_all_cnt(int chip_id, int inst_id, int ch) {
 cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
 int cnt_offset = 33;
 int addr = ((ch&0x3) << 7) | cnt_offset;
 int rdata = 0;
 mx_csr.dhs_mac_stats.entry[addr].read();
 rdata = mx_csr.dhs_mac_stats.entry[addr].value().convert_to<int>();
 return rdata;
}

int cap_mx_rx_frame_too_long_cnt(int chip_id, int inst_id, int ch) {
 cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
 int cnt_offset = 17;
 int addr = ((ch&0x3) << 7) | cnt_offset;
 int rdata = 0;
 mx_csr.dhs_mac_stats.entry[addr].read();
 rdata = mx_csr.dhs_mac_stats.entry[addr].value().convert_to<int>();
 return rdata;
}



void cap_mx_bist_run(int chip_id, int inst_id, int enable) {
  cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);

  PLOG_API_MSG("MX[" << inst_id << "]", ": cap_mx_bist_run, enable = " << enable << "\n");

  mx_csr.cfg_mac_gbl.read();
  mx_csr.cfg_mac_gbl.cg_fec_enable_i(1);
  mx_csr.cfg_mac_gbl.write();

  mx_csr.cfg_txfifo_mem.read();
  mx_csr.cfg_txfifo_mem.bist_run(enable);
  mx_csr.cfg_txfifo_mem.write();
  mx_csr.cfg_rxfifo_mem.read();
  mx_csr.cfg_rxfifo_mem.bist_run(enable);
  mx_csr.cfg_rxfifo_mem.write();
  mx_csr.cfg_stats_mem.read();
  mx_csr.cfg_stats_mem.bist_run(enable);
  mx_csr.cfg_stats_mem.write();
  mx_csr.cfg_fec_mem.read();
  mx_csr.cfg_fec_mem.bist_run(enable);
  mx_csr.cfg_fec_mem.write();
}

void cap_mx_bist_chk(int chip_id, int inst_id) {
  cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);

  PLOG_API_MSG("MX[" << inst_id << "]", ": cap_mx_bist_chk\n");

  int loop_cnt = 0;
  do {
     mx_csr.sta_txfifo_mem.read();
     mx_csr.sta_rxfifo_mem.read();
     mx_csr.sta_stats_mem.read();
     mx_csr.sta_fec_mem.read();
  } while((loop_cnt < 10000) &&
          ((mx_csr.sta_txfifo_mem.all() == 0) ||
           (mx_csr.sta_rxfifo_mem.all() == 0) ||
           (mx_csr.sta_stats_mem.all() == 0) ||
           (mx_csr.sta_fec_mem.all() == 0)));

  bool err = false;
  if (mx_csr.sta_txfifo_mem.bist_done_pass() != 0x1) {
    PLOG_ERR("mx_csr.sta_txfifo_mem.bist_done_pass(): exp = 0x1, rcv = 0x" << mx_csr.sta_txfifo_mem.bist_done_pass() << dec << endl);
    err = true;
  }
  if (mx_csr.sta_txfifo_mem.bist_done_fail() != 0x0) {
    PLOG_ERR("mx_csr.sta_txfifo_mem.bist_done_fail(): exp = 0x0, rcv = 0x" << mx_csr.sta_txfifo_mem.bist_done_fail() << dec << endl);
    err = true;
  }
  if (mx_csr.sta_rxfifo_mem.bist_done_pass() != 0x1) {
    PLOG_ERR("mx_csr.sta_rxfifo_mem.bist_done_pass(): exp = 0x1, rcv = 0x" << mx_csr.sta_rxfifo_mem.bist_done_pass() << dec << endl);
    err = true;
  }
  if (mx_csr.sta_rxfifo_mem.bist_done_fail() != 0x0) {
    PLOG_ERR("mx_csr.sta_rxfifo_mem.bist_done_fail(): exp = 0x0, rcv = 0x" << mx_csr.sta_rxfifo_mem.bist_done_fail() << dec << endl);
    err = true;
  }
  if (mx_csr.sta_stats_mem.bist_done_pass() != 0x1) {
    PLOG_ERR("mx_csr.sta_stats_mem.bist_done_pass(): exp = 0x1, rcv = 0x" << mx_csr.sta_stats_mem.bist_done_pass() << dec << endl);
    err = true;
  }
  if (mx_csr.sta_stats_mem.bist_done_fail() != 0x0) {
    PLOG_ERR("mx_csr.sta_stats_mem.bist_done_fail(): exp = 0x0, rcv = 0x" << mx_csr.sta_stats_mem.bist_done_fail() << dec << endl);
    err = true;
  }
  if (mx_csr.sta_fec_mem.bist_done_pass() != 0x1) {
    PLOG_ERR("mx_csr.sta_fec_mem.bist_done_pass(): exp = 0x1, rcv = 0x" << mx_csr.sta_fec_mem.bist_done_pass() << dec << endl);
    err = true;
  }
  if (mx_csr.sta_fec_mem.bist_done_fail() != 0x0) {
    PLOG_ERR("mx_csr.sta_fec_mem.bist_done_fail(): exp = 0x0, rcv = 0x" << mx_csr.sta_fec_mem.bist_done_fail() << dec << endl);
    err = true;
  }
  if (!err) {
     PLOG_API_MSG("PASSED: MX[" << inst_id << "]", ": cap_mx_bist_chk\n");
  }
}
#endif

// KCM 
int
cap_mx_base_r_pcs_status2 (int chip_id, int inst_id, int mac_ch)
{
#if 0
// TBD-ELBA-REBASE
    int addr = 0x0;

    switch (mac_ch) {
    case 1:
        addr = 0x921;
        break;

    case 2:
        addr = 0xA21;
        break;

    case 3:
        addr = 0xB21;
        break;

    case 0:
    default:
        addr = 0x821;
        break;
    }

    return cap_mx_apb_read(chip_id, inst_id, addr);
#else 
    return 0;
#endif
}

int
cap_mx_base_r_pcs_status2_clear (int chip_id, int inst_id, int mac_ch)
{
#if 0
// TBD-ELBA-REBASE
    int addr = 0x0;

    switch (mac_ch) {
    case 1:
        addr = 0x921;
        break;

    case 2:
        addr = 0xA21;
        break;

    case 3:
        addr = 0xB21;
        break;

    case 0:
    default:
        addr = 0x821;
        break;
    }

    cap_mx_apb_write(chip_id, inst_id, addr, 0x0);
    return 0;
#else 
    return 0;
#endif
}

void cap_mx_cfg_ch(int chip_id, int inst_id, int ch) {
#if 0
// TBD-ELBA-REBASE
    cap_mx_set_ch_mode(chip_id, inst_id, ch, mx[inst_id].ch_mode[ch]);

    if (mx[inst_id].speed[ch] == 100) {
       cap_mx_set_mtu_jabber(chip_id, inst_id, ch, 9216, 9216+16);
    } else if (mx[inst_id].speed[ch] == 50 || mx[inst_id].speed[ch] == 40) {
       cap_mx_set_mtu_jabber(chip_id, inst_id, ch, 9216, 9216+8);
    } else {
       cap_mx_set_mtu_jabber(chip_id, inst_id, ch, 9216, 9216+4);
    }
    // By default enable the RX len error check
    cap_mx_disable_eth_len_err(chip_id, inst_id, ch, 0);
#endif
}

void cap_mx_cfg_ch_en(int chip_id, int inst_id, int ch, int enable) {
#if 0
// TBD-ELBA-REBASE
    if (enable == 1) {
       cap_mx_set_tx_rx_enable(chip_id, inst_id, ch, 1, 1);
    } else {
       cap_mx_set_tx_rx_enable(chip_id, inst_id, ch, 0, 0);
    }
#endif
}

int cap_mx_check_ch_sync(int chip_id, int inst_id, int ch) {
#if 0
// TBD-ELBA-REBASE
 //cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
 CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
 int data = cap_mx_apb_read(chip_id, inst_id, 0x9);
 return ((data >> ch) & 0x1);
#else
 return 0;
#endif
}

void cap_mx_load_from_cfg_glbl1(int chip_id, int inst_id, int *ch_enable_vec) {
#if 0
// TBD-ELBA-REBASE
   // 
   // Comira MAC Configurations
   // 
   // bit[15]: slot2chmappingenable, bit[14:9]: 1 ch thres, bit[8:4]: 2 ch thres, bit[3:0]: 4 ch thres
   // 16'b1_010000_01000_1000 = 16'b1010_0000_1000_1000
   cap_mx_apb_write(chip_id, inst_id, 0x201, 0xA088);

   // Threshold used by APP_FIFO before reading from TX_FIFO
   cap_mx_apb_write(chip_id, inst_id, 0x20f, 0x1);
   cap_mx_apb_write(chip_id, inst_id, 0x210, 0x1);
   cap_mx_apb_write(chip_id, inst_id, 0x211, 0x1);
   cap_mx_apb_write(chip_id, inst_id, 0x212, 0x1);

   // global mode
   cap_mx_apb_write(chip_id, inst_id, 0x1, mx[inst_id].glbl_mode);

   uint32_t slot0 = mx[inst_id].tdm[0];
   uint32_t slot1 = mx[inst_id].tdm[1];
   uint32_t slot2 = mx[inst_id].tdm[2];
   uint32_t slot3 = mx[inst_id].tdm[3];

   // channel mode
   if (mx[inst_id].mac_mode == MAC_MODE_4x25g || mx[inst_id].mac_mode == MAC_MODE_4x10g || mx[inst_id].mac_mode == MAC_MODE_4x1g) {
      if (mx[inst_id].speed[0] == 25) {
         cap_mx_apb_write(chip_id, inst_id, 0x3, 0xAA);  // 4x25G
      } else if (mx[inst_id].speed[0] == 10) {
         cap_mx_apb_write(chip_id, inst_id, 0x3, 0x00);  // 4x10G
      } else {
         cap_mx_apb_write(chip_id, inst_id, 0x3, 0x55);  // 4x1G
      }
   } else if (mx[inst_id].mac_mode == MAC_MODE_1x50g_2x25g) {
     cap_mx_apb_write(chip_id, inst_id, 0x3, 0xAF);
   } else if (mx[inst_id].mac_mode == MAC_MODE_2x25g_1x50g) {
     cap_mx_apb_write(chip_id, inst_id, 0x3, 0xFA);
   }

   // Speed up SerDes locking (for simulation purpose only)
   if (mx[inst_id].for_simulation) {
      cap_mx_apb_write(chip_id, inst_id, 0x8ff, 0x1E05);
   }

   // App fifo port mapping, ChMapping0, ChMapping1, cfg_mac_tdm, PCS lane remapping
   if (mx[inst_id].mac_mode == MAC_MODE_1x100g) {
      cap_mx_set_appfifoportmap(chip_id, inst_id, 0x1, 0, 0, 0, 0);
      cap_mx_set_chmapping(chip_id, inst_id, 0, 0, 0, 0);
      cap_mx_set_cfg_mac_tdm(chip_id, inst_id, slot0, slot1, slot2, slot3);
      cap_mx_set_pcs_lane_remap(chip_id, inst_id, 7, 6, 5, 4);
      *ch_enable_vec = mx[inst_id].port_enable[0];
   } else if (mx[inst_id].mac_mode == MAC_MODE_2x25g_1x50g) {
      cap_mx_set_appfifoportmap(chip_id, inst_id, 0x7, 4, 2, 1, 0);
      cap_mx_set_chmapping(chip_id, inst_id, 2, 1, 2, 0);
      cap_mx_set_cfg_mac_tdm(chip_id, inst_id, 0, 2, 1, 2);
      cap_mx_set_pcs_lane_remap(chip_id, inst_id, 7, 6, 5, 4);
      cap_mx_apb_write(chip_id, inst_id, 0x1903, 0x0f9a); // SerDes remapping: 'b111_110_011_010
      cap_mx_apb_write(chip_id, inst_id, 0x1983, 0x0f9a);
      *ch_enable_vec = (mx[inst_id].port_enable[2]<<2) | (mx[inst_id].port_enable[1]<<1) | mx[inst_id].port_enable[0];
   } else if (mx[inst_id].mac_mode == MAC_MODE_1x40g || mx[inst_id].mac_mode == MAC_MODE_1x50g) {
      cap_mx_set_appfifoportmap(chip_id, inst_id, 0x1, 0, 0, 0, 0);
      cap_mx_set_chmapping(chip_id, inst_id, 0, 0, 0, 0);
      cap_mx_set_cfg_mac_tdm(chip_id, inst_id, slot0, slot1, slot2, slot3);
      cap_mx_set_pcs_lane_remap(chip_id, inst_id, 7, 6, 5, 4);
      *ch_enable_vec = mx[inst_id].port_enable[0];
   } else if (mx[inst_id].mac_mode == MAC_MODE_2x40g || mx[inst_id].mac_mode == MAC_MODE_2x50g) {
      cap_mx_set_appfifoportmap(chip_id, inst_id, 0x5, 4, 1, 4, 0);
      cap_mx_set_chmapping(chip_id, inst_id, 2, 0, 2, 0);
      cap_mx_set_cfg_mac_tdm(chip_id, inst_id, slot0, slot1, slot2, slot3);
      cap_mx_set_pcs_lane_remap(chip_id, inst_id, 7, 6, 5, 4);
      cap_mx_apb_write(chip_id, inst_id, 0x1903, 0x0f9a); // SerDes remapping: 'b111_110_011_010
      cap_mx_apb_write(chip_id, inst_id, 0x1983, 0x0f9a);
      *ch_enable_vec = (mx[inst_id].port_enable[1]<<2) | mx[inst_id].port_enable[0];
   } else if (mx[inst_id].mac_mode == MAC_MODE_1x50g_2x25g) {
      cap_mx_set_appfifoportmap(chip_id, inst_id, 0xd, 3, 2, 4, 0);
      cap_mx_set_chmapping(chip_id, inst_id, 3, 0, 2, 0);
      cap_mx_set_cfg_mac_tdm(chip_id, inst_id, 0, 2, 0, 3);
      cap_mx_set_pcs_lane_remap(chip_id, inst_id, 7, 6, 5, 4);
      *ch_enable_vec = (mx[inst_id].port_enable[3]<<3) | (mx[inst_id].port_enable[2]<<2) | mx[inst_id].port_enable[0];
   } else if (mx[inst_id].mac_mode == MAC_MODE_4x25g || mx[inst_id].mac_mode == MAC_MODE_4x10g || mx[inst_id].mac_mode == MAC_MODE_4x1g) {
      cap_mx_set_appfifoportmap(chip_id, inst_id, 0xf, 3, 2, 1, 0);
      cap_mx_set_chmapping(chip_id, inst_id, 3, 2, 1, 0);
      cap_mx_set_cfg_mac_tdm(chip_id, inst_id, slot0, slot1, slot2, slot3);
      cap_mx_set_pcs_lane_remap(chip_id, inst_id, 7, 6, 5, 4);
      *ch_enable_vec = (mx[inst_id].port_enable[3] << 3) | (mx[inst_id].port_enable[2]<<2) | (mx[inst_id].port_enable[1]<<1) | mx[inst_id].port_enable[0];
   } else {
      PLOG_MSG( "mx" << inst_id << " ERROR: invalid mac_mode = " << mx[inst_id].mac_mode << endl;);
      *ch_enable_vec = 0;
   }

   // rx fifo ctrl0 (Change soft min gap to 5, i.e., 'b0_01000000_1_00101)
   cap_mx_apb_write(chip_id, inst_id, 0x207, 0x1025);
   cap_mx_apb_write(chip_id, inst_id, 0x208, 0x1025);
   cap_mx_apb_write(chip_id, inst_id, 0x209, 0x1025);
   cap_mx_apb_write(chip_id, inst_id, 0x20A, 0x1025);

   // MAC Rx Configuration: bit4: Promiscuous Mode (1: disable MAC address check)
   cap_mx_apb_write(chip_id, inst_id, 0x402, 0x10);
   cap_mx_apb_write(chip_id, inst_id, 0x502, 0x10);
   cap_mx_apb_write(chip_id, inst_id, 0x602, 0x10);
   cap_mx_apb_write(chip_id, inst_id, 0x702, 0x10);
#endif
}

void cap_mx_mac_stat(int chip_id, int inst_id, int ch, int short_report,
                     uint64_t *stats_data)
{
#if 0
// TBD-ELBA-REBASE
 cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);

   string stats[180] = {
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
   "L", "Priority Pause Frames",
   "L", "Stomped CRC",
   "L", "Frame Too Long",
   "L", "Rx VLAN Frames (Good)",
   "A", "Frames Dropped (Buffer Full)",
   "L", "Frames Received Length<64",
   "L", "Frames Received Length=64",
   "L", "Frames Received Length=65~127",
   "L", "Frames Received Length=128~255",
   "L", "Frames Received Length=256~511",
   "L", "Frames Received Length=512~1023",
   "L", "Frames Received Length=1024~1518",
   "L", "Frames Received Length=1519~2047",
   "L", "Frames Received Length=2048~4095",
   "L", "Frames Received Length=4096~8191",
   "L", "Frames Received Length=8192~9215",
   "L", "Frames Received Length>=9216",
   "A", "Frames Transmitted OK",
   "A", "Frames Transmitted All (Good/Bad Frames)",
   "A", "Frames Transmitted Bad",
   "A", "Octets Transmitted Good",
   "A", "Octets Transmitted Total (Good/Bad)",
   "L", "Frames Transmitted with Unicast Address",
   "L", "Frames Transmitted with Multicast Address",
   "L", "Frames Transmitted with Broadcast Address",
   "L", "Frames Transmitted of type PAUSE",
   "L", "Frames Transmitted of type PriPAUSE",
   "L", "Frames Transmitted VLAN",
   "L", "Frames Transmitted Length<64",
   "L", "Frames Transmitted Length=64",
   "L", "Frames Transmitted Length=65~127",
   "L", "Frames Transmitted Length=128~255",
   "L", "Frames Transmitted Length=256~511",
   "L", "Frames Transmitted Length=512~1023",
   "L", "Frames Transmitted Length=1024~1518",
   "L", "Frames Transmitted Length=1519~2047",
   "L", "Frames Transmitted Length=2048~4095",
   "L", "Frames Transmitted Length=4096~8191",
   "L", "Frames Transmitted Length=8192~9215",
   "L", "Frames Transmitted Length>=9216",
   "L", "Pri#0 Frames Transmitted",
   "L", "Pri#1 Frames Transmitted",
   "L", "Pri#2 Frames Transmitted",
   "L", "Pri#3 Frames Transmitted",
   "L", "Pri#4 Frames Transmitted",
   "L", "Pri#5 Frames Transmitted",
   "L", "Pri#6 Frames Transmitted",
   "L", "Pri#7 Frames Transmitted",
   "L", "Pri#0 Frames Received",
   "L", "Pri#1 Frames Received",
   "L", "Pri#2 Frames Received",
   "L", "Pri#3 Frames Received",
   "L", "Pri#4 Frames Received",
   "L", "Pri#5 Frames Received",
   "L", "Pri#6 Frames Received",
   "L", "Pri#7 Frames Received",
   "L", "Transmit Pri#0 Pause 1US Count",
   "L", "Transmit Pri#1 Pause 1US Count",
   "L", "Transmit Pri#2 Pause 1US Count",
   "L", "Transmit Pri#3 Pause 1US Count",
   "L", "Transmit Pri#4 Pause 1US Count",
   "L", "Transmit Pri#5 Pause 1US Count",
   "L", "Transmit Pri#6 Pause 1US Count",
   "L", "Transmit Pri#7 Pause 1US Count",
   "L", "Receive Pri#0 Pause 1US Count",
   "L", "Receive Pri#1 Pause 1US Count",
   "L", "Receive Pri#2 Pause 1US Count",
   "L", "Receive Pri#3 Pause 1US Count",
   "L", "Receive Pri#4 Pause 1US Count",
   "L", "Receive Pri#5 Pause 1US Count",
   "L", "Receive Pri#6 Pause 1US Count",
   "L", "Receive Pri#7 Pause 1US Count",
   "L", "Receive Standard Pause 1US Count",
   "L", "Frames Truncated",
   "L", "Reserved"
   };

   for (int i = 0; i < MAX_MAC_STATS; i += 1) {
      string report_type = stats[i*2];
      if (short_report == 1 && (report_type.compare("L") == 0)) continue;

      int addr = ((ch&0x3) << 7) | i;
      mx_csr.dhs_mac_stats.entry[addr].read();
      cpp_int rdata = mx_csr.dhs_mac_stats.entry[addr].value();
      if (stats_data != NULL) {
          stats_data[i] = mx_csr.dhs_mac_stats.entry[addr].value().convert_to<uint64_t>();
      }
      string counter_name = stats[i*2+1];
      PLOG_MSG( "MX" << inst_id << " Channel" << ch << ": " << counter_name.append(50-counter_name.length(),' ') << " : " << rdata << "\n" );
   }
#endif
}

int
cap_mx_send_remote_faults (int chip_id, int inst_id, int mac_ch,
                           bool send)
{
#if 0
// TBD-ELBA-REBASE
   int addr = 0;

    // MAC txdebug config  bit 2
   addr = (mac_ch == 1) ? 0x591 : (mac_ch == 2) ? 0x691 : (mac_ch == 3) ? 0x791 : 0x491;
   cap_mx_set_bit(chip_id, inst_id, addr, 2, send);
#endif
   return 0;
}

void
cap_mx_serdes_lpbk_set (int chip_id, int inst_id, int ch, int value)
{
#if 0
// TBD-ELBA-REBASE
    int addr = 0x1901;
    int data = cap_mx_apb_read(chip_id, inst_id, addr);
    int mask = (1 << ch) & 0xf;
    data = (data & ~mask) | ( (value & 0x1) << ch);
    cap_mx_apb_write(chip_id, inst_id, addr, data);
#endif
}

void cap_mx_set_ch_enable(int chip_id, int inst_id, int ch, int value) { 
#if 0
// TBD-ELBA-REBASE
    // channel enable: {ch3, ch2, ch1, ch0}
    int rdata = cap_mx_apb_read(chip_id, inst_id, 0x4);
    int mask = ((1 << ch) ^ 0xf) & 0xf;
    int wdata = (rdata & mask) | ((value << ch) & 0xf);
    cap_mx_apb_write(chip_id, inst_id, 0x4, wdata);
#endif
}

void cap_mx_set_fec(int chip_id, int inst_id, int ch, int value) {
#if 0
// TBD-ELBA-REBASE

    /* RS FEC: 0x2
     * FC FEC: 0x1
     * NO FEC: 0x0
     */

    PLOG_MSG("cap_mx_set_fec :: mx" << inst_id << " channel " << ch << ": set fec " << value << endl);
    cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);

    int addr = (ch == 1) ? 0x910 : (ch == 2) ? 0xa10 : (ch == 3) ? 0xb10 : 0x810;
    int data = cap_mx_apb_read(chip_id, inst_id, addr);
    data = data & 0xfffc;
    int global_mode = cap_mx_apb_read(chip_id, inst_id, 0x1);

    // int fec = (global_mode == 4) ? 0x1 :  // 1X40/50G, FEC = FC
    //    0x2;  // 1X100G, 2X40/50G, 4X10/25G

    data = data | value;
    cap_mx_apb_write(chip_id, inst_id, addr, data);

    //25G RS FEC IEEE mode - From Comira spreadsheet
    if(global_mode == 8 && value == 0x2 /* RS FEC */) {
        //0x10FD,30fd,31fd,32fd => 16'h59F0
        addr = (ch == 1) ? 0x30fd : (ch == 2) ? 0x31fd : (ch == 3) ? 0x32fd : 0x10fd;
        cap_mx_apb_write(chip_id, inst_id, addr, 0x59f0);

        //0x0850,950,a50,b50 => 0x3
        addr = (ch == 1) ? 0x950 : (ch == 2) ? 0xa50 : (ch == 3) ? 0xb50 : 0x850;
        cap_mx_apb_write(chip_id, inst_id, addr, 0x3);

        //0x0863-0x085C => {0x337B,0x954D, 0x33E8,0x4B59, 0x338E,0x719D, 0x3321,0x68C1}
        cap_mx_apb_write(chip_id, inst_id, 0x85c, 0x68C1);
        cap_mx_apb_write(chip_id, inst_id, 0x85d, 0x3321);
        cap_mx_apb_write(chip_id, inst_id, 0x85e, 0x719D);
        cap_mx_apb_write(chip_id, inst_id, 0x85f, 0x338E);
        cap_mx_apb_write(chip_id, inst_id, 0x860, 0x4B59);
        cap_mx_apb_write(chip_id, inst_id, 0x861, 0x33E8);
        cap_mx_apb_write(chip_id, inst_id, 0x862, 0x954D);
        cap_mx_apb_write(chip_id, inst_id, 0x863, 0x337B);
    } else {
        // Below settings are the defaults when NOT in 25G RS FEC mode
        // Defaults needs to be set when resetting 25G RS FEC mode

        //0x10FD,30fd,31fd,32fd => 16'h99F0
        addr = (ch == 1) ? 0x30fd : (ch == 2) ? 0x31fd : (ch == 3) ? 0x32fd : 0x10fd;
        cap_mx_apb_write(chip_id, inst_id, addr, 0x99f0);

        //0x0850,950,a50,b50 => 0x2
        addr = (ch == 1) ? 0x950 : (ch == 2) ? 0xa50 : (ch == 3) ? 0xb50 : 0x850;
        cap_mx_apb_write(chip_id, inst_id, addr, 0x2);

        //0x0863-0x085C => {0x337B,0x954D, 0x33E8,0x4B59, 0x338E,0x719D, 0x3321,0x68C1}
        cap_mx_apb_write(chip_id, inst_id, 0x85c, 0x0);
        cap_mx_apb_write(chip_id, inst_id, 0x85d, 0x0);
        cap_mx_apb_write(chip_id, inst_id, 0x85e, 0x0);
        cap_mx_apb_write(chip_id, inst_id, 0x85f, 0x0);
        cap_mx_apb_write(chip_id, inst_id, 0x860, 0x0);
        cap_mx_apb_write(chip_id, inst_id, 0x861, 0x0);
        cap_mx_apb_write(chip_id, inst_id, 0x862, 0x0);
        cap_mx_apb_write(chip_id, inst_id, 0x863, 0x0);
    }

    addr = (ch == 1) ? 0x3000 : (ch == 2) ? 0x3100 : (ch == 3) ? 0x3200 : 0x1000;
    data = cap_mx_apb_read(chip_id, inst_id, addr);
    data = (value)? (data & 0xfffd) : (data | 0x2);
    cap_mx_apb_write(chip_id, inst_id, addr, data);

    mx_csr.cfg_mac_gbl.read();
    mx_csr.cfg_mac_gbl.cg_fec_enable_i(1);
    mx_csr.cfg_mac_gbl.write();

    if (value == 1) {
        // Don't enable programmable alignment marker length
        addr = (ch == 1) ? 0x9ff : (ch == 2) ? 0xaff : (ch == 3) ? 0xbff : 0x8ff;
        cap_mx_apb_write(chip_id, inst_id, addr, 0x1E01);
        mx[inst_id].for_simulation = 0;
    }

    // If the mac is already out of reset, toggle soft reset after FEC enable
    data = cap_mx_apb_read(chip_id, inst_id, 0x5);
    int mask = 1 << ch;
    if ((data & mask) == 0) {
        //int data_new = data | mask;
        //cap_mx_apb_write(chip_id, inst_id, 0x5, data_new);
        //cap_mx_apb_write(chip_id, inst_id, 0x5, data);
        cap_mx_set_soft_reset(chip_id, inst_id, ch, 1);
        cap_mx_set_soft_reset(chip_id, inst_id, ch, 0);
    }
#endif
}

void cap_mx_set_mtu(int chip_id, int inst_id, int ch, int speed, int max_value) {
#if 0
// TBD-ELBA-REBASE
    switch (speed) {
    case 100:
        cap_mx_set_mtu_jabber(chip_id, inst_id, ch, max_value, max_value+16);
        break;

    case 50:
    case 40:
        cap_mx_set_mtu_jabber(chip_id, inst_id, ch, max_value, max_value+8);
        break;

    default:
        cap_mx_set_mtu_jabber(chip_id, inst_id, ch, max_value, max_value+4);
        break;
    }
#endif
}

int
cap_mx_set_pause(int chip_id, int inst_id, int ch, int pri_vec, int legacy,
                 bool tx_pause_enable, bool rx_pause_enable)
{
#if 0
// TBD-ELBA-REBASE
    cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);

    int txfcxoff_enable  = 0;
    int txpfcxoff_enable = 0;
    uint32_t data = 0x0;

    int addr1 = (ch == 1) ? 0x501 : (ch == 2) ? 0x601 : (ch == 3) ? 0x701 : 0x401;
    int addr2 = (ch == 1) ? 0x502 : (ch == 2) ? 0x602 : (ch == 3) ? 0x702 : 0x402;
    int addr3 = (ch == 1) ? 0x506 : (ch == 2) ? 0x606 : (ch == 3) ? 0x706 : 0x406;

    if (pri_vec == 0) {
        // No pause
        cap_mx_apb_write(chip_id, inst_id, addr1, 0x030);
        cap_mx_set_bit(chip_id, inst_id, addr2, 4, 1);  // bit4 Promiscuous Mode (1: disable MAC address check)
        cap_mx_apb_write(chip_id, inst_id, addr3, 0x00);   // MAC Tx Priority Pause Vector
    } else {
        if (legacy) {
            // Link level pause
            txfcxoff_enable = 1;

            data = 0x30;
            // Bit8: Control Frame Generation Enable
            if (tx_pause_enable == true) {
                data = set_bit(data, 8);
            } else {
                data = reset_bit(data, 8);
            }
            cap_mx_apb_write(chip_id, inst_id, addr1, data);
            cap_mx_set_bit(chip_id, inst_id, addr2, 4, 1);
            cap_mx_set_bit(chip_id, inst_id, addr2, 8, 1);
            // bit5: Enable Rx Flow Control Decode
            cap_mx_set_bit(chip_id, inst_id, addr2, 5, rx_pause_enable);
            cap_mx_apb_write(chip_id, inst_id, addr3, 0x00);   // MAC Tx Priority Pause Vector
        } else {
            // PFC
            txpfcxoff_enable = 0xff;

            data = 0x30;
            // bit9: Priority Flow Control Generation Enable
            if (tx_pause_enable == true) {
                data = set_bit(data, 9);
            } else {
                data = reset_bit(data, 9);
            }
            cap_mx_apb_write(chip_id, inst_id, addr1, data);
            cap_mx_set_bit(chip_id, inst_id, addr2, 4, 1);
            cap_mx_set_bit(chip_id, inst_id, addr2, 8, 1);
            // bit5: Enable Rx Flow Control Decode
            cap_mx_set_bit(chip_id, inst_id, addr2, 5, rx_pause_enable);
            cap_mx_apb_write(chip_id, inst_id, addr3, pri_vec); // MAC Tx Priority Pause Vector
        }
    }

    /*
     * MAC channel to PB port mapping
     * inst: 0, ch: 0: PB port 0
     * inst: 0, ch: 1: PB port 2
     * inst: 0, ch: 2: PB port 4
     * inst: 0, ch: 3: PB port 5
     * inst: 1, ch: 0: PB port 1
     * inst: 1, ch: 1: PB port 3
     * inst: 1, ch: 2: PB port 6
     * inst: 1, ch: 3: PB port 7
     *
     * MODE         MAC_CH      MAC_PORT
     * 1x100G           0          0
     * 1x40G            0          0
     * 2x50G            0,2        0,1
     * 4x1G/10G/25G     0,1,2,3    0,1,2,3
     */

    switch(mx[inst_id].mac_mode) {
    case MAC_MODE_1x100g:
    case MAC_MODE_1x40g:
        mx_csr.cfg_mac_xoff.ff_tx0fcxoff_i(txfcxoff_enable);
        mx_csr.cfg_mac_xoff.ff_tx0pfcxoff_i(txpfcxoff_enable);
        break;

    case MAC_MODE_1x50g:
        if (ch == 0) {
            mx_csr.cfg_mac_xoff.ff_tx0fcxoff_i(txfcxoff_enable);
            mx_csr.cfg_mac_xoff.ff_tx0pfcxoff_i(txpfcxoff_enable);
        } else {
            mx_csr.cfg_mac_xoff.ff_tx1fcxoff_i(txfcxoff_enable);
            mx_csr.cfg_mac_xoff.ff_tx1pfcxoff_i(txpfcxoff_enable);
        }
        break;

    case MAC_MODE_4x25g:
    case MAC_MODE_4x10g:
    case MAC_MODE_4x1g:
        switch (ch) {
        case 1:
            mx_csr.cfg_mac_xoff.ff_tx1fcxoff_i(txfcxoff_enable);
            mx_csr.cfg_mac_xoff.ff_tx1pfcxoff_i(txpfcxoff_enable);
            break;

        case 2:
            mx_csr.cfg_mac_xoff.ff_tx2fcxoff_i(txfcxoff_enable);
            mx_csr.cfg_mac_xoff.ff_tx2pfcxoff_i(txpfcxoff_enable);
            break;

        case 3:
            mx_csr.cfg_mac_xoff.ff_tx3fcxoff_i(txfcxoff_enable);
            mx_csr.cfg_mac_xoff.ff_tx3pfcxoff_i(txpfcxoff_enable);
            break;

        case 0:
        default:
            mx_csr.cfg_mac_xoff.ff_tx0fcxoff_i(txfcxoff_enable);
            mx_csr.cfg_mac_xoff.ff_tx0pfcxoff_i(txpfcxoff_enable);
            break;
        }
        break;

    default:
        break;
    }

    mx_csr.cfg_mac_xoff.write();
#endif
    return 0;
}


int cap_mx_set_pause_src_addr (int chip_id, int inst_id, int ch, uint8_t *mac_addr)
{
#if 0
// TBD-ELBA-REBASE
    int addr = (ch == 1) ? 0x515 : (ch == 2) ? 0x615 : (ch == 3) ? 0x715 : 0x415;

    cap_mx_apb_write(chip_id, inst_id, addr,   mac_addr[0] | (mac_addr[1] << 8));
    cap_mx_apb_write(chip_id, inst_id, addr+1, mac_addr[2] | (mac_addr[3] << 8));
    cap_mx_apb_write(chip_id, inst_id, addr+2, mac_addr[4] | (mac_addr[5] << 8));
#endif
    return 0;
}

void cap_mx_set_rx_padding(int chip_id, int inst_id, int ch, bool enable) {
#if 0
// TBD-ELBA-REBASE
 //cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
 CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
 int addr = 0x207 + ch;
 int rdata = cap_mx_apb_read(chip_id, inst_id, addr);
 if(enable) {
   rdata |= (0x1<<5);
 } else {
   rdata &= (0xffdf);
 }
 cap_mx_apb_write(chip_id, inst_id, addr, rdata);
#endif
}

void cap_mx_soft_reset(int chip_id, int inst_id) {
#if 0
// TBD-ELBA-REBASE
   cap_mx_set_soft_reset(chip_id, inst_id, 0xf);
#endif
}

void cap_mx_set_soft_reset(int chip_id, int inst_id, int value) { 
#if 0
// TBD-ELBA-REBASE
    // Software reset for stats: {ch3, ch2, ch1, ch0}
    cap_mx_apb_write(chip_id, inst_id, 0x300, value);

    // umac software reset: {ch3, ch2, ch1, ch0}
    int global_mode = cap_mx_apb_read(chip_id, inst_id, 0x1);
    // In 2X50G mode, comira fec_rs_dec_top_p1 use soft reset 0 and 1 instead of 0 and 2.
    if (global_mode == 6) {
       if ((value & 0x4) != 0) {
          value = value | 0x6;
       } else {
          value = value & 0x9;
       }
    }
    cap_mx_apb_write(chip_id, inst_id, 0x5, value);
#endif
}

void cap_mx_set_tx_padding(int chip_id, int inst_id, int enable) {
#if 0
// TBD-ELBA-REBASE
   cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
   PLOG_MSG( "MX inst_id:" << inst_id << " Entered: " << __func__ << endl)
   mx_csr.cfg_mac_gbl.read();
   mx_csr.cfg_mac_gbl.ff_txdispad_i(enable);
   mx_csr.cfg_mac_gbl.write();
#endif
}

int
cap_mx_set_vlan_check (int chip_id, int inst_id, int mac_ch,
                       int num_tags, uint32_t tag1, uint32_t tag2,
                       uint32_t tag3)
{
#if 0
// TBD-ELBA-REBASE
    int addr = (mac_ch == 1) ? 0x502 : (mac_ch == 2) ? 0x602 : (mac_ch == 3) ? 0x702 : 0x402;

    switch(num_tags) {
    case 1:
        cap_mx_set_bits(chip_id, inst_id, addr, 2, 2, 0x1);
        break;
    case 2:
        cap_mx_set_bits(chip_id, inst_id, addr, 2, 2, 0x2);
        break;
    case 3:
        cap_mx_set_bits(chip_id, inst_id, addr, 2, 2, 0x3);
        break;
    case 0:
    default:
        cap_mx_set_bits(chip_id, inst_id, addr, 2, 2, 0x0);
        break;
    }

    // always program tags. MAC Receive config controls which tags to check
    addr = (mac_ch == 1) ? 0x507 : (mac_ch == 2) ? 0x607 : (mac_ch == 3) ? 0x707 : 0x407;
    cap_mx_apb_write(chip_id, inst_id, addr, tag1);
    addr = (mac_ch == 1) ? 0x508 : (mac_ch == 2) ? 0x608 : (mac_ch == 3) ? 0x708 : 0x408;
    cap_mx_apb_write(chip_id, inst_id, addr, tag2);
    addr = (mac_ch == 1) ? 0x509 : (mac_ch == 2) ? 0x609 : (mac_ch == 3) ? 0x709 : 0x409;
    cap_mx_apb_write(chip_id, inst_id, addr, tag3);
#endif
    return 0;
}


void cap_mx_stats_reset(int chip_id, int inst_id, int ch, int value) {
#if 0
// TBD-ELBA-REBASE
    int rdata;
    int wdata;
    int mask = ((1 << ch) ^ 0xf) & 0xf;

    // Software reset for stats: {ch3, ch2, ch1, ch0}
    rdata = cap_mx_apb_read(chip_id, inst_id, 0x300);
    wdata = (rdata & mask) | ((value << ch) & 0xf);
    cap_mx_apb_write(chip_id, inst_id, 0x300, wdata);
#endif
}


int
cap_mx_tx_drain (int chip_id, int inst_id, int mac_ch, bool drain)
{
#if 0
// TBD-ELBA-REBASE
   int addr = 0;

    // MAC control bit 5
   addr = (mac_ch == 1) ? 0x500 : (mac_ch == 2) ? 0x600 : (mac_ch == 3) ? 0x700 : 0x400;
   cap_mx_set_bit(chip_id, inst_id, addr, 5, drain);
   cap_mx_apb_read(chip_id, inst_id, addr);

   cap_mx_set_bit(chip_id, inst_id, addr, 0, drain == true? false : true);
   cap_mx_apb_read(chip_id, inst_id, addr);

   cap_mx_set_bit(chip_id, inst_id, addr, 1, drain == true? false : true);
   cap_mx_apb_read(chip_id, inst_id, addr);

    // MAC transmit config  bit 15
   addr = (mac_ch == 1) ? 0x501 : (mac_ch == 2) ? 0x601 : (mac_ch == 3) ? 0x701 : 0x401;
   cap_mx_set_bit(chip_id, inst_id, addr, 15, drain);
   cap_mx_apb_read(chip_id, inst_id, addr);

    // MAC txdebug config  bit 6
   addr = (mac_ch == 1) ? 0x591 : (mac_ch == 2) ? 0x691 : (mac_ch == 3) ? 0x791 : 0x491;
   cap_mx_set_bit(chip_id, inst_id, addr, 6, drain);
   cap_mx_apb_read(chip_id, inst_id, addr);
#endif
   return 0;
}

void cap_mx_set_soft_reset(int chip_id, int inst_id, int ch, int value) { 
#if 0
// TBD-ELBA-REBASE
    int rdata;
    int wdata;
    int mask = ((1 << ch) ^ 0xf) & 0xf;

    cap_mx_stats_reset(chip_id, inst_id, ch, value);

    // umac software reset: {ch3, ch2, ch1, ch0}
    rdata = cap_mx_apb_read(chip_id, inst_id, 0x5);
    int global_mode = cap_mx_apb_read(chip_id, inst_id, 0x1);
    // In 2X50G mode, comira fec_rs_dec_top_p1 use soft reset 0 and 1 instead of 0 and 2.
    if (global_mode == 6 && ch == 2) {
       if (value == 1) {
          wdata = (rdata & 0x9) | 0x6;
       } else {
          wdata = (rdata & 0x9);
       }
    } else {
       wdata = (rdata & mask) | ((value << ch) & 0xf);
    }
    cap_mx_apb_write(chip_id, inst_id, 0x5, wdata);
#endif
}

int cap_mx_apb_read(int chip_id, int inst_id, int addr) {
#if 0
// TBD-ELBA-REBASE
 cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
    mx_csr.dhs_apb.entry[addr].read();
    int rdata = mx_csr.dhs_apb.entry[addr].data().convert_to<int>();
    return rdata;
#else
    return 0;
#endif
}

void cap_mx_apb_write(int chip_id, int inst_id, int addr, int data) {
#if 0
// TBD-ELBA-REBASE
 cap_mx_csr_t & mx_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mx_csr_t, chip_id, inst_id);
    mx_csr.dhs_apb.entry[addr].data(data);
    mx_csr.dhs_apb.entry[addr].write();
    // mx_csr.dhs_apb.entry[addr].show();
#endif
}
