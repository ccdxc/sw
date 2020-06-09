#include "port_an.hpp"

namespace sdk {
namespace linkmgr {

int
port_an_start(uint32_t lane, uint32_t sbus_addr, serdes_info_t *serdes_info,
              uint32_t user_cap, bool fec_ability, uint32_t fec_request)
{
   return (&mac_fns)->mac_an_start(lane, user_cap, fec_ability, fec_request);
}

bool
port_an_wait_hcd(uint32_t lane, uint32_t sbus_addr)
{
   return (&mac_fns)->mac_an_wait_hcd(lane);
}

int
port_an_hcd_read(uint32_t lane, uint32_t sbus_addr)
{
   return (&mac_fns)->mac_an_hcd_read(lane);
}

int
port_an_fec_enable_read (uint32_t lane, uint32_t sbus_addr)
{
   return (&mac_fns)->mac_an_fec_enable_read(lane);
}

int
port_an_rsfec_enable_read (uint32_t lane, uint32_t sbus_addr)
{
   return (&mac_fns)->mac_an_rsfec_enable_read(lane);
}

}    // namespace linkmgr
}    // namespace sdk
