
#ifndef __LIF_HPP__
#define __LIF_HPP__

#include <vector>

#include "sdk/indexer.hpp"

#include "hal.hpp"
#include "types.hpp"


struct queue_spec {
    uint32_t type_num;           /* HW Queue Type */
    uint32_t size;               /* Qstate Size: 2^size */
    uint32_t entries;            /* Number of Queues: 2^entries */
    int purpose;                 /* Queue Purpose */
    const char* prog;            /* Program File Name */
    const char* label;           /* Program Entry Label */
    const char* qstate;          /* Qstate structure */
};

struct lif_spec {
  struct queue_spec queue_spec[NUM_QUEUE_TYPES];
  bool enable_rdma;
};

class Lif : public HalObject {
public:
  Lif(struct lif_spec &lif_spec);
  Lif(uint64_t lif_id);
  ~Lif();

  static void Probe();

  uint32_t GetId();

private:
  uint32_t id;
  uint64_t handle;
  uint64_t hw_lif_id;
  uint64_t qstate_addr[NUM_QUEUE_TYPES];

  intf::LifSpec spec;

  static sdk::lib::indexer *allocator;
  static constexpr uint32_t max_lifs = 1024;
};

#endif
