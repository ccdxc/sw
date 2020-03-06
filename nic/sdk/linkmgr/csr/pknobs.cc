#include "pknobs.hpp"

// TODO fix AAPL
extern "C" {

uint32_t jtag_wr (unsigned char chip,
                  unsigned long long int reg_addr,
                  uint32_t *sbus_data,
                  unsigned long long int flag)
{
    return 0;
}

uint32_t jtag_rd (unsigned char chip,
                  unsigned long long int reg_addr,
                  uint32_t sbus_data,
                  unsigned long long int flag)
{
    return 0;
}

}   // extern "C"
