// -*- C++ -*-
//************************************************************
// (C)  Copyright 2017 Pensando Systems Inc. All rights reserved.
//************************************************************
#ifndef _ELB_ENV_BASE_H_
#define _ELB_ENV_BASE_H_
#include "elb_model_base.h"
#include "pen_blk_env_base.h"
#include <vector>
#include <string>
class elb_top_csr_t;
class elb_env_base : public pen_blk_env_base {
protected:
    elb_model_base * elb_mod;
    int my_id;
    bool pad_enable = false;  // Pad packets to 64 bytes.
 public:
    elb_top_csr_t * elb0_ptr;
    elb_env_base(int id);
    // Call this function if you want to run just StandAlone Model without RTL.
    virtual void SAM_mode(void);

    virtual void load_cfg();
    virtual void load_prog();
    virtual char* disasm_opcode(uint64_t pc, uint64_t opcode);
    virtual void load_debug();
    virtual void load_hbm_pkt(uint64_t addr, const std::vector<uint8_t> & pkt);

    // Push the network packet to elba
    virtual void step_network_pkt(const std::vector<uint8_t> & pkt, uint32_t port);

    // Push DMA packet into elba, not implemented in base class
    virtual void step_host_pkt(const std::vector<uint8_t> & pkt, uint32_t lif, uint32_t qtype, uint32_t qid, uint32_t ring) {};

    // Get next packet from one of the ethernet ports. It returns the
    // port number and COS.
    virtual bool get_next_pkt(std::vector<uint8_t> &pkt, uint32_t &port, uint32_t& cos);

    // returns true if there are no more packets in the system
    virtual bool is_env_idle();
    virtual void show_pending();

    // Read elba register
    virtual bool read_reg (uint64_t addr, uint32_t& data);
    // Write elba register
    virtual bool write_reg(uint64_t addr, uint32_t  data);

    // Memory access:
    // Address map will define if it goes to HBM model or host
    // memory. Currently MSB of address indicates it going to "host
    // memory"
    // Range 0x80000000 to 0x80000000+0xffffffff is HBM
    // The size and address should be such that the data being written
    // or read does not cross 4KB boundary. Data buffer is allocated
    // by the caller.
    virtual bool read_mem(uint64_t addr, uint8_t * data, uint32_t size);
    virtual bool write_mem(uint64_t addr, uint8_t * data, uint32_t size);
    virtual bool write_mem_pcie(uint64_t addr, uint8_t * data, uint32_t size);
    // Doorbell
    virtual void step_doorbell(uint64_t addr, uint64_t data, bool model=true, bool rtl=true);
    // Timer update
    virtual void step_tmr_wheel_update(unsigned slowfas, unsigned ctime);

    virtual void register_model_callback(std::string cb_name, elb_model_cb_base * cb) ;

    virtual void init(void);
    virtual int get_chip_id(void)
    {
        return my_id;
    }
    virtual void set_host_mem_cpy(bool cpy)
    {
      elb_mod->set_host_mem_cpy(cpy);
    }
    virtual void save_axi_xn()
    {
        elb_mod->save_axi_xn();
    }
    virtual axi_info_t * get_next_axi_xn(uint32_t port)
    {
        return elb_mod->get_next_axi_xn(port);
    }
    virtual unsigned get_drop_count() 
    {
	return elb_mod->get_drop_count();
    }
    virtual void set_exception_pc(uint64_t pc) {
	elb_mod->set_exception_pc(pc);
    }
    virtual void set_exception_cases(bool tbl_err, bool phv_err) {
	elb_mod->set_exception_cases(tbl_err, phv_err);
    }
    virtual void enable_padding() { pad_enable = true; }
    virtual ~elb_env_base();
};
#endif
