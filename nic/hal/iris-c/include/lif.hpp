
#ifndef __LIF_HPP__
#define __LIF_HPP__

#include <vector>

#include "sdk/indexer.hpp"

#include "hal.hpp"
#include "types.hpp"


class Lif : public HalObject {
public:
  static Lif *Factory(uint32_t hw_lif_id, Uplink *pinned_uplink);
  static void Destroy(Lif *lif);

  uint32_t GetId();

private:
  Lif(uint32_t hw_lif_id, Uplink *pinned_uplink);
  ~Lif();
  uint32_t id;
  uint64_t handle;
  uint32_t hw_lif_id;

  intf::LifSpec spec;

  static sdk::lib::indexer *allocator;
  static constexpr uint32_t max_lifs = 1024;
};

#endif
