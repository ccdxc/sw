#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <cstdint>
#include "include/sdk/types.hpp"
#include "lib/catalog/catalog.hpp"
#include "linkmgr/linkmgr_internal.hpp"
#include "include/sdk/asic/elba/elb_mx_common_api.h"
#include "third-party/asic/elba/model/elb_mx/elb_mx_csr.h"
#include "third-party/asic/elba/model/elb_bx/elb_bx_csr.h"
#include "elb_blk_reg_model.h"

#ifndef MX_DEBUG_MSG

#define MX_DEBUG_INFO(...)
#define MX_DEBUG_MSG  SDK_LINKMGR_TRACE_DEBUG
#define MX_DEBUG_ERR  SDK_LINKMGR_TRACE_ERR

#endif

// Statistic, Status, Interrupt descriptions used to display in api functions
// 1st column is a 5-bit bit bitmap where each bit indicates type of the counter such as error count or pause count. 
// Meaning of the bits is documented in mx_mac_stat_type_t. A counter may belong to more than 1 type.
string elb_mx_mac_stat_info[MAX_MAC_STATS*3] = {
"1", "32", 	"FramesXmitOK",					// 0
"1", "33",	"FramesXmitAll",				// 1
"3", "34",	"FramesXmitErr",				// 2
"1", "35",	"OctetsXmitOK",					// 3 
"1", "36",	"OctetsXmitAll",				// 4
"1", "37",	"FramesXmitUnicast",				// 5 
"1", "38",	"FramesXmitMulticast",				// 6 
"1", "39",	"FramesXmitBroadcast",				// 7 
"5", "40",	"FramesXmitPause",				// 8
"5", "41",	"FramesXmitPriPause",				// 9 
"1", "42",	"FramesXmitVLAN",				// 10
"3", "43",	"FramesXmitSizeLT64",				// 11 
"1", "44",	"FramesXmitSizeEQ64",				// 12 
"1", "45",	"FramesXmitSize65to127",			// 13
"1", "46",	"FramesXmitSize128to255",			// 14 
"1", "47",	"FramesXmitSize256to511",			// 15
"1", "48",	"FramesXmitSize512to1023",			// 16
"1", "49",	"FramesXmitSize1024to1518",			// 17
"1", "50",	"FramesXmitSize1519to2047",			// 18
"1", "51",	"FramesXmitSize2048to4095",			// 19
"1", "52",	"FramesXmitSize4096to8191",			// 20
"1", "53",	"FramesSmitSize8912to9215",			// 21
"3", "54",	"FramesXmitSizeGT9216",				// 22 
"5", "55",	"FramesXmitPri0",				// 23 
"5", "56",	"FramesXmitPri1",				// 24 
"5", "57",	"FramesXmitPri2",				// 25 
"5", "58",	"FramesXmitPri3",				// 26 
"5", "59",	"FramesXmitPri4",				// 27 
"5", "60",	"FramesXmitPri5",				// 28 
"5", "61",	"FramesXmitPri6",				// 29 
"5", "62",	"FramesXmitPri7",				// 30
"5", "71",	"XmitPri0Pause1US",				// 31 
"5", "72",	"XmitPri1Pause1US",				// 32 
"5", "73",	"XmitPri2Pause1US",				// 33 
"5", "74",	"XmitPri3Pause1US",				// 34 
"5", "75",	"XmitPri4Pause1US",				// 35 
"5", "76",	"XmitPri5Pause1US",				// 36 
"5", "77",	"XmitPri6Pause1US",				// 37 
"5", "78",	"XmitPri7Pause1US",				// 38 
"17","NEW", 	"FramesXmitDrained",				// 39 
"3","NEW",	"FramesXmitJabbered",				// 40
"1","NEW",	"FramesXmitPadded",				// 41 
"3", "88",	"FramesXmitTruncated",				// 42 
"1",  "0", 	"FramesRcvdOK",					// 43 
"1",  "4",	"OctetsRcvdOK",					// 44 
"1",  "1",	"FramesRcvdAll",				// 45
"1",  "5",	"OctetsRcvdAll",				// 46
"3",  "2",	"FramesRcvdCRCErr",				// 47 
"3",  "3",	"FramesRcvdErr",				// 48
"1",  "6",	"FramesRcvdUnicast",				// 49 
"1",  "7",	"FramesRcvdMulticast",				// 50 
"1",  "8",	"FramesRcvdBroadcast",				// 51 
"5",  "9",	"FramesRcvdPause",				// 52
"3", "10",	"FramesRcvdLenErr",				// 53 
"1","NEW", 	"Reserved",					// 54 
"3", "12",	"FramesRcvdOversized",				// 55 
"3", "13",	"FramesRcvdFragments",				// 56 
"3", "14",	"FramesRcvdJabber",				// 57 
"5", "15",	"FramesRcvdPriPause",				// 58 
"3", "16",	"FramesRcvdCrcErrStomp",			// 59
"3", "17",	"FramesRcvdMaxFrmSizeVio",			// 60
"1", "18",	"FramesRcvdVLAN",				// 61 
"1","NEW",	"Reserved",					// 62 
"3", "20",	"FramesRcvdSizeLT64",				// 63 
"1", "21",	"FramesRcvdSizeEQ64",				// 64 
"1", "22",	"FramesRcvdSize65to127",			// 65
"1", "23",	"FramesRcvdSize128to255",			// 66
"1", "24",	"FramesRcvdSize256to511",			// 67
"1", "25",	"FramesRcvdSize512to1023",			// 68
"1", "26",	"FramesRcvdSize1024to1518",			// 69 
"1", "27",	"FramesRcvdSize1419to2047",			// 70 
"1", "28",	"FramesRcvdSize2048to4095",			// 71 
"1", "29",	"FramesRcvdSize4096to8191",			// 72 
"1", "30",	"FramesRcvdSize8912to9215",			// 73 
"3", "31",	"FramesRcvdSizeGT9216",				// 74 
"5", "63",	"FramesRcvdPri0",				// 75
"5", "64",	"FramesRcvdPri1",				// 76
"5", "65",	"FramesRcvdPri2",				// 77
"5", "66",	"FramesRcvdPri3",				// 78
"5", "67",	"FramesRcvdPri4",				// 79
"5", "68",	"FramesRcvdPri5",				// 80
"5", "69",	"FramesRcvdPri6",				// 81
"5", "70",	"FramesRcvdPri7",				// 82
"5", "79",	"RcvdPri0Pause1US",				// 83 
"5", "80",	"RcvdPri1Pause1US",				// 84 
"5", "81",	"RcvdPri2Pause1US",				// 85 
"5", "82",	"RcvdPri3Pause1US",				// 86 
"5", "83",	"RcvdPri4Pause1US",				// 87 
"5", "84",	"RcvdPri5Pause1US",				// 88 
"5", "85",	"RcvdPri6Pause1US",				// 89 
"5", "86",	"RcvdPri7Pause1US",				// 90
"5", "87",	"RcvdStdPause1US",				// 91
"1","NEW",	"Reserved",					// 92 
"1","NEW",	"Reserved",					// 93 
"3","NEW",	"InvalidPreamble",				// 94
"3","NEW",	"NormalLenInvalidCRC",				// 95
"11","NEW",	"HSMC_PCS BER Err Counter",			// 96 
"11","NEW",	"HSMC_PCS Err Blocks Counter",			// 97
"11","NEW",	"HSMC_PCS Valid_Err_Block_Counter",		// 98 
"11","NEW",	"HSMC_PCS Unknown Err Block_Counter",		// 99 
"11","NEW",	"HSMC_PCS Invalid_Err_Block_Counter",		// 100
"11","NEW",	"HSMC_PCS Test Pattern Err Counter",		// 101
"11","NEW",	"LSMC_PCS Sync Loss Counter",			// 102
"11","NEW",	"HSMC_PCS Block Lock Loss Counter",		// 103
"11","NEW",	"HSMC_PCS HiBER Counter",			// 104
"11","NEW",	"LSMC_PCS Invalid Code Counter",		// 105
"11","NEW",	"LSMC_PCS Disparity Err Counter",		// 106
"11","NEW",	"RSFEC Correctable CodeWords",			// 107
"11","NEW",	"RSFEC Uncorrectable CodeWords",		// 108
"11","NEW",	"RSFEC CH Symbol Err Counter"			// 109
};

// Status/Interrupt
// 1st column tells a meaning of the status value. 
// '0': 0 is expected good status value, '1': 1 is expected good status value, 'S': the status has no good/bad value, '?': 1 may be bad value.
//
string elb_mx_mac_chsts_info[15*2] = {
"1",	"txclkpresentall   : Aggregated serdes txclk vld statuses",			// 0
"1",	"rxclkpresentall   : Aggregated serdes rxclk vld statuses",			// 1
"1",	"rxsigokall        : Aggregated serdes rx sigok statuses",			// 2
"1",	"blocklockall      : Block lock status",					// 3
"1",	"amlockall         : AM Lock status",						// 4
"1",	"aligned           : deskew lock status",					// 5
"1",	"nohiber           : HiBER status",						// 6
"1",	"nolocalfault      : Link fault state status",					// 7
"1",	"noremotefault     : Link fault state status",					// 8
"1",	"linkup            : Derived from blocklockall, amlockall and aligned",		// 9
"0",	"hiser             : RSFEC HiSER status. This is only active when RSFEC err indication is bypassed which is uncommon",	// 10
"0",	"fecdegser         : Degraded SER status of the local RX PCS defined in Clause 119.2.5.3",	// 11
"0",	"rx_remote_degrade : A status received from link partner in rx_am_sf[2] defined in Clause 119.2.6.2.2. This is link partner's fecdegser status",	// 12
"0",	"rx_local_degrade  : A status received from link partner in rx_am_sf[1] defined in Clause 119.2.6.2.2. This is only used with devices that include clause 118 extender sublayers",	// 13
"0",	"Reserved"									// 14
};

string elb_mx_an_techability_info[23*2] = {
"S",	"KX 1G",				// 0
"S",	"KX4 10G",				// 1
"S",	"KR 10G",				// 2
"S",	"KR4 40G",				// 3
"S",	"CR4 40G",				// 4
"S",	"CR10 100G",				// 5
"S",	"KP4 100G",				// 6
"S",	"KP4 100G",				// 7
"S",	"CR4 100G",				// 8
"S",	"KRS/CRS 25G",				// 9
"S",	"KR/CR 25G",				// 10
"S",	"KX 2.5G",				// 11
"S",	"KR 5G",				// 12
"S",	"KR/CR 50G",				// 13
"S",	"KR2/CR2 100G",				// 14
"S",	"KR4/CR4 200G",				// 15
"S",	"KR/CR 100G",				// 16
"S",	"KR2/CR2 200G",				// 17
"S",	"KR4/CR4 400G",				// 18
"S",	"Reserved",				// 19
"S",	"Reserved",				// 20
"S",	"Reserved",				// 21
"S",	"Reserved",				// 22
};

string elb_mx_an_fec_info[4*2] = {
"S",	"25G RS-FEC requested",			// 0
"S",	"25G BASE-R FEC requested",		// 1
"S",	"10Gb/s per lane FEC ability",		// 2
"S",	"10Gb/s per lane FEC requested",	// 3
};

string elb_mx_an_pause_info[2*2] = {
"S",	"Pause is supported",			// 0
"S",	"Pause is asynchronous",		// 1
};

string elb_mx_an_ansts_info[40*2] = {
"S",	"lpanenable      : 0: Negotiation has not begun, 1: Negotiation has begun",	// 0
"S",	"ancomplete      : (ERRATA set after base page) Autonegotiation complete",					// 1
"S",	"receivedlast    : (ERRATA set after base page) Recived last page",						// 2
"S",	"anmode[0]       : 1000BASE-KX",						// 3
"S",	"anmode[1]       : 2.5GBASE-KX",						// 4
"S",	"anmode[2]       : 5GBASE-KR",							// 5
"S",	"anmode[3]       : 10GBASE-KX4",						// 6
"S",	"anmode[4]       : 10GBASE-KR",							// 7
"S",	"anmode[5]       : 25GBASE-KR1 or 25GBASE-CR1 (Consortium)",			// 8
"S",	"anmode[6]       : 25GBASE-KR-S or 25GBASE-CR-S",				// 9
"S",	"anmode[7]       : 25GBASE-KR or 25GBASE-CR",					// 10
"S",	"anmode[8]       : 40GBASE-KR4",						// 11
"S",	"anmode[9]       : 40GBASE-CR4",						// 12
"S",	"anmode[10]      : 50GBASE-KR2 or 50GBASE-CR2 (Consortium)",			// 13
"S",	"anmode[11]      : 50GBASE-KR or 50GBASE-CR",					// 14
"S",	"anmode[12]      : 100GBASE-CR10",						// 15
"S",	"anmode[13]      : 100GBASE-KP4",						// 16
"S",	"anmode[14]      : 100GBASE-KR4",						// 17
"S",	"anmode[15]      : 100GBASE-CR4",						// 18
"S",	"anmode[16]      : 100GBASE-CR2 or 100GBASE-KR2",				// 19
"S",	"anmode[17]      : 100GBASE-CR or 100GBASE-KR",					// 20
"S",	"anmode[18]      : 200GBASE-CR4 or 200GBASE-KR4",				// 21
"S",	"anmode[19]      : 200GBASE-CR2 or 200GBSAE-KR2",				// 22
"S",	"anmode[20]      : 400GBASE-CR4 or 400GBASE-KR4",				// 23
"S",	"anmode[21]      : Reserved",							// 24
"S",	"anmode[22]      : Reserved",							// 25
"S",	"anmode[23]      : Reserved",							// 26
"S",	"anmode[24]      : Reserved",							// 27
"S",	"fecmode[0]      : fcfec",							// 28
"S",	"fecmode[1]      : rsfec",							// 29
"S",	"pause[0]        : pause is supported",						// 30 
"S",	"pause[1]        : pause is asynchronous",					// 31
"S",	"abilitymatch    : (ERRATA live instead of stricky signal) 3 consective link codewords match, ignoring ack bit",	// 32
"S",	"ackmatch        : (ERRATA live instead of stricky signal) 3 consective link codewords match and have ack bit set",	// 33
"S",	"rxnp            : received a new next page from link partner",			// 34
"S",	"txdone          : (ERRATA set before sending next page) finished transmitting next pages",				// 35
"S",	"txbp            : transmitting base page",					// 36
"S",	"noncecollision  : tx nonce received from link partner matches ours",		// 37
"S",	"maxtimerdone    : link partner is sending page too slow > pagemaxtimer",	// 38
"S",	"mintimernotdone : link partner is sending page too fast < pagemintimer",	// 39
};

