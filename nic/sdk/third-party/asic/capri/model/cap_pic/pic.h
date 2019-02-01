//************************************************************
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.
//************************************************************
//   P4 memory controller reference model
//   
//************************************************************
#ifndef _PIC_MODEL_
#define _PIC_MODEL_

#include "cap_pics_csr.h"
#include "cap_pict_csr.h"
#include "cap_axi_decoders.h"
#include "cap_pic_decoders.h"
#include "cap_te_pic_tcamif_decoders.h"
#include <boost/multiprecision/gmp.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include "cpp_int_helper.h"

#include <queue>
#include <list>
#include <assert.h>

#define PIC_NUM_PORT                  8

#define PIC_SRAM_FULL_LINE            128

#define PIC_SRAM_WORD_BITMSK          (((1 << 3) - 1 ) & ~((1 << 0)  - 1))
#define PIC_SRAM_MACRO_BITMSK         (((1 << 7) - 1) & ~((1 << 3) - 1))
#define PIC_SRAM_LINE_BITMSK          (((1 << 19) - 1 ) & ~((1 << 7)  - 1))
#define PIC_SRAM_WORD_SHIFT           0
#define PIC_SRAM_MACRO_SHIFT          3
#define PIC_SRAM_LINE_SHIFT           7

#define PIC_TCAM_WORD_BITMSK          ((1 << 3) - 1 ) & ~((1 << 0) - 1)
#define PIC_TCAM_MACRO_BITMSK         ((1 << 6) - 1) & ~((1 << 3) - 1)
#define PIC_TCAM_LINE_BITMSK          ((1 << 16) - 1 ) & ~((1 << 6) - 1)
#define PIC_TCAM_WORD_SHIFT           0
#define PIC_TCAM_MACRO_SHIFT          3
#define PIC_TCAM_LINE_SHIFT           6

#define PIC_TBL_OPCODE_OPERATION_NONE    0
#define PIC_TBL_OPCODE_OPERATION_ADD     1
#define PIC_TBL_OPCODE_OPERATION_SUB     2
#define PIC_TBL_OPCODE_OPERATION_MIN     3
#define PIC_TBL_OPCODE_OPERATION_MAX     4
#define PIC_TBL_OPCODE_OPERATION_AVG     5
#define PIC_TBL_OPCODE_OPERATION_CPY     6
#define PIC_TBL_OPCODE_OPERATION_BITSET  7
#define PIC_TBL_OPCODE_OPERATION_CLEAR   8

#define PIC_TBL_OPCODE_OPRD1_SEL_COUNTER   0
#define PIC_TBL_OPCODE_OPRD1_SEL_RATE      1
#define PIC_TBL_OPCODE_OPRD1_SEL_TBKT      2

#define PIC_TBL_OPCODE_OPRD2_SEL_ONE       0
#define PIC_TBL_OPCODE_OPRD2_SEL_PKTSIZE   1
#define PIC_TBL_OPCODE_OPRD2_SEL_RATE      2
#define PIC_TBL_OPCODE_OPRD2_SEL_POLICER   3

#define PIC_TBL_OPCODE_SATURATE_NONE     0
#define PIC_TBL_OPCODE_SATURATE_MINMAX   1
#define PIC_TBL_OPCODE_SATURATE_OPRD3    2
#define PIC_TBL_OPCODE_SATURATE_NEG      3
                                                       
#define PIC_POL_COLOR_RED      3
#define PIC_POL_COLOR_YELLOW   1
#define PIC_POL_COLOR_GREEN    0
                                                       
class pic {
public:

  //************************************************************
  //  Constructor parameter for SCIG/SCEG/PC instantiation
  //  SGIG: num_axi_if     = 6
  //        num_tcam_if    = 6
  //        num_sram_macro = 10
  //        num_tcam_macro = 8
  //  SGEG: num_axi_if     = 6
  //        num_tcam_if    = 6
  //        num_sram_macro = 10
  //        num_tcam_macro = 4
  //  PC:   num_axi_if     = 8
  //        num_tcam_if    = 0
  //        num_sram_macro = 8
  //        num_tcam_macro = 0
  //************************************************************
  unsigned num_axi_if;
  unsigned num_tcam_if;
  unsigned num_sram_macro;
  unsigned num_tcam_macro;

  //************************************************************
  //  Input/Output
  //************************************************************
  //AXI interface
  cap_axi_bundle_t (*axi_if)[PIC_NUM_PORT];

  //Access to NOC interface for rate limiter CSR write
  cap_axi_bundle_t *noc_if;

  // New rate limiter interface only TD/PC uses it
  cap_pic_rl_vld_decoder_t *rl_if;

  //TCAM interface
  cap_te_pic_tcamif_bundle_t (*tcam_if)[PIC_NUM_PORT];

  //register/memory blocks
  cap_pics_csr_t & pics_reg_block;
  cap_pict_csr_t & pict_reg_block;

  //************************************************************
  //  Internal data structures
  //************************************************************
  cap_te_axi_addr_decoder_t axi_addr_dec;

  unsigned stage_id[8];
  unsigned table_id       ;
  unsigned table_idx      ;
  unsigned pktsize        ;
  unsigned color_bits     ;
  unsigned cur_color      ;

