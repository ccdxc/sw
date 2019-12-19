// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Common helper APIs header file for metaswitch stub programming 

#ifndef __PDSA_MGMT_UTILS_HPP__
#define __PDSA_MGMT_UTILS_HPP__
#include <nbase.h>
extern "C" {
#include <a0spec.h>
#include <o0mac.h>
#include <a0cust.h>
#include <a0glob.h>
#include <a0mib.h>
#include <ambips.h>
#include <a0stubs.h>
#include <a0cpif.h>
#include "smsiincl.h"
#include "smsincl.h"
}
#include "li_mgmt_if.h"
#include "lim_mgmt_if.h"
#include "psm_prod.h"
#include "ftm_mgmt_if.h"
#include "nrm_mgmt_if.h"
#include "nrm_prod.h"
#include "psm_mgmt_if.h"
#include "include/sdk/ip.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_config.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_ctm.hpp"
#include "gen/proto/types.pb.h"
#include "gen/proto/internal.pb.h"
#include "gen/proto/bgp.pb.h"
#include "gen/proto/evpn.pb.h"
#include "gen/proto/staticroute.pb.h"

#define PDSA_CTM_GRPC_CORRELATOR 0x101
#define PDSA_CTM_STUB_INIT_CORRELATOR 0x42
#define PDSA_BGP_RM_ENT_INDEX   1
#define PDSA_BGP_NM_ENT_INDEX   1
#define PDSA_SCK_ENT_INDEX      1
#define PDSA_I3_ENT_INDEX       1
#define PDSA_LI_ENT_INDEX       1
#define PDSA_LIM_ENT_INDEX      1
#define PDSA_L2F_ENT_INDEX      1
#define PDSA_NAR_ENT_INDEX      1
#define PDSA_NRM_ENT_INDEX      1
#define PDSA_RTM_ENT_INDEX      1
#define PDSA_PSM_ENT_INDEX      1
#define PDSA_SMI_ENT_INDEX      1
#define PDSA_FTM_ENT_INDEX      1
#define PDSA_FT_ENT_INDEX       1
#define PDSA_HALS_ENT_INDEX     1
#define PDSA_EVPN_ENT_INDEX     1

using namespace std;

NBB_VOID  pdsa_convert_ip_addr_to_amb_ip_addr (ip_addr_t     pdsa_ip_addr, 
                                               NBB_LONG      *type, 
                                               NBB_ULONG     *len, 
                                               NBB_BYTE      *amb_ip_addr);

NBB_VOID pdsa_convert_long_to_pdsa_ipv4_addr (NBB_ULONG ip,
                                              ip_addr_t *pdsa_ip_addr); 


NBB_VOID pdsa_set_address_oid(NBB_ULONG *oid,
                              const NBB_CHAR  *tableName,
                              const NBB_CHAR  *fieldName,
                              const types::IPAddress &addr);

NBB_VOID pdsa_set_address_field(AMB_GEN_IPS *mib_msg,
                                const NBB_CHAR  *tableName,
                                const NBB_CHAR  *fieldName,
                                NBB_VOID        *dest,
                                const types::IPAddress &addr);

NBB_LONG pdsa_nbb_get_long(NBB_BYTE *byteVal);

NBB_VOID
pdsa_set_string_in_byte_array_with_len(NBB_BYTE *field,
                                       NBB_ULONG &len,
                                       string in_str);

NBB_VOID
pdsa_set_string_in_byte_array_with_len_oid(NBB_ULONG *oid,
                                           string in_str,
                                           NBB_LONG setKeyOidIdx,
                                           NBB_LONG setKeyOidLenIdx);

string
pdsa_get_string_in_byte_array_with_len(NBB_BYTE *in_str,
                                       NBB_ULONG len);

NBB_VOID
pdsa_get_string_in_byte_array_with_len_oid(NBB_ULONG *oid,
                                           string in_str,
                                           NBB_LONG getKeyOidIdx,
                                           NBB_LONG getKeyOidLenIdx);

NBB_VOID
pdsa_set_string_in_byte_array(NBB_BYTE *field,
                              string in_str);

NBB_VOID
pdsa_set_string_in_byte_array_oid(NBB_ULONG *oid,
                                  string in_str,
                                  NBB_LONG setKeyOidIdx);

string
pdsa_get_string_in_byte_array(NBB_BYTE *val,
                              NBB_ULONG len);

NBB_VOID
pdsa_get_string_in_byte_array_oid(NBB_ULONG *oid,
                                  string in_str,
                                  NBB_LONG getKeyOidIdx);

