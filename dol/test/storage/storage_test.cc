// TODO Move to dol/test when sanity has build infra for dol.

// Storage DOL Testcases.
// These are involked by the DOL framework.
#include <stdint.h>
#include <stdio.h>
#include <grpc++/grpc++.h>
#include <memory>
#include "internal.pb.h"
#include "internal.grpc.pb.h"
#include "interface.pb.h"
#include "interface.grpc.pb.h"
#include <../../utils/host_mem/host_mem.hpp>
#include <../../gen/storage_tx/include/storage_tx_p4plus_ingress.h>
#include <../model_sim/include/lib_model_client.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using internal::Internal;
using intf::Interface;

namespace {

std::unique_ptr<Internal::Stub> internal_stub;
std::unique_ptr<Interface::Stub> interface_stub;

}  // anonymous namespace

bool init_hal_if() {
  std::unique_ptr<Internal::Stub> int_stub(
      Internal::NewStub(grpc::CreateChannel(
      "localhost:52929", grpc::InsecureChannelCredentials())));
  internal_stub = std::move(int_stub);

  std::unique_ptr<Interface::Stub> if_stub(
      Interface::NewStub(grpc::CreateChannel(
      "localhost:52929", grpc::InsecureChannelCredentials())));
  interface_stub = std::move(if_stub);

  return true;
}

// TODO: Enhance this API to take the right type,#queues argument
int create_lif(uint64_t *lif_id) {
  grpc::ClientContext context;
  intf::LifRequestMsg req_msg;
  intf::LifResponseMsg resp_msg;
  auto req = req_msg.add_request();
  req->mutable_key_or_handle()->set_lif_id(0);
  auto map = req->add_lif_qstate_map();
  map->set_type_num(0);
  map->set_size(1);

  auto status = interface_stub->LifCreate(&context, req_msg, &resp_msg);
  if (!status.ok())
    return -1;

  // TODO: Check number of responses ? 
  // TODO: Check status
  *lif_id = resp_msg.response(0).hw_lif_id();
  return 0;
}

int get_pgm_base_addr(const char *prog_name, uint64_t *base_addr) {
  grpc::ClientContext context;
  internal::GetProgramAddressRequestMsg req_msg;
  internal::ProgramAddressResponseMsg resp_msg;
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
  auto req = req_msg.add_reqs();
  printf("getting q state for lif %u type %u qid %u \n", lif, qtype, qid);
  req->set_lif_handle(lif);
  req->set_type_num(qtype);
  req->set_qid(qid);

  auto status = interface_stub->LifGetQState(&context, req_msg, &resp_msg);
  if (!status.ok())
    return -1;

  // TODO: Check number of responses ?
  printf("Response %d \n", resp_msg.resps(0).error_code());
  if (resp_msg.resps(0).error_code())
    return -1;

  *qaddr = resp_msg.resps(0).q_addr();
  return 0;
}

int set_lif_qstate(uint32_t lif, uint32_t qtype, uint32_t qid, uint8_t *qstate) {
  grpc::ClientContext context;
  intf::SetQStateRequestMsg req_msg;
  intf::SetQStateResponseMsg resp_msg;
  auto req = req_msg.add_reqs();
  req->set_lif_handle(lif);
  req->set_type_num(qtype);
  req->set_qid(qid);
  req->set_queue_state((const char *) qstate, 64);

  auto status = interface_stub->LifSetQState(&context, req_msg, &resp_msg);
  if (!status.ok())
    return -1;

  // TODO: Check number of responses ?
  if (resp_msg.resps(0).error_code())
    return -1;

  return 0;
}

void __write_bit_(uint8_t *p, unsigned bit_off, bool val) {
  unsigned start_byte = bit_off >> 3;
  uint8_t mask = 1 << (7 - (bit_off & 7));
  if (val)
    p[start_byte] |= mask;
  else
    p[start_byte] &= ~mask;
}

void write_q_state_field(void *ptr, unsigned start_bit_offset,
                      unsigned size_in_bits, uint64_t value) {
  uint8_t *p = (uint8_t *)ptr;
  int bit_no;
  int off;

  for (off = 0, bit_no = (size_in_bits - 1); bit_no >= 0; bit_no--, off++) {
    __write_bit_(p, start_bit_offset + off, value & (1ull << bit_no));
  }
}

void dump(uint8_t *buf) {
  int i;

  for (i = 0; i < 64; i++) {
    printf("%2.2x ", buf[i]);
    if ((i & 7) == 7) {
      printf(" ");
    }
    if ((i & 0xf) == 0xf) {
      printf("\n");
    }
  }
}


