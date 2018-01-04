
#include <iostream>
#include <cmath>

#include <grpc++/grpc++.h>
#include <zmq.h>

#include "nic/gen/proto/hal/interface.grpc.pb.h"

#include "nic/model_sim/include/buf_hdr.h"
#include "./lib_driver.hpp"
#include "nic/model_sim/include/lib_model_client.h"
#include "nic/utils/host_mem/host_mem.hpp"


using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using namespace grpc;
using namespace intf;
using namespace std;

utils::HostMem *g_host_mem = utils::HostMem::New();

#include <map>

typedef struct {
    uint64_t lif;
    uint32_t qtype;
    uint32_t qid;
} queue_id_t;

typedef struct {
    uint64_t qstate_addr;
    struct qstate *qstate;
    uint64_t queue_addr;
    void *queue;

    uint64_t cq_qstate_addr;
    struct qstate *cq_qstate;
    uint64_t cq_queue_addr;
    void *cq_queue;
} queue_info_t;

map<tuple<uint64_t, uint32_t, uint32_t>, queue_info_t> queue_map;


uint64_t
get_qstate_addr(uint64_t lif, uint32_t qtype, uint32_t qid) {
  char *grpc_port_env;
  std::string grpc_ep = "localhost:";
  if ((grpc_port_env = getenv("HAL_GRPC_PORT")) != NULL) {
      grpc_ep.append(grpc_port_env);
  } else {
      grpc_ep.append("50054");
  }
  shared_ptr<Channel> channel = CreateChannel(grpc_ep, InsecureChannelCredentials());
  StubOptions options;

  unique_ptr<Interface::Stub> svc = Interface::NewStub(channel, options);

  //cout << "Lif = " << lif << " Qtype = " << qtype << " Qid = " << qid << endl;

  ClientContext context;
  GetQStateRequestMsg request;
  GetQStateResponseMsg response;

  QStateGetReq *qstate_request = request.add_reqs();
  qstate_request->set_lif_handle(lif);
  qstate_request->set_type_num(qtype);
  qstate_request->set_qid(qid);

  Status status = svc->LifGetQState(&context, request, &response);
  if (status.ok()) {
    for (int i = 0; i < response.resps().size(); i++) {
      QStateGetResp qstate_response = response.resps(i);
      if (qstate_response.error_code()) {
        cout << "Error Code = " << qstate_response.error_code() << endl;
        return (0);
      } else {
        return qstate_response.q_addr();
      }
    }
  } else {
    cout << status.error_code() << ": " << status.error_message() << endl;
    return (0);
  }

  return (0);
}

pair<uint32_t, uint64_t>
make_doorbell(int upd, int lif, int type, int pid, int qid, int ring, int p_index) {
  uint32_t addr;
  addr = 0x400000ul + (upd << 17) + (lif << 6) + (type << 3); // HOST DOORBELL ADDRESS
  cout << "upd = " << hex << "0x" << upd << ", lif = " << hex << "0x" << lif << ", type = " << hex << "0x" << type
       << endl;
  cout << "addr = " << hex << "0x" << addr << endl;

  uint64_t val = 0;
  val = p_index;
  val = val | (ring << 16);
  val = val | (0 << 19);
  val = val | ((uint64_t) (qid) << 24);
  val = val | ((uint64_t) (pid) << 48);

  cout << "pid = " << hex << "0x" << pid << ", qid = " << hex << "0x" << qid << ", ring = " << hex << "0x" << ring
       << ", p_index = " << hex << "0x" << p_index << endl;
  cout << "data = " << hex << "0x" << val << endl;

  return pair<uint32_t, uint64_t>(addr, val);
}

queue_info_t
get_queue_info(uint64_t lif, queue_type qtype, uint32_t qid) {
  return queue_map[make_tuple(lif, qtype, qid)];
}

void
set_queue_info(uint64_t lif, queue_type qtype, uint32_t qid, queue_info_t queue_info) {
  queue_map[make_tuple(lif, qtype, qid)] = queue_info;
}

void
read_queue(uint64_t lif, queue_type qtype, uint32_t qid) {
  queue_info_t qi = get_queue_info(lif, qtype, qid);

  read_mem(qi.qstate_addr, (uint8_t *) qi.qstate, sizeof(struct qstate));
}

