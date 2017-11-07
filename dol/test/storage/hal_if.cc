#include <stdint.h>
#include <grpc++/grpc++.h>
#include <memory>
#include "nic/gen/proto/hal/internal.pb.h"
#include "nic/gen/proto/hal/internal.grpc.pb.h"
#include "nic/gen/proto/hal/interface.pb.h"
#include "nic/gen/proto/hal/interface.grpc.pb.h"
#include "nic/gen/proto/hal/crypto_keys.pb.h"
#include "nic/gen/proto/hal/crypto_keys.grpc.pb.h"
#include "dol/test/storage/hal_if.hpp"

extern uint64_t hal_port;

const static uint32_t  kDefaultQStateSize   = 64;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using internal::Internal;
using intf::Interface;
using cryptokey::CryptoKey;

std::shared_ptr<Channel> hal_channel;

namespace {

std::unique_ptr<Internal::Stub> internal_stub;
std::unique_ptr<Interface::Stub> interface_stub;
std::unique_ptr<CryptoKey::Stub> crypto_stub;

}  // anonymous namespace

namespace hal_if {

void init_hal_if() {
  if (internal_stub.get() != nullptr)
    return;

  char host_addr[32];
  sprintf(host_addr, "localhost:%lu", hal_port);
  hal_channel = std::move(grpc::CreateChannel(
      host_addr, grpc::InsecureChannelCredentials()));

  std::unique_ptr<Internal::Stub> int_stub(
      Internal::NewStub(hal_channel));
  internal_stub = std::move(int_stub);

  std::unique_ptr<Interface::Stub> if_stub(
      Interface::NewStub(hal_channel));
  interface_stub = std::move(if_stub);

  std::unique_ptr<CryptoKey::Stub> crypt_stub(
    CryptoKey::NewStub(grpc::CreateChannel(
      host_addr, grpc::InsecureChannelCredentials())));
  crypto_stub = std::move(crypt_stub);
}

// Start with a base lif id and count up
// TODO: Reconcile with other service lifs
static uint32_t curr_lif_id = 1900;

int create_lif(lif_params_t *params, uint64_t *lif_id) {
  grpc::ClientContext context;
  intf::LifRequestMsg req_msg;
  intf::LifResponseMsg resp_msg;

  if (!params || !lif_id)
    return -1;

  auto req = req_msg.add_request();
  if (params->sw_lif_id != 0) {
    req->mutable_key_or_handle()->set_lif_id(params->sw_lif_id);
  } else {
    req->mutable_key_or_handle()->set_lif_id(curr_lif_id);
    curr_lif_id++;
  }

  for (int i = 0; i < LIF_MAX_TYPES; i++) {
    if (params->type[i].valid) {
      if (params->type[i].queue_size < LIF_BASE_QUEUE_SIZE)
        return -1;
      auto map = req->add_lif_qstate_map();
      map->set_type_num(i);
      map->set_size(params->type[i].queue_size - LIF_BASE_QUEUE_SIZE);
      map->set_entries(params->type[i].num_queues);
      map->set_purpose(
          static_cast<::intf::LifQPurpose>(params->type[i].queue_purpose));
    }
  }
  if (params->rdma_enable) {
    req->set_enable_rdma(true);
    req->set_rdma_max_keys(params->rdma_max_keys);
    req->set_rdma_max_pt_entries(params->rdma_max_pt_entries);
  }

  auto status = interface_stub->LifCreate(&context, req_msg, &resp_msg);
  if (!status.ok()) {
    printf("LIF create failed: %s\n", status.error_message().c_str());
    return -1;
  }

  // TODO: Check number of responses ? 
  // TODO: Check status
  *lif_id = resp_msg.response(0).hw_lif_id();
  return 0;
}

int get_pgm_base_addr(const char *prog_name, uint64_t *base_addr) {
  grpc::ClientContext context;
  internal::GetProgramAddressRequestMsg req_msg;
  internal::ProgramAddressResponseMsg resp_msg;

  if (!prog_name || !base_addr)
    return -1;

  auto req = req_msg.add_reqs();
  req->set_handle("p4plus");
  req->set_prog_name(prog_name);
  req->set_resolve_label(false);

  auto status = internal_stub->GetProgramAddress(&context, req_msg, &resp_msg);
  if (!status.ok())
    return -1;

  // TODO: Check number of responses ? 
  *base_addr = resp_msg.resps(0).addr();
  return 0;
}

int get_pgm_label_offset(const char *prog_name, const char *label, uint8_t *off) {
  grpc::ClientContext context;
  internal::GetProgramAddressRequestMsg req_msg;
  internal::ProgramAddressResponseMsg resp_msg;

  if (!prog_name || !label || !off)
    return -1;

  auto req = req_msg.add_reqs();
  req->set_handle("p4plus");
  req->set_prog_name(prog_name);
  req->set_resolve_label(true);
  req->set_label(label);

  auto status = internal_stub->GetProgramAddress(&context, req_msg, &resp_msg);
  if (!status.ok())
    return -1;

  // TODO: Check number of responses ? 
  *off = ((resp_msg.resps(0).addr()) >> 6) & 0xFF;
  return 0;
}

int get_lif_qstate_addr(uint32_t lif, uint32_t qtype, uint32_t qid, uint64_t *qaddr) {
  grpc::ClientContext context;
  intf::GetQStateRequestMsg req_msg;
  intf::GetQStateResponseMsg resp_msg;

  if (!qaddr) 
    return -1;

  auto req = req_msg.add_reqs();
  req->set_lif_handle(lif);
  req->set_type_num(qtype);
  req->set_qid(qid);

  auto status = interface_stub->LifGetQState(&context, req_msg, &resp_msg);
  if (!status.ok())
    return -1;

  // TODO: Check number of responses ?
  if (resp_msg.resps(0).error_code())
    return -1;

  *qaddr = resp_msg.resps(0).q_addr();
  return 0;
}

int get_lif_qstate(uint32_t lif, uint32_t qtype, uint32_t qid, uint8_t *qstate) {
  grpc::ClientContext context;
  intf::GetQStateRequestMsg req_msg;
  intf::GetQStateResponseMsg resp_msg;

  if (!qstate) 
    return -1;

  auto req = req_msg.add_reqs();
  printf("getting q state for lif %u type %u qid %u \n", lif, qtype, qid);
  req->set_lif_handle(lif);
  req->set_type_num(qtype);
  req->set_qid(qid);
  req->set_ret_data_size(kDefaultQStateSize);

  auto status = interface_stub->LifGetQState(&context, req_msg, &resp_msg);
  if (!status.ok())
    return -1;

  // TODO: Check number of responses ?
  if (resp_msg.resps(0).error_code())
    return -1;

  memcpy(qstate, resp_msg.resps(0).queue_state().c_str(), kDefaultQStateSize);
  return 0;
}

int set_lif_qstate_size(uint32_t lif, uint32_t qtype, uint32_t qid, uint8_t *qstate, uint32_t size) {
  grpc::ClientContext context;
  intf::SetQStateRequestMsg req_msg;
  intf::SetQStateResponseMsg resp_msg;

  if (!qstate) {
    printf("q state null \n");
    return -1;
  }

  auto req = req_msg.add_reqs();
  req->set_lif_handle(lif);
  req->set_type_num(qtype);
  req->set_qid(qid);
  req->set_queue_state((const char *) qstate, size);

  auto status = interface_stub->LifSetQState(&context, req_msg, &resp_msg);
  if (!status.ok()) {
    printf("status %d \n", status.ok());
    return -1;
  }

  // TODO: Check number of responses ?
  if (resp_msg.resps(0).error_code()) {
    printf("resp error %d \n", resp_msg.resps(0).error_code());
    return -1;
  }

  return 0;
}

int set_lif_qstate(uint32_t lif, uint32_t qtype, uint32_t qid, uint8_t *qstate) {
  return set_lif_qstate_size(lif, qtype, qid, qstate, kDefaultQStateSize);
}

int alloc_hbm_address(uint64_t *addr, uint32_t *size) {
  grpc::ClientContext context;
  internal::AllocHbmAddressRequestMsg req_msg;
  internal::AllocHbmAddressResponseMsg resp_msg;

  if (!addr || !size) 
    return -1;

  auto req = req_msg.add_reqs();
  req->set_handle("storage");

  auto status = internal_stub->AllocHbmAddress(&context, req_msg, &resp_msg);
  if (!status.ok())
    return -1;

  // TODO: Check number of responses ? 
  *addr = resp_msg.resps(0).addr();
  *size = resp_msg.resps(0).size();
  return 0;
}

int get_xts_ring_base_address(uint64_t *addr) {
  grpc::ClientContext context;

  internal::AllocHbmAddressRequestMsg req_msg;
  internal::AllocHbmAddressResponseMsg resp_msg;

  if (!addr)
    return -1;

  auto req = req_msg.add_reqs();
  req->set_handle("brq-ring-xts");

  auto status = internal_stub->AllocHbmAddress(&context, req_msg, &resp_msg);
  if (!status.ok())
    return -1;

  // TODO: Check number of responses ?
  *addr = resp_msg.resps(0).addr();

  return 0;
}

int get_key_index(char* key, types::CryptoKeyType key_type, uint32_t key_size, uint32_t* key_index) {
  grpc::ClientContext context, context2;
  *key_index = 0xffffffff;
  cryptokey::CryptoKeyCreateRequestMsg cr_req_msg;
  cryptokey::CryptoKeyCreateResponseMsg cr_resp_msg;
  cr_req_msg.add_request();

  auto status = crypto_stub->CryptoKeyCreate(&context, cr_req_msg, &cr_resp_msg);
  if (!status.ok()) {
    printf("Create request failed \n");
    return -1;
  }
  if(cr_resp_msg.response(0).keyindex() == *key_index) {
    printf("Create request failed \n");
    return -1;
  }
  *key_index = cr_resp_msg.response(0).keyindex();

  cryptokey::CryptoKeyUpdateRequestMsg upd_req_msg;
  cryptokey::CryptoKeyUpdateResponseMsg upd_resp_msg;
  auto upd_req = upd_req_msg.add_request();
  cryptokey::CryptoKeySpec* spec =   upd_req->mutable_key();
  spec->set_key(key);
  spec->set_keyindex(*key_index);
  spec->set_key_size(key_size);
  spec->set_key_type(key_type);

  status = crypto_stub->CryptoKeyUpdate(&context2, upd_req_msg, &upd_resp_msg);
  if (!status.ok()) {
    printf("Update request failed \n");
    return -1;
  }
  if(upd_resp_msg.response(0).keyindex() != *key_index) {
    printf("Update request failed \n");
    return -1;
  }

  return 0;
}

int delete_key(uint32_t key_index) {
  grpc::ClientContext context;
  cryptokey::CryptoKeyDeleteRequestMsg del_req_msg;
  cryptokey::CryptoKeyDeleteResponseMsg del_resp_msg;
  auto req = del_req_msg.add_request();
  req->set_keyindex(key_index);
  auto status = crypto_stub->CryptoKeyDelete(&context, del_req_msg, &del_resp_msg);
  if (!status.ok()) {
    printf("Delete request failed \n");
    return -1;
  }
  if(del_resp_msg.response(0).keyindex() != key_index) {
    printf("Delete request failed with bad keyindex \n");
    return -1;
  }
  return 0;
}

}  // namespace hal_if
