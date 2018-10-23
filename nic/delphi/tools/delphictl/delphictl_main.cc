// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/delphi/sdk/delphi_metrics.hpp"
#include "argh/argh.h"
#include "gen/proto/example.delphi.hpp"
#include "gen/proto/example_stats.delphi.hpp"
#include "gen/proto/upgrade.delphi.hpp"

using namespace delphi;

class DelphictlService : public delphi::Service, public enable_shared_from_this<DelphictlService> {
private:
    delphi::SdkPtr            sdk_;
    delphi::shm::DelphiShmPtr shm_;
    vector<string>            mount_kinds_;
public:
    DelphictlService(delphi::SdkPtr sk);

    // OnMountComplete gets called when all the objects are mounted
    void OnMountComplete();

    // DumpKvstore dumps kvstore info
    void DumpKvstore(string tbl_name);

    // DumpMetrics dumps metrics of a kind
    void DumpMetrics(string met_name);

    // MountKind mounts a kind of object
    void MountKind(string kind);
};

DelphictlService::DelphictlService(delphi::SdkPtr sk) {
    sdk_ = sk;
    // get the shared memory object
    shm_ = delphi::metrics::DelphiMetrics::GetDelphiShm();

}

// MountKind mounts the kinds we are interested in
void DelphictlService::MountKind(string kind) {
    printf("Mounting kind %s\n", kind.c_str());

    sdk_->MountKind(kind, delphi::ReadMode);
    mount_kinds_.push_back(kind);
}

void DelphictlService::DumpKvstore(string tbl_name) {
    shm_->Kvstore()->DumpInfo();

    if (tbl_name != "") {
        printf("dumping table: %s\n", tbl_name.c_str());
        delphi::shm::TableMgrUptr tbl = shm_->Kvstore()->Table(tbl_name.c_str());
        tbl->DumpTable();
    }
}

// DumpMetrics dumps a metric of a kind
void DelphictlService::DumpMetrics(string met_name) {
    delphi::metrics::DelphiMetricsIterator miter(met_name);
    for (; miter.IsNotNil(); miter.Next()) {
        auto tmp = miter.Get();
        printf("%s\n", tmp->DebugString().c_str());
    }
}


void DelphictlService::OnMountComplete() {
    // walk all the mounted kinds
    for (vector<string>::iterator it=mount_kinds_.begin(); it!=mount_kinds_.end(); ++it) {
        printf("Printing objects of kind: %s\n", (*it).c_str());
        printf("---------------------------------------------------------------\n");

        auto objlist = sdk_->ListKind(*it);
        // walk all objects
        for (vector<BaseObjectPtr>::iterator oi=objlist.begin(); oi!=objlist.end(); ++oi) {
            printf("%s\n", (*oi)->GetMessage()->DebugString().c_str());
            printf("------------------------------\n");
        }
    }

    LogInfo("Mount complete. Exiting");
    exit(0);
}

int main(int argc, char **argv) {
    // Create delphi SDK
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());

    // create parser
    auto cmdl = argh::parser(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

    cout << "\nFlags:\n";
    for (auto& flag : cmdl.flags())
        cout << '\t' << flag << endl;

    cout << "\nParameters:\n";
    for (auto& param : cmdl.params())
        cout << '\t' << param.first << " : " << param.second << endl;

    // Create a service instance
    shared_ptr<DelphictlService> dctl = make_shared<DelphictlService>(sdk);
    assert(dctl != NULL);
    sdk->RegisterService(dctl);

    // dump shared memory
    if (cmdl["shm"]) {
        // read arguments
        string tbl_name = cmdl("tbl").str();

        // dump kvstore contents
        dctl->DumpKvstore(tbl_name);

        // dump metrics if requested
        if (cmdl("metrics").str() != "") {
            dctl->DumpMetrics(cmdl("metrics").str());
        }

        exit(0);
    }

    // see if a kind was specified
    if (cmdl("kind").str() != "") {
        dctl->MountKind(cmdl("kind").str());
    }

    // run the main loop
    return sdk->MainLoop();
}