void
write_queue(uint64_t lif, queue_type qtype, uint32_t qid) {
  queue_info_t qi = get_queue_info(lif, qtype, qid);

  write_mem(qi.qstate_addr, (uint8_t *) qi.qstate, sizeof(struct qstate));
}

void
alloc_queue(uint64_t lif, queue_type qtype, uint32_t qid, uint16_t size) {
  queue_info_t qi;

  struct tx_desc *txq;
  struct rx_desc *rxq;

  if (g_host_mem == NULL) {
    g_host_mem = utils::HostMem::New();
  }
  switch (qtype) {
  case TX:
    // Create TX descriptor ring in Host Memory
    qi.queue = g_host_mem->Alloc(size * sizeof(struct tx_desc));
    if (qi.queue == NULL) {
      assert(0);
    }
    qi.queue_addr = g_host_mem->VirtToPhys(qi.queue);

    // Create TX completion descriptor ring
    qi.cq_queue = g_host_mem->Alloc(size * 16);
    if (qi.cq_queue == NULL) {
      assert(0);
    }
    qi.cq_queue_addr = g_host_mem->VirtToPhys(qi.cq_queue);

    txq = (struct tx_desc *) qi.queue;
    memset(txq, 0, sizeof(txq[0]) * size);
    break;

  case RX:
    // Create RX descriptor ring in Host Memory
    qi.queue = g_host_mem->Alloc(size * sizeof(struct rx_desc));
    if (qi.queue == NULL) {
      assert(0);
    }
    qi.queue_addr = g_host_mem->VirtToPhys(qi.queue);

    // Create RX completion descriptor ring
    qi.cq_queue = g_host_mem->Alloc(size * 16);
    if (qi.cq_queue == NULL) {
      assert(0);
    }
    qi.cq_queue_addr = g_host_mem->VirtToPhys(qi.cq_queue);
 
    rxq = (struct rx_desc *) qi.queue;
    memset(rxq, 0, sizeof(rxq[0]) * size);
    break;

  default:
    assert(0);
  }

  qi.qstate_addr = get_qstate_addr(lif, qtype, qid);
  if (qi.qstate_addr == 0) {
    assert(0);
  }
  qi.qstate = (struct qstate *) calloc(1, sizeof(struct qstate));
  set_queue_info(lif, qtype, qid, qi);

  // Read-Modify-Write the qstate structure
  read_queue(lif, qtype, qid);
  qi.qstate->p_index0 = 0;
  qi.qstate->c_index0 = 0;
  qi.qstate->p_index1 = 0;
  qi.qstate->c_index1 = 0;
  qi.qstate->enable = 1;
  qi.qstate->ring_base = g_host_mem->VirtToPhys(qi.queue);
  qi.qstate->ring_size = (uint16_t)log2(size);
  qi.qstate->cq_ring_base = g_host_mem->VirtToPhys(qi.cq_queue);
  write_queue(lif, qtype, qid);

  printf("QSTATE: lif = %lu qtype = %d qid = %d addr = %lx ring_base %lx cq_ring_base %lx\n",
         lif, qtype, qid, qi.qstate_addr, qi.qstate->ring_base, qi.qstate->cq_ring_base);
}

bool
poll_queue(uint64_t lif, queue_type qtype, uint32_t qid, uint32_t max_count, uint16_t *prev_cindex) {
  queue_info_t qi = get_queue_info(lif, qtype, qid);

  
  uint32_t count = 1;

  if (*prev_cindex == 0xFFFF) {
    *prev_cindex = qi.qstate->c_index0;
  }


  do {
    printf("Polling QSTATE[prev_cindex %d] ... %u tries\n", *prev_cindex, count++);
    read_queue(lif, qtype, qid);
    sleep(1);
    if (count >= max_count) 
      return false;
  } while (*prev_cindex == qi.qstate->c_index0);
  return true;
}

