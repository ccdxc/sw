#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <grpc++/grpc++.h>
#include <memory>
#include "dol/test/storage/hal_if.hpp"
#include "dol/test/storage/utils.hpp"
#include "dol/test/storage/queues.hpp"
#include "dol/test/storage/tests.hpp"
#include "dol/test/storage/r2n.hpp"
#include "dol/test/storage/qstate_if.hpp"
#include "nic/gen/proto/hal/internal.grpc.pb.h"
#include "nic/gen/proto/hal/interface.grpc.pb.h"
#include "nic/gen/proto/hal/l2segment.grpc.pb.h"
#include "nic/gen/proto/hal/vrf.grpc.pb.h"
#include "nic/gen/proto/hal/endpoint.grpc.pb.h"
#include "nic/gen/proto/hal/session.grpc.pb.h"
#include "nic/gen/proto/hal/rdma.grpc.pb.h"
#include "nic/utils/host_mem/c_if.h"
#include "nic/model_sim/include/lib_model_client.h"

namespace tests {
void test_ring_doorbell(uint16_t lif, uint8_t qtype, uint32_t qid,
                        uint8_t ring, uint16_t index);
}

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using intf::Interface;
using vrf::Vrf;
using l2segment::L2Segment;
using endpoint::Endpoint;
using session::Session;
using rdma::Rdma;

extern std::shared_ptr<Channel> hal_channel;

namespace {

std::unique_ptr<Interface::Stub> interface_stub;
std::unique_ptr<Vrf::Stub> vrf_stub;;
std::unique_ptr<L2Segment::Stub> l2segment_stub;
std::unique_ptr<Endpoint::Stub> endpoint_stub;
std::unique_ptr<Session::Stub> session_stub;
std::unique_ptr<Rdma::Stub> rdma_stub;

const uint32_t	kRoceEntrySize	 = 6; // Default is 64 bytes
const uint32_t	kRoceNumEntries	 = 6; // Default is 64 entries
const uint32_t kPvmRoceSqXlateTblSize	= (64 * 64); // 64 SQs x 64 bytes each

const uint32_t kR2NNumBufs = 4;
const uint32_t kR2NDataSize = 4096;
const uint32_t kR2NDataBufOffset = 4096;

const uint32_t kMaxRDMAKeys = 64;
const uint32_t kMaxRDMAPTEntries = 1024;
const uint32_t kSQType = 0;
const uint32_t kRQType = 1;
const uint32_t kCQType = 5;
const uint32_t kLifID = 109;
const uint32_t kRdmaPD = 1;
const uint32_t kVrfID = 11;
const uint32_t kL2SegmentID = 2;
const uint32_t kInterfaceID1 = 4;
const uint32_t kInterfaceID2 = 5;
const uint32_t kSessionID = 6;
const uint32_t kIPAddr1 = 0x0a000002;
const uint32_t kIPAddr2 = 0x0a000003;
const uint64_t kMACAddr1 = 0x1234;
const uint64_t kMACAddr2 = 0x4567;
const uint32_t kDot1QEncapVal = 301;
const uint32_t kVlanId = 2;

uint64_t pvm_roce_sq_xlate_addr;
uint64_t g_rdma_hw_lif_id;
uint64_t g_l2seg_handle;
uint64_t g_enic1_handle, g_enic2_handle;
uint32_t g_rdma_pvm_roce_init_sq;
uint32_t g_rdma_pvm_roce_tgt_sq;
uint32_t g_rdma_pvm_roce_tgt_cq;


}  // anonymous namespace

void CreateStubs() {
  interface_stub = std::move(Interface::NewStub(hal_channel));
  vrf_stub = std::move(Vrf::NewStub(hal_channel));
  l2segment_stub = std::move(L2Segment::NewStub(hal_channel));
  endpoint_stub = std::move(Endpoint::NewStub(hal_channel));
  session_stub = std::move(Session::NewStub(hal_channel));
  rdma_stub = std::move(Rdma::NewStub(hal_channel));
}

int CreateRDMALIF(uint32_t sw_lif_id) {
  hal_if::lif_params_t params;
  bzero(&params, sizeof(params));
  params.sw_lif_id = sw_lif_id;
  // RDMA SQ
  params.type[kSQType].valid = true;
  params.type[kSQType].queue_size = 12;  // 4096
  params.type[kSQType].num_queues = 1;   // 2, 0-initiator 1-target
  params.type[kSQType].queue_purpose = intf::LIF_QUEUE_PURPOSE_RDMA_SEND;

  // RDMA RQ
  params.type[kRQType].valid = true;
  params.type[kRQType].queue_size = 12;  // 4096
  params.type[kRQType].num_queues = 1;   // 2, 0-initiator 1-target
  params.type[kRQType].queue_purpose = intf::LIF_QUEUE_PURPOSE_RDMA_RECV;

  // Fill not-needed types with a single QPCB of 4K size
  // to avoid the assert in LIF manager (till fixed).
  params.type[2].valid = true;
  params.type[2].queue_size = 12;  // 4096
  params.type[3].valid = true;
  params.type[3].queue_size = 12;  // 4096
  params.type[4].valid = true;
  params.type[4].queue_size = 12;  // 4096

  // RDMA CQ
  params.type[kCQType].valid = true;
  params.type[kCQType].queue_size = 5;   // 4096
  params.type[kCQType].num_queues = 1;   // 2, 0-initiator 1-target
  params.type[kCQType].queue_purpose = intf::LIF_QUEUE_PURPOSE_CQ;

  params.rdma_enable = true;
  params.rdma_max_keys = kMaxRDMAKeys;
  params.rdma_max_pt_entries = kMaxRDMAPTEntries;

  return hal_if::create_lif(&params, &g_rdma_hw_lif_id);
}

int CreateVrf(uint32_t ten_id) {
  grpc::ClientContext context;
  vrf::VrfRequestMsg req_msg;
  vrf::VrfResponseMsg resp_msg;

  auto req = req_msg.add_request();
  req->mutable_key_or_handle()->set_vrf_id(ten_id);

  auto status = vrf_stub->VrfCreate(&context, req_msg, &resp_msg);
  if (!status.ok()) return -1;

  return 0;
}

