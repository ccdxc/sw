#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <grpc++/grpc++.h>
#include <memory>
#include <math.h>
#include "dol/iris/test/storage/hal_if.hpp"
#include "dol/iris/test/storage/utils.hpp"
#include "dol/iris/test/storage/queues.hpp"
#include "dol/iris/test/storage/tests.hpp"
#include "dol/iris/test/storage/r2n.hpp"
#include "dol/iris/test/storage/qstate_if.hpp"
#include "dol/iris/test/storage/rdma.hpp"
#include "gen/proto/internal.grpc.pb.h"
#include "gen/proto/interface.grpc.pb.h"
#include "gen/proto/l2segment.grpc.pb.h"
#include "gen/proto/vrf.grpc.pb.h"
#include "gen/proto/endpoint.grpc.pb.h"
#include "gen/proto/session.grpc.pb.h"
#include "gen/proto/rdma.grpc.pb.h"
#include "gen/proto/nwsec.grpc.pb.h"
#ifdef ELBA
#include "third-party/asic/elba/model/elb_top/elb_top_csr_defines.h"
#include "third-party/asic/elba/model/elb_top/csr_defines/elb_txs_c_hdr.h"
#else
#include "third-party/asic/capri/design/common/cap_addr_define.h"
#include "third-party/asic/capri/model/cap_txs/cap_txs_csr_define.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using intf::Interface;
using vrf::Vrf;
using l2segment::L2Segment;
using endpoint::Endpoint;
using session::Session;
using rdma::Rdma;
using nwsec::NwSecurity;

extern std::shared_ptr<Channel> hal_channel;

namespace {

std::unique_ptr<Interface::Stub> interface_stub;
std::unique_ptr<Vrf::Stub> vrf_stub;;
std::unique_ptr<L2Segment::Stub> l2segment_stub;
std::unique_ptr<Endpoint::Stub> endpoint_stub;
std::unique_ptr<Session::Stub> session_stub;
std::unique_ptr<Rdma::Stub> rdma_stub;
std::unique_ptr<NwSecurity::Stub> nwsec_stub;

const uint32_t	kRoceEntrySize	 = 6; // Default is 64 bytes
const uint32_t	kRoceCQEntrySize	 = 5; // Default is 32 bytes
const uint32_t	kRoceNumEntries	 = 8; // Default is 256 entries
const uint32_t kPvmRoceSqXlateTblSize	= (64 * 64); // 64 SQs x 64 bytes each

const uint32_t kR2NNumBufs = 128;
const uint32_t kR2NDataSize = 4096;
const uint32_t kR2NDataBufOffset = 4096;

const uint32_t kMaxRDMAKeys = 512;
const uint32_t kMaxRDMAAhs = 64;
const uint32_t kMaxRDMAPTEntries = 1024;
const uint32_t kRDMAQStateSize = 4096;
const uint32_t kSQType = 3;
const uint32_t kRQType = 4;
const uint32_t kCQType = 5;
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

uint64_t pvm_roce_tgt_sq_xlate_addr;
uint64_t pvm_roce_init_sq_xlate_addr;
uint32_t g_rdma_lif_id;
uint64_t g_rdma_hw_lif_id;
uint64_t g_l2seg_handle;
uint64_t g_enic1_handle, g_enic2_handle;
uint32_t g_rdma_pvm_roce_init_sq;
uint32_t g_rdma_pvm_roce_init_cq;
uint32_t g_rdma_pvm_roce_tgt_sq;
uint32_t g_rdma_pvm_roce_tgt_cq;
bool nvme_dp_init = false;
uint64_t g_secprof_hdl;


}  // anonymous namespace

namespace qstate_if {
	extern int get_qstate_addr(int lif, int qtype, int qid, uint64_t *qaddr);
}

uint32_t rdma_r2n_data_size(void)
{
    return kR2NDataSize;
}

uint32_t rdma_r2n_data_offset(void)
{
    return kR2NDataBufOffset;
}

void CreateStubs() {
  interface_stub = std::move(Interface::NewStub(hal_channel));
  vrf_stub = std::move(Vrf::NewStub(hal_channel));
  l2segment_stub = std::move(L2Segment::NewStub(hal_channel));
  endpoint_stub = std::move(Endpoint::NewStub(hal_channel));
  session_stub = std::move(Session::NewStub(hal_channel));
  rdma_stub = std::move(Rdma::NewStub(hal_channel));
  nwsec_stub = std::move(NwSecurity::NewStub(hal_channel));
}

int CreateRDMALIF(void) {
  hal_if::lif_params_t params;
  int ret;

  bzero(&params, sizeof(params));
  // RDMA SQ
  params.type[kSQType].valid = true;
  params.type[kSQType].queue_size = (uint16_t)log2(kRDMAQStateSize);
  params.type[kSQType].num_queues = 1;   // 2, 0-initiator 1-target
  params.type[kSQType].queue_purpose = intf::LIF_QUEUE_PURPOSE_RDMA_SEND;

  // RDMA RQ
  params.type[kRQType].valid = true;
  params.type[kRQType].queue_size = (uint16_t)log2(kRDMAQStateSize);
  params.type[kRQType].num_queues = 1;   // 2, 0-initiator 1-target
  params.type[kRQType].queue_purpose = intf::LIF_QUEUE_PURPOSE_RDMA_RECV;

  // Fill not-needed types with a single QPCB of 4K size
  // to avoid the assert in LIF manager (till fixed).
  params.type[0].valid = true;
  params.type[0].queue_size = (uint16_t)log2(kRDMAQStateSize);
  params.type[1].valid = true;
  params.type[1].queue_size = (uint16_t)log2(kRDMAQStateSize);
  params.type[2].valid = true;
  params.type[2].queue_size = (uint16_t)log2(kRDMAQStateSize);

  // RDMA CQ
  params.type[kCQType].valid = true;
  params.type[kCQType].queue_size = 6;   // 4096
  params.type[kCQType].num_queues = 1;   // 2, 0-initiator 1-target
  params.type[kCQType].queue_purpose = intf::LIF_QUEUE_PURPOSE_CQ;

  params.rdma_enable = true;
  params.rdma_max_keys = kMaxRDMAKeys;
  params.rdma_max_ahs = kMaxRDMAAhs;
  params.rdma_max_pt_entries = kMaxRDMAPTEntries;

  ret = hal_if::create_lif(&params, &g_rdma_hw_lif_id);
  if (ret == 0) {
    g_rdma_lif_id = params.sw_lif_id;
  }

  return ret;
}

int CreateSecurityProfile(uint64_t *handle) {
  grpc::ClientContext context;
  nwsec::SecurityProfileRequestMsg req_msg;
  nwsec::SecurityProfileResponseMsg rsp_msg;

  auto req = req_msg.add_request();
  req->mutable_key_or_handle()->set_profile_id(2);

  auto status =  nwsec_stub->SecurityProfileCreate(&context, req_msg, &rsp_msg);
  if (!status.ok()) return -1;

  auto resp = rsp_msg.response(0);
  *handle = resp.mutable_profile_status()->profile_handle();

  return 0;
}

