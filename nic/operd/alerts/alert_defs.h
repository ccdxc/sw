#ifndef __OPERD_ALERT_DEFS_H__
#define __OPERD_ALERT_DEFS_H__


#include "nic/operd/alerts/alert_type.hpp"
namespace operd {
namespace alerts {

typedef enum operd_alerts_ {
	SERVICE_STARTED = 0,
	NAPLES_SERVICE_STOPPED = 1,
	SYSTEM_COLDBOOT = 2,
} operd_alerts_t;

extern const alert_t alerts[3];
}
}
#endif
