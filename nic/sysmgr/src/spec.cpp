#include <vector>

#include "spec.hpp"

#ifdef SYSMGR_TEST
const vector<Spec> SPECS = {
    Spec("delphi",  NO_WATCHDOG, "/sw/nic/build/x86_64/iris/bin/delphi_hub", {}),
    Spec("example", DEFAULT_SPEC_FLAGS, "/sw/nic/build/x86_64/iris/bin/sysmgr_example", {"delphi"}),
    Spec("test_complete", DEFAULT_SPEC_FLAGS, "/sw/nic/build/x86_64/iris/bin/sysmgr_test_complete", {"delphi", "example"}),
};
#else
const vector<Spec> SPECS;
#endif