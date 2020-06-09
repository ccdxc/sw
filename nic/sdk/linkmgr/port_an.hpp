#ifndef __SDK_PORT_AN_HPP__
#define __SDK_PORT_AN_HPP__

#include "include/sdk/base.hpp"
#include "linkmgr.hpp"
#ifdef ELBA
#include "port_mac.hpp"
#else
#include "port_serdes.hpp"
#endif

namespace sdk {
namespace linkmgr {

int port_an_start(uint32_t lane, uint32_t sbus_addr, serdes_info_t *serdes_info,
                  uint32_t user_cap, bool fec_ability, uint32_t fec_request);
bool port_an_wait_hcd(uint32_t lane, uint32_t sbus_addr);
int port_an_hcd_read (uint32_t lane, uint32_t sbus_addr);
int port_an_fec_enable_read (uint32_t lane, uint32_t sbus_addr);
int port_an_rsfec_enable_read (uint32_t lane, uint32_t sbus_addr);

}    // namespace linkmgr
}    // namespace sdk

#endif
