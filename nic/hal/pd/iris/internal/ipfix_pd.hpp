#ifndef __IPFIX_PD_HPP__
#define __IPFIX_PD_HPP__

namespace hal {
namespace pd {

hal_ret_t ipfix_init(uint16_t export_id, uint64_t pktaddr,
                     uint16_t payload_start, uint16_t payload_size);

} // namespace pd
} // namespace hal

#endif // _IPFIX_PD_HPP__
