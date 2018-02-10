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
void dpi_sv_puvm_avery_mem_write(uint64_t addr, const unsigned char* data, int len);
void dpi_sv_puvm_avery_mem_read(uint64_t addr, unsigned char* data, int len);
void dpi_sv_get_sim_time_ns(char* ts);
uint64_t dpi_sv_get_sim_time_ns_val();
void dpi_force_finish();

}


#define DPI_SV_SCOPE_BEGIN(X) \
    svScope _tmp_old_sv_scope = svGetScope(); \
    svScope _tmp_nxt_sv_scope = svGetScopeFromName(X); \
    svSetScope(_tmp_nxt_sv_scope);

#define DPI_SV_SCOPE_END \
    svSetScope(_tmp_old_sv_scope);

#define DPI_SV_TOP_SCOPE_BEGIN \
    svScope _tmp_old_sv_scope = svGetScope(); \

#define DPI_SV_TOP_SCOPE_END \
    svSetScope(_tmp_old_sv_scope);


#endif //_CSV_INCLUDED_
#endif //__CAP_COMMON_DPI_HH__
