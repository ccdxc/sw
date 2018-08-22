#include <vector>

#include "spec.h"

const vector<Spec> SPECS = {
    Spec("delphi", NON_RESTARTABLE, "/bin/ls -l", {}),
    Spec("agent1", NON_RESTARTABLE, "/bin/ls -l", {"delphi", "hal", "nicmgr"}),
    Spec("hal", NON_RESTARTABLE, "/bin/ls -l", {"delphi"}),
    Spec("nicmgr", NON_RESTARTABLE, "/bin/ls -l", {"delphi", "hal"}),
    Spec("agent2", NON_RESTARTABLE, "/bin/ls -l", {"delphi", "hal", "nicmgr"}),
};