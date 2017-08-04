#include "cpu_hal_if.h"

extern bool read_reg (uint64_t addr, uint32_t& data);
extern bool write_reg(uint64_t addr, uint32_t  data);

uint32_t
cpu_hal_if::read(uint32_t chip, uint64_t addr, cpu_access_type_e do_backdoor) {
    uint32_t data;

    (void)chip;
    read_reg(addr, data);
    return data;
}

void
cpu_hal_if::write(uint32_t chip, uint64_t addr, uint32_t data,
                  cpu_access_type_e do_backdoor) {
    (void)chip;
    write_reg(addr, data);
}
