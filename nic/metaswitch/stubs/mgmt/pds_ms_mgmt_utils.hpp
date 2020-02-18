// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Common helper APIs header file for metaswitch stub programming 

#ifndef __PDS_MS_MGMT_UTILS_HPP__
#define __PDS_MS_MGMT_UTILS_HPP__
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
#include "nic/metaswitch/stubs/mgmt/pds_ms_config.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_config.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_ctm.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "gen/proto/types.pb.h"
#include "gen/proto/internal.pb.h"
#include "gen/proto/bgp.pb.h"
#include "gen/proto/evpn.pb.h"
#include "gen/proto/cp_route.pb.h"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pds_ms_bgp_utils_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pds_ms_evpn_utils_gen.hpp"
#include "gen/proto/cp_test.pb.h"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pds_ms_internal_utils_gen.hpp"
#include "nic/apollo/api/include/pds.hpp"

#define PDS_MS_CTM_GRPC_CORRELATOR 0x101
#define PDS_MS_CTM_STUB_INIT_CORRELATOR 0x42
#define PDS_MS_DEFAULT_VRF_ID     0
#define PDS_MS_BGP_RM_ENT_INDEX   1
#define PDS_MS_BGP_NM_ENT_INDEX   1
#define PDS_MS_SCK_ENT_INDEX      1
#define PDS_MS_I3_ENT_INDEX       1
#define PDS_MS_LI_ENT_INDEX       1
#define PDS_MS_LIM_ENT_INDEX      1
#define PDS_MS_L2F_ENT_INDEX      1
#define PDS_MS_NAR_ENT_INDEX      1
#define PDS_MS_NRM_ENT_INDEX      1
#define PDS_MS_RTM_DEF_ENT_INDEX  1 // Default VRF
#define PDS_MS_PSM_ENT_INDEX      1
#define PDS_MS_SMI_ENT_INDEX      1
#define PDS_MS_FTM_ENT_INDEX      1
#define PDS_MS_FT_ENT_INDEX       1
#define PDS_MS_HALS_ENT_INDEX     1
#define PDS_MS_EVPN_ENT_INDEX     2 // Use a different entity id than BGP
                                    // Required to redistribute routes to EVPN
                                    // (Type 5 test) 
#define PDS_MS_BGP_ROUTE_MAP_DEF_INDEX  1
#define PDS_MS_BGP_ROUTE_MAP_DEF_NUBMER 1
using namespace std;

sdk_ret_t pds_ms_api_to_sdk_ret(types::ApiStatus api_err);
types::ApiStatus pds_ms_sdk_ret_to_api_status(sdk_ret_t sdk_ret);

void ip_addr_to_spec(const ip_addr_t *ip_addr,
                     types::IPAddress *ip_addr_spec);

NBB_VOID pds_ms_convert_amb_ip_addr_to_ip_addr (NBB_BYTE      *amb_ip_addr,
                                                NBB_LONG      type,
                                                NBB_ULONG     len,
                                                ip_addr_t     *pds_ms_ip_addr);
NBB_VOID  pds_ms_convert_ip_addr_to_amb_ip_addr (ip_addr_t     pds_ms_ip_addr, 
                                               NBB_LONG      *type, 
                                               NBB_ULONG     *len, 
                                               NBB_BYTE      *amb_ip_addr,
                                               bool          is_zero_ip_valid);

bool ip_addr_spec_to_ip_addr (const types::IPAddress& in_ipaddr,
                              ip_addr_t *out_ipaddr);

NBB_LONG pds_ms_nbb_get_long(NBB_BYTE *byteVal);

static inline NBB_LONG pds_ms_get_amb_bool (NBB_LONG amb_bool) {
    // convert from amb_bool to bool
    return (amb_bool == AMB_TRUE) ? 1 : 0;
}
static inline NBB_VOID pds_ms_set_amb_bool(NBB_LONG *amb_bool,
                                           NBB_LONG bool_val) {
    // convert bool to amb_bool
    *amb_bool = bool_val ? AMB_TRUE : AMB_FALSE;
}

NBB_VOID
pds_ms_get_uuid(pds_obj_key_t *out_uuid, const string& in_str);

NBB_VOID
pds_ms_set_string_in_byte_array_with_len(NBB_BYTE *field,
                                       NBB_ULONG *len,
                                       string in_str);

NBB_VOID
pds_ms_set_string_in_byte_array_with_len_oid(NBB_ULONG *oid,
                                           string in_str,
                                           NBB_LONG setKeyOidIdx,
                                           NBB_LONG setKeyOidLenIdx);

string
pds_ms_get_string_in_byte_array_with_len(NBB_BYTE *in_str,
                                       NBB_ULONG len);

NBB_VOID
pds_ms_get_string_in_byte_array_with_len_oid(NBB_ULONG *oid,
                                           string in_str,
                                           NBB_LONG getKeyOidIdx,
                                           NBB_LONG getKeyOidLenIdx);

NBB_VOID
pds_ms_set_string_in_byte_array(NBB_BYTE *field,
                              string in_str);