int CreateL2Segment(uint32_t ten_id, uint32_t l2_seg_id, uint64_t *handle) {
  grpc::ClientContext context;
  l2segment::L2SegmentRequestMsg req_msg;
  l2segment::L2SegmentResponseMsg resp_msg;

  auto req = req_msg.add_request();
  req->mutable_vrf_key_handle()->set_vrf_id(ten_id);
  req->mutable_vrf_key_handle()->set_vrf_id(ten_id);
  req->mutable_key_or_handle()->set_segment_id(l2_seg_id);
  req->mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
  req->mutable_wire_encap()->set_encap_value(kDot1QEncapVal);

  auto status = l2segment_stub->L2SegmentCreate(&context, req_msg, &resp_msg);
  if (!status.ok()) return -1;

  auto resp = resp_msg.response(0);
  *handle = resp.mutable_l2segment_status()->l2segment_handle();
  return 0;
}

int CreateEnicif(uint32_t ten_id, uint32_t lif_id, uint32_t if_id,
                 uint32_t l2seg_id, uint32_t vlan_id, uint64_t mac_addr,
                 uint64_t *handle) {
  grpc::ClientContext context;
  intf::InterfaceRequestMsg req_msg;
  intf::InterfaceResponseMsg resp_msg;

  auto req = req_msg.add_request();
  req->set_type(intf::IF_TYPE_ENIC);
  req->mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(lif_id);
  req->mutable_key_or_handle()->set_interface_id(if_id);
  req->mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
  req->mutable_if_enic_info()->mutable_enic_info()->set_l2segment_id(l2seg_id);
  req->mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(vlan_id);
  req->mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac_addr);

  auto status = interface_stub->InterfaceCreate(&context, req_msg, &resp_msg);
  if (!status.ok()) return -1;

  auto resp = resp_msg.response(0);
  *handle = resp.mutable_status()->if_handle();
  return 0;
}

int CreateEP(uint32_t ten_id, uint64_t l2seg_handle, uint64_t if_handle,
             uint64_t mac_addr, uint32_t ip_addr) {
  grpc::ClientContext context;
  endpoint::EndpointRequestMsg req_msg;
  endpoint::EndpointResponseMsg resp_msg;

  auto req = req_msg.add_request();
  req->mutable_vrf_key_handle()->set_vrf_id(ten_id);
  req->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_handle);
  req->mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(if_handle);
  req->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(mac_addr);
  req->mutable_endpoint_attrs()->add_ip_address();
  req->mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
  req->mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip_addr);

  auto status = endpoint_stub->EndpointCreate(&context, req_msg, &resp_msg);
  if (!status.ok()) return -1;

  return 0;
}

int CreateSession(uint32_t ten_id, uint32_t session_id, uint32_t ip_addr1, uint32_t ip_addr2) {
  grpc::ClientContext context;
  session::SessionRequestMsg req_msg;
  session::SessionResponseMsg resp_msg;

  auto req = req_msg.add_request();
  req->mutable_meta()->set_vrf_id(ten_id);
  req->set_session_id(session_id);
  req->mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip_addr1);
  req->mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip_addr2);
  req->mutable_initiator_flow()->mutable_flow_key()->
      mutable_v4_key()->set_ip_proto(types::IPPROTO_UDP);
  req->mutable_initiator_flow()->mutable_flow_key()->
      mutable_v4_key()->mutable_tcp_udp()->set_sport(0);
  req->mutable_initiator_flow()->mutable_flow_key()->
      mutable_v4_key()->mutable_tcp_udp()->set_dport(0x12B7);
  req->mutable_initiator_flow()->mutable_flow_data()->
       mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);
  req->mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip_addr2);
  req->mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip_addr1);
  req->mutable_responder_flow()->mutable_flow_data()->
       mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);
  req->mutable_responder_flow()->mutable_flow_key()->
      mutable_v4_key()->set_ip_proto(types::IPPROTO_UDP);
  req->mutable_responder_flow()->mutable_flow_key()->
      mutable_v4_key()->mutable_tcp_udp()->set_sport(0);
  req->mutable_responder_flow()->mutable_flow_key()->
      mutable_v4_key()->mutable_tcp_udp()->set_dport(0x12B7);

  auto status = session_stub->SessionCreate(&context, req_msg, &resp_msg);
  if (!status.ok()) return -1;

  return 0;
}

int rdma_p4_init() {
  CreateStubs();
  if (CreateRDMALIF(kLifID) < 0) return -1;
  printf("RDMA LIF created\n");
  if (CreateVrf(kVrfID) < 0) return -1;
  printf("Vrf created\n");
  if (CreateL2Segment(kVrfID, kL2SegmentID, &g_l2seg_handle) < 0) return -1;
  printf("L2 segment created\n");
  if (CreateEnicif(kVrfID, kLifID, kInterfaceID1, kL2SegmentID, kVlanId, kMACAddr1, &g_enic1_handle) < 0) return -1;
  printf("ENIC IF1 created\n");
  if (CreateEnicif(kVrfID, kLifID, kInterfaceID2, kL2SegmentID, kVlanId, kMACAddr2, &g_enic2_handle) < 0) return -1;
  printf("ENIC IF2 created\n");
  if (CreateEP(kVrfID, g_l2seg_handle, g_enic1_handle, kMACAddr1, kIPAddr1) < 0) return -1;
  printf("EP1 created\n");
  if (CreateEP(kVrfID, g_l2seg_handle, g_enic2_handle, kMACAddr2, kIPAddr2) < 0) return -1;
  printf("EP2 created\n");
  if (CreateSession(kVrfID, kSessionID, kIPAddr1, kIPAddr2) < 0) return -1;
  printf("session created\n");

  return 0;
}

