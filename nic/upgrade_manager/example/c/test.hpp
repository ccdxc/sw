// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __TEST_H__
#define __TEST_H__

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <error.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/export/upgcsdk/upgrade.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace test {

using namespace std;
using namespace upgrade;

extern bool CompatCheckFail;
extern bool PostRestartFail;
extern bool ProcessQuiesceFail;
extern bool LinkDownFail;
extern bool HostDownFail;
extern bool PostHostDownFail;
extern bool SaveStateFail;
extern bool HostUpFail;
extern bool LinkUpFail;
extern bool PostLinkUpFail;

extern bool CompatCheckPause;
extern bool PostRestartPause;
extern bool ProcessQuiescePause;
extern bool LinkDownPause;
extern bool HostDownPause;
extern bool PostHostDownPause;
extern bool SaveStatePause;
extern bool HostUpPause;
extern bool LinkUpPause;
extern bool PostLinkUpPause;

extern bool CompatCheckProcessKill;
extern bool PostRestartProcessKill;
extern bool ProcessQuiesceProcessKill;
extern bool LinkDownProcessKill;
extern bool HostDownProcessKill;
extern bool PostHostDownProcessKill;
extern bool SaveStateProcessKill;
extern bool HostUpProcessKill;
extern bool LinkUpProcessKill;
extern bool PostLinkUpProcessKill;

extern bool UnRegisterWithUpgrade;

extern string retStr;

// TestUpgSvc is the service object for test upgrade service 
class TestUpgSvc : public delphi::Service, public enable_shared_from_this<TestUpgSvc> {
private:
    UpgSdkPtr          upgsdk_;
    delphi::SdkPtr     sdk_;
    string             svcName_;
public:
    // TestUpgSvc constructor
    TestUpgSvc(delphi::SdkPtr sk);
    TestUpgSvc(delphi::SdkPtr sk, string name);

    // override service name method
    virtual string Name() { return svcName_; }

    // timer for creating a dummy object
    ev::timer          createTimer;
    void createTimerHandler(ev::timer &watcher, int revents);
    void unRegUpgTimerHandler(ev::timer &watcher, int revents);

    void OnMountComplete(void) {
        UPG_LOG_DEBUG("TestUpgSvc OnMountComplete called! Nothing to be done.");
    }
};
typedef std::shared_ptr<TestUpgSvc> TestUpgSvcPtr;

class ExSvcHandler : public UpgHandler {
public:
    ExSvcHandler(){}

    HdlrResp CompatCheckHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        if (CompatCheckFail) {
            resp = {.resp=FAIL, .errStr=retStr};
        }
        if (CompatCheckPause) {
            resp = {.resp=INPROGRESS, .errStr=""};
        }
        if (CompatCheckProcessKill) {
            FILE *getPIDS;
            char line[130];
            getPIDS = popen("pidof upgrade","r");
            while (fgets(line,sizeof line,getPIDS)) {
                int i = atoi(line);
                kill(i,SIGKILL);
            }
        }
        return resp;
    }

    HdlrResp PostRestartHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        if (PostRestartFail) {
            resp = {.resp=FAIL, .errStr=retStr};
        }
        if (PostRestartPause) {
            resp = {.resp=INPROGRESS, .errStr=""};
        }
        if (PostRestartProcessKill) {
            FILE *getPIDS;
            char line[130];
            getPIDS = popen("pidof upgrade","r");
            while (fgets(line,sizeof line,getPIDS)) {
                int i = atoi(line);
                kill(i,SIGKILL);
            }
        }
        return resp;
    }

    HdlrResp ProcessQuiesceHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        if (ProcessQuiesceFail) {
            resp = {.resp=FAIL, .errStr=retStr};
        }
        if (ProcessQuiescePause) {
            resp = {.resp=FAIL, .errStr=retStr};
        }
        if (ProcessQuiesceProcessKill) {
            FILE *getPIDS;
            char line[130];
            getPIDS = popen("pidof upgrade","r");
            while (fgets(line,sizeof line,getPIDS)) {
                int i = atoi(line);
                kill(i,SIGKILL);
            }
        }
        return resp;
    }

    HdlrResp LinkDownHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        if (LinkDownFail) {
            resp = {.resp=FAIL, .errStr=retStr};
        }
        if (LinkDownPause) {
            resp = {.resp=INPROGRESS, .errStr=""};
        }
        if (LinkDownProcessKill) {
            FILE *getPIDS;
            char line[130];
            getPIDS = popen("pidof upgrade","r");
            while (fgets(line,sizeof line,getPIDS)) {
                int i = atoi(line);
                kill(i,SIGKILL);
            }
        }
        return resp;
    }

    HdlrResp HostDownHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        if (HostDownFail) {
            resp = {.resp=FAIL, .errStr=retStr};
        }
        if (HostDownPause) {
            resp = {.resp=INPROGRESS, .errStr=""};
        }
        if (HostDownProcessKill) {
            FILE *getPIDS;
            char line[130];
            getPIDS = popen("pidof upgrade","r");
            while (fgets(line,sizeof line,getPIDS)) {
                int i = atoi(line);
                kill(i,SIGKILL);
            }
        }
        return resp;
    }

    HdlrResp PostHostDownHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        if (PostHostDownFail) {
            resp = {.resp=FAIL, .errStr=retStr};
        }
        if (PostHostDownPause) {
            resp = {.resp=INPROGRESS, .errStr=""};
        }
        if (PostHostDownProcessKill) {
            FILE *getPIDS;
            char line[130];
            getPIDS = popen("pidof upgrade","r");
            while (fgets(line,sizeof line,getPIDS)) {
                int i = atoi(line);
                kill(i,SIGKILL);
            }
        }
        return resp;
    }

    HdlrResp SaveStateHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        if (SaveStateFail) {
            resp = {.resp=FAIL, .errStr=retStr};
        }
        if (SaveStatePause) {
            resp = {.resp=INPROGRESS, .errStr=""};
        }
        if (SaveStateProcessKill) {
            FILE *getPIDS;
            char line[130];
            getPIDS = popen("pidof upgrade","r");
            while (fgets(line,sizeof line,getPIDS)) {
                int i = atoi(line);
                kill(i,SIGKILL);
            }
        }
        return resp;
    }

    HdlrResp HostUpHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        if (HostUpFail) {
            resp = {.resp=FAIL, .errStr=retStr};
        }
        if (HostUpPause) {
            resp = {.resp=INPROGRESS, .errStr=""};
        }
        if (HostUpProcessKill) {
            FILE *getPIDS;
            char line[130];
            getPIDS = popen("pidof upgrade","r");
            while (fgets(line,sizeof line,getPIDS)) {
                int i = atoi(line);
                kill(i,SIGKILL);
            }
        }
        return resp;
    }

    HdlrResp LinkUpHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        if (LinkUpFail) {
            resp = {.resp=FAIL, .errStr=retStr};
        }
        if (LinkUpPause) {
            resp = {.resp=INPROGRESS, .errStr=""};
        }
        if (LinkUpProcessKill) {
            FILE *getPIDS;
            char line[130];
            getPIDS = popen("pidof upgrade","r");
            while (fgets(line,sizeof line,getPIDS)) {
                int i = atoi(line);
                kill(i,SIGKILL);
            }
        }
        return resp;
    }

    HdlrResp PostLinkUpHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        if (PostLinkUpFail) {
            resp = {.resp=FAIL, .errStr=retStr};
        }
        if (PostLinkUpPause) {
            resp = {.resp=INPROGRESS, .errStr=""};
        }
        if (PostLinkUpProcessKill) {
            FILE *getPIDS;
            char line[130];
            getPIDS = popen("pidof upgrade","r");
            while (fgets(line,sizeof line,getPIDS)) {
                int i = atoi(line);
                kill(i,SIGKILL);
            }
        }
        return resp;
    }

    void AbortHandler(UpgCtx& upgCtx) {
        return;
    }

    void SuccessHandler(UpgCtx& upgCtx) {
        FILE *fp = fopen("/run/testupgapp.success", "w+");
        fprintf(fp, "success\n");
        fclose(fp);
        return;
    }

    HdlrResp FailedHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        FILE *fp = fopen("/run/testupgapp.fail", "w+");
        fprintf(fp, "fail\n");
        fclose(fp);
        return resp;
    }
};

} // namespace test

#endif // __TEST_H__
