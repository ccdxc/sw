// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>
#include "nic/delphi/example/example_stats.hpp"

namespace example {

using delphi::objects::examplemetrics_t;
using delphi::objects::nestedkeyexamplemetrics_t;

ExampleStatsMgr::ExampleStatsMgr() {
    // create interface stats
    srand(time(NULL));
    uint32_t if_key = rand() % 10000;
    if_stats_ = delphi::objects::ExampleMetrics::NewExampleMetrics(if_key);
    assert(if_stats_ != NULL);
    delphi::objects::NestedKeyExampleMetrics::CreateTable();

    // create a periodic timer to set stats
    this->statsTimer.set<ExampleStatsMgr, &ExampleStatsMgr::statsTimerHandler>(this);
    this->statsTimer.start(0.1, 0.1);

    return;
}

// PublishStats publishes stats periodically
error ExampleStatsMgr::PublishStats() {
    LogDebug("Publishing stats");

    // set or increments stats
    if_stats_->RxPkts()->Incr();
    if_stats_->TxPkts()->Incr();
    if_stats_->RxPktRate()->Set(rand() % 5000);
    if_stats_->TxPktRate()->Set(rand() % 8000);
    if_stats_->RxErrors()->Add(rand() % 3);
    if_stats_->TxErrors()->Add(rand() % 5);

    // publish stats
    nestedkeyexamplemetrics_t ifmet;
    ExampleKey key;
    key.set_ifidx(10001);
    ifmet.RxPkts = 2000;
    ifmet.RxPktRate = 200;
    ifmet.RxErrors = 50;
    delphi::objects::NestedKeyExampleMetrics::Publish(key, &ifmet);

    return error::OK();
}

// statsTimerHandler handles stats event
void ExampleStatsMgr::statsTimerHandler(ev::timer &watcher, int revents) {
    this->PublishStats();
}

}