namespace {

// Fixed keys:
const uint32_t kInitiatorCQLKey = 1;
const uint32_t kInitiatorSQLKey = 2;
const uint32_t kInitiatorRQLKey = 3;
const uint32_t kTargetCQLKey = 4;
const uint32_t kTargetSQLKey = 5;
const uint32_t kTargetRQLKey = 6;
// Target side recv buffers
const uint32_t kTargetRcvBuf1LKey = 7;
const uint32_t kTargetRcvBuf1RKey = 8;
// Initiator side recv buffers
const uint32_t kInitiatorRcvBuf1LKey = 10;
const uint32_t kInitiatorRcvBuf1RKey = 11;


// Variable keys: 
// Theese are base values which are incremented per test case

// Writeback data buf keys 
uint32_t WriteBackBufLKey = 16;
uint32_t WriteBackBufRKey = 24;
// Send Buf Lkey 
uint32_t SendBufLKey = 32;

// TODO: Fix this to at least 8K
const uint32_t kR2NBufSize = (2 * 4096);

const uint16_t kSQWQESize = 64;
const uint16_t kRQWQESize = 64;
const uint16_t kCQWQESize = 64;
const uint16_t kNumSQWQEs = 64;
const uint16_t kNumRQWQEs = 64;
const uint16_t kNumCQWQEs = 64;
void *initiator_cq_va;
void *initiator_sq_va;
void *initiator_rq_va;
uint16_t initiator_cq_cindex = 0;
uint16_t initiator_sq_pindex = 0;
uint16_t initiator_rq_pindex = 0;
void *initiator_rcv_buf_va = NULL;
void *initiator_rcv_buf_ptr_va = NULL;


void *target_cq_va;
void *target_sq_va;
void *target_rq_va;
uint16_t target_cq_cindex = 0;
uint16_t target_rq_pindex = 0;
void *target_rcv_buf_va = NULL;
void *target_rcv_buf_ptr_va = NULL;

}  // anonymous namespace

void SendBufLKeyIncr() { 
  SendBufLKey++;
}


void WriteBackBufKeysIncr() { 
  WriteBackBufLKey++;
  WriteBackBufRKey++;
}

void AllocHostMem() {
  assert((initiator_cq_va = alloc_page_aligned_host_mem(4096)) != nullptr);
  bzero(initiator_cq_va, 4096);
  assert((initiator_sq_va = alloc_page_aligned_host_mem(4096)) != nullptr);
  assert((initiator_rq_va = alloc_page_aligned_host_mem(4096)) != nullptr);

  assert((target_cq_va = alloc_page_aligned_host_mem(4096)) != nullptr);
  bzero(target_cq_va, 4096);
  assert((target_sq_va = alloc_page_aligned_host_mem(4096)) != nullptr);
  assert((target_rq_va = alloc_page_aligned_host_mem(4096)) != nullptr);
  assert((target_rcv_buf_va = alloc_page_aligned_host_mem(kR2NNumBufs * kR2NBufSize)) != nullptr);
  assert((initiator_rq_va = alloc_page_aligned_host_mem(4096)) != nullptr);
  assert((initiator_rcv_buf_va = alloc_page_aligned_host_mem(kR2NNumBufs * kR2NBufSize)) != nullptr);
}

void RdmaMemRegister(void *va, uint64_t pa, uint32_t len, uint32_t lkey,
                     uint32_t rkey, bool remote) {
  rdma::RdmaMemRegRequestMsg req;
  rdma::RdmaMemRegResponseMsg resp;
  rdma::RdmaMemRegSpec *mr = req.add_request();

  mr->set_hw_lif_id(g_rdma_hw_lif_id);
  mr->set_pd(kRdmaPD);
  mr->set_va((uint64_t)va);
  mr->set_len(len);
  mr->set_ac_local_wr(true);
  mr->set_lkey(lkey);
  if (remote) {
    mr->set_rkey(rkey);
    mr->set_ac_remote_wr(true);
    mr->set_ac_remote_rd(true);
  }
  mr->set_hostmem_pg_size(4096);
  uint32_t size_done = 0;
  while (size_done < len) {
    mr->add_va_pages_phy_addr(pa + size_done);
    size_done += (4096 - (pa & 0xFFF));
  }


  grpc::ClientContext context;
  auto status = rdma_stub->RdmaMemReg(&context, req, &resp);
  assert(status.ok());
}

void RdmaMemRegister(void *va, uint32_t lkey, uint32_t rkey, bool remote) {
  RdmaMemRegister(va, host_mem_v2p(va), 4096, lkey, rkey, remote);
}

void CreateCQ(uint32_t cq_num, uint32_t lkey) {
  rdma::RdmaCqRequestMsg req;
  rdma::RdmaCqResponseMsg resp;
  rdma::RdmaCqSpec *cq = req.add_request();

  cq->set_cq_num(cq_num);
  cq->set_hw_lif_id(g_rdma_hw_lif_id);
  cq->set_cq_wqe_size(kCQWQESize);
  cq->set_num_cq_wqes(kNumCQWQEs);
  cq->set_hostmem_pg_size(4096);
  cq->set_cq_lkey(lkey);

  grpc::ClientContext context;
  auto status = rdma_stub->RdmaCqCreate(&context, req, &resp);
  assert(status.ok());
}

void CreateInitiatorCQ() {
  RdmaMemRegister(initiator_cq_va, kInitiatorCQLKey, 0, false);
  CreateCQ(0, kInitiatorCQLKey);
}

void CreateTargetCQ() {
  RdmaMemRegister(target_cq_va, kTargetCQLKey, 0, false);
  CreateCQ(1, kTargetCQLKey);
}

