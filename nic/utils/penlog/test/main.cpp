// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <memory>
#include <iostream>
#include <string>

#include <stdio.h>

#include <ev.h>

#include "gen/proto/penlog.delphi.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/utils/penlog/lib/penlog.hpp"

using namespace std;

penlog::LoggerPtr logger;

static void timer_cb (EV_P_ ev_timer *w, int revents)
{
    logger->trace("Penlog test trace");
    logger->debug("Penlog test debug");
    logger->info("Penlog test info");
    logger->warn("Penlog test warn");
    logger->error("Penlog test error");
    logger->critical("Penlog test critical");
}

class PenlogTest : public delphi::Service
{
private:
    delphi::SdkPtr delphi;
    ev_timer timer_watcher;
public:
    PenlogTest(delphi::SdkPtr delphi) {
        this->delphi = delphi;
    }

    virtual void OnMountComplete() {
        struct ev_loop *loop = EV_DEFAULT;
        ev_timer_init(&timer_watcher, timer_cb, 0.1, 0.1);
        ev_timer_start(loop, &timer_watcher);
    }
};


int main(int argc, char *argv[]) {
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    
    auto svc = make_shared<PenlogTest>(sdk);
    sdk->RegisterService(svc);

    logger = penlog::logger_init(sdk, "penlog_test");
    
    return sdk->MainLoop();
}
