
#ifndef __UPLINK_HPP__
#define __UPLINK_HPP__

#include <vector>

#include "sdk/indexer.hpp"

#include "hal.hpp"
#include "l2seg.hpp"


class Uplink : public HalObject
{
public:
  Uplink(uint32_t port_num);
  ~Uplink();

  uint32_t GetId();
  uint64_t GetHandle();

  void AddL2Segment(std::shared_ptr<L2Segment> l2seg);
  void DelL2Segment(std::shared_ptr<L2Segment> l2seg);

  static void Probe();

private:
  uint32_t id;
  uint64_t handle;

  std::map<uint64_t, std::shared_ptr<L2Segment>> l2seg_refs;

  static sdk::lib::indexer *allocator;
  static constexpr uint64_t max_uplinks = 8;
};

#endif /* __UPLINK_HPP__ */
