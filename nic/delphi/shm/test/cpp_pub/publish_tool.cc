// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <unistd.h>
#include <ev++.h>
#include "nic/delphi/shm/shm.hpp"
#include "gen/proto/asicerrord.delphi.hpp"
#include "gen/proto/asicerrord_1.delphi.hpp"
#include "gen/proto/asicerrord_2.delphi.hpp"

using namespace std;
using namespace delphi;
using namespace delphi::shm;

int main(int argc, char **argv) {
    int numIter = 1000000;
    int numObjs = 1000;
    delphi::objects::pbpbcintwrite0metrics_t obj;

    if (argc > 1) {
        numIter = atoi(argv[1]);
    }

    // create the client
    auto client_shm = make_shared<DelphiShm>();
    error err = client_shm->MemMap(DELPHI_SHM_NAME, DELPHI_SHM_SIZE, true);
    assert(err.IsOK());

    delphi::objects::Pbpbcintwrite0Metrics::CreateTable();
    client_shm->DumpMeta();

    printf("\n\nrunning %d iterations of publish\n\n", numIter);
    for (int i = 0; i < numIter; i++) {
        for (int key = 1; key <= numObjs; key++) {
            delphi::objects::Pbpbcintwrite0Metrics::Publish(key, &obj);
        }
    }
    client_shm->DumpMeta();
}

