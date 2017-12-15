// -*- C++ -*-
//************************************************************
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.
//************************************************************

#ifndef _CAP_MODEL_BASE_H_
#define _CAP_MODEL_BASE_H_
#include <stdint.h>
#include <vector>

class cap_model_base {
public:
    cap_model_base() {}

    // Push the network packet to capri.
    virtual void step_network_pkt(const std::vector<uint8_t> & pkt, uint32_t port) = 0;
    // Write doorbell location. Address is the capri address after BAR mapping.
    virtual void step_doorbell(uint64_t addr, uint64_t data, bool model=true, bool rtl=true) = 0;

    // Get next packet from one of the ethernet ports. It returns the
    // port number and COS.
    virtual bool get_next_pkt(std::vector<uint8_t>& v, uint32_t& port, uint32_t& cos) = 0;

    // Read capri register
    virtual bool read_reg (uint64_t addr, uint32_t& data) = 0;
    // Write capri register
    virtual bool write_reg(uint64_t addr, uint32_t  data) = 0;

    // Read from memory
    // Address map will define if it goes to HBM model or host
    // memory. Currently MSB of address indicates it going to "host
    // memory"
    // Range 0x80000000 to 0x80000000+0xffffffff is HBM
    virtual bool read_mem (uint64_t addr, uint8_t * data, uint32_t size) = 0;
    virtual bool write_mem(uint64_t addr, uint8_t * data, uint32_t size) = 0;


    virtual void load_cfg(void) = 0;
    virtual void load_prog(void) = 0;
    virtual char* disasm_opcode(uint64_t pc, uint64_t opcode) = 0;
    virtual void load_debug(void) = 0;
    virtual void step_tmr_wheel_update(unsigned slowfas, unsigned ctime) = 0;
    virtual void step_he(void) = 0;
 
    virtual void set_host_mem_cpy(bool cpy) = 0;

    virtual ~cap_model_base() {}

};
#endif
