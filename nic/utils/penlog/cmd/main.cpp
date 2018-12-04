// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <memory>
#include <iostream>
#include <string>

#include <stdio.h>

#include <ev.h>

#include "gen/proto/penlog.delphi.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"

using namespace std;

static void timer_cb (EV_P_ ev_timer *w, int revents)
{
    printf("done\n");
    exit(-1);
}

class PenlogCtrl : public delphi::Service
{
private:
    delphi::SdkPtr delphi;
    ::penlog::Level level;
    string name;
    ev_timer timer_watcher;
public:
    PenlogCtrl(delphi::SdkPtr delphi, string name, ::penlog::Level level) {
        this->delphi = delphi;
        this->name = name;
        this->level = level;
        delphi::objects::PenlogReq::Mount(this->delphi, delphi::ReadWriteMode);
    }

    virtual void OnMountComplete() {
        auto obj = make_shared<delphi::objects::PenlogReq>();
        obj->set_key(name);
        obj->set_level(level);
        this->delphi->SetObject(obj);

        struct ev_loop *loop = EV_DEFAULT;
        ev_timer_init (&timer_watcher, timer_cb, 0.5, 0.0);
        ev_timer_start (loop, &timer_watcher);
    }

};

static bool starts_with (const char *str, const char *prefix)
{
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

static int stolevel(const char *s, ::penlog::Level *level)
{
    if (starts_with("critical", s))
    {
        *level =::penlog::Critical;
    }
    else if (starts_with("error", s))
    {
        *level =::penlog::Error;
    }
    else if (starts_with("warn", s))
    {
        *level =::penlog::Warn;
    }
    else if (starts_with("info", s))
    {
        *level =::penlog::Info;
    }
    else if (starts_with("debug", s))
    {
        *level =::penlog::Debug;
    }
    else if (starts_with("trace", s))
    {
        *level =::penlog::Trace;
    }
    else
    {
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        perror("Usage: %s <process name> <log level>\n");
        return -1;
    }

    ::penlog::Level level;
    int rc = stolevel(argv[2], &level);
    if (rc != 0)
    {
        perror("Valid levels are: \"critical\", \"error\", \"info\", "
            "\"debug\", \"trace\"\n");
        return -1;
    }

    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    
    auto svc = make_shared<PenlogCtrl>(sdk, argv[1], level);
    sdk->RegisterService(svc);
    
    return sdk->MainLoop();
}
