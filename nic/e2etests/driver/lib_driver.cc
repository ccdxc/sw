#include <cmath>
#include <iostream>
#include <map>
#include <unistd.h>

#include <grpc++/grpc++.h>
#include <zmq.h>

#include "gen/proto/interface.grpc.pb.h"
#include "lib_driver.hpp"
#include "nic/include/adminq.h"
#include "nic/include/eth_common.h"
#include "nic/model_sim/include/buf_hdr.h"
#include "nic/model_sim/include/lib_model_client.h"
#include "nic/utils/host_mem/host_mem.hpp"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using namespace grpc;
using namespace intf;
using namespace std;


utils::HostMem *g_host_mem = NULL;
map<tuple<uint64_t, uint32_t, uint32_t>, queue_info_t> queue_map;

uint64_t get_qstate_addr(uint16_t lif, uint32_t qtype, uint32_t qid) {
  char *grpc_port_env;
  std::string grpc_ep = "localhost:";
  if ((grpc_port_env = getenv("HAL_GRPC_PORT")) != NULL) {
    grpc_ep.append(grpc_port_env);
  } else {
    grpc_ep.append("50054");
  }
  shared_ptr<Channel> channel =
      CreateChannel(grpc_ep, InsecureChannelCredentials());
  StubOptions options;

  unique_ptr<Interface::Stub> svc = Interface::NewStub(channel, options);

  // cout << "Lif = " << lif << " Qtype = " << qtype << " Qid = " << qid <<
  // endl;

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

void doorbell(uint8_t upd, uint16_t lif, queue_type type, uint32_t pid,
              uint32_t qid, uint8_t ring, uint16_t index) {
  uint64_t addr = 0, val = 0;

  printf("[%s] upd 0x%x lif %u qtype %u pid 0x%x qid %u ring %u index %u\n", __FUNCTION__,
        upd, lif, type, pid, qid, ring, index);

  addr = 0x400000ul + (upd << 17) + (lif << 6) + (type << 3);
  val = ((uint64_t)pid << 48) + ((uint64_t)qid << 24) + (0 << 19) +
        (ring << 16) + index;

  printf("[%s] addr 0x%lx val 0x%lx\n", __FUNCTION__, addr, val);
  step_doorbell(addr, val);
}

queue_info_t &get_queue_info(uint16_t lif, queue_type qtype, uint32_t qid) {
  return queue_map[make_tuple(lif, qtype, qid)];
}

void set_queue_info(uint16_t lif, queue_type qtype, uint32_t qid,
                    queue_info_t queue_info) {
  queue_map[make_tuple(lif, qtype, qid)] = queue_info;
}

void read_queue(uint16_t lif, queue_type qtype, uint32_t qid) {
  queue_info_t qi = get_queue_info(lif, qtype, qid);
  read_mem(qi.qstate_addr, (uint8_t *)qi.qstate, 64);
}

void write_queue(uint16_t lif, queue_type qtype, uint32_t qid) {
  queue_info_t qi = get_queue_info(lif, qtype, qid);
  write_mem(qi.qstate_addr, (uint8_t *)qi.qstate, 64);
}

static queue_info_t alloc_rxq(uint16_t lif, queue_type qtype, uint32_t qid,
                              uint16_t size) {
  queue_info_t qi;
  eth_rx_qstate_t *qstate;

  // Create RX descriptor ring in Host Memory
  qi.q = g_host_mem->Alloc(size * sizeof(struct rx_desc));
  assert(qi.q != NULL);
  memset(qi.q, 0, size * sizeof(struct rx_desc));
  qi.ring_base = g_host_mem->VirtToPhys(qi.q);
  qi.ring_size = size;
  qi.head = 0;
  qi.tail = 0;

  // Create RX completion descriptor ring
  qi.cq = g_host_mem->Alloc(size * sizeof(struct rx_cq_desc));
  assert(qi.cq != NULL);
  memset(qi.cq, 0, size * sizeof(struct rx_cq_desc));
  qi.cq_ring_base = g_host_mem->VirtToPhys(qi.cq);
  qi.cq_tail = 0;
  qi.cq_color = 1;

  // Init Qstate
  qi.qstate_addr = get_qstate_addr(lif, qtype, qid);
  assert(qi.qstate_addr != 0);

  qstate = (eth_rx_qstate_t *)calloc(1, sizeof(eth_rx_qstate_t));
  assert(qstate != NULL);
  qi.qstate = (void *)qstate;

  // Read-Modify-Write Qstate
  read_mem(qi.qstate_addr, (uint8_t *)qstate, sizeof(eth_rx_qstate_t));

  // ring counts
  qstate->host = 1;
  qstate->total = 1;
  // ring pointers
  qstate->p_index0 = 0;
  qstate->c_index0 = 0;
  qstate->comp_index = 0;
  // ring status
  qstate->sta.color = 1;
  // ring cfg
  qstate->cfg.enable = 1;
  qstate->cfg.host_queue = 1;
  qstate->ring_base = (1ULL << 63) | qi.ring_base;
  qstate->ring_size = (uint16_t)log2(qi.ring_size);
  qstate->cq_ring_base = (1ULL << 63) | qi.cq_ring_base;

  write_mem(qi.qstate_addr, (uint8_t *)qstate, sizeof(eth_rx_qstate_t));

  printf("[%s] lif%u/rx%u/q: qstate %lx"
         " ring_base pa 0x%lx va %p, cq_ring_base pa 0x%lx va %p\n",
         __FUNCTION__, lif, qid, qi.qstate_addr, qi.ring_base, qi.q,
         qi.cq_ring_base, qi.cq);

  return qi;
}

static queue_info_t alloc_txq(uint16_t lif, queue_type qtype, uint32_t qid,
                              uint16_t size) {
  queue_info_t qi;
  eth_tx_qstate_t *qstate;

  // Create TX descriptor ring in Host Memory
  qi.q = g_host_mem->Alloc(size * sizeof(struct tx_desc));
  assert(qi.q != NULL);
  memset(qi.q, 0, size * sizeof(struct tx_desc));
  qi.ring_base = g_host_mem->VirtToPhys(qi.q);
  qi.ring_size = size;
  qi.head = 0;
  qi.tail = 0;

  // Create TX completion descriptor ring
  qi.cq = g_host_mem->Alloc(size * sizeof(struct tx_cq_desc));
  assert(qi.cq != NULL);
  memset(qi.cq, 0, size * sizeof(struct tx_cq_desc));
  qi.cq_ring_base = g_host_mem->VirtToPhys(qi.cq);
  qi.cq_tail = 0;
  qi.cq_color = 1;

  // Init Qstate
  qi.qstate_addr = get_qstate_addr(lif, qtype, qid);
  assert(qi.qstate_addr != 0);

  qstate = (eth_tx_qstate_t *)calloc(1, sizeof(eth_tx_qstate_t));
  assert(qstate != NULL);
  qi.qstate = (void *)qstate;

  // Read-Modify-Write Qstate
  read_mem(qi.qstate_addr, (uint8_t *)qstate, sizeof(eth_tx_qstate_t));

  // ring counts
  qstate->host = 1;
  qstate->total = 1;
  // ring pointers
  qstate->p_index0 = 0;
  qstate->c_index0 = 0;
  qstate->comp_index = 0;
  qstate->ci_fetch = 0;
  qstate->ci_miss = 0;
  // ring status
  qstate->sta.color = 1;
  qstate->sta.spec_miss = 0;
  // ring cfg
  qstate->cfg.enable = 1;
  qstate->cfg.host_queue = 1;
  // ring params
  qstate->ring_base = (1ULL << 63) | qi.ring_base;
  qstate->ring_size = (uint16_t)log2(qi.ring_size);
  qstate->cq_ring_base = (1ULL << 63) | qi.cq_ring_base;

  write_mem(qi.qstate_addr, (uint8_t *)qstate, sizeof(eth_tx_qstate_t));

  printf("[%s] lif%u/tx%u/q: qstate %lx"
         " ring_base pa 0x%lx va %p cq_ring_base pa 0x%lx va %p\n",
         __FUNCTION__, lif, qid, qi.qstate_addr, qi.ring_base, qi.q,
         qi.cq_ring_base, qi.cq);

  return qi;
}

static queue_info_t alloc_adminq(uint16_t lif, queue_type qtype, uint32_t qid,
                                 uint16_t size) {
  queue_info_t qi;
  admin_qstate_t *qstate;

  // Create ADMIN descriptor ring in Host Memory
  qi.q = g_host_mem->Alloc(size * sizeof(struct admin_cmd_desc));
  assert(qi.q != NULL);
  memset(qi.q, 0, size * sizeof(struct admin_cmd_desc));
  qi.ring_base = g_host_mem->VirtToPhys(qi.q);
  qi.ring_size = size;
  qi.head = 0;
  qi.tail = 0;

  // Create ADMIN completion descriptor ring
  qi.cq = g_host_mem->Alloc(size * sizeof(struct admin_comp_desc));
  assert(qi.cq != NULL);
  memset(qi.cq, 0, size * sizeof(struct admin_comp_desc));
  qi.cq_ring_base = g_host_mem->VirtToPhys(qi.cq);
  qi.cq_tail = 0;
  qi.cq_color = 1;

  // Init Qstate
  qi.qstate_addr = get_qstate_addr(lif, qtype, qid);
  assert(qi.qstate_addr != 0);

  qstate = (admin_qstate_t *)calloc(1, sizeof(admin_qstate_t));
  assert(qstate != NULL);
  qi.qstate = (void *)qstate;

  // Read-Modify-Write Qstate
  read_mem(qi.qstate_addr, (uint8_t *)qstate, sizeof(admin_qstate_t));

  // ring counts
  qstate->host = 1;
  qstate->total = 1;
  // ring pointers
  qstate->p_index0 = 0;
  qstate->c_index0 = 0;
  qstate->comp_index = 0;
  qstate->ci_fetch = 0;
  // ring status
  qstate->sta.color = 1;
  // ring cfg
  qstate->cfg.enable = 1;
  qstate->cfg.host_queue = 1;
  qstate->cfg.intr_enable = 1;
  qstate->intr_assert_index = 0;
  // ring params
  qstate->ring_base = (1ULL << 63) | qi.ring_base;
  qstate->ring_size = (uint16_t)log2(qi.ring_size);
  qstate->cq_ring_base = (1ULL << 63) | qi.cq_ring_base;

  write_mem(qi.qstate_addr, (uint8_t *)qstate, sizeof(admin_qstate_t));

  printf("[%s] lif%u/admin%u/q: qstate %lx"
         " ring_base pa 0x%lx va %p cq_ring_base pa 0x%lx va %p\n",
         __FUNCTION__, lif, qid, qi.qstate_addr, qi.ring_base, qi.q,
         qi.cq_ring_base, qi.cq);

  return qi;
}

void alloc_queue(uint16_t lif, queue_type qtype, uint32_t qid, uint16_t size) {
  queue_info_t qi;

  if (g_host_mem == NULL) {
    g_host_mem = utils::HostMem::New();
    assert(g_host_mem != NULL);
  }

  switch (qtype) {
  case TX:
    qi = alloc_txq(lif, qtype, qid, size);
    break;

  case RX:
    qi = alloc_rxq(lif, qtype, qid, size);
    break;

  case ADMIN:
    qi = alloc_adminq(lif, qtype, qid, size);
    break;

  default:
    assert(0);
  }

  set_queue_info(lif, qtype, qid, qi);
}

bool poll_queue(uint16_t lif, queue_type qtype, uint32_t qid) {
  queue_info_t &qi = get_queue_info(lif, qtype, qid);

  struct tx_cq_desc *tx_cq, *tx_cq_desc;
  struct rx_cq_desc *rx_cq, *rx_cq_desc;
  struct admin_comp_desc *admin_cq, *admin_cq_desc;

  switch (qtype) {
  case TX:
    tx_cq = (struct tx_cq_desc *)qi.cq;
    tx_cq_desc = &tx_cq[qi.cq_tail];
#ifdef DEBUG
    printf("[tx_poll_queue] lif%u/tx%u/cq: desc pa %0lx va %p color %d cq_color %d cq_tail %u\n",
      lif, qid,
      g_host_mem->VirtToPhys(tx_cq_desc), tx_cq_desc,
      tx_cq_desc->color, qi.cq_color, qi.cq_tail);
#endif
    return (tx_cq_desc->color == qi.cq_color);
  case RX:
    rx_cq = (struct rx_cq_desc *)qi.cq;
    rx_cq_desc = &rx_cq[qi.cq_tail];
#ifdef DEBUG
    printf("[rx_poll_queue] lif%u/rx%u/cq: desc pa %0lx va %p color %d cq_color %d cq_tail %u\n",
      lif, qid,
      g_host_mem->VirtToPhys(rx_cq_desc), rx_cq_desc,
      rx_cq_desc->color, qi.cq_color, qi.cq_tail);
#endif
    return (rx_cq_desc->color == qi.cq_color);
  case ADMIN:
    admin_cq = (struct admin_comp_desc *)qi.cq;
    admin_cq_desc = &admin_cq[qi.cq_tail];
#ifdef DEBUG
    printf("[adminq_poll_queue] lif%u/admin%u/cq: desc pa %0lx va %p color %d cq_color %d cq_tail %u\n",
      lif, qid,
      g_host_mem->VirtToPhys(admin_cq_desc), admin_cq_desc,
      admin_cq_desc->color, qi.cq_color, qi.cq_tail);
#endif
    return (admin_cq_desc->color == qi.cq_color);
  default:
    break;
  }

  return false;
}

bool queue_has_space(uint16_t lif, queue_type qtype, uint32_t qid) {
  queue_info_t &qi = get_queue_info(lif, qtype, qid);

  if (((qi.head + 1) % (qi.ring_size)) == qi.tail) {
    return false;
  }

  return true;
}

void tx_post_buffer(uint16_t lif, queue_type qtype, uint32_t qid, uint8_t *buf,
                    uint32_t size) {
  queue_info_t &qi = get_queue_info(lif, qtype, qid);
  struct tx_desc *txq, *tx_desc;

  if (!queue_has_space(lif, qtype, qid)) {
    assert(0);
  }

  txq = (struct tx_desc *)qi.q;
  tx_desc = &txq[qi.head];
  tx_desc->addr = g_host_mem->VirtToPhys(buf);
  tx_desc->len = size;
  printf("[%s] lif%u/tx%u/q: head %u, desc pa 0x%0lx va %p,"
         " buf pa 0x%0lx va %p, len 0x%x\n",
         __FUNCTION__, lif, qid, qi.head, g_host_mem->VirtToPhys(tx_desc),
         tx_desc, tx_desc->addr, buf, tx_desc->len);

  qi.head++;
  qi.head &= (qi.ring_size - 1);

  int upd = 0xb, pid = 0x0, ring = 0x0;
  doorbell(upd, lif, qtype, pid, qid, ring, qi.head);
}

void rx_post_buffer(uint16_t lif, queue_type qtype, uint32_t qid, uint8_t *buf,
                    uint32_t size) {
  queue_info_t &qi = get_queue_info(lif, qtype, qid);
  struct rx_desc *rxq, *rx_desc;

  if (!queue_has_space(lif, qtype, qid)) {
    assert(0);
  }

  rxq = (struct rx_desc *)qi.q;
  rx_desc = &rxq[qi.head];
  rx_desc->addr = g_host_mem->VirtToPhys(buf);
  rx_desc->len = size;
  printf("[%s] lif%u/rx%u/q: head %u, desc pa 0x%0lx va %p,"
         " buf pa 0x%0lx va %p, len 0x%x\n",
         __FUNCTION__, lif, qid, qi.head, g_host_mem->VirtToPhys(rx_desc),
         rx_desc, rx_desc->addr, buf, rx_desc->len);

  qi.head++;
  qi.head &= (qi.ring_size - 1);

  int upd = 0x8, pid = 0x0, ring = 0x0;
  doorbell(upd, lif, qtype, pid, qid, ring, qi.head);
}

void adminq_post_buffer(uint16_t lif, queue_type qtype, uint32_t qid,
                        uint8_t *buf, uint32_t size) {
  queue_info_t &qi = get_queue_info(lif, qtype, qid);
  struct admin_cmd_desc *adminq, *admin_cmd;

  if (!queue_has_space(lif, qtype, qid)) {
    assert(0);
  }

  adminq = (struct admin_cmd_desc *)qi.q;
  admin_cmd = &adminq[qi.head];
  memcpy(admin_cmd, buf, sizeof(struct admin_cmd_desc));
  printf("[%s] lif%u/admin%u: head %u, desc pa 0x%0lx va %p\n", __FUNCTION__,
         lif, qid, qi.head, g_host_mem->VirtToPhys(admin_cmd), admin_cmd);

  qi.head++;
  qi.head &= (qi.ring_size - 1);

  int upd = 0xb, pid = 0x0, ring = 0x0;
  doorbell(upd, lif, qtype, pid, qid, ring, qi.head);
}

void post_buffer(uint16_t lif, queue_type qtype, uint32_t qid, uint8_t *buf,
                 uint32_t size) {
  switch (qtype) {
  case TX:
    tx_post_buffer(lif, qtype, qid, buf, size);
    break;
  case RX:
    rx_post_buffer(lif, qtype, qid, buf, size);
    break;
  case ADMIN:
    adminq_post_buffer(lif, qtype, qid, buf, size);
    break;
  default:
    break;
  }
}

void rx_consume_buffer(uint16_t lif, queue_type qtype, uint32_t qid,
                       completion_cb cb, void *ctx) {

  queue_info_t &qi = get_queue_info(lif, qtype, qid);
  struct rx_desc *rxq = (struct rx_desc *)qi.q;
  struct rx_cq_desc *rx_cq = (struct rx_cq_desc *)qi.cq;
  struct rx_desc *rx_desc = &rxq[qi.tail];
  struct rx_cq_desc *rx_cq_desc = &rx_cq[qi.cq_tail];

  while (rx_cq_desc->color == qi.cq_color) { // process all completions

    printf("[%s] lif%u/rx%u/cq:"
           " comp_index %d, comp_color %d,"
           " cq_tail %d, cq_color %d"
           " cq_desc pa 0x%0lx va %p\n",
           __FUNCTION__, lif, qid, rx_cq_desc->comp_index, rx_cq_desc->color,
           qi.cq_tail, qi.cq_color, g_host_mem->VirtToPhys(rx_cq_desc),
           rx_cq_desc);

    uint16_t next_comp_index = (rx_cq_desc->comp_index + 1) & (qi.ring_size - 1);
    while (qi.tail != next_comp_index) { // callback for each completed packet

      uint8_t *buf = (uint8_t *)g_host_mem->PhysToVirt(rx_desc->addr);
      uint32_t size = rx_desc->len;
      uint16_t pktlen = rx_cq_desc->len;

      printf("[%s] lif%u/rx%u/q:"
             " head %d, tail %d,"
             " desc pa 0x%0lx va %p,"
             " buf pa 0x%0lx va %p, size %d\n",
             __FUNCTION__, lif, qid, qi.head, qi.tail,
             g_host_mem->VirtToPhys(rx_desc), rx_desc, rx_desc->addr, buf,
             size);

      if (cb) {
        cb(buf, pktlen, ctx);
      }

      qi.tail++;
      qi.tail &= (qi.ring_size - 1);

      rx_desc = &rxq[qi.tail];

      rx_post_buffer(lif, qtype, qid, buf, size);
    }

    qi.cq_tail++;
    qi.cq_tail &= (qi.ring_size - 1);
    if (qi.cq_tail == 0) {
      qi.cq_color = (qi.cq_color == 0) ? 1 : 0;
    }

    rx_cq_desc = &rx_cq[qi.cq_tail];
  }
}

void tx_consume_buffer(uint16_t lif, queue_type qtype, uint32_t qid,
                       completion_cb cb, void *ctx) {
  queue_info_t &qi = get_queue_info(lif, qtype, qid);
  struct tx_desc *txq = (struct tx_desc *)qi.q;
  struct tx_cq_desc *tx_cq = (struct tx_cq_desc *)qi.cq;
  struct tx_desc *tx_desc = &txq[qi.tail];
  struct tx_cq_desc *tx_cq_desc = &tx_cq[qi.cq_tail];

  while (tx_cq_desc->color == qi.cq_color) { // process all completions

    printf("[%s] lif%u/tx%u/cq:"
           " comp_index %d, comp_color %d"
           " cq_tail %d, cq_color %d"
           " cq_desc pa 0x%0lx va %p\n",
           __FUNCTION__, lif, qid, tx_cq_desc->comp_index, tx_cq_desc->color,
           qi.cq_tail, qi.cq_color, g_host_mem->VirtToPhys(tx_cq_desc),
           tx_cq_desc);

    uint16_t next_comp_index = (tx_cq_desc->comp_index + 1) & (qi.ring_size - 1);
    while (qi.tail != next_comp_index) { // callback for each completed packet

      uint8_t *buf = (uint8_t *)g_host_mem->PhysToVirt(tx_desc->addr);
      uint32_t size = tx_desc->len;

      printf("[%s] lif%u/tx%u/q:"
             " head %d, tail %d,"
             " desc pa 0x%0lx va %p,"
             " buf pa 0x%0lx va %p, size %d\n",
             __FUNCTION__, lif, qid, qi.head, qi.tail,
             g_host_mem->VirtToPhys(tx_desc), tx_desc, tx_desc->addr, buf,
             size);

      if (cb) {
        cb(buf, size, ctx);
      }

      free_buffer(buf);

      qi.tail++;
      qi.tail &= (qi.ring_size - 1);

      tx_desc = &txq[qi.tail];
    }

    qi.cq_tail++;
    qi.cq_tail &= (qi.ring_size - 1);
    if (qi.cq_tail == 0) {
      qi.cq_color = (qi.cq_color == 0) ? 1 : 0;
    }

    tx_cq_desc = &tx_cq[qi.cq_tail];
  }
}

void adminq_consume_buffer(uint16_t lif, queue_type qtype, uint32_t qid,
                      completion_cb cb, void *ctx) {
  queue_info_t &qi = get_queue_info(lif, qtype, qid);
  struct admin_cmd_desc *admin = (struct admin_cmd_desc *)qi.q;
  struct admin_comp_desc *admin_cq = (struct admin_comp_desc *)qi.cq;
  struct admin_cmd_desc *admin_desc = &admin[qi.tail];
  struct admin_comp_desc *admin_cq_desc = &admin_cq[qi.cq_tail];

  while (admin_cq_desc->color == qi.cq_color) { // process all completions

    printf("[%s] lif%u/admin%u/cq:"
           " comp_index %d, comp_color %d"
           " cq_tail %d, cq_color %d"
           " cq_desc pa 0x%0lx va %p\n",
           __FUNCTION__, lif, qid, admin_cq_desc->comp_index, admin_cq_desc->color,
           qi.cq_tail, qi.cq_color, g_host_mem->VirtToPhys(admin_cq_desc),
           admin_cq_desc);

    uint16_t next_comp_index = (admin_cq_desc->comp_index + 1) & (qi.ring_size - 1);
    while (qi.tail != next_comp_index) { // callback for each completed packet

      printf("[%s] lif%u/admin%u/q:"
             " head %d, tail %d,"
             " desc pa 0x%0lx va %p\n",
             __FUNCTION__, lif, qid, qi.head, qi.tail,
             g_host_mem->VirtToPhys(admin_desc), admin_desc);

      if (cb) {
        cb(NULL, 0, ctx);
      }

      qi.tail++;
      qi.tail &= (qi.ring_size - 1);

      admin_desc = &admin[qi.tail];
    }

    qi.cq_tail++;
    qi.cq_tail &= (qi.ring_size - 1);
    if (qi.cq_tail == 0) {
      qi.cq_color = (qi.cq_color == 0) ? 1 : 0;
    }

    admin_cq_desc = &admin_cq[qi.cq_tail];
  }
}

void consume_buffer(uint16_t lif, queue_type qtype, uint32_t qid,
                    completion_cb cb, void *ctx) {
  switch (qtype) {
  case TX:
    tx_consume_buffer(lif, qtype, qid, cb, ctx);
    break;
  case RX:
    rx_consume_buffer(lif, qtype, qid, cb, ctx);
    break;
  case ADMIN:
    adminq_consume_buffer(lif, qtype, qid, cb, ctx);
    break;
  default:
    break;
  }
}

uint8_t *alloc_buffer(uint32_t size) {
  return (uint8_t *)g_host_mem->Alloc(size);
}

void free_buffer(uint8_t *buf) { g_host_mem->Free(buf); }