string elb_mx_mac_int_info[16*2] = {
"0",	"TX FIFO LEVEL   : TX Application FIFO Overflow, TX Packet Underflow, Tx Packing Overflow", 	  // 0
"0",	"TX PROTOCOL ERR : TX Protocol Violation on SOF/EOF/VLD Input to Application FIFO", 		  // 1
"0",	"TX JABBER PKT   : TX Packet Jabbered",								  // 2
"0",	"TX SERDES FIFO  : TX SERDES Buffer Overflow, TX SERDES Buffer Underflow",			  // 3
"0",	"RX PCS GEARBOX  : RX PCS Gearbox Overflow",							  // 4
"0",	"RX PCS DESKEW   : RX PCS Deskew Overflow",							  // 5
"0",	"RX PCS HIBER    : Rx PCS hiber, hiser err triggered",					  	  // 6
"0",	"RX PCS ERR      : RX PCS Sync Header Err, Block Err, Codeword Err or BIP Err",		  	  // 7
"0",	"RX FAULT        : RX  Fault (Ordered Set) Receievd (or Generated by RX PCS)",			  // 8
"0",	"RX FRAME DROP   : Frame Dropped due to bad SFD/PREAMBLE",					  // 9
"0",	"RX FCS ERR      : Invalid FCS on Received Packet or PCS Err character found inside of packet",   // 10
"0",	"RX JABBER PKT   : Rx Packet Jabbered",								  // 11
"0",	"RX RUNT DROP    : Rx Runt Packet Filtered/Dropped",						  // 12
"0",	"RX FIFO LEVEL   : RX Application FIFO Overflow, RX unpacking Overflow",			  // 13
"0",	"RX LINK LOST    : Link Lost",									  // 14
"1",	"RX LINK GAIN    : Link Gain" };								  // 15

string elb_mx_an_int_info[4*2] = {
"S",	"AN resolved",								// 0
"?",	"Breaklinktimer is being reset",					// 1
"0",	"Err interrupt: noncecollision/maxtimerdone/mintimernotdone",		// 2
"0",	"Reserved" };								// 3

string elb_mx_int_groups_info[5*2] = {
"?",	"int_sd",		// 0
"?",	"int_mac",		// 1
"0",	"int_ecc",		// 2
"0",	"int_fixer",		// 3
"?",	"int_flush"		// 4
};

string elb_mx_int_sd_info[1*2] = {
"?",	"pmd_micro_ext_intr",	// 0
};

string elb_mx_int_mac_info[2*2] = {
"?",	"mac intr",		// 0
"?",	"an intr"		// 1
};

string elb_mx_int_ecc_info[2*4] = {
"0",	"txfifo/statsmem/rsfec/deskew memory uncorrectable ecc err",		// 0
"S",	"txfifo/statsmem/rsfec/deskew memory correctable ecc err",		// 1
"0",	"an/rxfifo memory uncorrectable ecc err",				// 2
"S",	"an/rxfifo memory correctable ecc err"					// 3
};

string elb_mx_int_fixer_info[2*10] = {
"0",	"port0 missing eof err",	// 0
"0",	"port0 timeout err",		// 1
"0",	"port1 missing eof err",	// 2
"0",	"port1 timeout err",		// 3
"0",	"port2 missing eof err",	// 4
"0",	"port2 timeout err",		// 5
"0",	"port3 missing eof err",	// 6
"0",	"port3 timeout err",		// 7
"0",	"port4 missing eof err",	// 8
"0",	"port4 timeout err"		// 9
};

string elb_mx_int_flush_info[2*5] = {
"?",	"port0 auto flush happened",	// 0
"?",	"port1 auto flush happened",	// 1
"?",	"port2 auto flush happened",	// 2
"?",	"port3 auto flush happened",	// 3
"?",	"port4 auto flush happened" 	// 4
};

////////////////////////////////////////////////////////////
// Helper functions
////////////////////////////////////////////////////////////
// helper function to set field
static int
elb_mx_set_field (uint64_t *data, int pos, int size, uint64_t value)
{
   MX_DEBUG_INFO("elb_mx_set_field pos = %d, size = %d, value = 0x%" PRIx64 "\n", pos, size, value);
   if (data == NULL) {
      MX_DEBUG_ERR("elb_mx_set_field argument data is NULL! Other arguments pos = %d, size = %d, value = 0x%" PRIx64 "\n", pos, size, value);
      return -1;
   }
   uint64_t mask;
   if (size == 64)
      mask = ((uint64_t) 0xffffffffffffffff) << pos;
   else
      mask = (((uint64_t) 1 << size)- (uint64_t) 1) << pos;
   *data = (*data & ~mask) | ((value << pos) & mask);
   return 0;
}

// helper function to get field
static uint64_t
elb_mx_get_field (uint64_t data, int pos, int size)
{
   uint64_t mask;
   if (size == 64)
      mask = (uint64_t) 0xffffffffffffffff;
   else
      mask = (((uint64_t) 1 << size)-1);
   uint64_t value = ((data >> pos) & mask);
   MX_DEBUG_INFO("elb_mx_get_field pos = %d, size = %d, value = 0x%" PRIx64 "\n", pos, size, value);
   return value;
}

static int
elb_mx_channel_mode_mapping(int speed, int num_lanes, int fec) {
   switch(speed) { 
     case 400 : return 56;		// 400GBASE-R8 + RSFEC (KP)
     case 200 : if (num_lanes == 4)
                   return 53; 		// 200GBASE-R4 + RSFEC (KP)
                else 
                   return 52; 		// 200GBASE-R8 + RSFEC (KP)
     case 100 : if (num_lanes == 2)  {
                   if (fec == 2 || fec == 3)   // ASIC verif uses 3 for KP
                      return 50;	// 100GBASE-R2 + RSFEC (KP)
                   else if (fec == 0) 
                      return 48;	// 100GBASE-R2
                   else
                      return -1;  	//
                } else { 
                   if (fec == 2) 
                      return 47; 	// 100GBASE-R4 + RSFEC
                   else if (fec == 0)  
                      return 46; 	// 100GBASE
                   else 
                      return -1;  	//
                }
      case 50 : if (num_lanes == 1) { 
                   if (fec == 2 || fec == 3)   // ASIC verif uses 3 for KP
                      return 43;	// 50GBASE-R1 + RSFEC (KP)
                   else if (fec == 0) 
                      return 40;	// 50GBASE-R1
                   else 
                      return -1;  	//
                } else {
                   if (fec == 2)  
                      return 39;	// 50GBASE-R2 + RSFEC
                   else if (fec == 1)  
                      return 38;	// 50GBASE-R2 + FCFEC
                   else 
                      return 37;	// 50GBASE-R2
                }
      case 40 : if (num_lanes == 2) {
                   if (fec == 2)  
                      return 39;	// 50GBASE-R2 + RSFEC
                   else if (fec == 1)  
                      return 38;	// 50GBASE-R2 + FCFEC
                   else 
                      return 37;	// 50GBASE-R2
                } else {
                   if (fec == 1) 
                      return 27;	// 40GBASE-R4 + FCFEC
                   else 
                      return 26;	// 40GBASE-R4
                }
      case 25 : if (fec == 2) 
                   return 23; 		// 25GBASE-R1 + RSFEC
                else if (fec == 1)
                   return 22;		// 25GBASE-R1 + FCFEC
                else 
                   return 21; 		// 25GBASE-R1
      case 10 : if (fec == 1) 
                   return 16;		// 10GBASE-R + FCFEC
                else 
                   return 15;		// 10GBASE-R
      case 1 :  return 9;		// 1G
      default : return -1;		// No support
  }
}

static string
elb_mx_int_msg(string s, int value)
{
   if (s.compare("S") == 0)      return "(Info)";
   else if (s.compare("?") == 0) return (value == 1) ? "(Maybe Bad)" : "(Good)";
   else if (s.compare("0") == 0 || s.compare("1") == 0) return (value == stoi(s)) ? "(Good)" : "(Bad) ";
   else return "?";  // unexpected type
}
////////////////////////////////////////////////////////////
// Comira MAC register read/write functions
////////////////////////////////////////////////////////////
uint64_t elb_mx_apb_read(int chip_id, int inst_id, int addr) {
    elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
    mx_csr.dhs_macreg.entry[addr].read();
    uint64_t rdata = mx_csr.dhs_macreg.entry[addr].data().convert_to<uint64_t>();
    MX_DEBUG_INFO("MX%d elb_mx_apb_read addr = %x, rdata = 0x%" PRIx64 "\n", inst_id, addr, rdata);
    return rdata;
}

void elb_mx_apb_write(int chip_id, int inst_id, int addr, uint64_t data) {
    elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
    mx_csr.dhs_macreg.entry[addr].data(data);
    mx_csr.dhs_macreg.entry[addr].write();
    MX_DEBUG_INFO("MX%d elb_mx_apb_write addr = %x, data = 0x%" PRIx64 "\n", inst_id, addr, data);
}

////////////////////////////////////////////////////////////
// MX Reset functions
////////////////////////////////////////////////////////////
void
elb_mx_set_soft_reset(int chip_id, int inst_id, int port, int ch, int value) {
   MX_DEBUG_MSG("MX%d ch%d elb_mx_set_soft_reset value=%d\n", inst_id, ch, value);
   int addr = elb_mx_mac_t::ELB_MX_MAC_CHMODE_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHMODE_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_TXSWRST_POS, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_TXSWRST_SZ, value);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_RXSWRST_POS, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_RXSWRST_SZ, value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);

   // Reset mx pack/unpack state machines
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.cfg_swrst.read();
   if (port == 0) {
      mx_csr.cfg_swrst.txsm0(value);
      mx_csr.cfg_swrst.rxsm0(value);
   } else if (port == 1) {
      mx_csr.cfg_swrst.txsm1(value);
      mx_csr.cfg_swrst.rxsm1(value);
   } else if (port == 2) {
      mx_csr.cfg_swrst.txsm2(value);
      mx_csr.cfg_swrst.rxsm2(value);
   } else if (port == 3) {
      mx_csr.cfg_swrst.txsm3(value);
      mx_csr.cfg_swrst.rxsm3(value);
   } else {
      mx_csr.cfg_swrst.txsm4(value);
      mx_csr.cfg_swrst.rxsm4(value);
   }
   mx_csr.cfg_swrst.write();

   if (value == 1) {
      elb_mx_set_auto_flush(chip_id, inst_id, port, 0);  // disable auto flush
   }
}

////////////////////////////////////////////////////////////
// Comira MAC configuration functions
////////////////////////////////////////////////////////////
// One micro second clock count, where value = clock freq in MHz
void
elb_mx_set_clock_info(int chip_id, int inst_id, int value)
{
   MX_DEBUG_MSG("MX%d elb_mx_set_clock_info value=%d\n", inst_id, value);
   for (int ch = 0; ch < 8; ch++) {
      int addr = elb_mx_mac_t::ELB_MX_MAC_CHCONFIG8_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHCONFIG8_ADDR_INC;
      uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
      elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG8_FLD_USCLKCNT_POS, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG8_FLD_USCLKCNT_SZ, value);
      elb_mx_apb_write(chip_id, inst_id, addr, data);
   }
}

int
elb_mx_set_ch_mode(int chip_id, int inst_id, int ch, int speed, int num_lanes, int fec) {
   int value = elb_mx_channel_mode_mapping(speed, num_lanes, fec);
   MX_DEBUG_MSG("MX%d ch%d elb_mx_set_ch_mode speed=%d, num_lanes=%d, fec=%d, value=%d\n", inst_id, ch, speed, num_lanes, fec, value);
   if (value == -1) return -1;
   int addr = elb_mx_mac_t::ELB_MX_MAC_CHMODE_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHMODE_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_MODE_POS, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_MODE_SZ, value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
   return 0;
}

// Comira MAC-PCS interface loopback
int
elb_mx_gmii_lpbk_get (int chip_id, int inst_id, int ch)
{
    int addr = elb_mx_mac_t::ELB_MX_MAC_CHMODE_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHMODE_ADDR_INC;
    uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
    data = elb_mx_get_field(data, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_GMIILPBK_POS, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_GMIILPBK_SZ);
    MX_DEBUG_MSG("MX%d ch%d elb_mx_gmii_lpbk_get data=%d\n", inst_id, ch, (int) data);
    return (int) data;
}  

void
elb_mx_gmii_lpbk_set (int chip_id, int inst_id, int ch, int value)
{
    MX_DEBUG_MSG("MX%d ch%d elb_mx_gmii_lpbk_set value=%d\n", inst_id, ch, value);
    int addr = elb_mx_mac_t::ELB_MX_MAC_CHMODE_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHMODE_ADDR_INC;
    uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
    elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_GMIILPBK_POS, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_GMIILPBK_SZ, value);
    elb_mx_apb_write(chip_id, inst_id, addr, data);
}  

// Comira PCS-Serdes interface loopback
int
elb_mx_pcs_lpbk_get (int chip_id, int inst_id, int ch)
{
    int addr = elb_mx_mac_t::ELB_MX_MAC_SDCFG_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_SDCFG_ADDR_INC;  // SDCFG0 and SDCFG1 have the same value
    uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
    data = elb_mx_get_field(data, elb_mx_mac_t::ELB_MX_MAC_SDCFG_FLD_SERDESLPBK_POS, elb_mx_mac_t::ELB_MX_MAC_SDCFG_FLD_SERDESLPBK_SZ);
    MX_DEBUG_MSG("MX%d ch%d elb_mx_pcs_lpbk_get data=%d\n", inst_id, ch, (int) data);
    return (int) data;
}

