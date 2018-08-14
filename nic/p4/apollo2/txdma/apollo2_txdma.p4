#include "../../include/intrinsic.p4"

#include "../include/defines.h"
#include "../include/table_sizes.h"
#include "../include/headers.p4"

#include "setup.p4"
#include "route.p4"
#include "dma.p4"
#include "metadata.p4"

parser start {
    return ingress;
}


control ingress {
    if (predicate_header.txdma_drop_event == FALSE) {
        setup();
        route();
        dma();
    }
}