int CreateVrf(uint32_t ten_id, uint64_t secprof_hdl) {
  grpc::ClientContext context;
  vrf::VrfRequestMsg req_msg;
  vrf::VrfResponseMsg resp_msg;

  auto req = req_msg.add_request();
  req->mutable_key_or_handle()->set_vrf_id(ten_id);
  req->mutable_security_key_handle()->set_profile_handle(secprof_hdl);

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
  *handle = resp.mutable_l2segment_status()->key_or_handle().l2segment_handle();
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
  req->mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(l2seg_id);
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
  req->mutable_vrf_key_handle()->set_vrf_id(ten_id);
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
  if (CreateRDMALIF() < 0) return -1;
  printf("RDMA LIF %u/%lu created\n", g_rdma_lif_id, g_rdma_hw_lif_id);
  if (CreateSecurityProfile(&g_secprof_hdl) < 0) return -1;
  printf("Security Profile created\n");
  if (CreateVrf(kVrfID, g_secprof_hdl) < 0) return -1;
  printf("Vrf created\n");
  if (CreateL2Segment(kVrfID, kL2SegmentID, &g_l2seg_handle) < 0) return -1;
  printf("L2 segment created\n");
  if (CreateEnicif(kVrfID, g_rdma_lif_id, kInterfaceID1, kL2SegmentID, kVlanId, kMACAddr1, &g_enic1_handle) < 0) return -1;
  printf("ENIC IF1 created\n");
  if (CreateEnicif(kVrfID, g_rdma_lif_id, kInterfaceID2, kL2SegmentID, kVlanId, kMACAddr2, &g_enic2_handle) < 0) return -1;
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
const uint16_t kCQWQESize = 32;
const uint16_t kNumSQWQEs = 256;
const uint16_t kNumRQWQEs = 256;
const uint16_t kNumCQWQEs = 256;
dp_mem_t *initiator_cq_va;
dp_mem_t *initiator_sq_va;
dp_mem_t *initiator_rq_va;
uint64_t initiator_rq_va_base;
uint16_t initiator_cq_cindex = 0;

dp_mem_t *target_cq_va;
dp_mem_t *target_sq_va;
dp_mem_t *target_rq_va;
uint64_t target_rq_va_base;
uint16_t target_cq_cindex = 0;

uint32_t roce_cq_mem_reg_size, roce_sq_mem_reg_size, roce_rq_mem_reg_size;
}  // anonymous namespace

dp_mem_t *initiator_rcv_buf_va = NULL;
dp_mem_t *target_rcv_buf_va = NULL;

uint32_t rdma_r2n_buf_size(void)
{
    return kR2NBufSize;
}

void SendBufLKeyIncr() {
  SendBufLKey++;
}


void WriteBackBufKeysIncr() {
  WriteBackBufLKey++;
  WriteBackBufRKey++;
}

void AllocRdmaMem() {
  roce_cq_mem_reg_size = NUM_TO_VAL(kRoceNumEntries) * NUM_TO_VAL(kRoceCQEntrySize);
  roce_sq_mem_reg_size = NUM_TO_VAL(kRoceNumEntries) * NUM_TO_VAL(kRoceEntrySize);
  roce_rq_mem_reg_size = NUM_TO_VAL(kRoceNumEntries) * NUM_TO_VAL(kRoceEntrySize);
  initiator_cq_va = new dp_mem_t(NUM_TO_VAL(kRoceNumEntries),
                                 NUM_TO_VAL(kRoceCQEntrySize), DP_MEM_ALIGN_PAGE);
  printf("Init CQ PA %lx %d \n", initiator_cq_va->pa(), roce_cq_mem_reg_size);
  initiator_sq_va = new dp_mem_t(NUM_TO_VAL(kRoceNumEntries),
                                 NUM_TO_VAL(kRoceEntrySize), DP_MEM_ALIGN_PAGE);
  printf("Init SQ PA %lx %d \n", initiator_sq_va->pa(), roce_sq_mem_reg_size);
  initiator_rq_va = new dp_mem_t(NUM_TO_VAL(kRoceNumEntries),
                                 NUM_TO_VAL(kRoceEntrySize), DP_MEM_ALIGN_PAGE);
  printf("Init RQ PA %lx %d \n", initiator_rq_va->pa(), roce_rq_mem_reg_size);
  initiator_rq_va_base = initiator_rq_va->pa();

  target_cq_va = new dp_mem_t(NUM_TO_VAL(kRoceNumEntries),
                              NUM_TO_VAL(kRoceCQEntrySize), DP_MEM_ALIGN_PAGE);
  printf("Tgt CQ PA %lx \n", target_cq_va->pa());
  target_sq_va = new dp_mem_t(NUM_TO_VAL(kRoceNumEntries),
                              NUM_TO_VAL(kRoceEntrySize), DP_MEM_ALIGN_PAGE);
  printf("Tgt SQ PA %lx \n", target_sq_va->pa());
  target_rq_va = new dp_mem_t(NUM_TO_VAL(kRoceNumEntries),
                              NUM_TO_VAL(kRoceEntrySize), DP_MEM_ALIGN_PAGE);
  printf("Tgt RQ PA %lx \n", target_rq_va->pa());
  target_rq_va_base = target_rq_va->pa();
  printf("Init RQ PA %lx; Tgt RQ PA %lx \n", initiator_rq_va_base, target_rq_va_base);

  target_rcv_buf_va = new dp_mem_t(kR2NNumBufs, kR2NBufSize, DP_MEM_ALIGN_PAGE);
  initiator_rcv_buf_va = new dp_mem_t(kR2NNumBufs, kR2NBufSize, DP_MEM_ALIGN_PAGE);
}

void RdmaMemRegister(uint64_t va, uint64_t pa, uint32_t len, uint32_t lkey,
                     uint32_t rkey, bool remote, bool is_host_mem) {
  rdma::RdmaMemRegRequestMsg req;
  rdma::RdmaMemRegResponseMsg resp;
  rdma::RdmaMemRegSpec *mr = req.add_request();
  Status status = Status::OK;

  mr->set_hw_lif_id(g_rdma_hw_lif_id);
  mr->set_pd(kRdmaPD);
  mr->set_va((uint64_t)va);
  mr->set_len(len);
  mr->set_ac_local_wr(true);
  mr->set_lkey(lkey);
  mr->set_host_addr(is_host_mem);
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
#ifdef RDMA_ADMIN
  status = rdma_stub->RdmaMemReg(&context, req, &resp);
#endif
  assert(status.ok());
}

void RdmaMemRegister(dp_mem_t *mem, uint32_t len, uint32_t lkey, uint32_t rkey, bool remote) {
  RdmaMemRegister(mem->va(), mem->pa(), len, lkey, rkey, remote, mem->is_mem_type_host_mem());
}

void CreateCQ(uint32_t cq_num, uint64_t pa, uint32_t len, bool is_targetCQ) {
  rdma::RdmaCqRequestMsg req;
  rdma::RdmaCqResponseMsg resp;
  rdma::RdmaCqSpec *cq = req.add_request();
  Status status = Status::OK;

  cq->set_cq_num(cq_num);
  cq->set_hw_lif_id(g_rdma_hw_lif_id);
  cq->set_cq_wqe_size(kCQWQESize);
  cq->set_num_cq_wqes(kNumCQWQEs);
  cq->set_hostmem_pg_size(4096);
  cq->set_host_addr(false);

  uint32_t size_done = 0;
  while (size_done < len) {
      cq->add_cq_va_pages_phy_addr(pa + size_done);
      size_done += (4096 - (pa & 0xFFF));
  }

  // when not using send_imm, configure RDMA to wake up target CQ
  if ((kRdmaSendOpType != RDMA_OP_TYPE_SEND_IMM) &&
      is_targetCQ) {

      uint64_t db_addr;
      uint64_t db_data;

      cq->set_wakeup_dpath(true);
      cq->set_wakeup_lif(queues::get_pvm_lif());
      cq->set_wakeup_qtype(CQ_TYPE);
      cq->set_wakeup_qid(g_rdma_pvm_roce_tgt_cq);
      cq->set_wakeup_ring_id(0);

      // RDMA will ring doorbell with pndx increment,
      // print out this info to make it easy to locate in model.log
      queues::get_capri_doorbell_with_pndx_inc(queues::get_pvm_lif(), CQ_TYPE,
                                               g_rdma_pvm_roce_tgt_cq, 0,
                                               &db_addr, &db_data);
      printf("Target CQ wakeup doorbell db_addr %lx db_data %lx\n", db_addr, db_data);
  }

  grpc::ClientContext context;
#ifdef RDMA_ADMIN
  status = rdma_stub->RdmaCqCreate(&context, req, &resp);
#endif
  assert(status.ok());
}

void CreateInitiatorCQ() {
    CreateCQ(0, initiator_cq_va->pa(), roce_cq_mem_reg_size, false);
}

