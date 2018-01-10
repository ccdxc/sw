#ifndef __LIB_MODEL_CLIENT_H__
#define __LIB_MODEL_CLIENT_H__

#include <stddef.h>
#include <vector>
#include "buf_hdr.h"

#ifdef __cplusplus
extern "C" {
#endif

int lib_model_connect();
int lib_model_conn_close();
void step_network_pkt(const std::vector<uint8_t> & pkt, uint32_t port);
bool get_next_pkt(std::vector<uint8_t> &pkt, uint32_t &port, uint32_t& cos);
bool read_reg (uint64_t addr, uint32_t& data);
bool write_reg(uint64_t addr, uint32_t  data);
bool read_mem(uint64_t addr, uint8_t * data, uint32_t size);
bool write_mem(uint64_t addr, uint8_t * data, uint32_t size);
void step_doorbell (uint64_t addr, uint64_t data);
bool dump_hbm(void);
void step_cpu_pkt(const uint8_t* pkt, size_t pkt_len);
void register_mem_addr(uint64_t addr);
void exit_simulation(void);
void config_done(void);
// mac config APIs to model
int lib_model_mac_msg_send (uint32_t port_num,
                            uint32_t speed,
                            uint32_t type,
                            uint32_t num_lanes,
                            uint32_t val);

#ifdef __cplusplus
}
#endif

#endif // __LIB_MODEL_CLIENT_H__
