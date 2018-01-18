#include "cpu_hal_if.h"
#include "nic/include/asic_pd.hpp"

uint32_t
cpu_hal_if::read(uint32_t chip, uint64_t addr,
                 cpu_access_type_e do_backdoor, uint32_t flags) {
    return hal::pd::asic_reg_read(addr);
}

void
cpu_hal_if::write(uint32_t chip, uint64_t addr, uint32_t data,
                  cpu_access_type_e do_backdoor, uint32_t flags) {
    hal::pd::asic_reg_write(addr, &data);
}