void CreateTargetCQ() {
  printf("eos_ignore_addr target CQ 0x%lx len %u\n", target_cq_va->pa(),
         roce_cq_mem_reg_size);
  EOS_IGNORE_ADDR(target_cq_va->pa(), roce_cq_mem_reg_size);
  CreateCQ(1, target_cq_va->pa(), roce_cq_mem_reg_size, true);
}

void CreateInitiatorQP() {
  rdma::RdmaQpRequestMsg req;
  rdma::RdmaQpResponseMsg resp;
  rdma::RdmaQpSpec *rq = req.add_request();
  Status status = Status::OK;

  rq->set_qp_num(0);
  rq->set_hw_lif_id(g_rdma_hw_lif_id);
  rq->set_sq_wqe_size(kSQWQESize);
  rq->set_rq_wqe_size(kRQWQESize);
  rq->set_num_sq_wqes(kNumSQWQEs);
  rq->set_num_rq_wqes(kNumRQWQEs);
  rq->set_pd(kRdmaPD);
  rq->set_pmtu(9200);
  rq->set_hostmem_pg_size(4096);
  rq->set_svc(rdma::RDMA_SERV_TYPE_RC);

  rq->set_rq_cq_num(0);
  rq->set_sq_cq_num(0);
  rq->set_immdt_as_dbell(true);

  rq->set_sq_in_nic_memory(true);
  rq->set_sq_base_addr(initiator_sq_va->pa());
  rq->set_rq_in_nic_memory(true);
  rq->set_rq_base_addr(initiator_rq_va->pa());

#if 0
  uint32_t size_done = 0;
  uint32_t num_sq_pages = 0;
  while (size_done < roce_sq_mem_reg_size) {
      rq->add_va_pages_phy_addr(initiator_sq_va->pa() + size_done);
      size_done += (4096 - (initiator_sq_va->pa() & 0xFFF));
      ++num_sq_pages;
  }

  size_done = 0;
  while (size_done < roce_rq_mem_reg_size) {
      rq->add_va_pages_phy_addr(initiator_rq_va->pa() + size_done);
      size_done += (4096 - (initiator_rq_va->pa() & 0xFFF));
  }

  rq->set_num_sq_pages(num_sq_pages);
#endif

  grpc::ClientContext context;
#ifdef RDMA_ADMIN
  status = rdma_stub->RdmaQpCreate(&context, req, &resp);
#endif
  assert(status.ok());
}

void CreateTargetQP() {
  rdma::RdmaQpRequestMsg req;
  rdma::RdmaQpResponseMsg resp;
  rdma::RdmaQpSpec *rq = req.add_request();
  Status status = Status::OK;

  rq->set_qp_num(1);
  rq->set_hw_lif_id(g_rdma_hw_lif_id);
  rq->set_sq_wqe_size(kSQWQESize);
  rq->set_rq_wqe_size(kRQWQESize);
  rq->set_num_sq_wqes(kNumSQWQEs);
  rq->set_num_rq_wqes(kNumRQWQEs);
  rq->set_pd(kRdmaPD);
  rq->set_pmtu(9200);
  rq->set_hostmem_pg_size(4096);
  rq->set_svc(rdma::RDMA_SERV_TYPE_RC);
  rq->set_rq_cq_num(1);
  rq->set_sq_cq_num(1);
  rq->set_immdt_as_dbell(true);

  rq->set_sq_in_nic_memory(true);
  rq->set_sq_base_addr(target_sq_va->pa());
  rq->set_rq_in_nic_memory(true);
  rq->set_rq_base_addr(target_rq_va->pa());

#if 0
  uint32_t size_done = 0;
  uint32_t num_sq_pages = 0;
  while (size_done < roce_sq_mem_reg_size) {
      rq->add_va_pages_phy_addr(target_sq_va->pa() + size_done);
      size_done += (4096 - (target_sq_va->pa() & 0xFFF));
      ++num_sq_pages;
  }

  size_done = 0;
  while (size_done < roce_rq_mem_reg_size) {
      rq->add_va_pages_phy_addr(target_rq_va->pa() + size_done);
      size_done += (4096 - (target_rq_va->pa() & 0xFFF));
  }

  rq->set_num_sq_pages(num_sq_pages);
#endif

  grpc::ClientContext context;
#ifdef RDMA_ADMIN
  status = rdma_stub->RdmaQpCreate(&context, req, &resp);
#endif
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
  Status status = Status::OK;
  qu->set_qp_num(qp1);
  qu->set_hw_lif_id(g_rdma_hw_lif_id);
  qu->set_oper(rdma::RDMA_UPDATE_QP_OPER_SET_DEST_QPN);
  qu->set_dst_qp_num(qp2);
  grpc::ClientContext context1;
#ifdef RDMA_ADMIN
  status = rdma_stub->RdmaQpUpdate(&context1, req, &resp);
#endif
  assert(status.ok());

  req.clear_request();
  resp.clear_response();
  qu = req.add_request();
  qu->set_qp_num(qp1);
  qu->set_hw_lif_id(g_rdma_hw_lif_id);
  qu->set_oper(rdma::RDMA_UPDATE_QP_OPER_SET_AV);
  qu->set_dst_qp_num(qp2);
  qu->set_header_template(hdr, sizeof(hdr));
  grpc::ClientContext context2;
#ifdef RDMA_ADMIN
  status = rdma_stub->RdmaQpUpdate(&context2, req, &resp);
#endif
  assert(status.ok());

  req.clear_request();
  resp.clear_response();
  qu = req.add_request();
  qu->set_qp_num(qp1);
  qu->set_hw_lif_id(g_rdma_hw_lif_id);
  qu->set_oper(rdma::RDMA_UPDATE_QP_OPER_SET_STATE);
  qu->set_dst_qp_num(qp2);
  qu->set_qstate(3);
  grpc::ClientContext context3;
#ifdef RDMA_ADMIN
  status = rdma_stub->RdmaQpUpdate(&context3, req, &resp);
#endif
  assert(status.ok());
}


void FillTargetRQWQE(dp_mem_t *rqwqe, dp_mem_t *cmd_buf, uint32_t size) {
  rqwqe->clear();
  // Fill the RQ WQE to post the buffer (at the offset)
  rqwqe->write_bit_fields(72, 8, 1);  // num_sges = 1
  rqwqe->write_bit_fields(256, 64, cmd_buf->va()); // sge0->va
  rqwqe->write_bit_fields(256+64, 32, size);  // sge0->len
  rqwqe->write_bit_fields(256+64+32, 32, kTargetRcvBuf1LKey << 8);  // sge0->l_key
  // wrid passed back in cq is the buffer offset passed in
  rqwqe->write_bit_fields(0, 64, cmd_buf->pa());
  rqwqe->write_thru();
}

void FillInitiatorRQWQE(dp_mem_t *rqwqe, dp_mem_t *status_buf, uint32_t size) {
  rqwqe->clear();
  // Fill the RQ WQE to post the buffer (at the offset)
  rqwqe->write_bit_fields(72, 8, 1);  // num_sges = 1
  rqwqe->write_bit_fields(256, 64, status_buf->va()); // sge0->va
  rqwqe->write_bit_fields(256+64, 32, size);  // sge0->len
  rqwqe->write_bit_fields(256+64+32, 32, kInitiatorRcvBuf1LKey << 8);  // sge0->l_key
  // wrid passed back in cq is the buffer offset passed in
  rqwqe->write_bit_fields(0, 64, status_buf->pa());
  rqwqe->write_thru();
}

