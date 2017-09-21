#ifndef __INTERNAL_HPP__
#define __INTERNAL_HPP__

#include <base.h>
#include <eth.h>
#include <ip.h>
#include <list.hpp>
#include <ht.hpp>
#include <bitmap.hpp>
#include <internal.pb.h>
#include <internal.hpp>


namespace hal {

void GetProgramAddress(const internal::ProgramAddressReq& req,
                       internal::ProgramAddressResp *resp);

void AllocHbmAddress(const internal::HbmAddressReq &req,
                       internal::HbmAddressResp *resp);

}    // namespace hal

#endif    // __INTERNAL_HPP__

