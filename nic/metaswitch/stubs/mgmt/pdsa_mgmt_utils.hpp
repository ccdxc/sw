// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Common helper APIs header filefor metaswitch stub programming 

#ifndef __PDSA_MGMT_UTILS_HPP__
#define __PDSA_MGMT_UTILS_HPP__
#include <nbase.h>
extern "C" {
#include <a0spec.h>
#include <o0mac.h>
#include <a0cust.h>
#include <a0mib.h>
#include <ambips.h>
}
#include "include/sdk/ip.hpp"


NBB_VOID  pdsa_convert_ip_addr_to_amb_ip_addr (ip_addr_t     pdsa_ip_addr, 
                                               NBB_LONG      *type, 
                                               NBB_ULONG     *len, 
                                               NBB_BYTE      *amb_ip_addr);

#endif /*__PDSA_MGMT_UTILS_HPP__*/