void PostTargetRcvBuf1() {

  // Post the offset of the command buffer in the 4K aligned page.
  // Correspondingly size is also lower by the same offset.
  uint32_t size = kR2NBufSize - offsetof(r2n::r2n_buf_t, cmd_buf);
  dp_mem_t *cmd_buf = target_rcv_buf_va->fragment_find(offsetof(r2n::r2n_buf_t, cmd_buf),
                                                       sizeof(r2n::nvme_be_cmd_t));
  printf("Posting target buffer of size %d VA %lx wrid %lx \n",
         size, cmd_buf->va(), cmd_buf->pa());

  // Fill the WQE to submit here in DOL infra
  dp_mem_t *rqwqe = target_rq_va->fragment_find(0, kRQWQESize);
  FillTargetRQWQE(rqwqe, cmd_buf, size);

  // Pre-form the buf post descriptor to post the buffer to via P4+
  dp_mem_t *r2n_buf_rqwqe = target_rcv_buf_va->fragment_find(0, kRQWQESize);
  FillTargetRQWQE(r2n_buf_rqwqe, cmd_buf, size);


  // Pre-form the Status descriptor to point to the status buffer
  dp_mem_t *sta_buf = target_rcv_buf_va->fragment_find(offsetof(r2n::r2n_buf_t, sta_buf),
                                                       sizeof(r2n::nvme_be_sta_t));
  dp_mem_t *sqwqe = target_rcv_buf_va->fragment_find(offsetof(r2n::r2n_buf_t, sta_desc.s),
                                                     sizeof(r2n::roce_sq_wqe_t));
  sqwqe->clear();

  // WRID, Num SGEs and data len are fixed
  sqwqe->write_bit_fields(0, 64, sta_buf->pa());  // wrid
  sqwqe->write_bit_fields(72, 8, 1);  // Num SGEs = 1
  sqwqe->write_bit_fields(224, 32, (uint32_t)sizeof(r2n::nvme_be_sta_t));  // data len

  // Optype and immediate data varies based on configuration
  // Store doorbell information of Initiator ROCE CQ in immediate data
  // TODO: FIXME later when kRdmaSendOpType changes but nvme_dp_init remains
  if (nvme_dp_init && kRdmaSendOpType == RDMA_OP_TYPE_SEND_IMM) {
      sqwqe->write_bit_fields(68, 4, kRdmaSendOpType);  // op_type = OP_TYPE_SEND_IMM
      sqwqe->write_bit_fields(96, 11, queues::get_pvm_lif());
      sqwqe->write_bit_fields(107, 3, CQ_TYPE);
      sqwqe->write_bit_fields(110, 18, g_rdma_pvm_roce_init_cq);
  } else {
    sqwqe->write_bit_fields(68, 4, 0);  // op_type = OP_TYPE_SEND
  }

  // write the SGE
  sqwqe->write_bit_fields(256, 64, sta_buf->va());  // SGE-va
  sqwqe->write_bit_fields(256+64, 32, (uint32_t)sizeof(r2n::nvme_be_sta_t)); // SGE-len
  sqwqe->write_bit_fields(256+64+32, 32, kTargetRcvBuf1LKey << 8); // SGE-lkey
  sqwqe->write_thru();

  // write the local buffer information of the Write descriptor
  dp_mem_t *write_desc_local = target_rcv_buf_va->fragment_find(offsetof(r2n::r2n_buf_t, write_desc_local),
                                                     sizeof(r2n::roce_sq_sge_t));
  write_desc_local->clear();

  dp_mem_t *write_data_buf = target_rcv_buf_va->fragment_find(kR2NDataBufOffset, kR2NDataSize);
  printf("Using write_buf VA %lx PA %lx \n", write_data_buf->va(), write_data_buf->pa());

  write_desc_local->write_bit_fields(0, 64, write_data_buf->va()); // SGE-va
  write_desc_local->write_bit_fields(64, 32, (uint32_t) kR2NDataSize); // SGE-len
  write_desc_local->write_bit_fields(64+32, 32, kTargetRcvBuf1LKey << 8); // SGE-lkey
  write_desc_local->write_thru();

  // Advance the line
  target_rq_va->line_advance();

  // Ring the doorbell
  tests::test_ring_doorbell(g_rdma_hw_lif_id, kRQType, 1, 0, target_rq_va->line_get());
}

void IncrTargetRcvBufPtr() {
  target_rcv_buf_va->line_advance();
}

void RegisterTargetRcvBufs() {

  /*
   * WARNING: A descendant of invokes test_ring_doorbell() which, when
   * RTL --skipverify is in effect, can trigger simulation to commence
   * prematurely during init time. This would cause all subsequent
   * tests setup to execute in simulation time (as opposed to zero time)
   * which is extremely slow. We use flags to instruct us to skip this
   * function, i.e., when we run certain tests under RTL which we know
   * do not need RDMA.
   */
  if (FLAGS_with_rtl_skipverify && run_acc_scale_tests_map) {
    return;
  }

  RdmaMemRegister(target_rcv_buf_va->va(), target_rcv_buf_va->pa(),
                  kR2NNumBufs * kR2NBufSize, kTargetRcvBuf1LKey << 8, kTargetRcvBuf1RKey << 8, true,
                  target_rcv_buf_va->is_mem_type_host_mem());

  // Post all buffers
  for (int i = 0; i < (int) kR2NNumBufs; i++) {
    PostTargetRcvBuf1();
    IncrTargetRcvBufPtr();
  }
  // Reinit the target recv buffer pointer to the start
  target_rcv_buf_va->line_set(0);
}


void PostInitiatorRcvBuf1() {


  dp_mem_t *status_buf = initiator_rcv_buf_va->fragment_find(IO_STATUS_BUF_BE_STATUS_OFFSET,
                                                             IO_STATUS_BUF_BE_STATUS_SIZE);

  // Fill the RQ WQE to post the buffer (at the offset)
  uint32_t size = kR2NBufSize - IO_STATUS_BUF_BE_STATUS_OFFSET;

  printf("Posting initiator buffer of size %d VA %lx wrid %lx \n",
         kR2NBufSize, status_buf->va(), status_buf->pa());

  // Fill the WQE to submit here in DOL infra
  dp_mem_t *rqwqe = initiator_rq_va->fragment_find(0, kRQWQESize);
  FillInitiatorRQWQE(rqwqe, status_buf, size);

  // Pre-form the buf post descriptor to post the buffer to via P4+
  dp_mem_t *r2n_buf_rqwqe = initiator_rcv_buf_va->fragment_find(0, kRQWQESize);
  FillInitiatorRQWQE(r2n_buf_rqwqe, status_buf, size);

  // Advance the line
  initiator_rq_va->line_advance();

  // Ring the doorbell
  tests::test_ring_doorbell(g_rdma_hw_lif_id, kRQType, 0, 0, initiator_rq_va->line_get());
}

void IncrInitiatorRcvBufPtr() {
    initiator_rcv_buf_va->line_advance();
}

void RegisterInitiatorRcvBufs() {

  /*
   * See WARNING in RegisterTargetRcvBufs()
   */
  if (FLAGS_with_rtl_skipverify && run_acc_scale_tests_map) {
    return;
  }

  RdmaMemRegister(initiator_rcv_buf_va->va(), initiator_rcv_buf_va->pa(),
                  kR2NNumBufs * kR2NBufSize, kInitiatorRcvBuf1LKey << 8, kInitiatorRcvBuf1RKey << 8, true,
                  initiator_rcv_buf_va->is_mem_type_host_mem());
  // Init the initiator recv buffer pointer
  // Post all buffers
  for (int i = 0; i < (int) kR2NNumBufs; i++) {
    PostInitiatorRcvBuf1();
    IncrInitiatorRcvBufPtr();
  }
  // Reinit the initiator recv buffer pointer
  initiator_rcv_buf_va->line_set(0);
}

bool PullCQEntry(dp_mem_t *cq_va, uint16_t *cq_cindex, uint32_t ent_size,
                 uint8_t expected_color, void *ret_buf) {
  uint8_t *cq_va_read;
  uint8_t *p;

  for (int i = 0; i < 100; i++) {
    cq_va_read = cq_va->read_thru();
    p = &cq_va_read[(*cq_cindex) * ent_size];

    if (((p[10] >> 5) & 1) == expected_color) {
      bcopy(p, ret_buf, ent_size);
      return true;
    }
    usleep(1000);
  }
  return false;
}