void
elb_mx_pcs_lpbk_set (int chip_id, int inst_id, int ch, int num_lanes, int value)
{
    MX_DEBUG_MSG("MX%d ch%d elb_mx_pcs_lpbk_set value=%d\n", inst_id, ch, value);
    for (int i = 0; i < num_lanes; i++) {
       // there are 2 registers per channel
       int lane = ch + i;

       int addr = elb_mx_mac_t::ELB_MX_MAC_SDCFG_ADDR0 + lane * elb_mx_mac_t::ELB_MX_MAC_SDCFG_ADDR_INC;
       uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
       elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_SDCFG_FLD_SERDESLPBK_POS, elb_mx_mac_t::ELB_MX_MAC_SDCFG_FLD_SERDESLPBK_SZ, value);
       elb_mx_apb_write(chip_id, inst_id, addr, data);

       addr = elb_mx_mac_t::ELB_MX_MAC_SDCFG_ADDR1 + lane * elb_mx_mac_t::ELB_MX_MAC_SDCFG_ADDR_INC;
       data = elb_mx_apb_read(chip_id, inst_id, addr);
       elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_SDCFG_FLD_SERDESLPBK_POS, elb_mx_mac_t::ELB_MX_MAC_SDCFG_FLD_SERDESLPBK_SZ, value);
       elb_mx_apb_write(chip_id, inst_id, addr, data);
    }
}  

// MX pack/unpack state machine
// Auto Tx flush when link is down longer than flush_timeout * clk
void
elb_mx_set_auto_flush(int chip_id, int inst_id, int port, int flush_timeout) {
   MX_DEBUG_MSG("MX%d port%d elb_mx_set_auto_flush timeout=%d\n", inst_id, port, flush_timeout);
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.cfg_port[port].read();
   mx_csr.cfg_port[port].flush_timeout(flush_timeout);
   mx_csr.cfg_port[port].write();
}

// Manual Tx flush at packet boundary
void
elb_mx_set_flush(int chip_id, int inst_id, int port, int flush) {
   MX_DEBUG_MSG("MX%d port%d elb_mx_set_flush flush=%d\n", inst_id, port, flush);
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.cfg_port[port].read();
   mx_csr.cfg_port[port].tx_flush(flush);
   mx_csr.cfg_port[port].write();
}

// Disable Rx path at packet boundary
void
elb_mx_set_rxsm_enable(int chip_id, int inst_id, int port, int enable) {
   MX_DEBUG_MSG("MX%d port%d elb_mx_set_rxsm_enable enable=%d\n", inst_id, port, enable);
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   cpp_int disable = (enable == 0) ? 1 : 0;
   mx_csr.cfg_port[port].read();
   mx_csr.cfg_port[port].rx_disable(disable);
   mx_csr.cfg_port[port].write();
}

// Auto Comira MAC tx drain when RX is in a fault state
void
elb_mx_set_tx_autodrain(int chip_id, int inst_id, int ch, int value) {
   MX_DEBUG_MSG("MX%d ch%d elb_mx_set_tx_autodrain value=%d\n", inst_id, ch, value);
   int addr = elb_mx_mac_t::ELB_MX_MAC_CHCONFIG3_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHCONFIG3_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG3_FLD_TXDRAINONFAULT_POS, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG3_FLD_TXDRAINONFAULT_SZ, value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

// Mx fixer timeout
void
elb_mx_set_fixer_timeout(int chip_id, int inst_id, int port, int value) {
   MX_DEBUG_MSG("MX%d port%d elb_mx_set_fixer_timeout timeout=%d\n", inst_id, port, value);
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.cfg_fixer[port].read();
   mx_csr.cfg_fixer[port].timeout_max( value ) ;
   mx_csr.cfg_fixer[port].write() ;
}

// Manual Comira MAC tx drain
void
elb_mx_set_tx_drain(int chip_id, int inst_id, int ch, bool drain) {
   MX_DEBUG_MSG("MX%d ch%d elb_mx_set_tx_drain drain=%d\n", inst_id, ch, drain);
   int addr = elb_mx_mac_t::ELB_MX_MAC_CHMODE_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHMODE_ADDR_INC;
   int value = (drain) ? 1 : 0;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_TXDRAIN_POS, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_TXDRAIN_SZ, value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

// Slot cfg
int
elb_mx_slot_gen(int inst_id, int *port_vec, int *port_ch_map, int *port_speed, int *slot_port, int *slot_ch)
{
   if (port_vec == NULL) {
      MX_DEBUG_ERR("elb_mx_slot_gen argument port_vec is NULL!\n")
      return -1;
   }
   if (port_ch_map == NULL) {
      MX_DEBUG_ERR("elb_mx_slot_gen argument port_ch_map is NULL!\n");
      return -1;
   }
   if (port_speed == NULL) {
      MX_DEBUG_ERR("elb_mx_slot_gen argument port_speed is NULL!\n");
      return -1;
   }
   if (slot_port == NULL) {
      MX_DEBUG_ERR("elb_mx_slot_gen argument slot_port is NULL!\n");
      return -1;
   }
   if (slot_ch == NULL) {
      MX_DEBUG_ERR("elb_mx_slot_gen argument slot_ch is NULL!\n");
      return -1;
   }
   int starting_slot[8]= {
    0 , 4 , 2 , 6 , 1 , 5 , 3 ,7
   } ;
   for (int slot = 0; slot < 8 ; slot++) {
      slot_port[slot] = 7; // invalid
      slot_ch[slot] = starting_slot[slot];  // default
   }

   for (int port = 0; port < 5; port++) {
      if (port_vec[port]) {
         int num_slot = (port_speed[port] == 400) ? 8 : (port_speed[port] == 200) ? 4 : (port_speed[port] == 100) ? 2 : 1;
         int increment = 8 / num_slot;
         for (int i = 0; i < num_slot; i++) {
            int j = (starting_slot[ port_ch_map[port] ]+i*increment)%8;
            if (slot_port[j] != 7) {
               MX_DEBUG_ERR("elb_mx_slot_gen: MX%d port = %d, ch = %d, speed = %d slot_port conflict\n", inst_id, port, port_ch_map[port], port_speed[port]);
               return -1;
            } else {
               slot_port[j] = port;
               slot_ch[j] = port_ch_map[port];
            }
         }
      }
   }

   for (int slot = 0; slot < 8 ; slot++) {
      MX_DEBUG_MSG("elb_mx_slot_gen: MX%d slot = %d: slot_port = %d, slot_ch = %d\n", inst_id, slot, slot_port[slot], slot_ch[slot]);
   }

   return 0;
}

void
elb_mx_get_slot(int chip_id, int inst_id)
{
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);

   mx_csr.cfg_mac_slot_select.read();

   if ( mx_csr.cfg_mac_slot_select.val().convert_to<int>() == 0  ) {
      mx_csr.cfg_mac_slotA.read();
      MX_DEBUG_MSG("MX%d elb_mx_get_slot_cfg: slot0_port = %d, slot0_channel = %d\n", inst_id, mx_csr.cfg_mac_slotA.slot0_port().convert_to<int>(), mx_csr.cfg_mac_slotA.slot0_channel().convert_to<int>());
      MX_DEBUG_MSG("MX%d elb_mx_get_slot_cfg: slot1_port = %d, slot1_channel = %d\n", inst_id, mx_csr.cfg_mac_slotA.slot1_port().convert_to<int>(), mx_csr.cfg_mac_slotA.slot1_channel().convert_to<int>());
      MX_DEBUG_MSG("MX%d elb_mx_get_slot_cfg: slot2_port = %d, slot2_channel = %d\n", inst_id, mx_csr.cfg_mac_slotA.slot2_port().convert_to<int>(), mx_csr.cfg_mac_slotA.slot2_channel().convert_to<int>());
      MX_DEBUG_MSG("MX%d elb_mx_get_slot_cfg: slot3_port = %d, slot3_channel = %d\n", inst_id, mx_csr.cfg_mac_slotA.slot3_port().convert_to<int>(), mx_csr.cfg_mac_slotA.slot3_channel().convert_to<int>());
      MX_DEBUG_MSG("MX%d elb_mx_get_slot_cfg: slot4_port = %d, slot4_channel = %d\n", inst_id, mx_csr.cfg_mac_slotA.slot4_port().convert_to<int>(), mx_csr.cfg_mac_slotA.slot4_channel().convert_to<int>());
      MX_DEBUG_MSG("MX%d elb_mx_get_slot_cfg: slot5_port = %d, slot5_channel = %d\n", inst_id, mx_csr.cfg_mac_slotA.slot5_port().convert_to<int>(), mx_csr.cfg_mac_slotA.slot5_channel().convert_to<int>());
      MX_DEBUG_MSG("MX%d elb_mx_get_slot_cfg: slot6_port = %d, slot6_channel = %d\n", inst_id, mx_csr.cfg_mac_slotA.slot6_port().convert_to<int>(), mx_csr.cfg_mac_slotA.slot6_channel().convert_to<int>());
      MX_DEBUG_MSG("MX%d elb_mx_get_slot_cfg: slot7_port = %d, slot7_channel = %d\n", inst_id, mx_csr.cfg_mac_slotA.slot7_port().convert_to<int>(), mx_csr.cfg_mac_slotA.slot7_channel().convert_to<int>());
   } else {
      mx_csr.cfg_mac_slotB.read();
      MX_DEBUG_MSG("MX%d elb_mx_get_slot_cfg: slot0_port = %d, slot0_channel = %d\n", inst_id, mx_csr.cfg_mac_slotB.slot0_port().convert_to<int>(), mx_csr.cfg_mac_slotB.slot0_channel().convert_to<int>());
      MX_DEBUG_MSG("MX%d elb_mx_get_slot_cfg: slot1_port = %d, slot1_channel = %d\n", inst_id, mx_csr.cfg_mac_slotB.slot1_port().convert_to<int>(), mx_csr.cfg_mac_slotB.slot1_channel().convert_to<int>());
      MX_DEBUG_MSG("MX%d elb_mx_get_slot_cfg: slot2_port = %d, slot2_channel = %d\n", inst_id, mx_csr.cfg_mac_slotB.slot2_port().convert_to<int>(), mx_csr.cfg_mac_slotB.slot2_channel().convert_to<int>());
      MX_DEBUG_MSG("MX%d elb_mx_get_slot_cfg: slot3_port = %d, slot3_channel = %d\n", inst_id, mx_csr.cfg_mac_slotB.slot3_port().convert_to<int>(), mx_csr.cfg_mac_slotB.slot3_channel().convert_to<int>());
      MX_DEBUG_MSG("MX%d elb_mx_get_slot_cfg: slot4_port = %d, slot4_channel = %d\n", inst_id, mx_csr.cfg_mac_slotB.slot4_port().convert_to<int>(), mx_csr.cfg_mac_slotB.slot4_channel().convert_to<int>());
      MX_DEBUG_MSG("MX%d elb_mx_get_slot_cfg: slot5_port = %d, slot5_channel = %d\n", inst_id, mx_csr.cfg_mac_slotB.slot5_port().convert_to<int>(), mx_csr.cfg_mac_slotB.slot5_channel().convert_to<int>());
      MX_DEBUG_MSG("MX%d elb_mx_get_slot_cfg: slot6_port = %d, slot6_channel = %d\n", inst_id, mx_csr.cfg_mac_slotB.slot6_port().convert_to<int>(), mx_csr.cfg_mac_slotB.slot6_channel().convert_to<int>());
      MX_DEBUG_MSG("MX%d elb_mx_get_slot_cfg: slot7_port = %d, slot7_channel = %d\n", inst_id, mx_csr.cfg_mac_slotB.slot7_port().convert_to<int>(), mx_csr.cfg_mac_slotB.slot7_channel().convert_to<int>());
   }
}

int
elb_mx_set_slot(int chip_id, int inst_id, int *slot_port, int *slot_ch)
{
   if (slot_port == NULL) {
      MX_DEBUG_ERR("elb_mx_set_slot argument slot_port is NULL!\n");
      return -1;
   }
   if (slot_ch == NULL) {
      MX_DEBUG_ERR("elb_mx_set_slot argument slot_ch is NULL!\n");
      return -1;
   }

   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);

   mx_csr.cfg_mac_slot_select.read();

   if ( mx_csr.cfg_mac_slot_select.val().convert_to<int>() == 1  ) {
      mx_csr.cfg_mac_slotA.slot0_port(slot_port[0]);
      mx_csr.cfg_mac_slotA.slot0_channel(slot_ch[0]);
      mx_csr.cfg_mac_slotA.slot1_port(slot_port[1]);
      mx_csr.cfg_mac_slotA.slot1_channel(slot_ch[1]);
      mx_csr.cfg_mac_slotA.slot2_port(slot_port[2]);
      mx_csr.cfg_mac_slotA.slot2_channel(slot_ch[2]);
      mx_csr.cfg_mac_slotA.slot3_port(slot_port[3]);
      mx_csr.cfg_mac_slotA.slot3_channel(slot_ch[3]);
      mx_csr.cfg_mac_slotA.slot4_port(slot_port[4]);
      mx_csr.cfg_mac_slotA.slot4_channel(slot_ch[4]);
      mx_csr.cfg_mac_slotA.slot5_port(slot_port[5]);
      mx_csr.cfg_mac_slotA.slot5_channel(slot_ch[5]);
      mx_csr.cfg_mac_slotA.slot6_port(slot_port[6]);
      mx_csr.cfg_mac_slotA.slot6_channel(slot_ch[6]);
      mx_csr.cfg_mac_slotA.slot7_port(slot_port[7]);
      mx_csr.cfg_mac_slotA.slot7_channel(slot_ch[7]);
      mx_csr.cfg_mac_slotA.write();
   } else {
      mx_csr.cfg_mac_slotB.slot0_port(slot_port[0]);
      mx_csr.cfg_mac_slotB.slot0_channel(slot_ch[0]);
      mx_csr.cfg_mac_slotB.slot1_port(slot_port[1]);
      mx_csr.cfg_mac_slotB.slot1_channel(slot_ch[1]);
      mx_csr.cfg_mac_slotB.slot2_port(slot_port[2]);
      mx_csr.cfg_mac_slotB.slot2_channel(slot_ch[2]);
      mx_csr.cfg_mac_slotB.slot3_port(slot_port[3]);
      mx_csr.cfg_mac_slotB.slot3_channel(slot_ch[3]);
      mx_csr.cfg_mac_slotB.slot4_port(slot_port[4]);
      mx_csr.cfg_mac_slotB.slot4_channel(slot_ch[4]);
      mx_csr.cfg_mac_slotB.slot5_port(slot_port[5]);
      mx_csr.cfg_mac_slotB.slot5_channel(slot_ch[5]);
      mx_csr.cfg_mac_slotB.slot6_port(slot_port[6]);
      mx_csr.cfg_mac_slotB.slot6_channel(slot_ch[6]);
      mx_csr.cfg_mac_slotB.slot7_port(slot_port[7]);
      mx_csr.cfg_mac_slotB.slot7_channel(slot_ch[7]);
      mx_csr.cfg_mac_slotB.write();
   }

   mx_csr.cfg_mac_slot_select.read();
   mx_csr.cfg_mac_slot_select.val(mx_csr.cfg_mac_slot_select.val().convert_to<int>() ^ 1);
   mx_csr.cfg_mac_slot_select.write();

   return 0;
}