NBB_VOID
pds_ms_set_string_in_byte_array_oid(NBB_ULONG *oid,
                                  string in_str,
                                  NBB_LONG setKeyOidIdx);

string
pds_ms_get_string_in_byte_array(NBB_BYTE *val,
                              NBB_ULONG len);

NBB_VOID
pds_ms_get_string_in_byte_array_oid(NBB_ULONG *oid,
                                  string in_str,
                                  NBB_LONG getKeyOidIdx);

namespace pds {
NBB_VOID bgp_rm_ent_get_fill_func (pds::BGPSpec &req,
                                   NBB_ULONG*   oid);

NBB_VOID bgp_rm_ent_set_fill_func (pds::BGPSpec   &req, 
                               AMB_GEN_IPS        *mib_msg, 
                               AMB_BGP_RM_ENT     *v_amb_bgp_rm_ent, 
                               NBB_LONG           row_status);
NBB_VOID bgp_rm_ent_pre_set (pds::BGPSpec &req, NBB_LONG row_status,
                             NBB_ULONG correlator, bool op_update);

NBB_VOID bgp_peer_pre_set(pds::BGPPeerSpec &req, NBB_LONG row_status,
                          NBB_ULONG correlator, bool op_update=false);

NBB_VOID
bgp_peer_afi_safi_pre_set(pds::BGPPeerAfSpec &req, NBB_LONG row_status,
                          NBB_ULONG correlator, bool op_update=false);

NBB_VOID bgp_peer_pre_get(pds::BGPPeerSpec &req, pds::BGPPeerGetResponse* resp);
NBB_VOID bgp_peer_afi_safi_pre_get(pds::BGPPeerAfSpec &req,
                                   pds::BGPPeerAfGetResponse* resp);
bool bgp_peer_afi_safi_pre_fill_get (amb_bgp_peer_afi_safi *data);
NBB_VOID bgp_peer_status_get_fill_func (pds::BGPPeerStatus& req,
                                        NBB_ULONG*          oid);

NBB_VOID bgp_peer_get_fill_func (pds::BGPPeerSpec&   req,
                                 NBB_ULONG*           oid);

NBB_VOID bgp_peer_set_fill_func (pds::BGPPeerSpec&   req,
                             AMB_GEN_IPS         *mib_msg,
                             AMB_BGP_PEER        *v_amb_bgp_peer,
                             NBB_LONG            row_status);

NBB_VOID
bgp_peer_af_get_fill_func (pds::BGPPeerAfSpec    &req,
                           NBB_ULONG*             oid);

NBB_VOID bgp_peer_af_set_fill_func (pds::BGPPeerAfSpec&        req,
                                AMB_GEN_IPS           *mib_msg,
                                AMB_BGP_PEER_AFI_SAFI *v_amb_bgp_peer_af,
                                NBB_LONG               row_status);
NBB_VOID evpn_evi_pre_set (EvpnEviSpec  &req,
                           NBB_LONG     row_status,
                           NBB_ULONG    test_correlator,
                           bool         op_update=false);
NBB_VOID evpn_evi_rt_pre_set (EvpnEviRtSpec  &req,
                              NBB_LONG       row_status,
                              NBB_ULONG      test_correlator,
                              bool           op_update=false);
NBB_VOID evpn_ip_vrf_pre_set (EvpnIpVrfSpec  &req,
                              NBB_LONG       row_status,
                              NBB_ULONG      test_correlator,
                              bool           op_update=false);
NBB_VOID evpn_ip_vrf_rt_pre_set (EvpnIpVrfRtSpec  &req,
                                 NBB_LONG       row_status,
                                 NBB_ULONG      test_correlator,
                                 bool           op_update=false);
NBB_VOID evpn_evi_get_fill_func (EvpnEviSpec&    req,
                                 NBB_ULONG*       oid);
NBB_VOID evpn_evi_set_fill_func (EvpnEviSpec&    req,
                             AMB_GEN_IPS     *mib_msg,
                             AMB_EVPN_EVI    *data,
                             NBB_LONG        row_status);
NBB_VOID  evpn_ip_vrf_get_fill_func (EvpnIpVrfSpec&   req,
                                 NBB_ULONG*         oid);
NBB_VOID  evpn_ip_vrf_set_fill_func (EvpnIpVrfSpec&   req,
                                 AMB_GEN_IPS      *mib_msg,
                                 AMB_EVPN_IP_VRF  *data,
                                 NBB_LONG         row_status);
NBB_VOID evpn_evi_rt_set_fill_func (EvpnEviRtSpec&   req,
                                AMB_GEN_IPS      *mib_msg,
                                AMB_EVPN_EVI_RT  *data,
                                NBB_LONG         row_status);
NBB_VOID evpn_evi_rt_get_fill_func (EvpnEviRtSpec&   req,
                                NBB_ULONG*         oid);
NBB_VOID evpn_ip_vrf_rt_get_fill_func (EvpnIpVrfRtSpec&      req,
                                   NBB_ULONG*         oid);
NBB_VOID evpn_ip_vrf_rt_set_fill_func (EvpnIpVrfRtSpec&      req,
                                   AMB_GEN_IPS           *mib_msg,
                                   AMB_EVPN_IP_VRF_RT    *data,
                                   NBB_LONG         row_status);
NBB_VOID evpn_mac_ip_get_fill_func (EvpnMacIpSpec& req, NBB_ULONG *oid);
NBB_VOID evpn_ip_vrf_fill_name_oid (EvpnIpVrfSpec& req, NBB_ULONG *oid);
NBB_VOID evpn_ip_vrf_fill_name_field (EvpnIpVrfSpec& req, AMB_GEN_IPS *mib_msg);
NBB_VOID evpn_ip_vrf_rt_fill_name_field (EvpnIpVrfRtSpec& req,
                                         AMB_GEN_IPS *mib_msg);
NBB_VOID evpn_ip_vrf_rt_fill_name_oid (EvpnIpVrfRtSpec& req, NBB_ULONG *oid);
NBB_VOID evpn_ip_vrf_get_name_field (EvpnIpVrfSpec* req, AMB_EVPN_IP_VRF *data);
NBB_VOID evpn_ip_vrf_rt_get_name_field (EvpnIpVrfRtSpec* req,
                                        AMB_EVPN_IP_VRF_RT *data);
NBB_VOID rtm_strt_set_fill_func (CPStaticRouteSpec&      req,
                                 AMB_GEN_IPS             *mib_msg,
                                 AMB_CIPR_RTM_STATIC_RT  *data,
                                 NBB_LONG                row_status);
NBB_VOID rtm_strt_get_fill_func (CPStaticRouteSpec& req, NBB_ULONG *oid);
NBB_VOID lim_intf_addr_fill_func (LimInterfaceAddrSpec&  req,
                                  AMB_GEN_IPS           *mib_msg,
                                  AMB_LIM_L3_IF_ADDR    *data,
                                  NBB_LONG              row_status);
NBB_VOID lim_sw_intf_fill_func (LimInterfaceSpec&    req,
                                AMB_GEN_IPS         *mib_msg,
                                AMB_LIM_SOFTWARE_IF *data,
                                NBB_LONG            row_status);
types::ApiStatus l2f_test_local_mac_ip_add (const CPL2fTestCreateSpec   *req,
                                            CPL2fTestResponse *resp);
types::ApiStatus l2f_test_local_mac_ip_del (const CPL2fTestDeleteSpec   *req,
                                            CPL2fTestResponse *resp);
NBB_VOID lim_l3_if_addr_pre_set(pds::LimInterfaceAddrSpec &req,
                                NBB_LONG row_status,
                                NBB_ULONG correlator,
                                bool      op_update=false);
NBB_VOID cp_route_pre_set(pds::CPStaticRouteSpec &req, NBB_LONG row_status,
                          NBB_ULONG correlator, bool op_update=false);
NBB_VOID update_bgp_route_map_table (NBB_ULONG correlator);
} // namespace pds

