#ifndef __IF_UTILS_HPP__
#define __IF_UTILS_HPP__

#include <types.pb.h>
#include <ip.h>

namespace hal {

hal_ret_t pltfm_get_port_from_front_port_num(uint32_t fp_num, 
                                             uint32_t *port_num);

}    // namespace hal

#endif    // __IF_UTILS_HPP__

