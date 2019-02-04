#ifndef CAP_CSR_UTIL_API_H
#define CAP_CSR_UTIL_API_H

#include "pen_csr_base.h"
#include "pknobs_reader.h"
void cap_csr_randomize(pen_csr_base * csr_ptr, bool do_write=false);
cpp_int show_interrupt_tree(pen_csr_base * base, bool enabled=1, bool print_only=1, int level=-1 , string slv_status_name="int_groups",unsigned indent=0);
void enable_all_interrupts(pen_csr_base * block_base,int level=-1);
void clear_all_interrupts(pen_csr_base * block_base, int level=-1);
void check_interrupts(pen_csr_base * block, bool print_only, int level=-1, bool root_enabled=1, string slv_status_name = "int_groups", int verbose=1);


std::vector<std::string> string_tokenize(std::string const &in, char sep);
#ifndef CAPRI_HAL
vector<unsigned> split_num_random(unsigned max, unsigned samples, Knob * rand_knob, unsigned min=1);
#endif
#endif