void CreateInitiatorQP() {
  RdmaMemRegister(initiator_sq_va, kInitiatorSQLKey, 0, false);
  RdmaMemRegister(initiator_rq_va, kInitiatorRQLKey, 0, false);
  rdma::RdmaQpRequestMsg req;
  rdma::RdmaQpResponseMsg resp;
  rdma::RdmaQpSpec *rq = req.add_request();

  rq->set_qp_num(0);
  rq->set_hw_lif_id(g_rdma_hw_lif_id);
  rq->set_sq_wqe_size(kSQWQESize);
  rq->set_rq_wqe_size(kRQWQESize);
  rq->set_num_sq_wqes(kNumSQWQEs);
  rq->set_num_rq_wqes(kNumRQWQEs);
  rq->set_pd(kRdmaPD);
  rq->set_pmtu(4096);
  rq->set_hostmem_pg_size(4096);
  rq->set_svc(rdma::RDMA_SERV_TYPE_RC);
  rq->set_sq_lkey(kInitiatorSQLKey);
  rq->set_rq_lkey(kInitiatorRQLKey);
  rq->set_rq_cq_num(0);
  rq->set_sq_cq_num(0);
  rq->set_immdt_as_dbell(true);

  grpc::ClientContext context;
  auto status = rdma_stub->RdmaQpCreate(&context, req, &resp);
  assert(status.ok());
}

void CreateTargetQP() {
  RdmaMemRegister(target_sq_va, kTargetSQLKey, 0, false);
  RdmaMemRegister(target_rq_va, kTargetRQLKey, 0, false);
  RdmaMemRegister(initiator_rq_va, kInitiatorRQLKey, 0, false);
  rdma::RdmaQpRequestMsg req;
  rdma::RdmaQpResponseMsg resp;
  rdma::RdmaQpSpec *rq = req.add_request();

  rq->set_qp_num(1);
  rq->set_hw_lif_id(g_rdma_hw_lif_id);
  rq->set_sq_wqe_size(kSQWQESize);
  rq->set_rq_wqe_size(kRQWQESize);
  rq->set_num_sq_wqes(kNumSQWQEs);
  rq->set_num_rq_wqes(kNumRQWQEs);
  rq->set_pd(kRdmaPD);
  rq->set_pmtu(4096);
  rq->set_hostmem_pg_size(4096);
  rq->set_svc(rdma::RDMA_SERV_TYPE_RC);
  rq->set_sq_lkey(kTargetSQLKey);
  rq->set_rq_lkey(kTargetRQLKey);
  rq->set_rq_cq_num(1);
  rq->set_sq_cq_num(1);
  rq->set_immdt_as_dbell(true);

  grpc::ClientContext context;
  auto status = rdma_stub->RdmaQpCreate(&context, req, &resp);
  assert(status.ok());
}

void mac_to_buf(uint8_t *buf, uint64_t mac) {
  buf[0] = (mac >> 40) & 0xff;
  buf[1] = (mac >> 32) & 0xff;
  buf[2] = (mac >> 24) & 0xff;
  buf[3] = (mac >> 16) & 0xff;
  buf[4] = (mac >> 8) & 0xff;
  buf[5] = mac & 0xff;
}

void ipv4_to_buf(uint8_t *buf, uint32_t ip) {
  buf[0] = (ip >> 24) & 0xff;
  buf[1] = (ip >> 16) & 0xff;
  buf[2] = (ip >> 8) & 0xff;
  buf[3] = ip & 0xff;
}

// 1 - src , 2 - dst
void ConnectInitiatorAndTarget(uint32_t qp1, uint32_t qp2, uint64_t mac1,
                               uint64_t mac2, uint32_t ip1, uint32_t ip2) {
  uint8_t hdr[46] = { 0,0,0,0,0,0, 0,0,0,0,0,0, 0x81,0,0xE0,(uint8_t)kVlanId,
                      8,0,0x45, 7, 0, 0, 0, 1, 0, 0, 0x40, 0x11, 0, 0,
                      0,0,0,0, 0,0,0,0, 0,0,0x12, 0xB7, 0, 0, 0, 0};
  mac_to_buf(hdr, mac2);
  mac_to_buf(hdr+6, mac1);
  ipv4_to_buf(hdr+0x1E, ip1);
  ipv4_to_buf(hdr+0x22, ip2);

  rdma::RdmaQpUpdateRequestMsg req;
  rdma::RdmaQpUpdateResponseMsg resp;
  rdma::RdmaQpUpdateSpec *qu = req.add_request();
  qu->set_qp_num(qp1);
  qu->set_hw_lif_id(g_rdma_hw_lif_id);
  qu->set_oper(rdma::RDMA_UPDATE_QP_OPER_SET_DEST_QP);
  qu->set_dst_qp_num(qp2);
  grpc::ClientContext context1;
  auto status = rdma_stub->RdmaQpUpdate(&context1, req, &resp);
  assert(status.ok());

  req.clear_request();
  resp.clear_response();
  qu = req.add_request();
  qu->set_qp_num(qp1);
  qu->set_hw_lif_id(g_rdma_hw_lif_id);
  qu->set_oper(rdma::RDMA_UPDATE_QP_OPER_SET_HEADER_TEMPLATE);
  qu->set_dst_qp_num(qp2);
  qu->set_header_template(hdr, sizeof(hdr));
  grpc::ClientContext context2;
  status = rdma_stub->RdmaQpUpdate(&context2, req, &resp);
  assert(status.ok());
}

