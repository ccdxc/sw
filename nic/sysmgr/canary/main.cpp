// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <memory>
#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>

#include "gen/proto/sysmgr.delphi.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/sysmgr/lib/sysmgr_client.hpp"

using namespace std;

static const uint64_t MAX_INTERVAL_MS = 1000;
static const int TIMEBUF_SIZE = 128;

class Canary : public delphi::Service
{
private:
    string name;
    delphi::SdkPtr delphi;
    sysmgr::ClientPtr sysmgr;

public:
    Canary(delphi::SdkPtr delphi, string name) {
        this->delphi = delphi;
        this->name = name;
        this->sysmgr = sysmgr::CreateClient(delphi, name);
    }

    virtual string Name() {
        return this->name;
    }

    virtual void OnMountComplete() override {
        this->sysmgr->init_done();
    }
};

static uint64_t
timespec_diff_ms (struct timespec *end, struct timespec *start)
{
    uint64_t diff = (end->tv_sec - start->tv_sec) * 1000;

    // divide by 1000000 to convert it to ms. we don't care about ms accuracy
    // really
    return (diff + ((end->tv_nsec / 1000000L)) - (start->tv_nsec / 1000000L));
    
}

static void
timer_cb (struct ev_loop *loop, ev_timer *timer, int revents)
{
    struct timespec *start = (struct timespec *)timer->data;
    struct timespec end;
    uint64_t interval;

    clock_gettime(CLOCK_MONOTONIC, &end);

    interval = timespec_diff_ms(&end, start);

    if (interval > MAX_INTERVAL_MS) {
        char timestring[TIMEBUF_SIZE];
        time_t current_time = time(NULL);
        
        strftime(timestring, TIMEBUF_SIZE, "%Y-%m-%d %H:%M:%S",
                 gmtime(&current_time));
        printf("%s overslept by %lums\n", timestring,
               interval - MAX_INTERVAL_MS);
    }

    clock_gettime(CLOCK_MONOTONIC, start);
}

int
main (int argc, char *argv[])
{
    ev_timer timer_watcher;
    struct timespec ts;
    
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    
    if (argc != 2) {
        printf("Please use: %s <SERICE_NAME>\n", argv[0]);
        return -1;
    }

    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    
    std::shared_ptr<Canary> svc = std::make_shared<Canary>(sdk, argv[1]);
    sdk->RegisterService(svc);

    ev_timer_init(&timer_watcher, timer_cb, .5, .5);
    ev_timer_start(EV_DEFAULT, &timer_watcher);

    clock_gettime(CLOCK_MONOTONIC, &ts);
    timer_watcher.data = &ts;

    return sdk->MainLoop();
}
