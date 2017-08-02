#ifndef CPU_H
#define CPU_H

#include "cpu_bus_stub.h"
#include <atomic>
#include <iostream>
#include <map>

using namespace std;

class cpu {

private:
  string cur_cpu_if_name;
  static atomic<cpu*> _cpu_if;
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

  virtual void add_if(string cpu_if_name, cpu_bus_stub_if *cpu_if) {
  }

  virtual uint32_t read(uint32_t chip, uint64_t addr);
  virtual void write(uint32_t chip, uint64_t addr, uint32_t data);
};


#endif // CPU_H
