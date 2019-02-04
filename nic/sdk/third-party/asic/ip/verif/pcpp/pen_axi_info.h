#ifndef _PEN_AXI_INFO_H_
#define _PEN_AXI_INFO_H_
#include <deque>
#include "cpp_int_helper.h"
struct axi_info_t {
  uint8_t   xn_type; /* 0: READ, 1: WRITE */
  uint64_t  addr;
  uint64_t  wrstrb;
  cpp_int   data;
  int       attr; /* interpretted by ports */
};
typedef std::deque<axi_info_t *> axi_xn_q;

#endif
