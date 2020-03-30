#include <cstdlib>
#include "alert_defs.h"

namespace operd {
namespace alerts {

const alert_t alerts[3] = {
	{
		name: "SERVICE_STARTED",
		category: "System",
		severity: "DEBUG",
		description: "Service started",
		message: NULL
	},
	{
		name: "NAPLES_SERVICE_STOPPED",
		category: "System",
		severity: "CRITICAL",
		description: "Naples service stopped",
		message: NULL
	},
	{
		name: "SYSTEM_COLDBOOT",
		category: "System",
		severity: "WARN",
		description: "System cold booted",
		message: NULL
	},
};

}
}
