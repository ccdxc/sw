#include "print.hpp"
#include <spdlog/fmt/ostr.h>
#include "logger.hpp"

char *
macaddr2str (mac_t mac_addr)
{
    static char       macaddr_str[4][20];
    static uint8_t    macaddr_str_next = 0;
    char              *buf;
    uint8_t           *mac_byte = (uint8_t *)&mac_addr;

    buf = macaddr_str[macaddr_str_next++ & 0x3];
    snprintf(buf, 20, "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_byte[5], mac_byte[4], mac_byte[3],
             mac_byte[2], mac_byte[1], mac_byte[0]);
    return buf;
}


// ----------------------------------------------------------------------------
// Use this at the begin and end of an API
// ----------------------------------------------------------------------------
#define NUM_DASHES 20
void
api_trace (const char *trace)
{
    fmt::MemoryWriter   buf;

    if (!trace) return;

    for (int i = 0; i < NUM_DASHES; i++) {
        buf.write("{}", "-");
    }
    buf.write(" {} ", trace);
    for (int i = 0; i < NUM_DASHES; i++) {
        buf.write("{}", "-");
    }
    NIC_LOG_INFO("{}", buf.c_str());
}