void
print_queue(uint64_t lif, queue_type qtype, uint32_t qid) {
  queue_info_t qi = get_queue_info(lif, qtype, qid);
  struct qstate *qstate = qi.qstate;
  struct rx_desc *rxq;
  struct tx_desc *txq;

  printf("pc_offset=0x%0x\n"
         "rsvd0=0x%0x\n"
         "cosA=0x%0x\ncosB=0x%0x\ncos_sel=0x%0x\n"
         "eval_last=0x%0x\n"
         "host=0x%0x\ntotal=0x%0x\n"
         "pid=0x%0x\n"
         "enable=0x%0x\nring_base=0x%0lx\nring_size=0x%0x\n"
         "p_index=0x%0x\nc_index=0x%0x\n"
         "cq_ring_base=0x%0lx\n",
         qstate->pc_offset,
         qstate->rsvd0,
         qstate->cosA, qstate->cosB, qstate->cos_sel,
         qstate->eval_last,
         qstate->host, qstate->total,
         qstate->pid,
         qstate->enable, qstate->ring_base, qstate->ring_size,
         qstate->p_index0, qstate->c_index0,
         qstate->cq_ring_base);

  switch (qtype) {
  case RX:
    rxq = (struct rx_desc *) qi.queue;
    for (int i = 0; i < qi.qstate->ring_size; i++) {
      printf("addr=0x%0lx len=0x%0x\n", rxq[i].addr, rxq[i].len);
    }
    break;
  case TX:
    txq = (struct tx_desc *) qi.queue;
    for (int i = 0; i < qi.qstate->ring_size; i++) {
      printf("addr=0x%0lx len=0x%x", txq[i].addr, txq[i].len);
    }
    break;
  default:
    break;
  }
}

uint8_t *
alloc_buffer(uint16_t size) {
  return (uint8_t *) g_host_mem->Alloc(size);
}

void
post_buffer(uint64_t lif, queue_type qtype, uint32_t qid, void *buf, uint16_t size) {
  queue_info_t qi = get_queue_info(lif, qtype, qid);

  struct tx_desc *txq;
  struct rx_desc *rxq;
  int upd;
  std::pair<uint32_t, uint64_t> db;

  switch (qtype) {
  case TX:
    txq = (struct tx_desc *) qi.queue;
    txq[qi.qstate->p_index0].addr = g_host_mem->VirtToPhys(buf);
    txq[qi.qstate->p_index0].len = size;
    printf("POST TXQ[%p:%lx:%d] addr 0x%lx len 0x%x\n", 
           txq,
           g_host_mem->VirtToPhys(txq),
           qi.qstate->p_index0,
           txq[qi.qstate->p_index0].addr,
           txq[qi.qstate->p_index0].len); 
    upd = 0xb;
    break;
  case RX:
    rxq = (struct rx_desc *) qi.queue;
    rxq[qi.qstate->p_index0].addr = g_host_mem->VirtToPhys(buf);
    rxq[qi.qstate->p_index0].len = size;
    printf("POST RXQ[%p:%lx:%d] addr 0x%lx len 0x%x\n", 
           rxq,
           g_host_mem->VirtToPhys(rxq),
           qi.qstate->p_index0,
           rxq[qi.qstate->p_index0].addr,
           rxq[qi.qstate->p_index0].len);
   upd = 0x8;
   break;
  default:
    break;
  }

  qi.qstate->p_index0++;
  qi.qstate->p_index0 &= ((uint16_t) pow(2, qi.qstate->ring_size) - 1);

  printf("\nDOORBELL\n");
  db = make_doorbell(upd, lif, qtype,
                     0 /* pid */, qid, 0 /* ring */, qi.qstate->p_index0);
  step_doorbell(db.first, db.second);
}

void
consume_buffer(uint64_t lif, queue_type qtype, uint32_t qid, void *buf, uint16_t *size) {
  queue_info_t qi = get_queue_info(lif, qtype, qid);

  //struct tx_desc *txq;
  struct rx_desc *rxq;

  switch (qtype) {
  case TX:
    break;
  case RX:
    rxq = (struct rx_desc *) qi.queue;
    //assert(rxq[qi.qstate->c_index1].addr == g_host_mem->VirtToPhys(buf));
    *size = rxq[qi.qstate->c_index1].len;
    break;
  default:
    break;
  }

  qi.qstate->c_index1++;
  qi.qstate->c_index1 &= ((uint16_t) pow(2, qi.qstate->ring_size) - 1);
}
