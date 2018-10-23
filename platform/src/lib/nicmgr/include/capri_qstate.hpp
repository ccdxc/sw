#ifndef _CAPRI_QSTATE_HPP_
#define _CAPRI_QSTATE_HPP_

#include "lif_manager.hpp"

void push_qstate_to_capri(LIFQState *qstate, int cos);
void clear_qstate(LIFQState *qstate);
void read_lif_params_from_capri(LIFQState *qstate);

int32_t read_qstate(uint64_t q_addr, uint8_t *buf, uint32_t q_size);
int32_t write_qstate(uint64_t q_addr, const uint8_t *buf, uint32_t q_size);
int32_t get_pc_offset(sdk::platform::program_info *pinfo, const char *prog_name,
                      const char *label, uint8_t *offset);


#endif // _CAPRI_QSTATE_HPP_
