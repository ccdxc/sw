#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/iris/include/defines.h"

struct ddos_service_policer_action_k     k;
struct ddos_service_policer_action_d     d;
struct phv_             p;

d = {
    u.ddos_service_policer_action_d.ddos_service_policer_saved_color     = 0x1;
    u.ddos_service_policer_action_d.ddos_service_policer_dropped_packets = 0x1;
    };

k = {
    ddos_metadata_ddos_service_policer_color = 0x1;
    };