void PostTargetRcvBuf1() {
  uint8_t rqwqe[64];
  bzero(rqwqe, 64);

  // Post the offset of the command buffer in the 4K aligned page.
  // Correspondingly size is also lower by the same offset.
  r2n::r2n_buf_t *r2n_buf = (r2n::r2n_buf_t *) target_rcv_buf_ptr_va;
  uint32_t size = kR2NBufSize - offsetof(r2n::r2n_buf_t, cmd_buf);
  printf("Posting target buffer of size %d VA %p wrid %lx \n", 
         size, (void *) &r2n_buf->cmd_buf, host_mem_v2p((void *) &r2n_buf->cmd_buf));

  // Fill the RQ WQE to post the buffer (at the offset)
  utils::write_bit_fields(rqwqe, 64, 8, 1);  // num_sges = 1
  utils::write_bit_fields(rqwqe, 256, 64, (uint64_t) &r2n_buf->cmd_buf); // sge0->va 
  utils::write_bit_fields(rqwqe, 256+64, 32, size);  // sge0->len
  utils::write_bit_fields(rqwqe, 256+64+32, 32, kTargetRcvBuf1LKey);  // sge0->l_key
  // wrid passed back in cq is the buffer offset passed in
  utils::write_bit_fields(rqwqe, 0, 64, host_mem_v2p((void *) &r2n_buf->cmd_buf));

  // Pre-form the Status descriptor to point to the status buffer
  uint8_t *sqwqe = (uint8_t *) &(r2n_buf->sta_desc.s);
  utils::write_bit_fields(sqwqe, 0, 64, host_mem_v2p((void *) &r2n_buf->sta_buf));  // wrid 
  utils::write_bit_fields(sqwqe, 64, 4, 0);  // op_type = OP_TYPE_SEND
  utils::write_bit_fields(sqwqe, 72, 8, 1);  // Num SGEs = 1
  utils::write_bit_fields(sqwqe, 192, 32, (uint32_t)sizeof(r2n::nvme_be_sta_t));  // data len
  // write the SGE
  utils::write_bit_fields(sqwqe, 256, 64, (uint64_t)&r2n_buf->sta_buf);  // SGE-va
  utils::write_bit_fields(sqwqe, 256+64, 32, (uint32_t)sizeof(r2n::nvme_be_sta_t)); // SGE-len
  utils::write_bit_fields(sqwqe, 256+64+32, 32, kTargetRcvBuf1LKey); // SGE-lkey

  uint32_t offset = target_rq_pindex;
  offset *= kRQWQESize;
  uint8_t *p = (uint8_t *)target_rq_va;
  bcopy(rqwqe, p + offset, 64);
  target_rq_pindex++;
  if (target_rq_pindex >= kNumRQWQEs)
    target_rq_pindex = 0;
  tests::test_ring_doorbell(g_rdma_hw_lif_id, kRQType, 1, 0, target_rq_pindex);
}

void IncrTargetRcvBufPtr() {
  // TODO: Add wrap around
  target_rcv_buf_ptr_va = (void *) (((uint8_t *) target_rcv_buf_ptr_va) + kR2NBufSize);
}

void RegisterTargetRcvBufs() {
  RdmaMemRegister(target_rcv_buf_va, host_mem_v2p(target_rcv_buf_va), 
                  kR2NNumBufs * kR2NBufSize, kTargetRcvBuf1LKey, kTargetRcvBuf1RKey, true);

  // Init the target recv buffer pointer
  target_rcv_buf_ptr_va = target_rcv_buf_va;
  // Post all buffers
  for (int i = 0; i < (int) kR2NNumBufs; i++) {
    PostTargetRcvBuf1();
    IncrTargetRcvBufPtr();
  }
  // Reinit the target recv buffer pointer to the start
  target_rcv_buf_ptr_va = target_rcv_buf_va;
}


void PostInitiatorRcvBuf1() {
  uint8_t rqwqe[64];
  bzero(rqwqe, 64);

  printf("Posting initiator buffer of size %d VA %p wrid %lx \n", 
         kR2NBufSize, (void *) initiator_rcv_buf_ptr_va, host_mem_v2p((void *) initiator_rcv_buf_ptr_va));

  // Fill the RQ WQE to post the buffer (at the offset)
  utils::write_bit_fields(rqwqe, 64, 8, 1);  // num_sges = 1
  utils::write_bit_fields(rqwqe, 256, 64, (uint64_t) initiator_rcv_buf_ptr_va); // sge0->va 
  utils::write_bit_fields(rqwqe, 256+64, 32, kR2NBufSize);  // sge0->len
  utils::write_bit_fields(rqwqe, 256+64+32, 32, kInitiatorRcvBuf1LKey);  // sge0->l_key
  // wrid passed back in cq is the buffer address
  utils::write_bit_fields(rqwqe, 0, 64, host_mem_v2p((void *) initiator_rcv_buf_ptr_va));

  uint32_t offset = initiator_rq_pindex;
  offset *= kRQWQESize;
  uint8_t *p = (uint8_t *)initiator_rq_va;
  bcopy(rqwqe, p + offset, 64);
  initiator_rq_pindex++;
  if (initiator_rq_pindex >= kNumRQWQEs)
    initiator_rq_pindex = 0;
  tests::test_ring_doorbell(g_rdma_hw_lif_id, kRQType, 0, 0, initiator_rq_pindex);
}

void IncrInitiatorRcvBufPtr() {
  // TODO: Add wrap around
  initiator_rcv_buf_ptr_va = (void *) (((uint8_t *) initiator_rcv_buf_ptr_va) + kR2NBufSize);
}

void RegisterInitiatorRcvBufs() {
  RdmaMemRegister(initiator_rcv_buf_va, host_mem_v2p(initiator_rcv_buf_va), 
                  kR2NNumBufs * kR2NBufSize, kInitiatorRcvBuf1LKey, kInitiatorRcvBuf1RKey, true);
  // Init the initiator recv buffer pointer
  initiator_rcv_buf_ptr_va = initiator_rcv_buf_va;
  // Post all buffers
  for (int i = 0; i < (int) kR2NNumBufs; i++) {
    PostInitiatorRcvBuf1();
    IncrInitiatorRcvBufPtr();
  }
  // Reinit the initiator recv buffer pointer
  initiator_rcv_buf_ptr_va = initiator_rcv_buf_va;
}

bool PullCQEntry(void *cq_va, uint16_t *cq_cindex, uint32_t ent_size,
                 uint8_t expected_color, void *ret_buf) {
  uint8_t *p = &((uint8_t *)cq_va)[(*cq_cindex) * ent_size];
  for (int i = 0; i < 100; i++) {
    if (((p[10] >> 5) & 1) == expected_color) {
      bcopy(p, ret_buf, ent_size);
      return true;
    }
    usleep(1000);
  }
  return false;
}

