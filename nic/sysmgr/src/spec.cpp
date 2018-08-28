#include <vector>

#include "spec.hpp"

#ifdef SYSMGR_TEST
const vector<Spec> SPECS = {
    Spec("delphi", NON_RESTARTABLE, "/sw/bazel-bin/nic/delphi/hub/delphi_hub", {}),
    Spec("example", NON_RESTARTABLE, "/sw/bazel-bin/nic/sysmgr/example/example", {"delphi"}),
    Spec("test_complete", NON_RESTARTABLE, "/sw/bazel-bin/nic/sysmgr/example/test_complete", {"delphi", "example"}),
};
#else
const vector<Spec> SPECS;
#endif