void SendSmallUspaceBuf() {
  // Increment the LKey at the beginning of each API
  SendBufLKeyIncr();

  dp_mem_t *sqwqe = initiator_sq_va->fragment_find(0, kSQWQESize);
  sqwqe->clear();

  sqwqe->write_bit_fields(0, 64, 0x10);  // wrid = 1
  sqwqe->write_bit_fields(68, 4, 0);  // op_type = OP_TYPE_SEND
  //sqwqe->write_bit_fields(71, 1, 1);  // inline data valid
  sqwqe->write_bit_fields(72, 8, 1);  // Num SGEs = 1

  // Add the buffer to WQE.
  dp_mem_t *r2n_buf_va = new dp_mem_t(1, kR2NBufSize, DP_MEM_ALIGN_PAGE);
  //uint32_t data_len = kR2NBufSize - offsetof(r2n::r2n_buf_t, cmd_buf);
  uint32_t data_len = kR2NBufSize;
  // Register R2N buf memory. Only LKey, no remote access.
  RdmaMemRegister(r2n_buf_va->va(), r2n_buf_va->pa(), kR2NBufSize, SendBufLKey << 8, 0, false,
                  r2n_buf_va->is_mem_type_host_mem());
  sqwqe->write_bit_fields(224, 32, (uint64_t)data_len);  // data len
  // write the SGE
  sqwqe->write_bit_fields(256, 64, r2n_buf_va->va());  // SGE-va, same as pa
  sqwqe->write_bit_fields(256+64, 32, data_len);
  sqwqe->write_bit_fields(256+64+32, 32, SendBufLKey << 8);
  sqwqe->write_thru();

  // Now post the WQE.
  initiator_sq_va->line_advance();
  tests::test_ring_doorbell(g_rdma_hw_lif_id, kSQType, 0, 0, initiator_sq_va->line_get());
}

int StartRoceWriteSeq(uint16_t ssd_handle, uint8_t byte_val, dp_mem_t **nvme_cmd_ptr, uint64_t slba, dp_mem_t **rolling_write_buf) {
  // Increment the LKey at the beginning of each API
  SendBufLKeyIncr();

  // Get userspace R2N buffer for write command and data.
  dp_mem_t *r2n_buf_va = new dp_mem_t(1, kR2NBufSize, DP_MEM_ALIGN_PAGE);

  // Initialize and form the write command
  r2n::r2n_nvme_be_cmd_buf_init(r2n_buf_va, NULL, 0, ssd_handle, 0, 0, 0, nvme_cmd_ptr);
  tests::form_write_cmd_no_buf(*nvme_cmd_ptr, slba);

  // Get the HBM buffer for the write data to be PDMA'ed to before sending over RDMA
  dp_mem_t *r2n_hbm_buf_pa = new dp_mem_t(1, kR2NBufSize, DP_MEM_ALIGN_PAGE);
  uint32_t data_len = kR2NBufSize - offsetof(r2n::r2n_buf_t, cmd_buf);

  // Register R2N buf memory. Only LKey, no remote access.
  RdmaMemRegister(r2n_hbm_buf_pa->va(), r2n_hbm_buf_pa->pa(), kR2NBufSize, SendBufLKey << 8, 0, false,
                  r2n_hbm_buf_pa->is_mem_type_host_mem());

  // Initialize the write data buffer (of size 4K) at an offset from the NVME backend
  // command
  dp_mem_t *wr_buf = r2n_buf_va->fragment_find(kR2NDataBufOffset - offsetof(r2n::r2n_buf_t, cmd_buf),
                                               kR2NDataSize);
  wr_buf->fill_thru(byte_val);
  *rolling_write_buf = wr_buf;

  // For the send wqe
  dp_mem_t *sqwqe = new dp_mem_t(1, kSQWQESize);

  sqwqe->write_bit_fields(0, 64, r2n_hbm_buf_pa->pa());  // wrid
  sqwqe->write_bit_fields(68, 4, kRdmaSendOpType);
  sqwqe->write_bit_fields(72, 8, 1);  // Num SGEs = 1

  // Store doorbell information of PVM's ROCE CQ in immediate data
  if (kRdmaSendOpType == RDMA_OP_TYPE_SEND_IMM) {
      sqwqe->write_bit_fields(96, 11, queues::get_pvm_lif());
      sqwqe->write_bit_fields(107, 3, CQ_TYPE);
      sqwqe->write_bit_fields(110, 18, g_rdma_pvm_roce_tgt_cq);
  }
  sqwqe->write_bit_fields(224, 32, data_len);  // data len

  // Form the SGE
  sqwqe->write_bit_fields(256, 64, r2n_hbm_buf_pa->pa());  // SGE-va, same as pa
  sqwqe->write_bit_fields(256+64, 32, data_len);
  sqwqe->write_bit_fields(256+64+32, 32, SendBufLKey << 8);
  sqwqe->write_thru();

  // Consume an entry in the Submission queue as the sequencer will take it.
  // If this is not done, subsequent cases wont work
  initiator_sq_va->line_advance();

  // Now kickstart the sequencer
  tests::test_seq_write_roce(queues::get_seq_pdma_sq(0),
                             queues::get_seq_roce_sq(0),
                             g_rdma_pvm_roce_init_sq, r2n_buf_va->pa(),
                             r2n_hbm_buf_pa->pa(), data_len,
                             sqwqe->pa(), kSQWQESize);

  return 0;
}

int StartRoceWritePdmaPrefilled(uint16_t seq_pdma_q,
                                uint16_t seq_pdma_index,
                                uint16_t seq_roce_q,
                                uint16_t seq_roce_index,
                                dp_mem_t *seq_roce_desc,
                                dp_mem_t *r2n_buf)
{
  // Increment the LKey at the beginning of each API
  SendBufLKeyIncr();

  // Register R2N buf memory. Only LKey, no remote access.
  uint32_t data_len = kR2NBufSize - offsetof(r2n::r2n_buf_t, cmd_buf);
  RdmaMemRegister(r2n_buf->va(), r2n_buf->pa(), kR2NBufSize,
                  SendBufLKey << 8, 0, false, r2n_buf->is_mem_type_host_mem());

  // For the send wqe
  dp_mem_t *sqwqe = new dp_mem_t(1, kSQWQESize);

  sqwqe->write_bit_fields(0, 64, r2n_buf->pa());  // wrid
  sqwqe->write_bit_fields(68, 4, kRdmaSendOpType);
  sqwqe->write_bit_fields(72, 8, 1);  // Num SGEs = 1

  // Store doorbell information of PVM's ROCE CQ in immediate data
  if (kRdmaSendOpType == RDMA_OP_TYPE_SEND_IMM) {
      sqwqe->write_bit_fields(96, 11, queues::get_pvm_lif());
      sqwqe->write_bit_fields(107, 3, CQ_TYPE);
      sqwqe->write_bit_fields(110, 18, g_rdma_pvm_roce_tgt_cq);
  }
  sqwqe->write_bit_fields(224, 32, data_len);  // data len

  // Form the SGE
  sqwqe->write_bit_fields(256, 64, r2n_buf->pa());  // SGE-va, same as pa
  sqwqe->write_bit_fields(256+64, 32, data_len);
  sqwqe->write_bit_fields(256+64+32, 32, SendBufLKey << 8);
  sqwqe->write_thru();

  // Consume an entry in the Submission queue as the sequencer will take it.
  // If this is not done, subsequent cases wont work
  initiator_sq_va->line_advance();

  // Now kickstart the sequencer
  tests::test_seq_roce_op_pdma_prefilled(seq_pdma_q, seq_pdma_index, seq_roce_desc,
                                         g_rdma_pvm_roce_init_sq, sqwqe);

  return 0;
}