void GetR2NUspaceBuf(void **va, uint64_t *pa, uint32_t *size) {
  *size = kR2NBufSize;
  *va = alloc_page_aligned_host_mem(*size);
  *pa = host_mem_v2p(*va);
}


void SendSmallUspaceBuf() {
  // Increment the LKey at the beginning of each API
  SendBufLKeyIncr();

  uint8_t sqwqe[64];
  bzero(sqwqe, 64);
  utils::write_bit_fields(sqwqe, 0, 64, 0x10);  // wrid = 1
  utils::write_bit_fields(sqwqe, 64, 4, 0);  // op_type = OP_TYPE_SEND
  //utils::write_bit_fields(sqwqe, 71, 1, 1);  // inline data valid
  utils::write_bit_fields(sqwqe, 72, 8, 1);  // Num SGEs = 1

  // Add the buffer to WQE.
  void *r2n_buf_va;
  uint64_t r2n_buf_pa;
  uint32_t r2n_buf_size;
  GetR2NUspaceBuf(&r2n_buf_va, &r2n_buf_pa, &r2n_buf_size);
  //uint32_t data_len = r2n_buf_size - offsetof(r2n::r2n_buf_t, cmd_buf);
  uint32_t data_len = r2n_buf_size;
  // Register R2N buf memory. Only LKey, no remote access.
  RdmaMemRegister(r2n_buf_va, r2n_buf_pa, r2n_buf_size, SendBufLKey, 0, false);
  utils::write_bit_fields(sqwqe, 192, 32, (uint64_t)data_len);  // data len
  // write the SGE
  utils::write_bit_fields(sqwqe, 256, 64, (uint64_t)r2n_buf_va);  // SGE-va, same as pa
  utils::write_bit_fields(sqwqe, 256+64, 32, data_len);
  utils::write_bit_fields(sqwqe, 256+64+32, 32, SendBufLKey);

  // Now post the WQE.
  uint32_t offset = initiator_sq_pindex;
  offset *= kSQWQESize;
  uint8_t *p = (uint8_t *)initiator_sq_va;
  bcopy(sqwqe, p + offset, 64);
  initiator_sq_pindex++;
  if (initiator_sq_pindex >= kNumSQWQEs)
    initiator_sq_pindex = 0;
  tests::test_ring_doorbell(g_rdma_hw_lif_id, kSQType, 0, 0, initiator_sq_pindex);
}

int GetR2NHbmBuf(uint64_t *pa, uint32_t *size) {
  *size = kR2NBufSize;
  // size + 4096 is done to get extra room for page alignment
  if (utils::hbm_addr_alloc((*size) + 4096, pa) < 0) {
    printf("Can't alloc R2N buffer in HBM \n");
    return -1;
  }
  // Align it to a 4K page
  *pa = (*pa + 4095) & 0xFFFFFFFFFFFFF000L;

  return 0;
}


int StartRoceWriteSeq(uint16_t ssd_handle, uint8_t byte_val, uint8_t **nvme_cmd_ptr, uint64_t slba) {
  // Increment the LKey at the beginning of each API
  SendBufLKeyIncr();

  // Get userspace R2N buffer for write command and data. 
  void *r2n_buf_va;
  uint64_t r2n_buf_pa;
  uint32_t r2n_buf_size;
  GetR2NUspaceBuf(&r2n_buf_va, &r2n_buf_pa, &r2n_buf_size);

  // Initialize and form the write command
  r2n::r2n_nvme_be_cmd_buf_init(r2n_buf_va, NULL, 0, ssd_handle, 0, 0, 0, nvme_cmd_ptr);
  tests::form_write_cmd_no_buf(*nvme_cmd_ptr, slba);

  // Get the HBM buffer for the write data to be PDMA'ed to before sending over RDMA
  uint64_t r2n_hbm_buf_pa;
  uint32_t r2n_hbm_buf_size;
  if (GetR2NHbmBuf(&r2n_hbm_buf_pa, &r2n_hbm_buf_size) < 0) {
    return -1;
  }
  assert(r2n_hbm_buf_size == r2n_buf_size);
  uint32_t data_len = r2n_hbm_buf_size - offsetof(r2n::r2n_buf_t, cmd_buf);

  // Register R2N buf memory. Only LKey, no remote access.
  RdmaMemRegister((void *) r2n_hbm_buf_pa, r2n_hbm_buf_pa, r2n_hbm_buf_size, SendBufLKey, 0, false);

  // Initialize the write data buffer (of size 4K) at an offset from the NVME backend
  // command
  memset(((uint8_t *) r2n_buf_va) + kR2NDataBufOffset - offsetof(r2n::r2n_buf_t, cmd_buf), 
         byte_val, kR2NDataSize);


  // For the send wqe
  uint8_t *sqwqe = (uint8_t *) alloc_host_mem(64);
  bzero(sqwqe, 64);
  utils::write_bit_fields(sqwqe, 0, 64, r2n_hbm_buf_pa);  // wrid 
  utils::write_bit_fields(sqwqe, 64, 4, 2);  // op_type = OP_TYPE_SEND_IMM
  utils::write_bit_fields(sqwqe, 72, 8, 1);  // Num SGEs = 1

  // Store doorbell information of PVM's ROCE CQ in immediate data 
  utils::write_bit_fields(sqwqe, 96, 11, queues::get_pvm_lif());
  utils::write_bit_fields(sqwqe, 107, 3, CQ_TYPE);
  utils::write_bit_fields(sqwqe, 110, 18, g_rdma_pvm_roce_tgt_cq);
  utils::write_bit_fields(sqwqe, 192, 32, data_len);  // data len

  // Form the SGE
  utils::write_bit_fields(sqwqe, 256, 64, r2n_hbm_buf_pa);  // SGE-va, same as pa
  utils::write_bit_fields(sqwqe, 256+64, 32, data_len);
  utils::write_bit_fields(sqwqe, 256+64+32, 32, SendBufLKey);

  // Consume an entry in the Submission queue as the sequencer will take it.
  // If this is not done, subsequent cases wont work
  initiator_sq_pindex++;
  if (initiator_sq_pindex >= kNumSQWQEs)
    initiator_sq_pindex = 0;

  // Now kickstart the sequencer
  tests::test_seq_write_roce(35, 60, g_rdma_pvm_roce_init_sq, r2n_buf_pa, 
		             r2n_hbm_buf_pa, data_len, 
                             host_mem_v2p((void *) sqwqe), 64);

  return 0;
}