int setup_q_state(int src_lif, int src_qtype, int src_qid, char *pgm_bin,
                  uint8_t total_rings, uint8_t host_rings, uint16_t num_entries,
                  uint64_t base_addr, uint64_t entry_size, uint16_t dst_lif, 
                  uint8_t dst_qtype, uint32_t dst_qid, uint16_t vf_id, uint16_t sq_id,
                  uint64_t ssd_bm_addr, uint16_t ssd_q_num, uint16_t ssd_q_size) {

  s2_tbl_q_state_push_d q_state;
  uint8_t pc_offset;
  uint64_t next_pc;
  uint64_t dst_qaddr = 0;

  bzero(&q_state, sizeof(q_state));

  if (!pgm_bin) {
    printf("Input error\n");
    return -1;
  }

  if (get_lif_qstate_addr(dst_lif, dst_qtype, dst_qid, &dst_qaddr) < 0) {
    printf("Failed to get lif_qstate addr \n");
    return -1;
  }

  if ((get_pgm_base_addr(pgm_bin, &next_pc)) < 0) {
    printf("Failed to get base addr of pgm_bin\n");
    return -1;
  }
  next_pc = next_pc >> 6;

  if ((get_pgm_label_offset("txdma_stage0.bin", "storage_tx_stage0", &pc_offset)) < 0) {
    printf("Failed to get pc offset for storage tx stage0\n");
    return -1;
  }

  write_q_state_field(&q_state, 0, 8, pc_offset);
  write_q_state_field(&q_state, 40, 4, total_rings);
  write_q_state_field(&q_state, 44, 4, host_rings);
  write_q_state_field(&q_state, 112, 16, num_entries);
  write_q_state_field(&q_state, 128, 64, base_addr);
  write_q_state_field(&q_state, 192, 16, entry_size);
  write_q_state_field(&q_state, 208, 28, next_pc);
  write_q_state_field(&q_state, 236, 34, dst_qaddr);
  write_q_state_field(&q_state, 270, 11, dst_lif);
  write_q_state_field(&q_state, 281, 3, dst_qtype);
  write_q_state_field(&q_state, 284, 24, dst_qaddr);
  write_q_state_field(&q_state, 340, 34, ssd_bm_addr);
  write_q_state_field(&q_state, 374, 16, ssd_q_num);
  write_q_state_field(&q_state, 390, 16, ssd_q_size);

  dump((uint8_t *) &q_state);

  if (set_lif_qstate(src_lif, src_qtype, src_qid, (uint8_t*) &q_state) < 0) {
    printf("Failed to set lif_qstate addr \n");
    return -1;
  }

  printf("Q state created\n");
  return 0;
}

int main(int argc, char *argv[]) {
  init_hal_if();
  printf("Initialized hal \n");


  // Allocate NVME and PVM LIF
  uint64_t nvme_lif = 0, pvm_lif = 0;
  if (create_lif(&nvme_lif) < 0) {
    printf("can't create nvme lif \n");
    return -1;
  }
  if (create_lif(&pvm_lif) < 0) {
    printf("can't create nvme lif \n");
    return -1;
  }
  
  // Allocate host memory.
  std::unique_ptr<utils::HostMem> hostmem(utils::HostMem::New());
  uint8_t *nvme_queue = (uint8_t *)hostmem->Alloc(64 * 64);
  uint8_t *pvm_queue = (uint8_t *)hostmem->Alloc(64 * 64);
  if ((nvme_queue == nullptr) || (pvm_queue == nullptr)) {
    printf("Unable to allocate host memory.\n");
    return -1;
  }
  printf("Host memory allocated\n");
  uint64_t nvme_paddr = hostmem->VirtToPhys(nvme_queue);
  uint64_t pvm_paddr = hostmem->VirtToPhys(pvm_queue);
  pvm_paddr = pvm_paddr;

  if (setup_q_state(nvme_lif, 0, 0, (char *) "storage_tx_nvme_sq_handler.bin", 1, 1, 6,
                    nvme_paddr, 6, pvm_lif, 0, 0, 0, 0, 0, 0, 0) < 0) {
    printf("Failed to setup NVME queue state \n");
    return -1;
  }
  printf("Setup NVME queue state \n");

  if (setup_q_state(pvm_lif, 0, 0, (char *) "storage_tx_pvm_cq_handler.bin", 1, 1, 6,
                    pvm_paddr, 6, nvme_lif, 0, 0, 0, 0, 0, 0, 0) < 0) {
    printf("Failed to setup NVME queue state \n");
    return -1;
  }
  printf("Setup PVM queue state \n");

  uint8_t *nvme_cmd = nvme_queue;
  uint8_t *pvm_cmd = pvm_queue;
  bzero(nvme_cmd, 64);
  bzero(pvm_cmd, 64);

  // TODO: Init NVME command as admin command
  for (int i = 0; i < 64; i++) {
     nvme_cmd[i] = 0xBB;
  }

  if (lib_model_connect() < 0) {
    printf("Cant connect to model \n");
    return -1;
  }

  printf("PRE doorbell NVME vs PVM cmd comparison %d \n",
         memcmp(nvme_cmd, pvm_cmd, 64));

  // TODO: Get the right constant for doorbell address
  uint64_t db_data = 1;
  uint64_t db_addr = 0x400000 |  0xB << 17 | nvme_lif << 6;

  step_doorbell(db_addr, db_data);

  printf("POST doorbell NVME vs PVM cmd comparison %d \n",
         memcmp(nvme_cmd, pvm_cmd, 64));
  return 0;
}

