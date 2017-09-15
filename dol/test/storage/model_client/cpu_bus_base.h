#ifndef CPU_BUS_BASE_H
#define CPU_BUS_BASE_H

#include <stdint.h>
#include <string>
#include <iostream>
#include <vector>
#include "LogMsg.h"

using namespace std;

typedef enum state {front_door_e, back_door_e, air_slave_e } cpu_access_type_e;

class cpu_csr_node_info_base {
    public:
        string node_path;
        uint64_t start_byte_addr;
        uint64_t csr_size;
        uint32_t chip_id;

        cpu_csr_node_info_base(string _node_path, uint64_t _start_byte_addr, uint64_t _csr_size, uint32_t _chip_id) {
            node_path = _node_path;
            start_byte_addr = _start_byte_addr;
            csr_size = _csr_size;
            chip_id = _chip_id;
        }
        virtual ~cpu_csr_node_info_base() {}

        virtual void write(uint64_t addr, uint32_t data) {};
        virtual uint32_t read(uint64_t addr) { return 0xdeadbeef; };
        virtual void block_write(uint64_t addr, int size, vector<uint32_t> data) {}
        virtual vector<uint32_t> block_read(uint64_t addr, int size) {
            vector<uint32_t> a;
            for(int ii = 0; ii < size; ii++) { a.push_back(0xdeadbeef); }
            return a;
        }
};



class cpu_bus_base {

private:
  string _cpu_if_name;      // interface name
  string _cpu_if_hier_path; // verilog path for this interface
  uint32_t agt_idx; // store the index to the agent

public:

  cpu_bus_base(string cpu_if_name, string cpu_if_hier_path) {
    _cpu_if_name      = cpu_if_name;
    _cpu_if_hier_path = cpu_if_hier_path;
    agt_idx = 0; // by default
  }

  ~cpu_bus_base() {
  }

  void set_agt_idx(uint32_t idx) { agt_idx = idx; }
  uint32_t get_agt_idx() { return agt_idx; }
  string get_if_name(void) const { return _cpu_if_name; };
  string get_hier_path(void) const { return _cpu_if_hier_path; };

  virtual uint32_t read(uint32_t chip, uint64_t addr, cpu_access_type_e do_backdoor=front_door_e) = 0 ; 
  virtual void write(uint32_t chip, uint64_t addr, uint32_t data, cpu_access_type_e do_backdoor=front_door_e) = 0;
  virtual void block_write(uint32_t chip, uint64_t addr, int size, vector<uint32_t> data, cpu_access_type_e do_backdoor=front_door_e) = 0;
  virtual vector<uint32_t> block_read(uint32_t chip, uint64_t addr, int size, cpu_access_type_e do_backdoor=front_door_e) = 0; 

  //virtual int call_uvm_hdl_deposit(uint32_t chip, char * path, vector<uint32_t> & value) { return 0; }

};

#endif // CPU_BUS_BASE_H