int StartRoceReadSeq(uint32_t seq_pdma_q, uint32_t seq_roce_q, uint16_t ssd_handle,
                     dp_mem_t **nvme_cmd_ptr, dp_mem_t **read_buf_ptr, uint64_t slba,
                     uint8_t pdma_dst_lif_override, uint16_t pdma_dst_lif, uint32_t bdf) {

  uint64_t db_addr;
  uint64_t db_data;

  if (!nvme_cmd_ptr || !read_buf_ptr) return -1;

  // Increment the variable keys at the beginning of each API
  SendBufLKeyIncr();
  WriteBackBufKeysIncr();

  // Get userspace R2N buffer for read command and data.
  dp_mem_t *r2n_buf_va;
  if (pdma_dst_lif_override != 0) {
    r2n_buf_va = new dp_mem_t(1, kR2NBufSize, DP_MEM_ALIGN_PAGE, DP_MEM_TYPE_HOST_MEM);
  } else {
    r2n_buf_va = new dp_mem_t(1, kR2NBufSize, DP_MEM_ALIGN_PAGE);
  }

  // Initialize and form the read command
  r2n::r2n_nvme_be_cmd_buf_init(r2n_buf_va, NULL, 0, ssd_handle, 0, 1, 0, nvme_cmd_ptr);
  tests::form_read_cmd_no_buf(*nvme_cmd_ptr, slba);

  // Register it with RDMA.
  RdmaMemRegister(r2n_buf_va->va(), r2n_buf_va->pa(), kR2NBufSize, SendBufLKey << 8, 0, false,
                  r2n_buf_va->is_mem_type_host_mem());

  // Initialize helper fields
  uint32_t r2n_data_len = sizeof(r2n::r2n_buf_t) - offsetof(r2n::r2n_buf_t, cmd_buf);

  dp_mem_t *rd_buf = r2n_buf_va->fragment_find(kR2NDataBufOffset, kR2NDataSize);
  rd_buf->clear_thru();
  *read_buf_ptr = rd_buf;

  // per VP, we need to find a different way to test LIF override since
  // all the RDMA buffers are now in HBM memory.

  // Register the memory if LIF override is setup
  if (pdma_dst_lif_override != 0) {
    // Add the BDF to bits 62:52 of the read_data_buf's PA to form the match address
    uint64_t match_addr = rd_buf->pa() | (((uint64_t) (bdf & 0xFF)) << 52);
    // Clear bit 63 as model sets only the BDF and not bit 63
    match_addr &= 0x7FFFFFFFFFFFFFFFULL;
    // Register the address to match fully
    printf("Registering address %lx \n", match_addr);
    REGISTER_MEM_ADDR(match_addr);
  }

  // Get the HBM buffer for the write back data for the read command
  dp_mem_t *r2n_hbm_buf_pa = new dp_mem_t(1, kR2NBufSize, DP_MEM_ALIGN_PAGE);
  // Register the HBM buffer with RDMA
  RdmaMemRegister(r2n_hbm_buf_pa->va(), r2n_hbm_buf_pa->pa(), kR2NBufSize,
                  WriteBackBufLKey << 8, WriteBackBufRKey << 8, true,
                  r2n_hbm_buf_pa->is_mem_type_host_mem());


  // For the RDMA send WQE
  dp_mem_t *sqwqe = new dp_mem_t(1, kSQWQESize);

  sqwqe->write_bit_fields(0, 64, r2n_buf_va->pa());  // wrid
  sqwqe->write_bit_fields(68, 4, kRdmaSendOpType);
  sqwqe->write_bit_fields(72, 8, 1);  // Num SGEs = 1

  // Store doorbell information of PVM's ROCE CQ in immediate data
  if (kRdmaSendOpType == RDMA_OP_TYPE_SEND_IMM) {
      sqwqe->write_bit_fields(96, 11, queues::get_pvm_lif());
      sqwqe->write_bit_fields(107, 3, CQ_TYPE);
      sqwqe->write_bit_fields(110, 18, g_rdma_pvm_roce_tgt_cq);
  }
  sqwqe->write_bit_fields(224, 32, r2n_data_len);  // data len

  // Local read command buffer goes into SGE
  sqwqe->write_bit_fields(256, 64, r2n_buf_va->va());  // SGE-va
  sqwqe->write_bit_fields(256+64, 32, r2n_data_len);
  sqwqe->write_bit_fields(256+64+32, 32, SendBufLKey << 8);
  sqwqe->write_thru();


  // Pre-form the (RDMA) write descriptor to point to the data buffer
  uint32_t write_wqe_offset = offsetof(r2n::r2n_buf_t, write_desc) - offsetof(r2n::r2n_buf_t, cmd_buf);
  dp_mem_t *write_wqe = r2n_buf_va->fragment_find(write_wqe_offset, 64);

  // Start the write WQE formation (WRID will be filled by P4+)
  write_wqe->write_bit_fields(68, 4, 5);  // op_type = OP_TYPE_WRITE_IMM
  write_wqe->write_bit_fields(72, 8, 1);  // Num SGEs = 1
  write_wqe->write_bit_fields(224, 32, (uint32_t) kR2NDataSize);  // data len

  // RDMA will ring the next doorbell with pndx increment,
  // print out this info to make it easy to locate in model.log
  queues::get_capri_doorbell_with_pndx_inc(queues::get_seq_lif(), SQ_TYPE, seq_pdma_q, 0,
                                           &db_addr, &db_data);
  printf("write_wqe next doorbell db_addr %lx db_data %lx\n", db_addr, db_data);

  // Write WQE: remote side buffer with immediate data as doorbell
  write_wqe->write_bit_fields(96, 11, queues::get_seq_lif());
  write_wqe->write_bit_fields(107, 3, SQ_TYPE);
  write_wqe->write_bit_fields(110, 18, seq_pdma_q);
  write_wqe->write_bit_fields(128, 64, r2n_hbm_buf_pa->va()); // va == pa
  write_wqe->write_bit_fields(128+64, 32, WriteBackBufRKey << 8); // rkey
  write_wqe->write_bit_fields(128+64+32, 32, (uint32_t) kR2NDataSize); // len
  write_wqe->write_thru();

  initiator_sq_va->line_advance();

  // Now kickstart the sequencer
  tests::test_seq_read_roce(seq_pdma_q, seq_roce_q, g_rdma_pvm_roce_init_sq,
                            r2n_hbm_buf_pa->pa(), rd_buf->pa(),
                            kR2NDataSize, pdma_dst_lif_override, pdma_dst_lif,
                            sqwqe->pa(), kSQWQESize);

  return 0;
}

