#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct ipolicer_k k;
struct phv_ p;

%%

execute_ipolicer:
    nop.e
    nop
