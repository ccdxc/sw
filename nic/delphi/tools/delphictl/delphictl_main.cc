// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/delphi/shm/delphi_metrics.hpp"
#include "argh/argh.h"
#include "gen/proto/delphi_objects.hpp"
#include "nic/sdk/lib/pal/pal.hpp"

using namespace delphi;

class DelphictlService : public delphi::Service, public enable_shared_from_this<DelphictlService> {
private:
    delphi::SdkPtr            sdk_;
    vector<string>            mount_kinds_;
public:
    string                    matchKey;

    // constructor
    DelphictlService(delphi::SdkPtr sk);

    // OnMountComplete gets called when all the objects are mounted
    void OnMountComplete();

    // MountKind mounts a kind of object
    void MountKind(string kind);
};

DelphictlService::DelphictlService(delphi::SdkPtr sk) {
    sdk_ = sk;

}

// MountKind mounts the kinds we are interested in
void DelphictlService::MountKind(string kind) {
    printf("Mounting kind %s\n", kind.c_str());

    sdk_->MountKind(kind, delphi::ReadMode);
    mount_kinds_.push_back(kind);
}

void DelphictlService::OnMountComplete() {
    // walk all the mounted kinds
    for (vector<string>::iterator it=mount_kinds_.begin(); it!=mount_kinds_.end(); ++it) {
        printf("Printing objects of kind: %s\n", (*it).c_str());
        printf("---------------------------------------------------------------\n");

        auto objlist = sdk_->ListKind(*it);
        // walk all objects
        for (vector<BaseObjectPtr>::iterator oi=objlist.begin(); oi!=objlist.end(); ++oi) {
            string key = (*oi)->GetKey();

            // if match key was not specified or key contains the match
            if ((this->matchKey == "") || (key.find(this->matchKey) != std::string::npos)) {
                printf("%s\n", (*oi)->GetMessage()->DebugString().c_str());
                printf("------------------------------\n");
            }
        }
    }

    exit(0);
}

void printKvstoreInfo(string tbl_name) {
    auto shm = delphi::metrics::DelphiMetrics::GetDelphiShm();
    shm->Kvstore()->DumpInfo();

    if (tbl_name != "") {
        printf("dumping table: %s\n", tbl_name.c_str());
        delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table(tbl_name.c_str());
        tbl->DumpTable();
    }
}

void printShmInfo() {
    auto shm = delphi::metrics::DelphiMetrics::GetDelphiShm();
    shm->Kvstore()->DumpInfo();
}

void listMetricsKind(string kind, string matchstr) {
    // initialize pal
#ifdef __x86_64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_MOCK) == sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HAPS) == sdk::lib::PAL_RET_OK);
#endif

    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = delphi::metrics::DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // get the table
    delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table(kind);
    if (tbl == NULL) {
        printf("%s table not found\n", kind.c_str());
        return;
    }

    // create the iterator
    delphi::metrics::DelphiMetricsIterator miter(kind);
    for (; miter.IsNotNil(); miter.Next()) {
        auto tmp = miter.Get();
        string dbgstr = tmp->DebugString();
        if ((matchstr == "") || (dbgstr.find(matchstr) != std::string::npos)) {
            printf("%s\n", dbgstr.c_str());
        }
    }
}

void printMetricsKindList() {
    // factory list
    map<string, delphi::metrics::MetricsFactory*> fctries = *(delphi::metrics::DelphiMetrics::GetFactoryMap());

    cout << "Usage: delphictl metrics list <kind> [options]" << endl;
    cout << "Available kinds:" << endl;

    for (map<string, delphi::metrics::MetricsFactory *>::iterator i = fctries.begin(); i != fctries.end(); ++i) {
        cout << "    " <<  i->first << endl;
    }
}

void printMetricsCmdUsage() {
    cout << "Usage: delphictl metrics <command> <kind> [options]" << endl;
    cout << "Available options:" << endl;
    cout << "    list      - list all objects of kind" << endl;
    cout << "    get       - display an object of kind, key" << endl;
}

void listDbKind(string kind, string matchKey) {
    // Create delphi SDK
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());

    // Create a service instance
    shared_ptr<DelphictlService> dctl = make_shared<DelphictlService>(sdk);
    assert(dctl != NULL);
    sdk->RegisterService(dctl);
    dctl->matchKey = matchKey;

    // enter admin mode so that we can read/write any object
    sdk->enterAdminMode(); 

    // mount kind
    dctl->MountKind(kind);

    // run the main loop
    sdk->MainLoop();
}

void printDbkindList() {
    // factory list
    map<string, ObjectFactory *> fctries = *(BaseObject::GetFactoryMap());

    cout << "Usage: delphictl db list <kind> [options]" << endl;
    cout << "Available kinds:" << endl;

    for (map<string, ObjectFactory *>::iterator i = fctries.begin(); i != fctries.end(); ++i) {
        cout << "    " <<  i->first << endl;
    }
}

void printDbcmdUsage() {
    cout << "Usage: delphictl db <command> <kind> [options]" << endl;
    cout << "Available options:" << endl;
    cout << "    list      - list all objects of kind" << endl;
    cout << "    get       - display an object of kind and key" << endl;
}

void printUsage() {
    cout << "Usage: delphictl <db> <command> <kind> [options]" << endl;
    cout << "Available options:" << endl;
    cout << "    db      - operations on delphi database" << endl;
    cout << "    metrics - operations on metrics" << endl;
    cout << "    shm     - information about shared memory" << endl;
    cout << "    help    - this help string" << endl;
}

int main(int argc, char **argv) {

    // create parser
    auto cmdl = argh::parser(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

    // check first command
    if (!cmdl(1)) {
        printUsage();
        exit(1);
    }

    if (cmdl[1] == "db") {
        if (!cmdl(2)) {
            printDbcmdUsage();
            exit(1);
        } else if (cmdl[2] == "list") {
            if (!cmdl(3)) {
                printDbkindList();
                exit(1);
            } else {
                listDbKind(cmdl[3], "");
                exit(0);
            }
        } else if (cmdl[2] == "get") {
            if (!cmdl(3)) {
                printDbkindList();
                exit(1);
            } else {
                string matchstr("");
                if (cmdl(4)) {
                    matchstr = cmdl[4];
                }
                listDbKind(cmdl[3], matchstr);
                exit(0);
            }
        } else {
            printDbcmdUsage();
            exit(1);
        }
    } else if (cmdl[1] == "metrics") {
        if (!cmdl(2)) {
            printMetricsCmdUsage();
            exit(1);
        } else if (cmdl[2] == "list") {
            if (!cmdl(3)) {
                printMetricsKindList();
                exit(1);
            } else {
                listMetricsKind(cmdl[3], "");
                exit(0);
            }
        } else if (cmdl[2] == "get") {
            if (!cmdl(3)) {
                printMetricsKindList();
                exit(1);
            } else {
                string matchstr("");
                if (cmdl(4)) {
                    matchstr = cmdl[4];
                }
                listMetricsKind(cmdl[3], matchstr);
                exit(0);
            }
        } else {
            printMetricsCmdUsage();
            exit(1);
        }
    } else if (cmdl[1] == "shm") {
        if (!cmdl(2)) {
            printShmInfo();
            exit(0);
        } else {
            printKvstoreInfo(cmdl[2]);
            exit(0);
        }
    } else {
        printUsage();
        exit(1);
    }
}
