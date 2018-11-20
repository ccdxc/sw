#ifndef __QOS_HPP__
#define __QOS_HPP__

#include <vector>

#include "hal.hpp"
#include "hal_types.hpp"

#include "gen/proto/qos.grpc.pb.h"


class QosClass : public HalObject {
public:
  static int32_t GetTxTrafficClassCos(const std::string &group, 
                                      uint32_t uplink_port);
};

#endif