namespace pds_ms {
NBB_VOID pds_ms_rtm_redis_connected (pds_ms::pds_ms_config_t *conf);
NBB_VOID pds_ms_li_stub_create (pds_ms_config_t *conf);
NBB_VOID pds_ms_l2f_stub_create (pds_ms_config_t *conf);
NBB_VOID pds_ms_smi_stub_create (pds_ms_config_t *conf);
NBB_VOID pds_ms_sck_stub_create (pds_ms_config_t *conf);
NBB_VOID pds_ms_hals_stub_create (pds_ms_config_t *conf);
NBB_VOID pds_ms_nar_stub_create (pds_ms_config_t *conf);
NBB_VOID pds_ms_ft_stub_create (pds_ms_config_t *conf);
NBB_VOID pds_ms_lim_create (pds_ms_config_t *conf);
NBB_VOID pds_ms_ftm_create (pds_ms_config_t *conf);
NBB_VOID pds_ms_nrm_create (pds_ms_config_t *conf);
NBB_VOID pds_ms_psm_create (pds_ms_config_t *conf);
NBB_VOID pds_ms_rtm_ft_stub_join(pds_ms_config_t *conf, int entity_index);
NBB_VOID pds_ms_rtm_create (pds_ms_config_t *conf, int entity_index,
                            bool is_default);
NBB_VOID pds_ms_bgp_create (pds_ms_config_t *conf);
NBB_VOID pds_ms_evpn_create (pds_ms_config_t *conf);
void pds_ms_evpn_rtm_join(pds_ms_config_t *conf, int rtm_entity_index);
}

namespace pds_ms_test {
NBB_VOID pds_ms_test_row_update_l2f_mac_ip_cfg (ip_addr_t ip_addr,
                                              NBB_ULONG host_ifindex);
} // end namespace pds_ms_test

namespace pds_ms {
void populate_lim_addr_spec (ip_prefix_t                 *ip_prefix,
                             pds::LimInterfaceAddrSpec&   req,
                             uint32_t                    if_type,
                             uint32_t                    if_id);
} // end namespace pds_ms
#endif /*__PDS_MS_MGMT_UTILS_HPP__*/
