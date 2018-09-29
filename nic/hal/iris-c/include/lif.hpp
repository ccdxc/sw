
#ifndef __LIF_HPP__
#define __LIF_HPP__

#include <vector>

#include "nic/sdk/include/sdk/indexer.hpp"

#include "hal.hpp"
#include "types.hpp"


class Lif : public HalObject {
public:
  static Lif *Factory(EthLif *eth_lif);
  static void Destroy(Lif *lif);

  uint32_t GetId();

  void TriggerHalUpdate();
private:
  Lif(EthLif *eth_lif);
  ~Lif();
  uint32_t id_;
  uint64_t handle_;
  uint32_t hw_lif_id_;
  EthLif *eth_lif_;

  intf::LifSpec spec;

  static sdk::lib::indexer *allocator;
  static constexpr uint32_t max_lifs = 1024;

};

#endif
