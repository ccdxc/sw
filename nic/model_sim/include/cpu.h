#ifndef CPU_H
#define CPU_H

#include "cpu_bus_base.h"


#ifndef _COSIM_
// TODO: add _COSIM_
#ifdef _CSV_INCLUDED_
#include "cpu_bus_if.h"
#else
#include "cpu_bus_stub.h"
#endif
#else // _COSIM_

#ifdef _ZEBU_
#include "cpu_bus_zebu.h"
#endif
#endif


#include <atomic>
#include <mutex>
#include <map>
#include <iostream>

using namespace std;


class cpu {

private:
  map <string, cpu_bus_base *>  cpu_if_map; // map of cpu interface name -> cpu interface pointer
  vector< std::shared_ptr< cpu_csr_node_info_base > > cpu_csr_info_array;
  string cur_cpu_if_name;
  static atomic<cpu*> _cpu_if;
  static mutex m_;
  cpu_access_type_e access_type;
  cpu();
  virtual ~cpu() { };

public:
  static cpu* access(void) ;

  virtual void set_cur_if_name(string if_name) { // method to set the current active interface associated with this driver
     cur_cpu_if_name = if_name;
  }

  virtual string get_cur_if_name() {
     return cur_cpu_if_name;
  }

  // adding an interface-name=>interface-ptr association. each cpu-bus manager can have
  // multiple such if-name=>if-ptr associations for the different bus-driver protocols
  virtual void add_if(string cpu_if_name, cpu_bus_base *cpu_if) {
     cpu_if_map.insert(std::pair<string, cpu_bus_base *> (cpu_if_name, cpu_if));
  }

  //virtual uint32_t add_mem_prop(uint64_t addr_lo, uint64_t addr_hi, string hier_path, cpu_access_type_e access_type, bool add_index);
  //virtual mem_property* get_mem_prop(uint64_t addr ); // gets a memory hierarchy, given the address

  virtual uint32_t read(uint32_t chip, uint64_t addr, bool no_zero_time=false, uint32_t flags=secure_acc_e);
  virtual void write(uint32_t chip, uint64_t addr, uint32_t data, bool no_zero_time=false, uint32_t flags=secure_acc_e);

  bool check_csr_node_info_exists(uint64_t addr, uint64_t csr_size, uint32_t chip_id);
  virtual void set_access_type(cpu_access_type_e _access);
  virtual cpu_access_type_e get_access_type();
  void add_cpu_csr_node_info(std::shared_ptr<cpu_csr_node_info_base> node_ptr);
  bool cpu_csr_node_write(uint32_t chip, uint64_t addr, uint32_t data, uint32_t flags=secure_acc_e);
  bool cpu_csr_node_read(uint32_t chip, uint64_t addr, uint32_t & data, uint32_t flags=secure_acc_e);
  bool cpu_csr_node_block_write(uint32_t chip, uint64_t addr, uint32_t size, vector<uint32_t> data, uint32_t flags=secure_acc_e);
  bool cpu_csr_node_block_read(uint32_t chip, uint64_t addr, uint32_t size, vector<uint32_t> & data, uint32_t flags=secure_acc_e);
  void block_write(uint32_t chip, uint64_t addr, int size, vector<uint32_t> data, bool no_zero_time, uint32_t flags=secure_acc_e);
  vector<uint32_t> block_read(uint32_t chip, uint64_t addr, int size, bool no_zero_time, uint32_t flags=secure_acc_e);
  //virtual int call_uvm_hdl_deposit(uint32_t chip, char * path, vector<uint32_t> & value);

};


#endif // CPU_H