int StartRoceReadWithNextLifQueue(uint16_t seq_roce_q,
                                  uint16_t seq_roce_index,
                                  dp_mem_t *seq_roce_desc,
                                  dp_mem_t *r2n_send_buf,
                                  dp_mem_t *r2n_write_buf,
                                  uint32_t data_len,
                                  uint32_t next_lif,
                                  uint32_t next_qtype,
                                  uint32_t next_qid)
{
  uint64_t db_addr, db_data;

  assert(data_len == kR2NDataSize);

  // Increment the LKey at the beginning of each API
  SendBufLKeyIncr();
  WriteBackBufKeysIncr();

  // Register R2N buf memory. Only LKey, no remote access.
  RdmaMemRegister(r2n_send_buf->va(), r2n_send_buf->pa(), kR2NBufSize,
                  SendBufLKey << 8, 0, false, r2n_send_buf->is_mem_type_host_mem());

  // Register the buffer for the write back data for the read command with RDMA
  uint32_t r2n_data_len = sizeof(r2n::r2n_buf_t) - offsetof(r2n::r2n_buf_t, cmd_buf);
  RdmaMemRegister(r2n_write_buf->va(), r2n_write_buf->pa(), kR2NBufSize,
                  WriteBackBufLKey << 8, WriteBackBufRKey << 8, true,
                  r2n_write_buf->is_mem_type_host_mem());

  // For the RDMA send WQE
  dp_mem_t *sqwqe = new dp_mem_t(1, kSQWQESize);

  /*
   * Point sqwqe to the nvme command in r2n_buf
   */
  sqwqe->write_bit_fields(0, 64, r2n_send_buf->pa());  // wrid
  sqwqe->write_bit_fields(68, 4, kRdmaSendOpType);
  sqwqe->write_bit_fields(72, 8, 1);  // Num SGEs = 1

  // Store doorbell information of PVM's ROCE CQ in immediate data
  if (kRdmaSendOpType == RDMA_OP_TYPE_SEND_IMM) {
      sqwqe->write_bit_fields(96, 11, queues::get_pvm_lif());
      sqwqe->write_bit_fields(107, 3, CQ_TYPE);
      sqwqe->write_bit_fields(110, 18, g_rdma_pvm_roce_tgt_cq);
  }
  sqwqe->write_bit_fields(224, 32, r2n_data_len);

  // Local read command buffer goes into SGE
  sqwqe->write_bit_fields(256, 64, r2n_send_buf->va());  // SGE-va
  sqwqe->write_bit_fields(256+64, 32, r2n_data_len);
  sqwqe->write_bit_fields(256+64+32, 32, SendBufLKey << 8);
  sqwqe->write_thru();


  // Pre-form the (RDMA) write descriptor to point to the data buffer
  uint32_t write_wqe_offset = offsetof(r2n::r2n_buf_t, write_desc) -
                              offsetof(r2n::r2n_buf_t, cmd_buf);
  dp_mem_t *write_wqe = r2n_send_buf->fragment_find(write_wqe_offset, kSQWQESize);
  dp_mem_t *write_data_buf = target_rcv_buf_va->fragment_find(kR2NDataBufOffset,
                                                              data_len);

  // Start the write WQE formation (WRID will be filled by P4+)
  write_wqe->write_bit_fields(68, 4, 5);  // op_type = OP_TYPE_WRITE_IMM
  write_wqe->write_bit_fields(72, 8, 1);  // Num SGEs = 1
  write_wqe->write_bit_fields(224, 32, data_len);  // data len

  // RDMA will ring the next doorbell with pndx increment,
  // print out this info to make it easy to locate in model.log
  queues::get_capri_doorbell_with_pndx_inc(next_lif, next_qtype, next_qid, 0,
                                           &db_addr, &db_data);
  printf("write_wqe next doorbell db_addr %lx db_data %lx\n", db_addr, db_data);

  // Write WQE: remote side buffer with immediate data as doorbell
  write_wqe->write_bit_fields(96, 11, next_lif);
  write_wqe->write_bit_fields(107, 3, next_qtype);
  write_wqe->write_bit_fields(110, 18, next_qid);
  write_wqe->write_bit_fields(128, 64, r2n_write_buf->va()); // va == pa
  write_wqe->write_bit_fields(128+64, 32, WriteBackBufRKey << 8); // rkey
  write_wqe->write_bit_fields(128+64+32, 32, data_len);

  // Write SGE: local side buffer
  // TODO: Remove the write_data_buf pointer and do this in P4+ in production code
  //       The reason it can't be done in DOL environment is because the P4+ code
  //       does not know the VA of the host. In production code, this buffer will be
  //       setup with the VA:PA identity mapping of the HBM buffer.
  write_wqe->write_bit_fields(256, 64, write_data_buf->va()); // SGE-va
  write_wqe->write_bit_fields(256+64, 32, data_len); // SGE-len
  write_wqe->write_bit_fields(256+64+32, 32, kTargetRcvBuf1LKey << 8); // SGE-lkey
  write_wqe->write_thru();

  initiator_sq_va->line_advance();

  // Now kickstart the sequencer
  tests::test_seq_roce_op_pdma_prefilled(seq_roce_q, seq_roce_index, seq_roce_desc,
                                         g_rdma_pvm_roce_init_sq, sqwqe);
  return 0;
}

void rdma_queues_init() {
  AllocRdmaMem();
  CreateInitiatorCQ();
  CreateInitiatorQP();
}

void rdma_queues_init2() {
  CreateTargetCQ();
  CreateTargetQP();
  ConnectInitiatorAndTarget(0, 1, kMACAddr1, kMACAddr2, kIPAddr1, kIPAddr2);
  ConnectInitiatorAndTarget(1, 0, kMACAddr2, kMACAddr1, kIPAddr2, kIPAddr1);
}

int set_rtl_qstate_cmp_ignore(int src_lif, int src_qtype, int src_qid) {
  uint64_t qaddr;
  if (qstate_if::get_qstate_addr(src_lif, src_qtype, src_qid, &qaddr) < 0) {
    printf("Failed to get q state address \n");
    return -1;
  }

  // Storage/RDMA P4+ running operating in model is known to be able to
  // consolidate certain processing, resulting in fewer RDMA acks than RTL.
  // This would lead to EOS miscompares on RDMA qstate. The call below
  // sets ignore range for CB up to 4KB in length.
  printf("eos_ignore_addr RDMA qstate 0x%lx size %u\n", qaddr, kRDMAQStateSize);
  EOS_IGNORE_ADDR(qaddr, kRDMAQStateSize);
	return 0;
}

int rdma_roce_ini_sq_info(uint16_t *lif, uint8_t *qtype, uint32_t *qid, uint64_t *qaddr) {
  if (!lif || !qtype || !qid || !qaddr) return -1;
  *lif = g_rdma_hw_lif_id;
  *qtype = kSQType;
  *qid = 0; // 0 - initiator; 1 - target

  return qstate_if::get_qstate_addr((int) *lif, (int) *qtype, (int) *qid, qaddr);
}

int rdma_roce_tgt_sq_info(uint16_t *lif, uint8_t *qtype, uint32_t *qid, uint64_t *qaddr) {
  if (!lif || !qtype || !qid || !qaddr) return -1;
  *lif = g_rdma_hw_lif_id;
  *qtype = kSQType;
  *qid = 1; // 0 - initiator; 1 - target

  return qstate_if::get_qstate_addr((int) *lif, (int) *qtype, (int) *qid, qaddr);
}

int rdma_roce_ini_rq_info(uint16_t *lif, uint8_t *qtype, uint32_t *qid, uint64_t *qaddr, uint64_t *base_pa) {
  if (!lif || !qtype || !qid || !qaddr || !base_pa) return -1;
  *lif = g_rdma_hw_lif_id;
  *qtype = kRQType;
  *qid = 0; // 0 - initiator; 1 - target
  *base_pa = initiator_rq_va_base;

  return qstate_if::get_qstate_addr((int) *lif, (int) *qtype, (int) *qid, qaddr);
}

int rdma_roce_tgt_rq_info(uint16_t *lif, uint8_t *qtype, uint32_t *qid, uint64_t *qaddr, uint64_t *base_pa) {
  if (!lif || !qtype || !qid || !qaddr || !base_pa) return -1;
  *lif = g_rdma_hw_lif_id;
  *qtype = kRQType;
  *qid = 1; // 0 - initiator; 1 - target
  *base_pa = target_rq_va_base;

  return qstate_if::get_qstate_addr((int) *lif, (int) *qtype, (int) *qid, qaddr);
}

uint32_t get_rdma_pvm_roce_init_sq() {
  return g_rdma_pvm_roce_init_sq;
}

uint32_t get_rdma_pvm_roce_init_cq() {
  return g_rdma_pvm_roce_init_cq;
}

uint32_t get_rdma_pvm_roce_tgt_sq() {
  return g_rdma_pvm_roce_tgt_sq;
}

uint32_t get_rdma_pvm_roce_tgt_cq() {
  return g_rdma_pvm_roce_tgt_cq;
}

