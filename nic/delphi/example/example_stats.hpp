// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_EXAMPLE_STATS_H_
#define _DELPHI_EXAMPLE_STATS_H_

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/example_stats.delphi.hpp"

namespace example {
using namespace delphi;

class ExampleStatsMgr {
public:
    ExampleStatsMgr();
    error PublishStats();

private:
    delphi::objects::ExampleMetricsPtr if_stats_;
    ev::timer          statsTimer;
    void statsTimerHandler(ev::timer &watcher, int revents);
};
typedef std::shared_ptr<ExampleStatsMgr> ExampleStatsMgrPtr;

} // namespace example

#endif // _DELPHI_EXAMPLE_STATS_H_
