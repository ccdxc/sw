#ifndef CAP_CSR_PY_IF_H
#define CAP_CSR_PY_IF_H
#include <inttypes.h>
#include <string>
#include <vector>

using namespace std;

void register_chip_inst(string path, uint32_t chip_id, uint32_t inst_id); 
void delete_chip_inst(string path, uint32_t chip_id, uint32_t inst_id);


void csr_write_addr(uint64_t addr, uint32_t data, uint32_t flags=0);
uint32_t csr_read_addr(uint64_t addr, uint32_t flags=0);

void csr_write(string path, string data);
string csr_read(string path);

void csr_block_write(string path, string data);
string csr_block_read(string path);

void csr_block_write_addr(uint64_t addr, int size, const std::vector<unsigned int>& data, uint32_t secure=0);
std::vector<unsigned int> csr_block_read_addr(uint64_t addr, int size, uint32_t secure=0);

void csr_write_field(string path, string field, string data, int shadow_update_only=0);
string csr_read_field(string path, string field, int shadow_update_only=0);

void csr_set_field_val(string path, string field, string data);
string csr_get_field_val(string path, string field);

string csr_convert_2_sbrackets(string path_name);

void csr_list(string path, int level=1, string action="");
void csr_find(string path, string pattern, int level=1, string action="");
void csr_prefix(string path, string pattern, int level=1, string action="");
void csr_show(string path, int level=1);
void csr_write_burst(uint64_t addr, uint32_t len, string data, uint32_t flags=0);
string csr_read_burst(uint64_t addr, uint32_t len, uint32_t flags=0);

uint32_t plog_get_err_count(); 
void     plog_clr_err_count(); 
void     plog_set_max_err_count(uint32_t max_err_cnt); 
void     plog_err(string msg);
void     plog_msg(string msg);
void     plog_start(string fname, uint32_t size_in_bytes=0xfffffff);
void     plog_stop();

uint64_t csr_get_offset(string path);
//string csr_get_type_name(string path);


void plog_add_scope(string scope);
void plog_remove_scope(string scope);
void plog_set_verbose(uint32_t level) ;

// cpu if
void cpu_set_cur_if_name(char * _name);
char * cpu_get_cur_if_name();
void cpu_force_global_flags(unsigned);

#endif