int rdma_pvm_qs_init() {
  int rc;
  // Init the initiator SQ
  if ((rc = queues::pvm_roce_sq_init(g_rdma_hw_lif_id,
                                     kSQType, 0, // 0 - initiator; 1 - target
                                     g_rdma_hw_lif_id,
                                     kRQType, 0, // 0 - initiator; 1 - target
                                     initiator_sq_va,
                                     kRoceNumEntries, kRoceEntrySize,
                                     initiator_rq_va->pa(),
                                     0 // don't post the buffer
                                     )) < 0) {
    printf("RDMA Initiator ROCE SQ init failure\n");
    return -1;
  } else {
    g_rdma_pvm_roce_init_sq = (uint32_t) rc;
  }
  printf("RDMA Initiator ROCE SQ init success\n");

  // Init the initiator CQ only if NVME is running in the datapath
  if  (nvme_dp_init) {
    // Init the initiator CQ
    if ((rc = queues::pvm_roce_cq_init(g_rdma_hw_lif_id,
                                       kCQType, 0, // 0 - initiator; 1 - target
                                       initiator_cq_va,
                                       kRoceNumEntries, kRoceCQEntrySize,
                                       pvm_roce_init_sq_xlate_addr)) < 0) {
      printf("RDMA Initiator ROCE CQ init failure\n");
      return -1;
    } else {
      g_rdma_pvm_roce_init_cq = (uint32_t) rc;
    }
    printf("RDMA Initiator ROCE CQ init success\n");

    // Initiator SQ Xlate
    qstate_if::update_xlate_entry(queues::get_pvm_lif(), SQ_TYPE,
                                  g_rdma_pvm_roce_init_sq,
                                  pvm_roce_init_sq_xlate_addr + (1 * 64), NULL);

    // Initiator R2N Xlate
    qstate_if::update_xlate_entry(queues::get_pvm_lif(), SQ_TYPE,
                                  queues::get_pvm_r2n_init_sq(0),  // Only one R2N SQ
                                  pvm_roce_init_sq_xlate_addr, NULL);
  }

  // Init the target SQ
  if ((rc = queues::pvm_roce_sq_init(g_rdma_hw_lif_id,
                                     kSQType, 1, // 0 - initiator; 1 - target
                                     g_rdma_hw_lif_id,
                                     kRQType, 1, // 0 - initiator; 1 - target
                                     target_sq_va,
                                     kRoceNumEntries, kRoceEntrySize,
                                     target_rq_va->pa(),
                                     1 // post the buffer
                                     )) < 0) {
    printf("RDMA PVM Target ROCE SQ init failure\n");
    return -1;
  } else {
    g_rdma_pvm_roce_tgt_sq = (uint32_t) rc;
  }
  printf("RDMA PVM Target ROCE SQ init success %p\n", target_sq_va);

  // Init the target CQ
  if ((rc = queues::pvm_roce_cq_init(g_rdma_hw_lif_id,
                                     kCQType, 1, // 0 - initiator; 1 - target
                                     target_cq_va,
                                     kRoceNumEntries, kRoceCQEntrySize,
                                     pvm_roce_tgt_sq_xlate_addr)) < 0) {
    printf("RDMA PVM Target ROCE CQ init failure\n");
    return -1;
  } else {
    g_rdma_pvm_roce_tgt_cq = (uint32_t) rc;
  }
  printf("RDMA PVM Target ROCE CQ init success\n");

	rc = set_rtl_qstate_cmp_ignore(g_rdma_hw_lif_id, kSQType, 0);
	if(rc < 0) return rc;
	rc = set_rtl_qstate_cmp_ignore(g_rdma_hw_lif_id, kSQType, 1);
	if(rc < 0) return rc;
	rc = set_rtl_qstate_cmp_ignore(g_rdma_hw_lif_id, kCQType, 0);
	if(rc < 0) return rc;
	rc = set_rtl_qstate_cmp_ignore(g_rdma_hw_lif_id, kCQType, 1);
	if(rc < 0) return rc;
	rc = set_rtl_qstate_cmp_ignore(g_rdma_hw_lif_id, kRQType, 0);
	if(rc < 0) return rc;
	rc = set_rtl_qstate_cmp_ignore(g_rdma_hw_lif_id, kRQType, 1);
	if(rc < 0) return rc;

  // Target SQ Xlate
  qstate_if::update_xlate_entry(queues::get_pvm_lif(), SQ_TYPE,
                                g_rdma_pvm_roce_tgt_sq,
                                pvm_roce_tgt_sq_xlate_addr + (1 * 64), NULL);

  // Target R2N Xlate
  qstate_if::update_xlate_entry(queues::get_pvm_lif(), SQ_TYPE,
                                queues::get_pvm_r2n_tgt_sq(1), // 1 for P4+ to update wp_ndx
                                pvm_roce_tgt_sq_xlate_addr, NULL);
  return 0;
}


int rdma_init(bool dp_init) {
  uint8_t ent[64];

  // nicmgr would have created VRF, l2segments, endpoints, etc. For simplicity,
  // we will not run RDMA related sanity when nicmgr is present.
  if (run_nicmgr_tests) {
      printf("RDMA tests will not be run with nicmgr\n");
      return 0;
  }

  // Flag to indicate whether there is an NVME datapath initiator
  // => cq needs to be in capri, sq xlate needs to be setup
  nvme_dp_init = dp_init;

  // Allocate the PVM ROCE XLATE table in HBM for the sequencer
  if (utils::hbm_addr_alloc(kPvmRoceSqXlateTblSize, &pvm_roce_tgt_sq_xlate_addr) < 0) {
    printf("Can't allocate Target Xlate table\n");
    return -1;
  }

  // Allocate the NVME ROCE XLATE table in HBM for the sequencer only if NVME datapath
  // is enabled
  if  (nvme_dp_init) {
    if (utils::hbm_addr_alloc(kPvmRoceSqXlateTblSize, &pvm_roce_init_sq_xlate_addr) < 0) {
      printf("Can't allocate Initiator Xlate table \n");
      return -1;
    }
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
  rdma_queues_init2();

  printf("RDMA PVM queues Init success\n");
  assert(PullCQEntry(initiator_cq_va, &initiator_cq_cindex, 64, 1, &ent) == false);
  assert(PullCQEntry(target_cq_va, &target_cq_cindex, 64, 1, &ent) == false);
  RegisterTargetRcvBufs();
  printf("Registered target recv buf\n");
  RegisterInitiatorRcvBufs();
  printf("Registered initiator recv buf\n");
  return 0;
}

void rdma_shutdown()
{
  // Some Capri timers are enabled by the RDMA transport stack. These timers
  // are not stopped even at the end of simulation and can cause spurious
  // EOS timers error messages.
  rdma_tmr_global_disable();
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
  utils::dump(target_rcv_buf_va->read_thru());
  if (PullCQEntry(initiator_cq_va, &initiator_cq_cindex, 64, 1, &ent)) {
    initiator_cq_cindex++;
    printf("Got CQ entry at the initiator\n");
    utils::dump(ent);
  }
}

dp_mem_t *rdma_get_initiator_rcv_buf() {
  uint32_t size = kR2NBufSize - IO_STATUS_BUF_BE_STATUS_OFFSET;
  return initiator_rcv_buf_va->fragment_find(IO_STATUS_BUF_BE_STATUS_OFFSET,
                                             size);
}

dp_mem_t *rdma_get_target_write_data_buf() {
  return target_rcv_buf_va->fragment_find(kR2NDataSize, kR2NDataSize);
}

// Global disable of Capri fast and slow timers
void rdma_tmr_global_disable(void)
{
#ifdef ELBA
    uint64_t txs_cfw_tmr_global = ELB_ADDR_BASE_TXS_TXS_OFFSET +
                                  ELB_TXS_CSR_CFW_TIMER_GLB_BYTE_ADDRESS;
    uint32_t global_val = tests::test_csr_32bit_get(txs_cfw_tmr_global);

    global_val = ELB_TXS_CSR_CFW_TIMER_GLB_STMR_ENABLE_MODIFY(global_val, 0);
    global_val = ELB_TXS_CSR_CFW_TIMER_GLB_FTMR_ENABLE_MODIFY(global_val, 0);
#else
    uint64_t txs_cfw_tmr_global = CAP_ADDR_BASE_TXS_TXS_OFFSET +
                                  CAP_TXS_CSR_CFW_TIMER_GLB_BYTE_ADDRESS;
    uint32_t global_val = tests::test_csr_32bit_get(txs_cfw_tmr_global);

    global_val = CAP_TXS_CSR_CFW_TIMER_GLB_STMR_ENABLE_MODIFY(global_val, 0);
    global_val = CAP_TXS_CSR_CFW_TIMER_GLB_FTMR_ENABLE_MODIFY(global_val, 0);
#endif
    tests::test_csr_32bit_set(txs_cfw_tmr_global, global_val);
    printf("rdma_tmr_global_disable addr 0x%lx data 0x%x\n", txs_cfw_tmr_global, global_val);
}
