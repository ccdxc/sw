#ifndef CPU_BUS_STUB_H
#define CPU_BUS_STUB_H

#include "cpu_bus_base.h"

using namespace std;

class cpu_bus_stub_if : public cpu_bus_base {

public:

  cpu_bus_stub_if(string cpu_if_name, string cpu_if_hier_path) : cpu_bus_base(cpu_if_name, cpu_if_hier_path) {
  }

  ~cpu_bus_stub_if() {
  }

  virtual uint32_t read(uint32_t chip, uint64_t addr, cpu_access_type_e do_backdoor=front_door_e) {return 0;} ; 
  virtual void write(uint32_t chip, uint64_t addr, uint32_t data, cpu_access_type_e do_backdoor=front_door_e) {};

};

#endif // CPU_BUS_STUB_H