// TCL friendly arguments
void
elb_mx_slot_gen(int inst_id, int port_vec0, int port_ch_map0, int port_speed0, int port_vec1, int port_ch_map1, int port_speed1, int port_vec2, int port_ch_map2, int port_speed2, int port_vec3, int port_ch_map3, int port_speed3, int port_vec4, int port_ch_map4, int port_speed4)
{
   int port_vec[5]    = {port_vec0, port_vec1, port_vec2, port_vec3, port_vec4};
   int port_ch_map[5] = {port_ch_map0, port_ch_map1, port_ch_map2, port_ch_map3, port_ch_map4};
   int port_speed[5]  = {port_speed0, port_speed1, port_speed2, port_speed3, port_speed4};
   int slot_port[8];
   int slot_ch[8];
   elb_mx_slot_gen(inst_id, port_vec, port_ch_map, port_speed, slot_port, slot_ch);
}

// TCL friendly arguments
void
elb_mx_set_slot(int chip_id, int inst_id, int slot_port0, int slot_ch0, int slot_port1, int slot_ch1, int slot_port2, int slot_ch2, int slot_port3, int slot_ch3, int slot_port4, int slot_ch4, int slot_port5, int slot_ch5, int slot_port6, int slot_ch6, int slot_port7, int slot_ch7)
{
   int slot_port[8] = {slot_port0, slot_port1, slot_port2, slot_port3, slot_port4, slot_port5, slot_port6, slot_port7};
   int slot_ch[8]   = {slot_ch0, slot_ch1, slot_ch2, slot_ch3, slot_ch4, slot_ch5, slot_ch6, slot_ch7};
   elb_mx_set_slot(chip_id, inst_id, slot_port, slot_ch);
}

void 
elb_mx_set_mtu_jabber(int chip_id , int inst_id, int ch, int max_value, int jabber_value) {
   MX_DEBUG_MSG("MX%d ch%d elb_mx_set_mtu_jabber max_value=%d, jabber_value=%d\n", inst_id, ch, max_value, jabber_value);
   int addr = elb_mx_mac_t::ELB_MX_MAC_CHMODE_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHMODE_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_TXJABBER_POS, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_TXJABBER_SZ, jabber_value);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_RXJABBER_POS, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_RXJABBER_SZ, jabber_value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);

   addr = elb_mx_mac_t::ELB_MX_MAC_CHCONFIG3_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHCONFIG3_ADDR_INC;
   data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG3_FLD_RXMAXFRMSIZE_POS, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG3_FLD_RXMAXFRMSIZE_SZ, max_value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

int
elb_mx_set_pause(int chip_id, int inst_id, int port, int ch, int pri_vec, int legacy,
                 bool tx_pause_enable, bool rx_pause_enable)
{
   MX_DEBUG_MSG("MX%d ch%d port%d elb_mx_set_pause pri_vec=0x%x, legacy=%d tx_pause_enable=%d rx_pause_enable=%d\n", inst_id, ch, port, pri_vec, legacy, tx_pause_enable, rx_pause_enable);
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);

   // umac maccfg pause configuration fields
   int txfcen = 0;	// enable tx (legacy) pause frame generation
   int rxfcen = 0;	// enable rx (legacy) flow control decoder
   int rxfilterfc = 1;	// filter out rx (legacy) pause frame
   int rxfctotx = 0;	// If rxfcen == 1, setting this bit will disable the transmission of user data frames for the time given in the PAUSE_TIME field of the PAUSE Control Frame when UMAC Core receives a valid PAUSE Control Frame.

   int txpfcen = 0;	// 8-bit enable tx priority pause frame generation
   int rxpfcen = 0;	// enable rx priority flow control decoder
   int rxfilterpfc = 1;	// filter out rx priority pause frame

   // mx pause configuration
   int txxoff_enable = 0;	// 8-bit enable tx priority xoff from pb to mx
   int txxoff_fcxoff = 0;	// 1-bit legacy pause

   if (pri_vec != 0) {
       if (legacy) {
           txfcen = (tx_pause_enable) ? 1 : 0;
           rxfcen = (rx_pause_enable) ? 1 : 0;
           rxfctotx = 1;
           txxoff_enable = 0xff;
           txxoff_fcxoff = 1;
       } else {
           txpfcen = (tx_pause_enable) ? pri_vec : 0;
           rxpfcen = (rx_pause_enable) ? 1 : 0;
           rxfctotx = 0;
           txxoff_enable = 0xff;
           txxoff_fcxoff = 0;
       }
   }

   int addr = elb_mx_mac_t::ELB_MX_MAC_MACCFG_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_MACCFG_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_TXFCEN_POS, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_TXFCEN_SZ, txfcen);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_RXFCEN_POS, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_RXFCEN_SZ, rxfcen);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_RXFILTERFC_POS, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_RXFILTERFC_SZ, rxfilterfc);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_RXFCTOTX_POS, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_RXFCTOTX_SZ, rxfctotx);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_TXPFCEN_POS, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_TXPFCEN_SZ, txpfcen);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_RXPFCEN_POS, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_RXPFCEN_SZ, rxpfcen);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_RXFILTERPFC_POS, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_RXFILTERPFC_SZ, rxfilterpfc);
   elb_mx_apb_write(chip_id, inst_id, addr, data);

   mx_csr.cfg_mac_pfc_txxoff.read();
   if (port == 0) {
      mx_csr.cfg_mac_pfc_txxoff.p0_enable(txxoff_enable);
      mx_csr.cfg_mac_pfc_txxoff.p0fcxoff(txxoff_fcxoff);
   } else if (port == 1) {
      mx_csr.cfg_mac_pfc_txxoff.p1_enable(txxoff_enable);
      mx_csr.cfg_mac_pfc_txxoff.p1fcxoff(txxoff_fcxoff);
   } else if (port == 2) {
      mx_csr.cfg_mac_pfc_txxoff.p2_enable(txxoff_enable);
      mx_csr.cfg_mac_pfc_txxoff.p2fcxoff(txxoff_fcxoff);
   } else if (port == 3) {
      mx_csr.cfg_mac_pfc_txxoff.p3_enable(txxoff_enable);
      mx_csr.cfg_mac_pfc_txxoff.p3fcxoff(txxoff_fcxoff);
   } else if (port == 4) {
      mx_csr.cfg_mac_pfc_txxoff.p4_enable(txxoff_enable);
      mx_csr.cfg_mac_pfc_txxoff.p4fcxoff(txxoff_fcxoff);
   }
   mx_csr.cfg_mac_pfc_txxoff.write();

   return 0;
}

// Mac address used for SA in TX pause frames and DA in RX pause frames
int
elb_mx_set_pause_src_addr(int chip_id, int inst_id, int ch, uint8_t *mac_addr)
{
   MX_DEBUG_MSG("MX%d ch%d elb_mx_set_pause_src_addr mac_addr = 0x%02x%02x%02x%02x%02x%02x\n", inst_id, ch, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
   if (mac_addr == NULL) {
      MX_DEBUG_ERR("elb_mx_set_pause_src_addr argument mac_addr is NULL!\n");
      return -1;
   }
   int addr = elb_mx_mac_t::ELB_MX_MAC_CHCONFIG4_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHCONFIG4_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   uint64_t value = 0;
   for (int i = 0; i < 6; i++) {
      value = value | (mac_addr[i] << 8*(5-i));
   }
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG4_FLD_MACADDR_POS, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG4_FLD_MACADDR_SZ, value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);

   return 0;
}

// Pause DA to send in pause frames. Default = 0x180c2000001
int
elb_mx_set_pause_dest_addr(int chip_id, int inst_id, int ch, uint8_t *mac_addr)
{
   MX_DEBUG_MSG("MX%d ch%d elb_mx_set_pause_dest_addr mac_addr = 0x%02x%02x%02x%02x%02x%02x\n", inst_id, ch, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
   if (mac_addr == NULL) {
      MX_DEBUG_ERR("elb_mx_set_pause_dest_addr argument mac_addr is NULL!\n");
      return -1;
   }
   int addr = elb_mx_mac_t::ELB_MX_MAC_CHCONFIG5_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHCONFIG5_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   uint64_t value = 0;
   for (int i = 0; i < 6; i++) {
      value = value | (mac_addr[i] << 8*(5-i));
   }
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG5_FLD_PAUSEDEST_POS, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG5_FLD_PAUSEDEST_SZ, value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);

   return 0;
}

// Pause refresh time to send in pause time
int
elb_mx_set_pauseontime(int chip_id, int inst_id, int ch, int value)
{
   MX_DEBUG_MSG("MX%d ch%d elb_mx_set_pauseontime value = 0x%x\n", inst_id, ch, value);
   int addr = elb_mx_mac_t::ELB_MX_MAC_CHCONFIG4_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHCONFIG4_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG4_FLD_PAUSEONTIME_POS, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG4_FLD_PAUSEONTIME_SZ, value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);

   return 0;
}

// Pause refresh time to begin retransmission
int
elb_mx_set_pauserefresh(int chip_id, int inst_id, int ch, int value)
{
   MX_DEBUG_MSG("MX%d ch%d elb_mx_set_pauserefresh value = 0x%x\n", inst_id, ch, value);
   int addr = elb_mx_mac_t::ELB_MX_MAC_CHCONFIG5_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHCONFIG5_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG5_FLD_PAUSEREFRESH_POS, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG5_FLD_PAUSEREFRESH_SZ, value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);

   return 0;
}

int 
elb_mx_check_ch_sync(int chip_id, int inst_id, int ch) {
   int addr = elb_mx_mac_t::ELB_MX_MAC_CHSTS_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHSTS_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   return (int) elb_mx_get_field(data, elb_mx_mac_t::ELB_MX_MAC_CHSTS_FLD_LINKUP_POS, elb_mx_mac_t::ELB_MX_MAC_CHSTS_FLD_LINKUP_SZ);
}

void
elb_mx_set_txthresh(int chip_id, int inst_id, int ch, int txwrthresh, int txrdthresh) {
   MX_DEBUG_MSG("MX%d ch%d elb_mx_set_txthresh txwrthresh=0x%x, txrdthresh=0x%x\n", inst_id, ch, txwrthresh, txrdthresh);
   int addr = elb_mx_mac_t::ELB_MX_MAC_MACCFG_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_MACCFG_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_TXWRTHRESH_POS, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_TXWRTHRESH_SZ, txwrthresh);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_TXRDTHRESH_POS, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_TXRDTHRESH_SZ, txrdthresh);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

// Minimum IFG length on transmit frames
void
elb_mx_set_ifglen(int chip_id, int inst_id, int ch, int value) {
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.cfg_mac.read();
   mx_csr.cfg_mac.tx_ipg(0);  // in order to program chmode ifglen field, cfg_mac.tx_ipg need to be 0.
   mx_csr.cfg_mac.write();

   MX_DEBUG_MSG("MX%d ch%d elb_mx_set_ifglen value = %d\n", inst_id, ch, value);
   int addr = elb_mx_mac_t::ELB_MX_MAC_CHMODE_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHMODE_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_IFGLEN_POS, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_IFGLEN_SZ, value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

void
elb_mx_disable_eth_len_err(int chip_id, int inst_id, int ch, int value) {
   MX_DEBUG_MSG("MX%d ch%d elb_mx_disable_eth_len_err value = 0x%x\n", inst_id, ch, value);
   int addr = elb_mx_mac_t::ELB_MX_MAC_CHCONFIG3_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHCONFIG3_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG3_FLD_RXETHLENERRMASK_POS, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG3_FLD_RXETHLENERRMASK_SZ, value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

void
elb_mx_set_ch_enable(int chip_id, int inst_id, int ch, int txen, int rxen) {
   MX_DEBUG_MSG("MX%d ch%d elb_mx_set_ch_enable txen=%d, rxen=%d\n", inst_id, ch, txen, rxen);
   int addr = elb_mx_mac_t::ELB_MX_MAC_CHMODE_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHMODE_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_TXEN_POS, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_TXEN_SZ, txen);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_RXEN_POS, elb_mx_mac_t::ELB_MX_MAC_CHMODE_FLD_RXEN_SZ, rxen);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

void
elb_mx_set_vlan_check (int chip_id, int inst_id, int ch,
                       int num_tags, uint32_t tag1, uint32_t tag2,
                       uint32_t tag3)
{
   MX_DEBUG_MSG("MX%d ch%d elb_mx_set_vlan_check num_tags=%d, tag1=0x%x, tag2=0x%x, tage3=0x%x\n", inst_id, ch, num_tags, tag1, tag2, tag3);
   int addr = elb_mx_mac_t::ELB_MX_MAC_CHCONFIG8_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHCONFIG8_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG8_FLD_VLANTAG1_POS, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG8_FLD_VLANTAG1_SZ, tag1);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG8_FLD_VLANTAG2_POS, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG8_FLD_VLANTAG2_SZ, tag2);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG8_FLD_VLANTAG3_POS, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG8_FLD_VLANTAG3_SZ, tag3);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG8_FLD_MAXVLANCNT_POS, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG8_FLD_MAXVLANCNT_SZ, num_tags);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