int StartRoceReadSeq(uint32_t seq_pdma_q, uint16_t ssd_handle, uint8_t **nvme_cmd_ptr, 
                     uint8_t **read_buf_ptr, uint64_t slba, 
                     uint8_t pdma_dst_lif_override, uint16_t pdma_dst_lif) {

  if (!nvme_cmd_ptr || !read_buf_ptr) return -1;

  // Increment the variable keys at the beginning of each API
  SendBufLKeyIncr();
  WriteBackBufKeysIncr();

  // Get userspace R2N buffer for read command and data. 
  void *r2n_buf_va;
  uint64_t r2n_buf_pa;
  uint32_t r2n_buf_size;
  GetR2NUspaceBuf(&r2n_buf_va, &r2n_buf_pa, &r2n_buf_size);

  // Initialize and form the read command
  r2n::r2n_nvme_be_cmd_buf_init(r2n_buf_va, NULL, 0, ssd_handle, 0, 1, 0, nvme_cmd_ptr);
  tests::form_read_cmd_no_buf(*nvme_cmd_ptr, slba);

  // Register it with RDMA.
  RdmaMemRegister((void *) r2n_buf_va, r2n_buf_pa, r2n_buf_size, SendBufLKey, 0, false);

  // Initialize helper fields
  uint32_t r2n_data_len = sizeof(r2n::r2n_buf_t) - offsetof(r2n::r2n_buf_t, cmd_buf);
  uint8_t *read_data_buf = ((uint8_t *) r2n_buf_va) + kR2NDataBufOffset;
  memset(read_data_buf, 0, kR2NDataSize);
  *read_buf_ptr = read_data_buf;

  // Get the HBM buffer for the write back data for the read command
  uint64_t r2n_hbm_buf_pa;
  uint32_t r2n_hbm_buf_size;
  if (GetR2NHbmBuf(&r2n_hbm_buf_pa, &r2n_hbm_buf_size) < 0) {
    return -1;
  }
  // Register the HBM buffer with RDMA
  RdmaMemRegister((void *) r2n_hbm_buf_pa, r2n_hbm_buf_pa, r2n_hbm_buf_size, 
                  WriteBackBufLKey, WriteBackBufRKey, true);


  // For the RDMA send WQE
  uint8_t *sqwqe = (uint8_t *) alloc_host_mem(64);
  bzero(sqwqe, 64);
  utils::write_bit_fields(sqwqe, 0, 64, r2n_buf_pa);  // wrid 
  utils::write_bit_fields(sqwqe, 64, 4, 2);  // op_type = OP_TYPE_SEND_IMM
  utils::write_bit_fields(sqwqe, 72, 8, 1);  // Num SGEs = 1

  // Store doorbell information of PVM's ROCE CQ in immediate data 
  utils::write_bit_fields(sqwqe, 96, 11, queues::get_pvm_lif());
  utils::write_bit_fields(sqwqe, 107, 3, CQ_TYPE);
  utils::write_bit_fields(sqwqe, 110, 18, g_rdma_pvm_roce_tgt_cq);
  utils::write_bit_fields(sqwqe, 192, 32, r2n_data_len);  // data len

  // Local read command buffer goes into SGE
  utils::write_bit_fields(sqwqe, 256, 64, (uint64_t) r2n_buf_va);  // SGE-va
  utils::write_bit_fields(sqwqe, 256+64, 32, r2n_data_len);
  utils::write_bit_fields(sqwqe, 256+64+32, 32, SendBufLKey);


  // Pre-form the (RDMA) write descriptor to point to the data buffer
  // TODO: Remove the write_data_buf pointer and do this in P4+
  uint32_t write_wqe_offset = offsetof(r2n::r2n_buf_t, write_desc) - offsetof(r2n::r2n_buf_t, cmd_buf);
  uint8_t *write_wqe = ((uint8_t *) r2n_buf_va) + write_wqe_offset;
  void *write_data_buf = (void *) (((uint8_t *) target_rcv_buf_ptr_va) + kR2NDataBufOffset);  

  // Start the write WQE formation
  utils::write_bit_fields(write_wqe, 0, 64,  host_mem_v2p(write_data_buf)); // wrid
  utils::write_bit_fields(write_wqe, 64, 4, 5);  // op_type = OP_TYPE_WRITE_IMM
  utils::write_bit_fields(write_wqe, 72, 8, 1);  // Num SGEs = 1
  utils::write_bit_fields(write_wqe, 192, 32, (uint32_t) kR2NDataSize);  // data len

  // Write WQE: remote side buffer with immediate data as doorbell
  utils::write_bit_fields(write_wqe, 96, 11, queues::get_pvm_lif());
  utils::write_bit_fields(write_wqe, 107, 3, SQ_TYPE);
  utils::write_bit_fields(write_wqe, 110, 18, seq_pdma_q);
  utils::write_bit_fields(write_wqe, 128, 64, (uint64_t) r2n_hbm_buf_pa); // va == pa
  utils::write_bit_fields(write_wqe, 128+64, 32, (uint32_t) kR2NDataSize); // len
  utils::write_bit_fields(write_wqe, 128+64+32, 32, WriteBackBufRKey); // rkey
  // Write SGE: local side buffer
  utils::write_bit_fields(write_wqe, 256, 64, (uint64_t) write_data_buf); // SGE-va
  utils::write_bit_fields(write_wqe, 256+64, 32, (uint32_t) kR2NDataSize); // SGE-len
  utils::write_bit_fields(write_wqe, 256+64+32, 32, kTargetRcvBuf1LKey); // SGE-lkey

  // Now post the WQE into initiator send queue
  uint32_t offset = initiator_sq_pindex;
  offset *= kSQWQESize;
  uint8_t *p = (uint8_t *)initiator_sq_va;
  bcopy(sqwqe, p + offset, 64);
  initiator_sq_pindex++;
  if (initiator_sq_pindex >= kNumSQWQEs)
    initiator_sq_pindex = 0;

  // Now kickstart the sequencer
  tests::test_seq_read_roce(seq_pdma_q, r2n_hbm_buf_pa, host_mem_v2p((void *) read_data_buf),
                            kR2NDataSize, pdma_dst_lif_override, pdma_dst_lif,
                            g_rdma_hw_lif_id, kSQType, 0, 0, initiator_sq_pindex);

  return 0;
}

