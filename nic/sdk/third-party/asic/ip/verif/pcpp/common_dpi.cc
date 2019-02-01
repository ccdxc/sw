#ifdef _CSV_INCLUDED_
#include <cassert>
#include <cstring>
#include "common_dpi.h"
#include "msg_man.h"


static const int mem_buffer_len = 1024;

extern "C" {

extern void dpi_sv_test();
extern void dpi_export_puvm_mem_write(unsigned long addr, int total, const unsigned char*, int start, int len);
extern void dpi_export_puvm_mem_read(unsigned long addr, int total, unsigned char* data, int start, int len);
extern void dpi_export_puvm_avery_mem_write(unsigned long addr, int total, const unsigned char*, int start, int len);
extern void dpi_export_puvm_avery_mem_read(unsigned long addr, int total, unsigned char* data, int start, int len);
extern const char* dpi_export_get_sim_time_ns();
extern const char* dpi_export_get_sim_time_ns_val();
extern void dpi_export_force_finish();

void dpi_c_init_all()
{
    msg_man::init();
}



void dpi_sv_puvm_mem_write(uint64_t addr, const unsigned char* data, int len)
{
    int start = 0;
    int write_len = 0;

    string scope_name;

    if (sknobs_exists((char *)"stub_build")) {
      scope_name = "\\LIBVERIF.puvm_cpp_mem_shim_pkg ";
    } else {
      scope_name = "puvm_cpp_mem_shim_pkg";
    }

    DPI_SV_SCOPE_BEGIN(scope_name.c_str());

    unsigned char* tdata = new unsigned char [mem_buffer_len];
    while (start < len) {
        write_len = min(mem_buffer_len, len - start);
        memcpy(tdata, data + start, write_len);
        dpi_export_puvm_mem_write(addr, len, tdata, start, write_len);
        start += write_len;
    }
    delete [] tdata;
    DPI_SV_SCOPE_END;
}



void dpi_sv_puvm_mem_read(uint64_t addr, unsigned char* data, int len)
{
    int start = 0;
    int read_len = 0;

    string scope_name;

    if (sknobs_exists((char *)"stub_build")) {
      scope_name = "\\LIBVERIF.puvm_cpp_mem_shim_pkg ";
    } else {
      scope_name = "puvm_cpp_mem_shim_pkg";
    }

    DPI_SV_SCOPE_BEGIN(scope_name.c_str());

    unsigned char* tdata = new unsigned char [mem_buffer_len];
    while (start < len) {
        read_len = min(mem_buffer_len, len - start);
        dpi_export_puvm_mem_read(addr, len, tdata, start, read_len);
        memcpy(data + start, tdata, read_len);
        start += read_len;
    }
    delete [] tdata;
    DPI_SV_SCOPE_END;
}


void dpi_sv_puvm_avery_mem_write(uint64_t addr, const unsigned char* data, int len)
{
#ifdef USE_AVERY_HBM_MODEL
    int start = 0;
    int write_len = 0;

    string scope_name = "top_tb";
    DPI_SV_SCOPE_BEGIN(scope_name.c_str());
    unsigned char* tdata = new unsigned char [mem_buffer_len];
    while (start < len) {
        write_len = min(mem_buffer_len, len - start);
        memcpy(tdata, data + start, write_len);
        dpi_export_puvm_avery_mem_write(addr, len, tdata, start, write_len);
        start += write_len;
    }
    delete [] tdata;
    DPI_SV_SCOPE_END;
#endif
}



void dpi_sv_puvm_avery_mem_read(uint64_t addr, unsigned char* data, int len)
{
#ifdef USE_AVERY_HBM_MODEL
    int start = 0;
    int read_len = 0;
    string scope_name= "top_tb";
    DPI_SV_SCOPE_BEGIN(scope_name.c_str());
    unsigned char* tdata = new unsigned char [mem_buffer_len];
    while (start < len) {
        read_len = min(mem_buffer_len, len - start);
        dpi_export_puvm_avery_mem_read(addr, len, tdata, start, read_len);
        memcpy(data + start, tdata, read_len);
        start += read_len;
    }
    delete [] tdata;
    DPI_SV_SCOPE_END;
#endif
}


void dpi_sv_get_sim_time_ns(char* ts)
{
    string scope_name;

    if (sknobs_exists((char *)"stub_build")) {
      scope_name = "\\LIBVERIF.puvm_pkg ";
    } else {
      scope_name = "puvm_pkg";
    }

    DPI_SV_SCOPE_BEGIN(scope_name.c_str());
    const char* sv_str = dpi_export_get_sim_time_ns();
    strcpy(ts, sv_str);
    DPI_SV_SCOPE_END;
}


uint64_t dpi_sv_get_sim_time_ns_val()
{
    string scope_name;

    if (sknobs_exists((char *)"stub_build")) {
      scope_name = "\\LIBVERIF.puvm_pkg ";
    } else {
      scope_name = "puvm_pkg";
    }

    DPI_SV_SCOPE_BEGIN(scope_name.c_str());
    const char* ns = dpi_export_get_sim_time_ns_val();
    uint64_t tval = atol(ns);
    DPI_SV_SCOPE_END;

    return tval;
}


void dpi_force_finish()
{
    string scope_name;

    if (sknobs_exists((char *)"stub_build")) {
      scope_name = "\\LIBVERIF.puvm_pkg ";
    } else {
      scope_name = "puvm_pkg";
    }

    DPI_SV_SCOPE_BEGIN(scope_name.c_str());
    dpi_export_force_finish();
    DPI_SV_SCOPE_END;
}


}


#endif //_CSV_INCLUDED_
