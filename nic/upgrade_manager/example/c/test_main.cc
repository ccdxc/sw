// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "nic/upgrade_manager/example/c/test.hpp"

using namespace std;
using namespace upgrade;
using namespace test;

bool test::CompatCheckFail;
bool test::PostRestartFail;
bool test::ProcessQuiesceFail;
bool test::LinkDownFail;
bool test::HostDownFail;
bool test::PostHostDownFail;
bool test::SaveStateFail;
bool test::HostUpFail;
bool test::LinkUpFail;
bool test::PostLinkUpFail;

bool test::CompatCheckPause;
bool test::PostRestartPause;
bool test::ProcessQuiescePause;
bool test::LinkDownPause;
bool test::HostDownPause;
bool test::PostHostDownPause;
bool test::SaveStatePause;
bool test::HostUpPause;
bool test::LinkUpPause;
bool test::PostLinkUpPause;

int main(int argc, char **argv) {
    FILE *fp;

    fp = fopen("/run/testupgapp.pid", "w+");
    fprintf(fp, "%d\n", getpid());
    fclose(fp);

    if (argc > 1) {
        if (strcmp(argv[1], "compatcheckfail") == 0) {
            test::CompatCheckFail = true;
        } else if (strcmp(argv[1], "postrestartfail") == 0) {
            test::PostRestartFail = true;
        } else if (strcmp(argv[1], "processquiescefail") == 0) {
            test::ProcessQuiesceFail = true;
        } else if (strcmp(argv[1], "linkdownfail") == 0) {
            test::LinkDownFail = true;
        } else if (strcmp(argv[1], "hostdownfail") == 0) {
            test::HostDownFail = true;
        } else if (strcmp(argv[1], "posthostdownfail") == 0) {
            test::PostHostDownFail = true;
        } else if (strcmp(argv[1], "savestatefail") == 0) {
            test::SaveStateFail = true;
        } else if (strcmp(argv[1], "hostupfail") == 0) {
            test::HostUpFail = true;
        } else if (strcmp(argv[1], "linkupfail") == 0) {
            test::LinkUpFail = true;
        } else if (strcmp(argv[1], "postlinkupfail") == 0) {
            test::PostLinkUpFail = true;
        } else if (strcmp(argv[1], "compatcheckpause") == 0) {
            test::CompatCheckPause = true;
        } else if (strcmp(argv[1], "postrestartpause") == 0) {
            test::PostRestartPause = true;
        } else if (strcmp(argv[1], "processquiescepause") == 0) {
            test::ProcessQuiescePause = true;
        } else if (strcmp(argv[1], "linkdownpause") == 0) {
            test::LinkDownPause = true;
        } else if (strcmp(argv[1], "hostdownpause") == 0) {
            test::HostDownPause = true;
        } else if (strcmp(argv[1], "posthostdownpause") == 0) {
            test::PostHostDownPause = true;
        } else if (strcmp(argv[1], "savestatepause") == 0) {
            test::SaveStatePause = true;
        } else if (strcmp(argv[1], "hostuppause") == 0) {
            test::HostUpPause = true;
        } else if (strcmp(argv[1], "linkuppause") == 0) {
            test::LinkUpPause = true;
        } else if (strcmp(argv[1], "postlinkuppause") == 0) {
            test::PostLinkUpPause = true;
        } else {
            cout << "Unknown flag " << argv[1];
        }
    }
    // Create delphi SDK
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    string myName = "TestUpgradeService";

    // Create a service instance
    shared_ptr<TestUpgSvc> exupgsvc = make_shared<TestUpgSvc>(sdk, myName);
    assert(exupgsvc != NULL);

    sdk->RegisterService(exupgsvc);

    // start a timer to create an object
    exupgsvc->createTimer.set<TestUpgSvc, &TestUpgSvc::createTimerHandler>(exupgsvc.get());
    exupgsvc->createTimer.start(305, 0);

    // run the main loop
    return sdk->MainLoop();
}