void rdma_queues_init() {
  AllocHostMem();
  CreateInitiatorCQ();
  CreateTargetCQ();
  CreateInitiatorQP();
  CreateTargetQP();
  ConnectInitiatorAndTarget(0, 1, kMACAddr1, kMACAddr2, kIPAddr1, kIPAddr2);
  ConnectInitiatorAndTarget(1, 0, kMACAddr2, kMACAddr1, kIPAddr2, kIPAddr1);
}

int rdma_pvm_qs_init() {
  int rc;
  // Init the initiator SQ 
  if ((rc = queues::pvm_roce_sq_init(g_rdma_hw_lif_id, 
                                     kSQType, 0, // 0 - initiator; 1 - target
                                     host_mem_v2p(initiator_sq_va), 
                                     kRoceNumEntries, kRoceEntrySize)) < 0) {
    printf("RDMA PVM Initiator ROCE SQ init failure\n");
    return -1;
  } else {
    g_rdma_pvm_roce_init_sq = (uint32_t) rc;
  }
  printf("RDMA PVM Initiator ROCE SQ init success\n");

  // Init the target SQ 
  if ((rc = queues::pvm_roce_sq_init(g_rdma_hw_lif_id, 
                                     kSQType, 1, // 0 - initiator; 1 - target
                                     host_mem_v2p(target_sq_va), 
                                     kRoceNumEntries, kRoceEntrySize)) < 0) {
    printf("RDMA PVM Target ROCE SQ init failure\n");
    return -1;
  } else {
    g_rdma_pvm_roce_tgt_sq = (uint32_t) rc;
  }
  printf("RDMA PVM Target ROCE SQ init success %p\n", target_sq_va);

  // Init the target CQ 
  if ((rc = queues::pvm_roce_cq_init(g_rdma_hw_lif_id, 
                                     kCQType, 1, // 0 - initiator; 1 - target
                                     host_mem_v2p(target_cq_va), 
                                     kRoceNumEntries, kRoceEntrySize,
                                     pvm_roce_sq_xlate_addr)) < 0) {
    printf("RDMA PVM Target ROCE CQ init failure\n");
    return -1;
  } else {
    g_rdma_pvm_roce_tgt_cq = (uint32_t) rc;
  }
  printf("RDMA PVM Target ROCE CQ init success\n");

  // Target SQ Xlate
  qstate_if::update_xlate_entry(queues::get_pvm_lif(), SQ_TYPE, 
                                g_rdma_pvm_roce_tgt_sq, 
                                pvm_roce_sq_xlate_addr + (1 * 64), NULL);

  // Target R2N Xlate
  qstate_if::update_xlate_entry(queues::get_pvm_lif(), SQ_TYPE, 
                                queues::get_pvm_r2n_sq(0),  // Only one R2N SQ
                                pvm_roce_sq_xlate_addr, NULL);
  return 0;
}


int rdma_init() {
  uint8_t ent[64];
  // Allocate the PVM ROCE XLATE address in HBM for the sequencer
  if (utils::hbm_addr_alloc(kPvmRoceSqXlateTblSize, &pvm_roce_sq_xlate_addr) < 0) {
    printf("Can't allocate Read HBM buffer \n");
    return -1;
  }

  printf("RDMA init start\n");
  if (rdma_p4_init() < 0) {
    printf("RDMA P4 init failed\n");
    return -1;
  }
  printf("RDMA P4 Init success\n");
  rdma_queues_init();
  printf("RDMA queues Init success\n");
  if (rdma_pvm_qs_init() < 0) {
    printf("RDMA PVM queues Init failed\n");
    return -1;
  }
  printf("RDMA PVM queues Init success\n");
  assert(PullCQEntry(initiator_cq_va, &initiator_cq_cindex, 64, 1, &ent) == false);
  assert(PullCQEntry(target_cq_va, &target_cq_cindex, 64, 1, &ent) == false);
  RegisterTargetRcvBufs();
  printf("Registered target recv buf\n");
  RegisterInitiatorRcvBufs();
  printf("Registered initiator recv buf\n");
  return 0;
}

void rdma_uspace_test() {
  uint8_t ent[64];
  SendSmallUspaceBuf();
  printf("Sent small buffer\n");
  if (!PullCQEntry(target_cq_va, &target_cq_cindex, 64, 1, &ent)) {
    printf("Failed to pull CQ entry from target\n");
    return;
  }
  target_cq_cindex++;
  printf("Got CQ entry at the target\n");
  utils::dump(ent);
  printf("Dumping buf1\n");
  utils::dump((uint8_t *)target_rcv_buf_ptr_va);
  if (PullCQEntry(initiator_cq_va, &initiator_cq_cindex, 64, 1, &ent)) {
    initiator_cq_cindex++;
    printf("Got CQ entry at the initiator\n");
    utils::dump(ent);
  }
}

void *rdma_get_initiator_rcv_buf() {
  return initiator_rcv_buf_ptr_va;
}

uint8_t *rdma_get_target_write_data_buf() {
  return ((uint8_t *) target_rcv_buf_ptr_va) + 4096;
}
