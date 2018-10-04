#include <unistd.h>
#include <string>
#include <sstream>
#include <ostream>
#include <iomanip>
#include "nic/hal/hal.hpp"
#include "nic/hal/lkl/lklshim.hpp"
#include "nic/hal/lkl/lklshim_tls.hpp"
#include "nic/include/cpupkt_headers.hpp"

extern "C" {
#include "lkl.h"
#include "lkl_host.h"
}


namespace hal {
// byte array to hex string for logging
std::string hex_dump(const uint8_t *buf, size_t sz)
{
    std::ostringstream result;

    for(size_t i = 0; i < sz; i+=8) {
        result << " 0x";
        for (size_t j = i ; j < sz && j < i+8; j++) {
            result << std::setw(2) << std::setfill('0') << std::hex << (int)buf[j];
        }
    }

    return result.str();
}

} // namespace hal


