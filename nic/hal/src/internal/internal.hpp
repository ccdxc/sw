//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __INTERNAL_HPP__
#define __INTERNAL_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "lib/list/list.hpp"
#include "lib/ht/ht.hpp"
#include "lib/bitmap/bitmap.hpp"
#include "gen/proto/internal.pb.h"
#include "nic/hal/src/internal/internal.hpp"
#include "gen/proto/fwlog.pb.h"
#include "gen/proto/types.pb.h"

namespace hal {

void getprogram_address(const internal::ProgramAddressReq& req,
                        internal::ProgramAddressResponseMsg *rsp);

void allochbm_address(const internal::HbmAddressReq &req,
                      internal::HbmAddressResp *resp);

void configurelif_bdf(const internal::LifBdfReq &req,
                      internal::LifBdfResp *resp);

hal_ret_t software_phv_get (internal::SoftwarePhvGetRequest &req, internal::SoftwarePhvGetResponseMsg *rsp);
hal_ret_t software_phv_inject (internal::SoftwarePhvInject &req, internal::SoftwarePhvResponse *rsp);
hal_ret_t log_flow (fwlog::FWEvent &req, internal::LogFlowResponse *rsp);
hal_ret_t quiesce_msg_snd(const types::Empty &request, types::Empty* rsp);
hal_ret_t quiesce_start(const types::Empty &request, types::Empty* rsp);
hal_ret_t quiesce_stop(const types::Empty &request, types::Empty* rsp);

hal_ret_t tcpcb_create(internal::TcpCbSpec& spec,
                       internal::TcpCbResponse *rsp);

hal_ret_t tcpcb_update(internal::TcpCbSpec& spec,
                       internal::TcpCbResponse *rsp);

hal_ret_t tcpcb_delete(internal::TcpCbDeleteRequest& req,
                       internal::TcpCbDeleteResponseMsg *rsp);

hal_ret_t tcpcb_get(internal::TcpCbGetRequest& req,
                    internal::TcpCbGetResponseMsg *rsp);

hal_ret_t tlscb_create(internal::TlsCbSpec& spec,
                       internal::TlsCbResponse *rsp);

hal_ret_t tlscb_update(internal::TlsCbSpec& spec,
                       internal::TlsCbResponse *rsp);

hal_ret_t tlscb_delete(internal::TlsCbDeleteRequest& req,
                       internal::TlsCbDeleteResponseMsg *rsp);

hal_ret_t tlscb_get(internal::TlsCbGetRequest& req,
                    internal::TlsCbGetResponseMsg *rsp);

hal_ret_t wring_create(internal::WRingSpec& spec,
                       internal::WRingResponse *rsp);

hal_ret_t wring_update(internal::WRingSpec& spec,
                       internal::WRingResponse *rsp);

hal_ret_t wring_get_entries(internal::WRingGetEntriesRequest& req, internal::WRingGetEntriesResponseMsg *rsp);

hal_ret_t wring_get_meta(internal::WRingSpec& spec, internal::WRingGetMetaResponseMsg *rsp);

hal_ret_t wring_set_meta (internal::WRingSpec& spec, internal::WRingSetMetaResponse *rsp);

hal_ret_t   get_opaque_tag_addr(internal::GetOpaqueTagAddrRequest& request,
                             internal::GetOpaqueTagAddrResponseMsg *response);

hal_ret_t   barco_get_req_descr_entry(internal::BarcoGetReqDescrEntryRequest& request,
                  internal::BarcoGetReqDescrEntryResponseMsg *response);

hal_ret_t   barco_get_ring_meta(internal::BarcoGetRingMetaRequest& request,
                 internal::BarcoGetRingMetaResponseMsg *response);

hal_ret_t   barco_get_ring_meta_config(internal::BarcoGetRingMetaConfigRequest& request,
                 internal::BarcoGetRingMetaConfigResponseMsg *response);


hal_ret_t   cryptokey_create(internal::CryptoKeyCreateRequest &request,
        internal::CryptoKeyCreateResponse *response);

hal_ret_t   cryptokeycreatewith_id(internal::CryptoKeyCreateWithIdRequest &request,
        internal::CryptoKeyCreateWithIdResponse *response);

hal_ret_t   cryptokey_read(internal::CryptoKeyReadRequest &request,
        internal::CryptoKeyReadResponse *response);

hal_ret_t   cryptokey_update(internal::CryptoKeyUpdateRequest &request,
        internal::CryptoKeyUpdateResponse *response);

hal_ret_t   cryptokey_delete(internal::CryptoKeyDeleteRequest &request,
        internal::CryptoKeyDeleteResponse *response);

hal_ret_t   cryptoasymkey_create(internal::CryptoAsymKeyCreateRequest &request,
        internal::CryptoAsymKeyCreateResponse *response);

hal_ret_t   cryptoasymkey_delete(internal::CryptoAsymKeyDeleteRequest &request,
        internal::CryptoAsymKeyDeleteResponse *response);

hal_ret_t   cryptoasymkey_write(internal::CryptoAsymKeyWriteRequest &request,
        internal::CryptoAsymKeyWriteResponse *response);

hal_ret_t cryptoapi_invoke(internal::CryptoApiRequest &req,
        internal::CryptoApiResponse *resp);

hal_ret_t descraol_get(internal::DescrAolRequest& request,
                       internal::DescrAolResponseMsg *response);

hal_ret_t proxyccb_create(internal::ProxycCbSpec& spec,
                          internal::ProxycCbResponse *rsp);

hal_ret_t proxyccb_update(internal::ProxycCbSpec& spec,
                          internal::ProxycCbResponse *rsp);

hal_ret_t proxyccb_delete(internal::ProxycCbDeleteRequest& req,
                          internal::ProxycCbDeleteResponseMsg *rsp);

hal_ret_t proxyccb_get(internal::ProxycCbGetRequest& req,
                       internal::ProxycCbGetResponseMsg *rsp);

hal_ret_t proxyrcb_create(internal::ProxyrCbSpec& spec,
                          internal::ProxyrCbResponse *rsp);

hal_ret_t proxyrcb_update(internal::ProxyrCbSpec& spec,
                          internal::ProxyrCbResponse *rsp);

hal_ret_t proxyrcb_delete(internal::ProxyrCbDeleteRequest& req,
                          internal::ProxyrCbDeleteResponseMsg *rsp);

hal_ret_t proxyrcb_get(internal::ProxyrCbGetRequest& req,
                       internal::ProxyrCbGetResponseMsg *rsp);

hal_ret_t rawrcb_create(internal::RawrCbSpec& spec,
                        internal::RawrCbResponse *rsp);

hal_ret_t rawrcb_update(internal::RawrCbSpec& spec,
                        internal::RawrCbResponse *rsp);

hal_ret_t rawrcb_delete(internal::RawrCbDeleteRequest& req,
                        internal::RawrCbDeleteResponseMsg *rsp);

hal_ret_t rawrcb_get(internal::RawrCbGetRequest& req,
                     internal::RawrCbGetResponseMsg *rsp);

hal_ret_t rawccb_create(internal::RawcCbSpec& spec,
                        internal::RawcCbResponse *rsp);

hal_ret_t rawccb_update(internal::RawcCbSpec& spec,
                        internal::RawcCbResponse *rsp);

hal_ret_t rawccb_delete(internal::RawcCbDeleteRequest& req,
                        internal::RawcCbDeleteResponse *rsp);

hal_ret_t rawccb_get(internal::RawcCbGetRequest& req,
                     internal::RawcCbGetResponseMsg *rsp);

hal_ret_t vmotion_debug_req(internal::VmotionDebugSendRequest& req,
                            internal::VmotionDebugResponse *rsp);

hal_ret_t testsendfin_req(internal::TestSendFinRequest& req,
                          internal::TestSendFinResponse *rsp);

hal_ret_t flow_hash_get(internal::FlowHashGetRequest& req,
                        internal::FlowHashGetResponseMsg *rsp);

hal_ret_t testclocksync_req(internal::TestClockSyncRequest& req,
                          internal::TestClockSyncResponse *rsp);

hal_ret_t internal_port_get(internal::InternalPortRequest& req,
                            internal::InternalPortResponseMsg *rsp);

hal_ret_t testfteinject_packets(internal::TestInjectFtePacketRequest& req,
                                internal::TestInjectFtePacketResponse *rsp);

}    // namespace hal

#endif    // __INTERNAL_HPP__

