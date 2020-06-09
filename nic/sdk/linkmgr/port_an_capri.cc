#include "port_an.hpp"

namespace sdk {
namespace linkmgr {

int
port_an_start(uint32_t lane, uint32_t sbus_addr, serdes_info_t *serdes_info,
              uint32_t user_cap, bool fec_ability, uint32_t fec_request)
{
   return (&serdes_fns)->serdes_an_start(sbus_addr, serdes_info, user_cap, fec_ability, fec_request);
}

bool
port_an_wait_hcd(uint32_t lane, uint32_t sbus_addr)
{
   return (&serdes_fns)->serdes_an_wait_hcd(sbus_addr);
}

int
port_an_hcd_read(uint32_t lane, uint32_t sbus_addr)
{
   return (&serdes_fns)->serdes_an_hcd_read(sbus_addr);
}

int
port_an_fec_enable_read (uint32_t lane, uint32_t sbus_addr)
{
   return (&serdes_fns)->serdes_an_fec_enable_read(sbus_addr);
}

int
port_an_rsfec_enable_read (uint32_t lane, uint32_t sbus_addr)
{
   return (&serdes_fns)->serdes_an_rsfec_enable_read(sbus_addr);
}

}    // namespace linkmgr
}    // namespace sdk
