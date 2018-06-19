// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>
#include "nic/delphi/example/example_stats.hpp"

namespace example {

InterfaceStatsMgr::InterfaceStatsMgr() {
    // create interface stats
    srand(time(NULL));
    uint32_t if_key = rand() % 10000;
    if_stats_ = delphi::objects::InterfaceMetrics::NewInterfaceMetrics(if_key);
    assert(if_stats_ != NULL);

    // create a periodic timer to set stats
    this->statsTimer.set<InterfaceStatsMgr, &InterfaceStatsMgr::statsTimerHandler>(this);
    this->statsTimer.start(5.0, 5.0);

    return;
}

// PublishStats publishes stats periodically
error InterfaceStatsMgr::PublishStats() {
    LogDebug("Publishing stats");

    // set or increments stats
    if_stats_->RxPkts()->Incr();
    if_stats_->TxPkts()->Incr();
    if_stats_->RxPktRate()->Set(rand() % 5000);
    if_stats_->TxPktRate()->Set(rand() % 8000);
    if_stats_->RxErrors()->Add(rand() % 3);
    if_stats_->TxErrors()->Add(rand() % 5);

    return error::OK();
}

// statsTimerHandler handles stats event
void InterfaceStatsMgr::statsTimerHandler(ev::timer &watcher, int revents) {
    this->PublishStats();
}

}
