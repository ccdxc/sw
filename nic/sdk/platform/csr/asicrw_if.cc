#include "lib/pal/pal.hpp"
#include "platform/csr/asicrw_if.hpp"

uint32_t
cpu_hal_if::read(uint32_t chip, uint64_t addr,
                 cpu_access_type_e do_backdoor, uint32_t flags) {
    uint32_t data = 0;

    sdk::lib::pal_reg_read(addr, &data, 1);
    return data;
}

void
cpu_hal_if::write(uint32_t chip, uint64_t addr, uint32_t data,
                  cpu_access_type_e do_backdoor, uint32_t flags) {
    sdk::lib::pal_reg_write(addr, &data, 1);
}