void
elb_mx_set_txpadrunt(int chip_id, int inst_id, int ch, int value)
{
   MX_DEBUG_MSG("MX%d ch%d elb_mx_set_txpadrunt value = %d\n", inst_id, ch, value);
   int addr = elb_mx_mac_t::ELB_MX_MAC_MACCFG_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_MACCFG_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_TXPADRUNT_POS, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_TXPADRUNT_SZ, value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

void
elb_mx_set_tx_padding(int chip_id, int inst_id, int ch, int enable)
{
   MX_DEBUG_MSG("MX%d ch%d elb_mx_set_tx_padding enable = %d\n", inst_id, ch, enable);
   if (enable == 1) {
      elb_mx_set_txpadrunt(chip_id, inst_id, ch, 0x40);
   } else {
      elb_mx_set_txpadrunt(chip_id, inst_id, ch, 0x0);
   }
}

void
elb_mx_set_rx_padding(int chip_id, int inst_id, int ch, int value) {
   MX_DEBUG_MSG("MX%d ch%d elb_mx_set_rx_padding value = %d\n", inst_id, ch, value);
   int addr = elb_mx_mac_t::ELB_MX_MAC_MACCFG_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_MACCFG_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_RXPADRUNT_POS, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_RXPADRUNT_SZ, value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

void
elb_mx_send_remote_faults(int chip_id, int inst_id, int ch, bool send)
{
   MX_DEBUG_MSG("MX%d ch%d elb_mx_send_remote_faults send = %d\n", inst_id, ch, send);
   int addr = elb_mx_mac_t::ELB_MX_MAC_MACCFG_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_MACCFG_ADDR_INC;
   int value = (send) ? 1 : 0;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_TXRFAULT_POS, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_TXRFAULT_SZ, value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

// FEC
// HiSER symbol error threshold for interrupt and status. Default = 0x15b8
void
elb_mx_set_hiser_thresh(int chip_id, int inst_id, int ch, int value)
{
   MX_DEBUG_MSG("MX%d ch%d elb_mx_set_hiser_thresh value = 0x%x\n", inst_id, ch, value);
   int addr = elb_mx_mac_t::ELB_MX_MAC_CHCONFIG6_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHCONFIG6_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG6_FLD_HISERTHRESH_POS, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG6_FLD_HISERTHRESH_SZ, value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

void
elb_mx_enable_false_linkup_detection(int chip_id , int inst_id, int ch, int value) {
   MX_DEBUG_MSG("MX%d, ch%d: elb_mx_enable_false_linkup_detection\n", inst_id, ch);
   int addr = elb_mx_mac_t::ELB_MX_MAC_CHCONFIG3_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHCONFIG3_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG3_FLD_SJSIZE_POS, elb_mx_mac_t::ELB_MX_MAC_CHCONFIG3_FLD_SJSIZE_SZ, value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

////////////////////////////////////////////////////////////
// MAC Status
////////////////////////////////////////////////////////////
int
elb_mx_get_mac_chsts(int chip_id, int inst_id, int ch, bool display)
{
   int addr = elb_mx_mac_t::ELB_MX_MAC_CHSTS_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHSTS_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   if (display == 1) {
      int len = sizeof(elb_mx_mac_chsts_info)/(2*sizeof(elb_mx_mac_chsts_info[0]));
      for (int i = 0; i < len; i++) {
         int field_value = (data >> i)&0x1;
         string field_status = elb_mx_int_msg(elb_mx_mac_chsts_info[i*2+0], field_value);
         MX_DEBUG_MSG("MX%d ch%d chsts[%2d] = %d %s, field: %s\n", inst_id, ch, i, field_value, field_status.c_str(), elb_mx_mac_chsts_info[i*2+1].c_str());
      }

   }
   return (int) data;
}

int
elb_mx_get_ch_sync(int chip_id, int inst_id, int ch)
{
   int data = elb_mx_get_mac_chsts(chip_id, inst_id, ch, 0);
   return (int) elb_mx_get_field(data, elb_mx_mac_t::ELB_MX_MAC_CHSTS_FLD_LINKUP_POS, 1);
}

int
elb_mx_get_ch_fault(int chip_id, int inst_id, int ch)
{
   int data = elb_mx_get_mac_chsts(chip_id, inst_id, ch, 0);
   uint64_t nolocalfault = elb_mx_get_field(data, elb_mx_mac_t::ELB_MX_MAC_CHSTS_FLD_NOLOCALFAULT_POS, 1);
   uint64_t noremotefault = elb_mx_get_field(data, elb_mx_mac_t::ELB_MX_MAC_CHSTS_FLD_NOREMOTEFAULT_POS, 1);
   MX_DEBUG_MSG("MX%d ch%d elb_mx_get_ch_fault nolocalfault=%d, noremotefault=%d\n", inst_id, ch, (int) nolocalfault, (int) noremotefault);
   return (((int) nolocalfault == 0) || ((int) noremotefault == 0));
}

int
elb_mx_get_ch_pcs_err(int chip_id, int inst_id, int ch)
{
   int data = elb_mx_get_mac_chsts(chip_id, inst_id, ch, 0);
   // TODO: need to check with Comira
   uint64_t nohiber   = elb_mx_get_field(data, elb_mx_mac_t::ELB_MX_MAC_CHSTS_FLD_NOHIBER_POS, 1);
   uint64_t hiser     = elb_mx_get_field(data, elb_mx_mac_t::ELB_MX_MAC_CHSTS_FLD_HISER_POS, 1);
   MX_DEBUG_MSG("MX%d ch%d elb_mx_get_ch_pcs_err nohiber=%d, hiser=%d\n", inst_id, ch, (int) nohiber, (int) hiser);
   return (((int) nohiber == 0) || ((int) hiser == 1));
}

////////////////////////////////////////////////////////////
// MAC Statistic Counters
////////////////////////////////////////////////////////////
uint64_t
elb_mx_read_stats(int chip_id, int inst_id, int ch, int index, bool clear_on_read)
{
   int addr = 0x400*ch + index*8 + 0xc000;
   return elb_mx_apb_read(chip_id, inst_id, addr);
}

void
elb_mx_mac_stat(int chip_id, int inst_id, int ch, uint64_t *stats_data)  // For software
{
   for (int i = 0; i < MAX_MAC_STATS; i++) {
      uint64_t data = elb_mx_read_stats(chip_id, inst_id, ch, i);
      if (stats_data != NULL) {
          if (elb_mx_mac_stat_info[i*3+1].compare("NEW") != 0) {
             int cap_stat_idx = stoi(elb_mx_mac_stat_info[i*3+1]);
             stats_data[cap_stat_idx] = data;
          }
      }
      MX_DEBUG_MSG("MX%d ch%d %50s : %" PRIu64 "\n", inst_id, ch, elb_mx_mac_stat_info[i*3+2].c_str(), data);
   }
   if (stats_data != NULL) {
      // Does not exist in Elba
      stats_data[11] = 0;  // received undersize
      stats_data[19] = 0;  // buffer full
      stats_data[89] = 0;  // reserved
   }
}

void 
elb_mx_clear_mac_stat(int chip_id, int inst_id, int ch, int value) {
   MX_DEBUG_MSG("MX%d ch%d elb_mx_clear_mac_stat value = %d\n", inst_id, ch, value);
   int addr = elb_mx_mac_t::ELB_MX_MAC_MACCFG_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_MACCFG_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_STATSCLR_POS, elb_mx_mac_t::ELB_MX_MAC_MACCFG_FLD_STATSCLR_SZ, value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

void
elb_mx_get_mac_stat(int chip_id, int inst_id, int ch, int stat_type, bool skip_zero, bool clear_on_read, bool display)
{
   bool statscor_zero = false;
   uint64_t orig_appcfg0 = 0;
   if (clear_on_read) {
      orig_appcfg0 = elb_mx_apb_read(chip_id, inst_id, elb_mx_mac_t::ELB_MX_MAC_APPCFG0_ADDR);
      statscor_zero = elb_mx_get_field(orig_appcfg0, elb_mx_mac_t::ELB_MX_MAC_APPCFG0_FLD_STATSCOR_POS, elb_mx_mac_t::ELB_MX_MAC_APPCFG0_FLD_STATSCOR_SZ) == 0;
      if (statscor_zero) {
         uint64_t data = orig_appcfg0;
         elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_APPCFG0_FLD_STATSCOR_POS, elb_mx_mac_t::ELB_MX_MAC_APPCFG0_FLD_STATSCOR_SZ, 1);
         elb_mx_apb_write(chip_id, inst_id, elb_mx_mac_t::ELB_MX_MAC_APPCFG0_ADDR, data);
      }
   }

   bool all_stats_zero = true;
   for (int i = 0; i < MAX_MAC_STATS; i++) {
      bool match = ((1 << stat_type) & stoi(elb_mx_mac_stat_info[i*3+0])) != 0;
      if (match) {
         uint64_t data = elb_mx_read_stats(chip_id, inst_id, ch, i);
         if (skip_zero == false || data > 0) {
            all_stats_zero = false;
            MX_DEBUG_MSG("MX%d ch%d %50s : %" PRIu64 "\n", inst_id, ch, elb_mx_mac_stat_info[i*3+2].c_str(), data);
         }
      }
   }
   if (skip_zero && all_stats_zero) {
      string name = (stat_type == elb_mx_mac_stat_type_t::ELB_MX_MAC_STAT_ERR) ? "err" :
		    (stat_type == elb_mx_mac_stat_type_t::ELB_MX_MAC_STAT_PAUSE) ? "pause" :
		    (stat_type == elb_mx_mac_stat_type_t::ELB_MX_MAC_STAT_PCS) ? "pcs" :
		    (stat_type == elb_mx_mac_stat_type_t::ELB_MX_MAC_STAT_TXDRAIN) ? "txdrain" : "";
      MX_DEBUG_MSG("MX%d, ch%d: all %s statistic counters = 0\n", inst_id, ch, name.c_str());
   }

   if (statscor_zero) {
      elb_mx_apb_write(chip_id, inst_id, elb_mx_mac_t::ELB_MX_MAC_APPCFG0_ADDR, orig_appcfg0);
   }
}

void
elb_mx_get_mac_stat_err(int chip_id, int inst_id, int ch, bool skip_zero, bool clear_on_read, bool display)
{
   elb_mx_get_mac_stat(chip_id, inst_id, ch, elb_mx_mac_stat_type_t::ELB_MX_MAC_STAT_ERR, skip_zero, clear_on_read, display);
}

void
elb_mx_get_mac_stat_pause(int chip_id, int inst_id, int ch, bool skip_zero, bool clear_on_read, bool display)
{
   elb_mx_get_mac_stat(chip_id, inst_id, ch, elb_mx_mac_stat_type_t::ELB_MX_MAC_STAT_PAUSE, skip_zero, clear_on_read, display);
}

////////////////////////////////////////////////////////////
// Comira MAC Interrupt
////////////////////////////////////////////////////////////
int
elb_mx_get_mac_int(int chip_id, int inst_id, int ch, bool clear_on_read, bool display)
{
   int addr = elb_mx_mac_t::ELB_MX_MAC_INTCONTROL_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_INTCONTROL_ADDR_INC;
   int data = elb_mx_get_field(elb_mx_apb_read(chip_id, inst_id, addr), elb_mx_mac_t::ELB_MX_MAC_INTCONTROL_FLD_INTSTS_POS, elb_mx_mac_t::ELB_MX_MAC_INTCONTROL_FLD_INTSTS_SZ);
   MX_DEBUG_MSG("elb_mx_get_mac_int, MX%d ch%d, clear_on_read = %d, data = 0x%x\n", inst_id, ch, clear_on_read, data);
   if (display && data != 0) {
      int len = sizeof(elb_mx_mac_int_info)/(2*sizeof(elb_mx_mac_int_info[0]));
      for (int i = 0; i < len; i++) {
         int field_value = (data >> i)&0x1; 
         string field_status = elb_mx_int_msg(elb_mx_mac_int_info[i*2+0], field_value);
         if (field_value == 1) {
            MX_DEBUG_MSG("MX%d ch%d mac_intsts[%2d]=1 %s, field: %s\n", inst_id, ch, i, field_status.c_str(), elb_mx_mac_int_info[i*2+1].c_str());
         }
      }
   }
   if (clear_on_read && data != 0) {
      elb_mx_clear_mac_int(chip_id, inst_id, ch, data);
   }
   return data;
}

void
elb_mx_enable_mac_int(int chip_id, int inst_id, int ch, int vec)
{
   MX_DEBUG_MSG("elb_mx_enable_mac_int, MX%d ch%d, vec = 0x%x\n", inst_id, ch, vec);
   int addr = elb_mx_mac_t::ELB_MX_MAC_INTCONTROL_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_INTCONTROL_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   // clear intsts because it is also interrupt override field
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_INTCONTROL_FLD_INTSTS_POS, elb_mx_mac_t::ELB_MX_MAC_INTCONTROL_FLD_INTSTS_SZ, 0);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_INTCONTROL_FLD_INTENA_POS, elb_mx_mac_t::ELB_MX_MAC_INTCONTROL_FLD_INTENA_SZ, vec);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

void
elb_mx_clear_mac_int(int chip_id, int inst_id, int ch, int vec)
{
   MX_DEBUG_MSG("elb_mx_clear_mac_int, MX%d ch%d, vec = 0x%x\n", inst_id, ch, vec);
   int addr = elb_mx_mac_t::ELB_MX_MAC_INTCONTROL_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_INTCONTROL_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   // clear intsts because it is also interrupt override field
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_INTCONTROL_FLD_INTSTS_POS, elb_mx_mac_t::ELB_MX_MAC_INTCONTROL_FLD_INTSTS_SZ, 0);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_INTCONTROL_FLD_INTCLR_POS, elb_mx_mac_t::ELB_MX_MAC_INTCONTROL_FLD_INTCLR_SZ, vec);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
   elb_mx_set_field(&data, elb_mx_mac_t::ELB_MX_MAC_INTCONTROL_FLD_INTCLR_POS, elb_mx_mac_t::ELB_MX_MAC_INTCONTROL_FLD_INTCLR_SZ, 0);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

// Comira AN Interrupt
int
elb_mx_get_an_int(int chip_id, int inst_id, int ch, bool clear_on_read, bool display)
{
   int addr = elb_mx_an_t::ELB_MX_AN_ANINTCONTROL_ADDR0 + ch * elb_mx_an_t::ELB_MX_AN_ANINTCONTROL_ADDR_INC;
   int data = elb_mx_get_field(elb_mx_apb_read(chip_id, inst_id, addr), elb_mx_an_t::ELB_MX_AN_ANINTCONTROL_FLD_INTSTS_POS, elb_mx_an_t::ELB_MX_AN_ANINTCONTROL_FLD_INTSTS_SZ);
   MX_DEBUG_MSG("elb_mx_get_an_int, MX%d ch%d, clear_on_read = %d, data = 0x%x\n", inst_id, ch, clear_on_read, data);
   if (display && data != 0) {
      int len = sizeof(elb_mx_an_int_info)/(2*sizeof(elb_mx_an_int_info[0]));
      for (int i = 0; i < len; i++) {
         int field_value = (data >> i)&0x1;
         string field_status = elb_mx_int_msg(elb_mx_an_int_info[i*2+0], field_value);
         if (field_value == 1) {
            MX_DEBUG_MSG("MX%d ch%d anint[%2d]=1 %s, field: %s\n", inst_id, ch, i, field_status.c_str(), elb_mx_an_int_info[i*2+1].c_str());
         }
      }
   }
   if (clear_on_read && data != 0) {
      elb_mx_clear_an_int(chip_id, inst_id, ch, data);
   }
   return data;
}

void
elb_mx_enable_an_int(int chip_id, int inst_id, int ch, int vec)
{
   MX_DEBUG_MSG("elb_mx_enable_an_int, MX%d ch%d, vec = 0x%x\n", inst_id, ch, vec);
   int addr = elb_mx_an_t::ELB_MX_AN_ANINTCONTROL_ADDR0 + ch * elb_mx_an_t::ELB_MX_AN_ANINTCONTROL_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   // clear intsts because it is also interrupt override field
   elb_mx_set_field(&data, elb_mx_an_t::ELB_MX_AN_ANINTCONTROL_FLD_INTSTS_POS, elb_mx_an_t::ELB_MX_AN_ANINTCONTROL_FLD_INTSTS_SZ, 0);
   elb_mx_set_field(&data, elb_mx_an_t::ELB_MX_AN_ANINTCONTROL_FLD_INTENA_POS, elb_mx_an_t::ELB_MX_AN_ANINTCONTROL_FLD_INTENA_SZ, vec);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

void
elb_mx_clear_an_int(int chip_id, int inst_id, int ch, int vec)
{
   MX_DEBUG_MSG("elb_mx_clear_an_int, MX%d ch%d, vec = 0x%x\n", inst_id, ch, vec);
   int addr = elb_mx_an_t::ELB_MX_AN_ANINTCONTROL_ADDR0 + ch * elb_mx_an_t::ELB_MX_AN_ANINTCONTROL_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   // clear intsts because it is also interrupt override field
   elb_mx_set_field(&data, elb_mx_an_t::ELB_MX_AN_ANINTCONTROL_FLD_INTSTS_POS, elb_mx_an_t::ELB_MX_AN_ANINTCONTROL_FLD_INTSTS_SZ, 0);
   elb_mx_set_field(&data, elb_mx_an_t::ELB_MX_AN_ANINTCONTROL_FLD_INTCLR_POS, elb_mx_an_t::ELB_MX_AN_ANINTCONTROL_FLD_INTCLR_SZ, vec);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
   elb_mx_set_field(&data, elb_mx_an_t::ELB_MX_AN_ANINTCONTROL_FLD_INTCLR_POS, elb_mx_an_t::ELB_MX_AN_ANINTCONTROL_FLD_INTCLR_SZ, 0);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

////////////////////////////////////////////////////////////
// MX Interrupt
////////////////////////////////////////////////////////////
int
elb_mx_get_int_groups(int chip_id, int inst_id, bool display)
{
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_groups.read();
   int data = mx_csr.int_groups.all().convert_to<int>();
   MX_DEBUG_MSG("elb_mx_get_int_groups, MX%d data = 0x%x\n", inst_id, data);
   if (display && data != 0) {
      int len = sizeof(elb_mx_int_groups_info)/(2*sizeof(elb_mx_int_groups_info[0]));
      for (int i = 0; i < len; i++) {
         int field_value = (data >> i)&0x1;
         string field_status = elb_mx_int_msg(elb_mx_int_groups_info[i*2+0], field_value);
         if (field_value == 1) {
            MX_DEBUG_MSG("MX%d int_groups[%2d]=1 %s, field: %s\n", inst_id, i, field_status.c_str(), elb_mx_int_groups_info[i*2+1].c_str());
         }
      }
   }
   return data;
}

int
elb_mx_get_int_sd(int chip_id, int inst_id, bool clear_on_read, bool display)
{
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_sd.read();
   int data = mx_csr.int_sd.all().convert_to<int>();
   MX_DEBUG_MSG("elb_mx_get_int_sd, MX%d clear_on_read = %d, data = 0x%x\n", inst_id, clear_on_read, data);
   if (display && data != 0) {
      int len = sizeof(elb_mx_int_sd_info)/(2*sizeof(elb_mx_int_sd_info[0]));
      for (int i = 0; i < len; i++) {
         int field_value = (data >> i)&0x1;
         string field_status = elb_mx_int_msg(elb_mx_int_sd_info[i*2+0], field_value);
         if (field_value == 1) {
            MX_DEBUG_MSG("MX%d %10s[%2d]=1 %s, field: %s\n", inst_id, "int_sd", i, field_status.c_str(), elb_mx_int_sd_info[i*2+1].c_str());
         }
      }
   }
   if (clear_on_read && data != 0) {
      elb_mx_clear_int_sd(chip_id, inst_id, data);
   }
   return data;
}

void
elb_mx_enable_int_sd(int chip_id, int inst_id, int vec)
{
   MX_DEBUG_MSG("elb_mx_enable_int_sd, MX%d vec = 0x%x\n", inst_id, vec);
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_sd.int_enable_set.all(vec);
   mx_csr.int_sd.int_enable_set.write();
}

void
elb_mx_disable_int_sd(int chip_id, int inst_id, int vec)
{
   MX_DEBUG_MSG("elb_mx_disable_int_sd, MX%d vec = 0x%x\n", inst_id, vec);
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_sd.int_enable_clear.all(vec);
   mx_csr.int_sd.int_enable_clear.write();
}

void
elb_mx_clear_int_sd(int chip_id, int inst_id, int vec)
{
   MX_DEBUG_MSG("elb_mx_clear_int_sd, MX%d vec = 0x%x\n", inst_id, vec);
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_sd.intreg.all(vec);
   mx_csr.int_sd.intreg.write();
}

int
elb_mx_get_int_mac(int chip_id, int inst_id, bool clear_on_read, bool display)
{
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_mac.read();
   int data = mx_csr.int_mac.all().convert_to<int>();
   MX_DEBUG_MSG("elb_mx_get_int_mac, MX%d clear_on_read = %d, data = 0x%x\n", inst_id, clear_on_read, data);
   if (display && data != 0) {
      int len = sizeof(elb_mx_int_mac_info)/(2*sizeof(elb_mx_int_mac_info[0]));
      for (int i = 0; i < len; i++) {
         int field_value = (data >> i)&0x1;
         string field_status = elb_mx_int_msg(elb_mx_int_mac_info[i*2+0], field_value);
         if (field_value == 1) {
            MX_DEBUG_MSG("MX%d %10s[%2d]=1 %s, field: %s\n", inst_id, "int_mac", i, field_status.c_str(), elb_mx_int_mac_info[i*2+1].c_str());
         }
      }
   }
   if (clear_on_read && data != 0) {
      elb_mx_clear_int_mac(chip_id, inst_id, data);
   }
   return data;
}

void
elb_mx_enable_int_mac(int chip_id, int inst_id, int vec)
{
   MX_DEBUG_MSG("elb_mx_enable_int_mac, MX%d vec = 0x%x\n", inst_id, vec);
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_mac.int_enable_set.all(vec);
   mx_csr.int_mac.int_enable_set.write();
}

void
elb_mx_disable_int_mac(int chip_id, int inst_id, int vec)
{
   MX_DEBUG_MSG("elb_mx_disable_int_mac, MX%d vec = 0x%x\n", inst_id, vec);
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_mac.int_enable_clear.all(vec);
   mx_csr.int_mac.int_enable_clear.write();
}

void
elb_mx_clear_int_mac(int chip_id, int inst_id, int vec)
{
   MX_DEBUG_MSG("elb_mx_clear_int_mac, MX%d vec = 0x%x\n", inst_id, vec);
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_mac.intreg.all(vec);
   mx_csr.int_mac.intreg.write();
}

int
elb_mx_get_int_ecc(int chip_id, int inst_id, bool clear_on_read, bool display)
{
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_ecc.read();
   int data = mx_csr.int_ecc.all().convert_to<int>();
   MX_DEBUG_MSG("elb_mx_get_int_ecc, MX%d clear_on_read = %d, data = 0x%x\n", inst_id, clear_on_read, data);
   if (display && data != 0) {
      int len = sizeof(elb_mx_int_ecc_info)/(2*sizeof(elb_mx_int_ecc_info[0]));
      for (int i = 0; i < len; i++) {
         int field_value = (data >> i)&0x1;
         string field_status = elb_mx_int_msg(elb_mx_int_ecc_info[i*2+0], field_value);
         if (field_value == 1) {
            MX_DEBUG_MSG("MX%d %10s[%2d]=1 %s, field: %s\n", inst_id, "int_ecc", i, field_status.c_str(), elb_mx_int_ecc_info[i*2+1].c_str());
         }
      }
   }
   if (clear_on_read && data != 0) {
      elb_mx_clear_int_ecc(chip_id, inst_id, data);
   }
   return data;
}

void
elb_mx_enable_int_ecc(int chip_id, int inst_id, int vec)
{
   MX_DEBUG_MSG("elb_mx_enable_int_ecc, MX%d vec = 0x%x\n", inst_id, vec);
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_ecc.int_enable_set.all(vec);
   mx_csr.int_ecc.int_enable_set.write();
}

void
elb_mx_disable_int_ecc(int chip_id, int inst_id, int vec)
{
   MX_DEBUG_MSG("elb_mx_disable_int_ecc, MX%d vec = 0x%x\n", inst_id, vec);
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_ecc.int_enable_clear.all(vec);
   mx_csr.int_ecc.int_enable_clear.write();
}

void
elb_mx_clear_int_ecc(int chip_id, int inst_id, int vec)
{
   MX_DEBUG_MSG("elb_mx_clear_int_ecc, MX%d vec = 0x%x\n", inst_id, vec);
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_ecc.intreg.all(vec);
   mx_csr.int_ecc.intreg.write();
}

int
elb_mx_get_int_fixer(int chip_id, int inst_id, bool clear_on_read, bool display)
{
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_fixer.read();
   int data = mx_csr.int_fixer.all().convert_to<int>();
   MX_DEBUG_MSG("elb_mx_get_int_fixer, MX%d clear_on_read = %d, data = 0x%x\n", inst_id, clear_on_read, data);
   if (display && data != 0) {
      int len = sizeof(elb_mx_int_fixer_info)/(2*sizeof(elb_mx_int_fixer_info[0]));
      for (int i = 0; i < len; i++) {
         int field_value = (data >> i)&0x1;
         string field_status = elb_mx_int_msg(elb_mx_int_fixer_info[i*2+0], field_value);
         if (field_value == 1) {
            MX_DEBUG_MSG("MX%d %10s[%2d]=1 %s, field: %s\n", inst_id, "int_fixer", i, field_status.c_str(), elb_mx_int_fixer_info[i*2+1].c_str());
         }
      }
   }
   if (clear_on_read && data != 0) {
      elb_mx_clear_int_fixer(chip_id, inst_id, data);
   }
   return data;
}

void
elb_mx_enable_int_fixer(int chip_id, int inst_id, int vec)
{
   MX_DEBUG_MSG("elb_mx_enable_int_fixer, MX%d vec = 0x%x\n", inst_id, vec);
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_fixer.int_enable_set.all(vec);
   mx_csr.int_fixer.int_enable_set.write();
}

void
elb_mx_disable_int_fixer(int chip_id, int inst_id, int vec)
{
   MX_DEBUG_MSG("elb_mx_disable_int_fixer, MX%d vec = 0x%x\n", inst_id, vec);
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_fixer.int_enable_clear.all(vec);
   mx_csr.int_fixer.int_enable_clear.write();
}

void
elb_mx_clear_int_fixer(int chip_id, int inst_id, int vec)
{
   MX_DEBUG_MSG("elb_mx_clear_int_fixer, MX%d vec = 0x%x\n", inst_id, vec);
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_fixer.intreg.all(vec);
   mx_csr.int_fixer.intreg.write();
}

int
elb_mx_get_int_flush(int chip_id, int inst_id, bool clear_on_read, bool display)
{
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_flush.read();
   int data = mx_csr.int_flush.all().convert_to<int>();
   MX_DEBUG_MSG("elb_mx_get_int_flush, MX%d clear_on_read = %d, data = 0x%x\n", inst_id, clear_on_read, data);
   if (display && data != 0) {
      int len = sizeof(elb_mx_int_flush_info)/(2*sizeof(elb_mx_int_flush_info[0]));
      for (int i = 0; i < len; i++) {
         int field_value = (data >> i)&0x1;
         string field_status = elb_mx_int_msg(elb_mx_int_flush_info[i*2+0], field_value);
         if (field_value == 1) {
            MX_DEBUG_MSG("MX%d %10s[%2d]=1 %s, field: %s\n", inst_id, "int_flush", i, field_status.c_str(), elb_mx_int_flush_info[i*2+1].c_str());
         }
      }
   }
   if (clear_on_read && data != 0) {
      elb_mx_clear_int_flush(chip_id, inst_id, data);
   }
   return data;
}

void
elb_mx_enable_int_flush(int chip_id, int inst_id, int vec)
{
   MX_DEBUG_MSG("elb_mx_enable_int_flush, MX%d vec = 0x%x\n", inst_id, vec);
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_flush.int_enable_set.all(vec);
   mx_csr.int_flush.int_enable_set.write();
}

void
elb_mx_disable_int_flush(int chip_id, int inst_id, int vec)
{
   MX_DEBUG_MSG("elb_mx_disable_int_flush, MX%d vec = 0x%x\n", inst_id, vec);
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_flush.int_enable_clear.all(vec);
   mx_csr.int_flush.int_enable_clear.write();
}

void
elb_mx_clear_int_flush(int chip_id, int inst_id, int vec)
{
   MX_DEBUG_MSG("elb_mx_clear_int_flush, MX%d vec = 0x%x\n", inst_id, vec);
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.int_flush.intreg.all(vec);
   mx_csr.int_flush.intreg.write();
}

// Functions for all interrupts
void
elb_mx_get_int(int chip_id, int inst_id, bool clear_on_read)
{
   int int_groups = elb_mx_get_int_groups(chip_id, inst_id);
   if (elb_mx_get_field((uint64_t) int_groups, elb_mx_int_groups_t::ELB_MX_INT_GROUPS_INT_SD, 1)) {
      elb_mx_get_int_sd(chip_id , inst_id, clear_on_read);
   }
   int int_mac;
   if (elb_mx_get_field((uint64_t) int_groups, elb_mx_int_groups_t::ELB_MX_INT_GROUPS_INT_MAC, 1)) {
      int_mac = elb_mx_get_int_mac(chip_id , inst_id);
      if (elb_mx_get_field((uint64_t) int_mac, elb_mx_int_mac_t::ELB_MX_INT_MAC_INT_MAC, 1)) {
         for (int ch = 0; ch < 8; ch++) {
            int mac_int = elb_mx_get_mac_int(chip_id, inst_id, ch, clear_on_read);
            if (mac_int != 0) {
               elb_mx_get_mac_chsts(chip_id, inst_id, ch);
               //elb_mx_get_mac_stat_err(chip_id, inst_id, ch);
            }
         }
      }
      if (elb_mx_get_field((uint64_t) int_mac, elb_mx_int_mac_t::ELB_MX_INT_MAC_INT_AN, 1)) {
         for (int ch = 0; ch < 8; ch++) {
            elb_mx_get_an_int(chip_id, inst_id, ch);  // AN interrupt is important. Clear separately.
         }
      }
      if (clear_on_read && int_mac != 0) {
         elb_mx_clear_int_mac(chip_id, inst_id, 0x3);  // clear int_mac after clearning mac_int and an_int
      }
   }
   if (elb_mx_get_field((uint64_t) int_groups, elb_mx_int_groups_t::ELB_MX_INT_GROUPS_INT_ECC, 1)) {
      elb_mx_get_int_ecc(chip_id , inst_id, clear_on_read);
   }
   if (elb_mx_get_field((uint64_t) int_groups, elb_mx_int_groups_t::ELB_MX_INT_GROUPS_INT_FIXER, 1)) {
      elb_mx_get_int_fixer(chip_id , inst_id, clear_on_read);
   }
   if (elb_mx_get_field((uint64_t) int_groups, elb_mx_int_groups_t::ELB_MX_INT_GROUPS_INT_FLUSH, 1)) {
      elb_mx_get_int_flush(chip_id , inst_id, clear_on_read);
   }
}

void
elb_mx_enable_int(int chip_id, int inst_id)
{
   for (int i = 0; i < 8; i++) {
      elb_mx_enable_mac_int(chip_id, inst_id, i, 0x7fff);  // disable link up interrupt
      elb_mx_enable_an_int(chip_id, inst_id, i, 0xffff);
   }

   elb_mx_enable_int_sd(chip_id, inst_id, 0x1);
   elb_mx_enable_int_mac(chip_id, inst_id, 0x3);
   elb_mx_enable_int_ecc(chip_id, inst_id, 0xf);
   elb_mx_enable_int_fixer(chip_id, inst_id, 0x3ff);
   elb_mx_enable_int_flush(chip_id, inst_id, 0x1f);
}

void
elb_mx_clear_int(int chip_id, int inst_id)
{
   // Clear interrupts from bottom up
   for (int i = 0; i < 8; i++) {
      elb_mx_clear_mac_int(chip_id, inst_id, i, 0xffff);
      elb_mx_clear_an_int(chip_id, inst_id, i, 0xffff);
   }

   elb_mx_clear_int_sd(chip_id, inst_id, 0x1);
   elb_mx_clear_int_mac(chip_id, inst_id, 0x3);
   elb_mx_clear_int_ecc(chip_id, inst_id, 0xf);
   elb_mx_clear_int_fixer(chip_id, inst_id, 0x3ff);
   elb_mx_clear_int_flush(chip_id, inst_id, 0x1f);
}

////////////////////////////////////////////////////////////
// Debug
////////////////////////////////////////////////////////////
void
elb_mx_dump_mx_cfg(int chip_id, int inst_id) {
   elb_mx_csr_t & mx_csr = PEN_BLK_REG_MODEL_ACCESS(elb_mx_csr_t, chip_id, inst_id);
   mx_csr.cfg_swrst.read();
   MX_DEBUG_MSG("MX%d elb_mx_show_mx_cfg cfg_swrst=%x\n", inst_id, mx_csr.cfg_swrst.all().convert_to<int>());
   mx_csr.cfg_mac.read();
   MX_DEBUG_MSG("MX%d elb_mx_show_mx_cfg cfg_mac=%x\n", inst_id, mx_csr.cfg_mac.all().convert_to<int>());
   mx_csr.cfg_mac_pfc_txxoff.read();
   MX_DEBUG_MSG("MX%d elb_mx_show_mx_cfg cfg_mac_pfc_txxoff=%x\n", inst_id, mx_csr.cfg_mac_pfc_txxoff.all().convert_to<int>());
   for (int i = 0; i < 5; i++) {
      mx_csr.cfg_port[i].read();
      MX_DEBUG_MSG("MX%d elb_mx_show_mx_cfg cfg_port[%d]=%x\n", inst_id, i, mx_csr.cfg_port[i].all().convert_to<int>());
      mx_csr.cfg_fixer[i].read();
      MX_DEBUG_MSG("MX%d elb_mx_show_mx_cfg cfg_fixer[%d]=%x\n", inst_id, i, mx_csr.cfg_fixer[i].all().convert_to<int>());
   }
   elb_mx_get_slot(chip_id, inst_id);
}

void
elb_mx_dump_ch_cfg(int chip_id, int inst_id, int ch) {
   int addr = elb_mx_mac_t::ELB_MX_MAC_CHMODE_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHMODE_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   MX_DEBUG_MSG("MX%d ch%d elb_mx_show_ch_cfg chmode=0x%" PRIx64 "\n", inst_id, ch, data);
   addr = elb_mx_mac_t::ELB_MX_MAC_MACCFG_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_MACCFG_ADDR_INC;
   data = elb_mx_apb_read(chip_id, inst_id, addr);
   MX_DEBUG_MSG("MX%d ch%d elb_mx_show_ch_cfg maccfg=0x%" PRIx64 "\n", inst_id, ch, data);
   addr = elb_mx_mac_t::ELB_MX_MAC_CHCONFIG3_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHCONFIG3_ADDR_INC;
   data = elb_mx_apb_read(chip_id, inst_id, addr);
   MX_DEBUG_MSG("MX%d ch%d elb_mx_show_ch_cfg chconfig3=0x%" PRIx64 "\n", inst_id, ch, data);
   addr = elb_mx_mac_t::ELB_MX_MAC_CHCONFIG4_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHCONFIG4_ADDR_INC;
   data = elb_mx_apb_read(chip_id, inst_id, addr);
   MX_DEBUG_MSG("MX%d ch%d elb_mx_show_ch_cfg chconfig4=0x%" PRIx64 "\n", inst_id, ch, data);
   addr = elb_mx_mac_t::ELB_MX_MAC_CHCONFIG5_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHCONFIG5_ADDR_INC;
   data = elb_mx_apb_read(chip_id, inst_id, addr);
   MX_DEBUG_MSG("MX%d ch%d elb_mx_show_ch_cfg chconfig5=0x%" PRIx64 "\n", inst_id, ch, data);
   addr = elb_mx_mac_t::ELB_MX_MAC_CHCONFIG6_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHCONFIG6_ADDR_INC;
   data = elb_mx_apb_read(chip_id, inst_id, addr);
   MX_DEBUG_MSG("MX%d ch%d elb_mx_show_ch_cfg chconfig6=0x%" PRIx64 "\n", inst_id, ch, data);
   addr = elb_mx_mac_t::ELB_MX_MAC_CHCONFIG7_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHCONFIG7_ADDR_INC;
   data = elb_mx_apb_read(chip_id, inst_id, addr);
   MX_DEBUG_MSG("MX%d ch%d elb_mx_show_ch_cfg chconfig7=0x%" PRIx64 "\n", inst_id, ch, data);
   addr = elb_mx_mac_t::ELB_MX_MAC_CHCONFIG8_ADDR0 + ch * elb_mx_mac_t::ELB_MX_MAC_CHCONFIG8_ADDR_INC;
   data = elb_mx_apb_read(chip_id, inst_id, addr);
   MX_DEBUG_MSG("MX%d ch%d elb_mx_show_ch_cfg chconfig8=0x%" PRIx64 "\n", inst_id, ch, data);
}

////////////////////////////////////////////////////////////
// Comira MAC Auto-Negotiation
////////////////////////////////////////////////////////////
int
elb_mx_an_get_page_addr(int ch, int tx)
{
   if (tx == 1) {
      return 0x5200 + 0x80*ch;
   } else {
      return 0x5600 + 0x80*ch;
   }
}

void
elb_mx_an_clean_rx_page(int chip_id, int inst_id, int ch)
{
   int base_addr = elb_mx_an_get_page_addr(ch, 0);
   for (int i = 0; i < 8; i++) {
      elb_mx_apb_write(chip_id, inst_id, base_addr+i*8, 0);
   }
}

uint64_t
elb_mx_an_get_rx_page(int chip_id, int inst_id, int ch, int page_idx)
{
   int base_addr = elb_mx_an_get_page_addr(ch, 0);
   return elb_mx_apb_read(chip_id, inst_id, base_addr+page_idx*8);
}

uint64_t
elb_mx_an_get_rx_base_page(int chip_id, int inst_id, int ch, bool display)
{
   uint64_t page0 = elb_mx_an_get_rx_page(chip_id, inst_id, ch, 0);

   if (display) {
      int pause_cap = (int) elb_mx_get_field(page0, elb_mx_an_base_config_t::ELB_MX_AN_BASE_PAUSE_CAPABILITY_POS, elb_mx_an_base_config_t::ELB_MX_AN_BASE_PAUSE_CAPABILITY_SZ);
      int len = sizeof(elb_mx_an_pause_info)/(2*sizeof(elb_mx_an_pause_info[0]));
      for (int i = 0; i < len; i++) {
         int field_value = (pause_cap >> i)&0x1;
         MX_DEBUG_MSG("MX%d ch%d rx base_page[%2d] = %s = %d\n", inst_id, ch, elb_mx_an_base_config_t::ELB_MX_AN_BASE_PAUSE_CAPABILITY_POS+i, elb_mx_an_pause_info[i*2+1].c_str(), field_value);
      }

      int np = (int) elb_mx_get_field(page0, elb_mx_an_base_config_t::ELB_MX_AN_BASE_NP_POS, elb_mx_an_base_config_t::ELB_MX_AN_BASE_NP_SZ);
      MX_DEBUG_MSG("MX%d ch%d rx base_page[%2d] = np = %d\n", inst_id, ch, elb_mx_an_base_config_t::ELB_MX_AN_BASE_NP_POS, np);

      int tech_cap = (int) elb_mx_get_field(page0, elb_mx_an_base_config_t::ELB_MX_AN_BASE_TECHABILITY_POS, elb_mx_an_base_config_t::ELB_MX_AN_BASE_TECHABILITY_SZ);
      len = sizeof(elb_mx_an_techability_info)/(2*sizeof(elb_mx_an_techability_info[0]));
      for (int i = 0; i < len; i++) {
         int field_value = (tech_cap >> i)&0x1;
         MX_DEBUG_MSG("MX%d ch%d rx base_page[%2d] = %s = %d\n", inst_id, ch, elb_mx_an_base_config_t::ELB_MX_AN_BASE_TECHABILITY_POS+i, elb_mx_an_techability_info[i*2+1].c_str(), field_value);
      }

      int fec_cap = (int) elb_mx_get_field(page0, elb_mx_an_base_config_t::ELB_MX_AN_BASE_FEC_CAPABILITY_POS, elb_mx_an_base_config_t::ELB_MX_AN_BASE_FEC_CAPABILITY_SZ);
      len = sizeof(elb_mx_an_fec_info)/(2*sizeof(elb_mx_an_fec_info[0]));
      for (int i = 0; i < len; i++) {
         int field_value = (fec_cap >> i)&0x1;
         MX_DEBUG_MSG("MX%d ch%d rx base_page[%2d] = %s = %d\n", inst_id, ch, elb_mx_an_base_config_t::ELB_MX_AN_BASE_FEC_CAPABILITY_POS+i, elb_mx_an_fec_info[i*2+1].c_str(), field_value);
      }
   }
   return page0;
}

void
elb_mx_an_write_tx_page(int chip_id, int inst_id, int ch, int idx, uint64_t page)
{
   MX_DEBUG_MSG("MX%d ch%d elb_mx_an_write_tx_page idx = %d, page = 0x%" PRIx64 "\n", inst_id, ch, idx, page);
   int base_addr = elb_mx_an_get_page_addr(ch, 1);
   elb_mx_apb_write(chip_id, inst_id, base_addr+idx*8, page);
}

int
elb_mx_an_write_tx_page(int chip_id, int inst_id, int ch, uint64_t *pages)
{
   if (pages == NULL) {
      MX_DEBUG_ERR("elb_mx_an_write_tx_page argument pages is NULL!\n");
      return -1;
   }
   for (int i = 0; i < 3; i++) {
      elb_mx_an_write_tx_page(chip_id, inst_id, ch, i, pages[i]);
   }
   return 0;
}

void
elb_mx_an_reset(int chip_id, int inst_id, int ch, int value)
{
   MX_DEBUG_MSG("MX%d ch%d elb_mx_an_reset value = %d\n", inst_id, ch, value);
   int addr = elb_mx_an_t::ELB_MX_AN_ANCFG_ADDR0 + ch * elb_mx_an_t::ELB_MX_AN_ANCFG_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_an_t::ELB_MX_AN_ANCFG_FLD_ANSWRST_POS, elb_mx_an_t::ELB_MX_AN_ANCFG_FLD_ANSWRST_SZ, value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

void
elb_mx_an_enable(int chip_id, int inst_id, int ch, int value)
{
   MX_DEBUG_MSG("MX%d ch%d elb_mx_an_enable value = %d\n", inst_id, ch, value);
   int addr = elb_mx_an_t::ELB_MX_AN_ANCFG_ADDR0 + ch * elb_mx_an_t::ELB_MX_AN_ANCFG_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_an_t::ELB_MX_AN_ANCFG_FLD_ANENABLE_POS, elb_mx_an_t::ELB_MX_AN_ANCFG_FLD_ANENABLE_SZ, value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

// set 1 to ignore nonce collision (for loopback test)
void
elb_mx_an_ignore_nonce(int chip_id, int inst_id, int ch, int value)
{
   MX_DEBUG_MSG("MX%d ch%d elb_mx_an_ignore_nonce value = %d\n", inst_id, ch, value);
   int addr = elb_mx_an_t::ELB_MX_AN_ANCFG_ADDR0 + ch * elb_mx_an_t::ELB_MX_AN_ANCFG_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_an_t::ELB_MX_AN_ANCFG_FLD_ANIGNORENONCE_POS, elb_mx_an_t::ELB_MX_AN_ANCFG_FLD_ANIGNORENONCE_SZ, value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

// Skip the setting if value == 0.
void
elb_mx_an_set_breaklinktimer(int chip_id, int inst_id, int ch, uint32_t value)
{
   MX_DEBUG_MSG("MX%d ch%d elb_mx_an_set_breaklinktimer value = 0x%" PRIx32 "\n", inst_id, ch, value);
   if (value == 0) return;  // keep default value 60ms~75ms
   int addr = elb_mx_an_t::ELB_MX_AN_ANCFG_ADDR0 + ch * elb_mx_an_t::ELB_MX_AN_ANCFG_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   elb_mx_set_field(&data, elb_mx_an_t::ELB_MX_AN_ANCFG_FLD_BREAKLINKTIMER_POS, elb_mx_an_t::ELB_MX_AN_ANCFG_FLD_BREAKLINKTIMER_SZ, value);
   elb_mx_apb_write(chip_id, inst_id, addr, data);
}

void 
elb_mx_an_init(int chip_id, int inst_id, int ch, int user_cap, int user_fec, int user_pause, int ignore_nonce, uint32_t breaklinktimer)
{
   MX_DEBUG_MSG("MX%d ch%d elb_mx_an_init user_cap = 0x%x, user_fec = 0x%x, user_pause = %d, ignore_nonce = %d, breaklinktimer = 0x%x\n", inst_id, ch, user_cap, user_fec, user_pause, ignore_nonce, breaklinktimer);

   // Prepare pages
   uint64_t pages[3] = {0, 0, 0};

   elb_mx_set_field(&pages[0], elb_mx_an_base_config_t::ELB_MX_AN_BASE_SELECTOR_POS, elb_mx_an_base_config_t::ELB_MX_AN_BASE_SELECTOR_SZ, 0x1);  // Set to 0x1 for base pages

   elb_mx_set_field(&pages[0], elb_mx_an_base_config_t::ELB_MX_AN_BASE_PAUSE_CAPABILITY_POS, elb_mx_an_base_config_t::ELB_MX_AN_BASE_PAUSE_CAPABILITY_SZ, user_pause);  // Bit0: Pause is supported. Bit1: Pause is asynchronous

   int cap_value;
   int fec_value;
   if (user_cap == 0) {
      cap_value = (1 << elb_mx_an_techability_t::ELB_MX_AN_TECH_KR_10G) |
                  (1 << elb_mx_an_techability_t::ELB_MX_AN_TECH_KR4_40G) |
                  (1 << elb_mx_an_techability_t::ELB_MX_AN_TECH_CR4_40G) |
                  (1 << elb_mx_an_techability_t::ELB_MX_AN_TECH_KP4_100G) |
                  (1 << elb_mx_an_techability_t::ELB_MX_AN_TECH_CR4_100G) |
                  (1 << elb_mx_an_techability_t::ELB_MX_AN_TECH_KRS_CRS_25G) |
                  (1 << elb_mx_an_techability_t::ELB_MX_AN_TECH_KR_CR_25G);
      fec_value = (1 << elb_mx_an_fec_capability_t::ELB_MX_AN_FEC_RSFEC_25G_REQ);
   } else {
      cap_value = user_cap;
      fec_value = user_fec;
   }

   elb_mx_set_field(&pages[0], elb_mx_an_base_config_t::ELB_MX_AN_BASE_TECHABILITY_POS, elb_mx_an_base_config_t::ELB_MX_AN_BASE_TECHABILITY_SZ, cap_value);
   elb_mx_set_field(&pages[0], elb_mx_an_base_config_t::ELB_MX_AN_BASE_FEC_CAPABILITY_POS, elb_mx_an_base_config_t::ELB_MX_AN_BASE_FEC_CAPABILITY_SZ, fec_value);

   // reset, program and unreset an cfg
   elb_mx_an_reset(chip_id, inst_id, ch, 1);
   elb_mx_an_ignore_nonce(chip_id, inst_id, ch, ignore_nonce);
   elb_mx_an_clean_rx_page(chip_id, inst_id, ch);
   elb_mx_an_write_tx_page(chip_id, inst_id, ch, pages);
   elb_mx_an_set_breaklinktimer(chip_id, inst_id, ch, breaklinktimer);
   elb_mx_an_enable(chip_id, inst_id, ch, 1);
   elb_mx_an_reset(chip_id, inst_id, ch, 0);
}

uint64_t
elb_mx_get_ansts(int chip_id, int inst_id, int ch, bool display)
{
   int addr = elb_mx_an_t::ELB_MX_AN_ANSTS_ADDR0 + ch * elb_mx_an_t::ELB_MX_AN_ANSTS_ADDR_INC;
   uint64_t data = elb_mx_apb_read(chip_id, inst_id, addr);
   if (display == 1) {
      int len = sizeof(elb_mx_an_ansts_info)/(2*sizeof(elb_mx_an_ansts_info[0]));
      for (int i = 0; i < len; i++) {
         int field_value = (data >> i)&0x1;
         string field_status = elb_mx_int_msg(elb_mx_an_ansts_info[i*2+0], field_value);
         MX_DEBUG_MSG("MX%d ch%d ansts[%2d] = %d %s, field: %s\n", inst_id, ch, i, field_value, field_status.c_str(), elb_mx_an_ansts_info[i*2+1].c_str());
      }

   }
   return data;
}

bool
elb_mx_an_get_an_complete(int chip_id, int inst_id, int ch)
{
   uint64_t rdata = elb_mx_get_ansts(0, 0, ch, 0);
   bool ancomplete   = (elb_mx_get_field(rdata, elb_mx_an_t::ELB_MX_AN_ANSTS_FLD_ANCOMPLETE_POS, 1) == 1);
   bool receivedlast = (elb_mx_get_field(rdata, elb_mx_an_t::ELB_MX_AN_ANSTS_FLD_RECEIVEDLAST_POS, 1) == 1);
   MX_DEBUG_MSG("MX%d ch%d elb_mx_an_get_an_complete ancomplete = %d, receivedlast = %d\n", inst_id, ch, ancomplete, receivedlast);
   return (ancomplete & receivedlast);
}

uint32_t
elb_mx_an_get_hcd(int chip_id, int inst_id, int ch)
{
   uint64_t rdata = elb_mx_get_ansts(0, 0, ch, 0);
   uint64_t anmode = elb_mx_get_field(rdata, elb_mx_an_t::ELB_MX_AN_ANSTS_FLD_ANMODE_POS, elb_mx_an_t::ELB_MX_AN_ANSTS_FLD_ANMODE_SZ);

   // For old speed, anmode is mapped to capri's serdes hcd value
   uint32_t hcd;
   if (((anmode >> 20) & 0x1) == 1)      hcd = 0x24; // 400GBASE-CR4 or 400GBASE-KR4
   else if (((anmode >> 19) & 0x1) == 1) hcd = 0x23; // 200GBASE-CR2 or 200GBASE-KR2
   else if (((anmode >> 18) & 0x1) == 1) hcd = 0x11; // 200GBASE-CR4 or 200GBASE-KR4
   else if (((anmode >> 17) & 0x1) == 1) hcd = 0x22; // 100GBASE-CR or 100GBASE-KR
   else if (((anmode >> 16) & 0x1) == 1) hcd = 0x10; // 100GBASE-CR2 or 100GBASE-KR2
   else if (((anmode >> 15) & 0x1) == 1) hcd = 0x9;  // 100GBASE-CR4
   else if (((anmode >> 14) & 0x1) == 1) hcd = 0x8;  // 100GBASE-KR4
   else if (((anmode >> 13) & 0x1) == 1) hcd = 0x6;  // 100GBASE-KP4
   else if (((anmode >> 12) & 0x1) == 1) hcd = 0x5;  // 100GBASE-CR10
   else if (((anmode >> 11) & 0x1) == 1) hcd = 0xE;  // 50GBASE-KR or 50GBASE-CR
   else if (((anmode >> 10) & 0x1) == 1) hcd = 0x21; // 50GBASE-KR2 or 50GBASE-CR2 (Consortium)
   else if (((anmode >>  9) & 0x1) == 1) hcd = 0x4;  // 40GBASE-CR4
   else if (((anmode >>  8) & 0x1) == 1) hcd = 0x3;  // 40GBASE-KR4
   else if (((anmode >>  7) & 0x1) == 1) hcd = 0xB;  // 25GBASE-KR or 25GBASE-CR
   else if (((anmode >>  6) & 0x1) == 1) hcd = 0xA;  // 25GBASE-KR-S or 25GBASE-CR-S
   else if (((anmode >>  5) & 0x1) == 1) hcd = 0x20; // 25GBASE-KR1 or 25GBASE-CR1 (Consortium)
   else if (((anmode >>  4) & 0x1) == 1) hcd = 0x2;  // 10GBASE-KR
   else if (((anmode >>  3) & 0x1) == 1) hcd = 0x1;  // 10GBASE-KX4
   else if (((anmode >>  2) & 0x1) == 1) hcd = 0xD;  // 5GBASE-KR
   else if (((anmode >>  1) & 0x1) == 1) hcd = 0xC;  // 2.5GBASE-KX
   else if (((anmode >>  0) & 0x1) == 1) hcd = 0x0;  // 1000BASE-KX
   else hcd = 0x0;

   MX_DEBUG_MSG("MX%d ch%d elb_mx_an_get_hcd anmode = 0x%" PRIx64 ", hcd = 0x%" PRIx32 "\n", inst_id, ch, anmode, hcd);
   return hcd;
}

int
elb_mx_an_get_25g_fcfec(int chip_id, int inst_id, int ch)
{
   uint64_t rdata = elb_mx_get_ansts(0, 0, ch, 0);
   uint64_t fecmode = elb_mx_get_field(rdata, elb_mx_an_t::ELB_MX_AN_ANSTS_FLD_FECMODE_POS, elb_mx_an_t::ELB_MX_AN_ANSTS_FLD_FECMODE_SZ);
   int fcfec = (fecmode & 0x1) == 1;
   MX_DEBUG_MSG("MX%d ch%d elb_mx_an_get_25g_fcfec fecmode = 0x%" PRIx64 ", fcfec = %d\n", inst_id, ch, fecmode, fcfec);
   return fcfec;
}

int
elb_mx_an_get_25g_rsfec(int chip_id, int inst_id, int ch)
{
   uint64_t rdata = elb_mx_get_ansts(0, 0, ch, 0);
   uint64_t fecmode = elb_mx_get_field(rdata, elb_mx_an_t::ELB_MX_AN_ANSTS_FLD_FECMODE_POS, elb_mx_an_t::ELB_MX_AN_ANSTS_FLD_FECMODE_SZ);
   int rsfec = ((fecmode >> 1) & 0x1) == 1;
   MX_DEBUG_MSG("MX%d ch%d elb_mx_an_get_25g_rsfec fecmode = 0x%" PRIx64 ", rsfec = %d\n", inst_id, ch, fecmode, rsfec);
   return rsfec;
}
