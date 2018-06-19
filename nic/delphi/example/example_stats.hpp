// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_EXAMPLE_STATS_H_
#define _DELPHI_EXAMPLE_STATS_H_

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/delphi/example/example_stats.delphi.hpp"

namespace example {
using namespace delphi;

class InterfaceStatsMgr {
public:
    InterfaceStatsMgr();
    error PublishStats();

private:
    delphi::objects::InterfaceMetricsPtr if_stats_;
    ev::timer          statsTimer;
    void statsTimerHandler(ev::timer &watcher, int revents);
};
typedef std::shared_ptr<InterfaceStatsMgr> InterfaceStatsMgrPtr;

} // namespace example

#endif // _DELPHI_EXAMPLE_STATS_H_
