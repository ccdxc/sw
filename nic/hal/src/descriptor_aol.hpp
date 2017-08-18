#ifndef __DESCRIPTOR_AOL_HPP__
#define __DESCRIPTOR_AOL_HPP__


#include <base.h>
#include <pd.hpp>
#include <hal_state.hpp>
#include <descriptor_aol.pb.h>

namespace hal {
hal_ret_t descriptor_aol_get(descraol::DescrAolRequest& request, 
                             descraol::DescrAolSpec *response);
}
#endif /* __DESCRIPTOR_AOL_HPP__ */
