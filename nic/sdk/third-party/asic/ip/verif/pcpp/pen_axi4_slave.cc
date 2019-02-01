#include "pen_axi4_slave.h"
#include "sknobs.h"
#define SKNOBS_GET_V(s,def) sknobs_get_value((char *)((s).c_str()), def)
#define SKNOBS_GET_S(s,def) sknobs_get_string((char *)((s).c_str()), def)
void pen_addr_filter::read_from_cfg() {
  uint32_t num_ranges;
  if ((num_ranges=SKNOBS_GET_V(prefix+"num_ranges", 0))) {
    for (uint32_t i = 0; i < num_ranges; i++) {
      uint64_t min = SKNOBS_GET_V(prefix+"/"+std::to_string(i)+"/min", 0);
      uint64_t max = SKNOBS_GET_V(prefix+"/"+std::to_string(i)+"/max", 0);
      uint64_t assert_p = SKNOBS_GET_V(prefix+"/"+std::to_string(i)+"/assert", 1);
      std::string perm_s = SKNOBS_GET_S(prefix+"/"+std::to_string(i)+"/perm", 0);
      perms_e perm = NONE;
      if (perm_s == "read" || perm_s == "READ") {
	perm = READ;
      } else if (perm_s == "write" || perm_s == "WRITE") {
	perm = WRITE;
      } else if (perm_s == "all" || perm_s == "ALL") {
	perm = ALL;
      } 
      if (max > min) {
	add_filter(min,max, perm, assert_p != 0);
      }
    }
  }
}