types::IPAddress* pdsa_get_address(const NBB_CHAR  *tableName,
                                  const NBB_CHAR  *fieldName,
                                  NBB_VOID    *src);
namespace pds {
NBB_VOID bgp_rm_ent_fill_func (pds::BGPGlobalSpec &req, 
                               AMB_GEN_IPS        *mib_msg, 
                               AMB_BGP_RM_ENT     *v_amb_bgp_rm_ent, 
                               NBB_LONG           row_status);
NBB_VOID bgp_peer_fill_func (pds::BGPPeerSpec&   req,
                             AMB_GEN_IPS         *mib_msg,
                             AMB_BGP_PEER        *v_amb_bgp_peer,
                             NBB_LONG            row_status);
NBB_VOID evpn_evi_fill_func (EvpnEviSpec&    req,
                             AMB_GEN_IPS     *mib_msg,
                             AMB_EVPN_EVI    *data,
                             NBB_LONG        row_status);
NBB_VOID  evpn_ip_vrf_fill_func (EvpnIpVrfSpec&   req,
                                 AMB_GEN_IPS      *mib_msg,
                                 AMB_EVPN_IP_VRF  *data,
                                 NBB_LONG         row_status);
NBB_VOID evpn_evi_rt_fill_func (EvpnEviRtSpec&   req,
                                AMB_GEN_IPS      *mib_msg,
                                AMB_EVPN_EVI_RT  *data,
                                NBB_LONG         row_status);
NBB_VOID evpn_ip_vrf_rt_fill_func (EvpnIpVrfRtSpec&      req,
                                   AMB_GEN_IPS           *mib_msg,
                                   AMB_EVPN_IP_VRF_RT    *data,
                                   NBB_LONG         row_status);
NBB_VOID evpn_ip_vrf_fill_name_oid (EvpnIpVrfSpec& req, NBB_ULONG *oid);
NBB_VOID evpn_ip_vrf_fill_name_field (EvpnIpVrfSpec& req, AMB_GEN_IPS *mib_msg);
NBB_VOID evpn_ip_vrf_rt_fill_name_field (EvpnIpVrfRtSpec& req,
                                         AMB_GEN_IPS *mib_msg);
NBB_VOID evpn_ip_vrf_rt_fill_name_oid (EvpnIpVrfRtSpec& req, NBB_ULONG *oid);
NBB_VOID evpn_ip_vrf_get_name_field (EvpnIpVrfSpec* req, AMB_EVPN_IP_VRF *data);
NBB_VOID evpn_ip_vrf_rt_get_name_field (EvpnIpVrfRtSpec* req,
                                        AMB_EVPN_IP_VRF_RT *data);
NBB_VOID rtm_strt_fill_func (StaticRouteSpec&        req,
                             AMB_GEN_IPS             *mib_msg,
                             AMB_CIPR_RTM_STATIC_RT  *data,
                             NBB_LONG                row_status);
} // namespace pds

namespace pdsa_stub {
NBB_VOID pdsa_li_stub_create (pdsa_config_t *conf);
NBB_VOID pdsa_l2f_stub_create (pdsa_config_t *conf);
NBB_VOID pdsa_smi_stub_create (pdsa_config_t *conf);
NBB_VOID pdsa_sck_stub_create (pdsa_config_t *conf);
NBB_VOID pdsa_hals_stub_create (pdsa_config_t *conf);
NBB_VOID pdsa_nar_stub_create (pdsa_config_t *conf);
NBB_VOID pdsa_ft_stub_create (pdsa_config_t *conf);
NBB_VOID pdsa_lim_create (pdsa_config_t *conf);
NBB_VOID pdsa_ftm_create (pdsa_config_t *conf);
NBB_VOID pdsa_nrm_create (pdsa_config_t *conf);
NBB_VOID pdsa_psm_create (pdsa_config_t *conf);
NBB_VOID pdsa_rtm_create (pdsa_config_t *conf);
NBB_VOID pdsa_bgp_create (pdsa_config_t *conf);
NBB_VOID pdsa_evpn_create (pdsa_config_t *conf);
}

namespace pds_ms_test {
NBB_VOID pdsa_test_row_update_l2f_mac_ip_cfg (ip_addr_t ip_addr,
                                              NBB_ULONG host_ifindex);
} // end namespace pds_ms_test
#endif /*__PDSA_MGMT_UTILS_HPP__*/
