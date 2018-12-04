#include "utils.hpp"

bool is_multicast(uint64_t mac) {
    return ((mac & 0x010000000000) == 0x010000000000);
}

