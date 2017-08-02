//
// Pkt gen class using Scapy
//

#ifndef SCAPY_PKT_GEN
#define SCAPY_PKT_GEN

#include <string>
#include <vector>
#include <boost/crc.hpp>
#include <iomanip>
#include <sstream>
#include "sknobs.h"
#include "c2scapy.h"

using namespace std;

typedef enum {
  ETH,
  LLC,
  SNAP,
  VLAN,
  QINQ,
  MPLS,
  IPV4,
  IPV6,
  ARP,
  RARP,
  NSH,
  /* ROCE, */
  /* TRILL, */
  /* VNTAG, */
  LLDP,
  LACP,
  ICMP,
  TCP,
  UDP,
  GRE,
  /* NVGRE, */
  ERSPAN,
  NUM_HEADER_TYPES
} HeaderType;

struct HeaderTypeInfo {
  string header_name;
  string scapy_header_name;
  vector<string> field_names;
  string is_field_str;
  int hdr_size;
  unsigned int curr_level;
};

class ScapyPktGen {
 public:
  ScapyPktGen(string prefix, int seed = 0xbeef);
  ~ScapyPktGen() {};
  void GenPkt();
  char *GetPktBytes() {
    pkt_bytes_char_v_.resize(pkt_bytes_.size());
    copy(pkt_bytes_.begin(), pkt_bytes_.end(), pkt_bytes_char_v_.begin());
    pkt_bytes_char_v_.push_back('\0');
    return (&pkt_bytes_char_v_[0]);
  }
  vector<unsigned char> GenPktBuff() {
    vector<unsigned char> retVal;
    GenPkt();
    retVal.resize(pkt_bytes_.size()/2+4);
    unsigned int val = 0;
    unsigned int byte_val = 0;
    boost::crc_32_type crc;
    unsigned int idx = 0;
    for ( unsigned int i = 0; i < pkt_bytes_.size(); i++) {
      unsigned char ascii_val = (unsigned char)(pkt_bytes_[i]);
      if (ascii_val > '0' && ascii_val <= '9') {
	val =  (ascii_val - '0'); 
      } else if (ascii_val >= 'a' && ascii_val <='f') {
	val = ascii_val - 'a' + 10;
      } else if (ascii_val >= 'A' && ascii_val <='F') {
	val = ascii_val - 'A' + 10;
      }
      if ((i&1) == 0) {
	byte_val = val << 4;
      } else {
	byte_val |= val;
	val = 0;
	retVal[idx] = byte_val;	
	crc.process_byte(byte_val);
	idx++;
      }
    }
    auto csum = crc.checksum();
    for (unsigned int i = 0; i < 4; i++) {
      // TODO: Fix CRC byte order
      retVal[idx+i] = csum & 0xff;
      csum >>= 8;
    }
    return retVal;
  }
    
 private:
  static const int unsigned MAX_HEADERS = 20;
  static const int unsigned MAX_MPLS_HEADERS = 7;
  // info we store for each header type
  static HeaderTypeInfo hdr_type_info_[NUM_HEADER_TYPES];
  static int unsigned hdr_info_loaded_;
  // used to find the sknobs for this generator instance
  string sknobs_prefix_;
  string pkt_bytes_;
  vector<char> pkt_bytes_char_v_;

  HeaderType PickHeaderType(vector<HeaderType> & hdr_type_options);
  void GenScapyStr(vector<HeaderType> & hdr_types, string & scapy_fmt_str);
  void GenScapyPkt(string scapy_fmt_str, string & pkt_bytes);
  void GenLegalHeaderStack(vector<HeaderType> & hdr_types, int unsigned pkt_length);
};

#endif