  unsigned tbl_start_addr ;
  unsigned tbl_width      ;
  unsigned tbl_end_addr   ;
  unsigned tbl_hash       ;
  unsigned tbl_opcode     ;
  unsigned tbl_log2bkts   ;
  unsigned tbl_axishift   ;
  unsigned tbl_rlimit_en  ;

  unsigned entry_start_addr;
  unsigned entry_end_addr;

  unsigned sram_macro ;
  unsigned sram_line  ;
  unsigned sram_word  ;

  unsigned sram_data;
  cpp_int  read_data_line;

  unsigned sram_macro_start ;
  unsigned sram_macro_end ;
  cpp_int  sram_data_be;

  cap_pic_policer_decoder_t prev_policer_dec;
  cap_pic_policer_decoder_t policer_dec;

  unsigned tcam_stage_id[8];
  unsigned tcam_table_id      ;  
  unsigned tcam_tbl_start_addr; 
  unsigned tcam_tbl_end_addr  ; 
  unsigned tcam_tbl_width     ; 
  unsigned tcam_tbl_bkts      ; 
  unsigned tcam_tbl_keyshift  ;

  unsigned tcam_macro_start   ; 
  unsigned tcam_macro_end     ; 
  unsigned tcam_macro_key_end ; 
  unsigned tcam_key_num_macro ; 
  unsigned tcam_line_start    ;
  unsigned tcam_line_end      ;
  unsigned tcam_key_shift     ;

  pu_cpp_int<512> tcam_full_msk;

  cpp_int tcam_full_key;
  cpp_int tcam_key;
  cpp_int tcam_msk;
  cpp_int tcam_entry_msk;

  bool     tcam_key_match;
  unsigned tcam_entry_idx;
  unsigned tcam_dhs_idx;

  cpp_int  alu_oprd1;
  cpp_int  alu_oprd2;
  cpp_int  alu_oprd3;
  cpp_int  alu_mask;
  cpp_int  alu_sign;
  cpp_int  alu_result;

  cap_pic_opcode_decoder_t tbl_opcode_dec;

  cpp_int global_clk;
  cpp_int next_global_clk;

  unsigned bg_sram_macro ;
  unsigned bg_sram_line  ;
  unsigned bg_sram_word  ;
  unsigned bg_ptr  ;
  unsigned rl_cnt  ;

  unsigned bg_sram_start_macro ;
  unsigned bg_sram_start_line  ;
  unsigned bg_sram_start_word  ;

  unsigned bg_sram_end_macro ;
  unsigned bg_sram_end_line  ;
  unsigned bg_sram_end_word  ;

  unsigned bg_next_addr;

  struct bg_sram_update_fifo_entry {
     unsigned addr;
     unsigned opcode;
     unsigned rlimit_en;
     unsigned table_idx;
     uint32_t mode;
     uint32_t bg_idx;
  };
  typedef struct bg_sram_update_fifo_entry bg_sram_update_fifo_entry_t;
  queue <bg_sram_update_fifo_entry_t> bg_sram_update_fifo;

  //************************************************************
  //  constructor
  //************************************************************
  pic(unsigned _num_axi_if,
      unsigned _num_tcam_if,
      unsigned _num_sram_macro,
      unsigned _num_tcam_macro,
      cap_axi_bundle_t (*_axi_if)[PIC_NUM_PORT],
      cap_axi_bundle_t (*_noc_if),
      cap_pic_rl_vld_decoder_t (*_rl_if),
      cap_te_pic_tcamif_bundle_t (*_tcam_if)[PIC_NUM_PORT],
      cap_pics_csr_t & _pics_reg_block ,
      cap_pict_csr_t & _pict_reg_block) :
    num_axi_if  (_num_axi_if  ),
    num_tcam_if  (_num_tcam_if  ),
    num_sram_macro(_num_sram_macro),
    num_tcam_macro(_num_tcam_macro),
    axi_if       (_axi_if       ),
    noc_if       (_noc_if       ),
    rl_if        (_rl_if        ),
    tcam_if      (_tcam_if      ),
    pics_reg_block(_pics_reg_block),
    pict_reg_block(_pict_reg_block)
  {
    global_clk = 0;
    next_global_clk = 0;
    bg_ptr = 0;
    rl_cnt = 0;
  }

  //************************************************************
  //  internal functions
  //************************************************************
  void sram_entry_addr_decode(unsigned port, cpp_int addr, unsigned & entry_start, unsigned & entry_end);
  void sram_addr_decode(unsigned   addr, unsigned & macro, unsigned & line, unsigned & word);
  void sram_addr_encode(unsigned & addr, unsigned & macro, unsigned & line, unsigned & word);

  void sram_update(unsigned update_entry_start_addr,
                   unsigned update_entry_end_addr,
                   unsigned update_tbl_width,
                   unsigned update_tbl_opcode,
                   unsigned update_pktsize,
                   unsigned update_color_bits,
                   unsigned update_rlimit_en);

  void rate_limit_noc_write(uint8_t  rlimit_prof,
                            unsigned rlimit_idx,
                            unsigned stop);

  //************************************************************
  //  Steps
  //************************************************************
  void step_axi_read(unsigned port);
  void step_axi_write(unsigned port);
  void step_tcam_search(unsigned port);
  void step_bg_setup(uint32_t bg_idx);
  void step_bg_clock_tick(void);
  void step_bg_sram_update(void);
  bool check_legal_entry(uint32_t entry_start_addr, uint32_t entry_end_addr);
                            
protected:

};

#endif // PIC_H
