#ifndef LIB_MODEL_CLIENT_H_
#define LIB_MODEL_CLIENT_H_

#include <vector>

int lib_model_connect();
int lib_model_conn_close();
void step_network_pkt(const std::vector<uint8_t> & pkt, uint32_t port);
bool get_next_pkt(std::vector<uint8_t> &pkt, uint32_t &port, uint32_t& cos);
bool read_reg (uint64_t addr, uint32_t& data);
bool write_reg(uint64_t addr, uint32_t  data);
bool read_mem(uint64_t addr, uint8_t * data, uint32_t size);
bool write_mem(uint64_t addr, uint8_t * data, uint32_t size);
bool dump_hbm(void);

#endif
