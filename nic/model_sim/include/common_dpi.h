#ifndef __CAP_COMMON_DPI_HH__
#define __CAP_COMMON_DPI_HH__
#ifdef _CSV_INCLUDED_

#include <iostream>
#include "svdpi.h"


using namespace std;

extern "C" {

void dpi_c_init_all();
void dpi_sv_puvm_mem_write(uint64_t addr, const unsigned char* data, int len);
void dpi_sv_puvm_mem_read(uint64_t addr, unsigned char* data, int len);
void dpi_sv_get_sim_time_ns(char* ts);


}
#endif //_CSV_INCLUDED_
#endif //__CAP_COMMON_DPI_HH__
