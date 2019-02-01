#ifndef DPI_UTILS_H
#define DPI_UTILS_H

#include "LogMsg.h"
#include "axi_xn_db.h"
#ifdef _CSV_INCLUDED_
#include "svdpi.h"
#endif

using namespace std;

extern "C" void cpp_dpi_soft_reset(); 
extern "C" void cpp_dpi_cfg(); 
extern "C" void cpp_dpi_init_start(); 
extern "C" void cpp_dpi_init_done(); 
extern "C" void cpp_dpi_eos(); 

/*
void     cpp_dpi_write(uint32_t chip, uint64_t addr, uint32_t data, prp_access_type_e do_backdoor);
uint32_t cpp_dpi_read(uint32_t chip, uint64_t addr, prp_access_type_e do_backdoor);

packet* cpp_dpi_getpkt();
void    cpp_dpi_sndpkt(packet *ptr);
packet* cpp_dpi_rcvpkt();
*/

// decode a hex string representing the bytes of an Ethernet packet using scapy
extern "C" void cpp_decode_eth_pkt(char *pkt_str);
// generate a packet using scapy
extern "C" char *cpp_create_pkt(char *scapy_fmt_str);
// get a handle to a scapy C++ pkt generator
extern "C" void *cpp_create_pkt_gen(char *prefix, int seed);
// get the pkt bytes for a new packet from the scapy C++ pkt generator
extern "C" char *cpp_get_pkt(void *pkt_gen_handle);

extern "C" uint64_t cpp_evalknob(char *knob_path);
extern "C" uint32_t cpp_knobExists(char *knob_path);

extern "C" char *cpp_get_dv_pkt(char* ifname, uint32_t & stream_id, uint32_t & not_empty);

extern "C" void cpp_set_seed(int seed);

extern "C" void cpp_check_interrupts(char * csr_blk_type_str, int chip_id, int inst_id);

#ifdef _CSV_INCLUDED_

extern "C" void c2sv_get_csr_shdw_bv(char * csr_blk_type_str, char * blk_reg_str, int & size, svBitVecVal * sv_rsp_signals_out);
extern "C" void c2sv_get_csr_bv(char *csr_blk_type_str, 
                                int chip_id,
                                int inst_id,
                                char *blk_reg_str, 
                                int index,
                                svBit read_from_hw, 
                                svLogicVecVal *sv_rsp_signals_out);
extern "C" void c2sv_set_csr_bv(char *csr_blk_type_str, 
                                int chip_id,
                                int inst_id,
                                char *blk_reg_str, 
                                int index,
                                svBit write_shadow, 
                                svLogicVecVal *sv_val);

extern "C" void c2sv_axi_get_req(int          master_id, 
                                 int &        xact_type,
                                 svBitVecVal  *addr,
                                 int &        size,
                                 svBitVecVal  *data,
                                 int &        xn_id,
                                 int &        xn_burst_length,
                                 int &        xn_burst_size,
                                 svBitVecVal  *xn_wstrb,
                                 int &        xn_prot_type,
                                 int &        xn_cache_type,
                                 int &        xn_context_id
                                 );



extern "C" int c2sv_axi_is_req_available(int master_id);                                 

extern "C" void c2sv_axi_get_response(int          master_id,
                                      int          context_id,
                                      int          size,
                                      svBitVecVal  *data,
                                      int          status);

#endif
extern "C" void cpp_set_seed(int seed);
extern "C" uint32_t get_test_done();


#endif // DPI_UTILS_H
