#include <vector>

#include "spec.hpp"

#ifdef SYSMGR_TEST
const vector<Spec> SPECS = {
   Spec("delphi",  NO_WATCHDOG, "/sw/nic/build/x86_64/iris/bin/delphi_hub", {}),
   Spec("example", DEFAULT_SPEC_FLAGS, "/sw/nic/build/x86_64/iris/bin/sysmgr_example", {"delphi"}),
   Spec("test_complete", DEFAULT_SPEC_FLAGS, "/sw/nic/build/x86_64/iris/bin/sysmgr_test_complete", {"delphi", "example"}),
};
#else
const vector<Spec> SPECS = {
   Spec("delphi",  NO_WATCHDOG | NON_CRITICAL,
	"/nic/bin/delphi_hub", {}),
   Spec("hal", NO_WATCHDOG | NON_CRITICAL,
	"/bin/sh /nic/tools/start-hal-haps.sh", {
	 "delphi"}),
   Spec("nicmgrd", NO_WATCHDOG | NON_CRITICAL,
	"/bin/sh /platform/tools/start-nicmgr-haps.sh", {
	 "delphi", "hal"}),
   Spec("netagent", NO_WATCHDOG | NON_CRITICAL,
	"/nic/bin/netagent -datapath hal "
	"-logtofile /agent.log -hostif lo &", {"delphi", "hal"}),
};
#endif
