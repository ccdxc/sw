#ifndef _CAPRI_QSTATE_HPP_
#define _CAPRI_QSTATE_HPP_

void push_qstate_to_capri(hal::LIFQState *qstate, int cos);
void clear_qstate(hal::LIFQState *qstate);
int32_t read_qstate(uint64_t q_addr, uint8_t *buf, uint32_t q_size);
int32_t write_qstate(uint64_t q_addr, const uint8_t *buf, uint32_t q_size);
int32_t get_pc_offset(const char *handle, const char *prog_name,
                      const char *label, uint8_t *offset);


#endif // _CAPRI_QSTATE_HPP_
