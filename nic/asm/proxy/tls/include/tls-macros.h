#ifndef _TLS_MACROS_H_
#define _TLS_MACROS_H_

#include "capri-macros.h"
#include "cpu-macros.h"

#define TLS_NEXT_TABLE_READ            CAPRI_NEXT_TABLE_READ

#define TLS_READ_IDX                   CAPRI_READ_IDX

#define TLS_READ_ADDR                  CAPRI_READ_ADDR

/*
 * Generate random number using Barco DRBG. The read of the random number
 * will be done after the request has indicated completion.
 */
#define CAPRI_BARCO_DRBG_RANDOM0_GENERATE(_reg1, _reg2)         \
  addi        _reg1, r0, 0xC0000000;				\
  addi        _reg2, r0, CAPRI_BARCO_MD_HENS_REG_DRBG_RNG;	\
  memwr.wx    _reg2, _reg1[31:0];				\
  CAPRI_OPERAND_DEBUG(_reg1[31:0]);				\
  CAPRI_OPERAND_DEBUG(_reg2);

#endif /* #ifndef _TLS_MACROS_H_ */
