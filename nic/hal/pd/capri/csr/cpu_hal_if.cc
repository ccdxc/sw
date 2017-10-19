#include "cpu_hal_if.h"

extern uint32_t read_reg_base (uint32_t chip, uint64_t addr);
extern void write_reg_base(uint32_t chip, uint64_t addr, uint32_t  data);

uint32_t
cpu_hal_if::read(uint32_t chip, uint64_t addr,
                 cpu_access_type_e do_backdoor, uint32_t flags) {
    return read_reg_base(chip, addr);
}

void
cpu_hal_if::write(uint32_t chip, uint64_t addr, uint32_t data,
                  cpu_access_type_e do_backdoor, uint32_t flags) {
    write_reg_base(chip, addr, data);
}
