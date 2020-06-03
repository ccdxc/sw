
#include "pen_c_useful_wrp.h"

#ifndef PEN_PAL_IMPL
#ifdef CAPRI_SW
#define PEN_PAL_IMPL 1
#endif
#ifdef ELBA_SW
#define PEN_PAL_IMPL 1
#endif
#endif

#ifdef __cplusplus
#ifndef PEN_PAL_IMPL
#include "cpu.h"
#include "LogMsg.h"

void pen_c_csr_write(uint64_t addr, uint32_t data, uint8_t no_zero_time, uint32_t flags) {
#ifdef PEN_CPU_DEBUG
    PLOG_MSG("pen_csr_write: addr:0x" << hex << addr << " data:0x" << data << endl);
#endif
    cpu::access()->write(0, addr, data, no_zero_time, flags);
}


uint32_t pen_c_csr_read(uint64_t addr, uint8_t no_zero_time, uint32_t flags) {
#ifdef PEN_CPU_DEBUG
    PLOG_MSG("pen_csr_read:addr:0x" << hex << addr << endl);
#endif
    return cpu::access()->read(0, addr, no_zero_time, flags);
#ifdef PEN_CPU_DEBUG
    PLOG_MSG("pen_csr_read:addr:0x" << hex << addr << " rdata:0x" << hex << rd_data << endl);
#endif
}

void pen_c_printf_msg(char * buf) {
    PLOG_MSG(buf);
}
void pen_c_printf_err(char * buf) {
    PLOG_ERR(buf);
}
#endif // PEN_PAL_IMPL
#else // __cplusplus

#ifdef ELBA_SW
#include "elb_sw_glue.h"

void pen_c_csr_write(uint64_t addr, uint32_t data, uint8_t no_zero_time, uint32_t flags) {
    elb_sw_writereg(addr, data);
}


uint32_t pen_c_csr_read(uint64_t addr, uint8_t no_zero_time, uint32_t flags) {
    return elb_sw_readreg(addr); 
}

void pen_c_printf_msg(char * buf) {
    printf("%s",buf); 
}
void pen_c_printf_err(char * buf) {
    printf("ERROR: %s", buf); 
}
#endif // ELBA_SW

#endif // __cplusplus
