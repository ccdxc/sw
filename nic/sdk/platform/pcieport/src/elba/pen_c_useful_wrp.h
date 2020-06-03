
#ifndef PEN_C_USEFUL_H
#define PEN_C_USEFUL_H 
#include <stdint.h>
#include <unistd.h>
// commenting this out breaks sw_api compile.
// also we should not use chip specific ifdefs in common area
// need to come up with a cleaner fix
//#ifndef ELBA_SW
#ifdef _CSV_INCLUDED_
#ifdef __cplusplus
extern "C" void SLEEP(int N);
#else 
void SLEEP(int N);
#endif
#else

#ifdef ELBA_SW
#include "elb_sw_glue.h"
#else
    #define SLEEP(N) usleep(N);
#endif

#endif
//#endif //ELBA_SW

#ifdef __cplusplus
extern "C" {
#endif
void pen_c_csr_write(uint64_t addr, uint32_t data, uint8_t no_zero_time, uint32_t flags);
uint32_t pen_c_csr_read(uint64_t addr, uint8_t no_zero_time, uint32_t flags);
void pen_c_printf_msg(char * buf);
void pen_c_printf_err(char * buf);
#ifdef __cplusplus
}
#endif

#endif
