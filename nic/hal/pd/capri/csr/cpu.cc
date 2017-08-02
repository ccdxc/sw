#include "cpu.h"
#include "lib_model_client.h"

atomic<cpu*> cpu::_cpu_if { nullptr };

cpu *cpu::access(void) {

  if(_cpu_if == nullptr) {
     if(_cpu_if == nullptr) {
        _cpu_if = new cpu;
     }
  }
  return _cpu_if;
}

cpu::cpu() {
}

uint32_t cpu::read(uint32_t chip, uint64_t addr) {
    uint32_t data;

    (void)chip;
    read_reg(addr, data);
    return data;
}

void cpu::write(uint32_t chip, uint64_t addr, uint32_t data) {
    (void)chip;
    write_reg(addr, data);
}
