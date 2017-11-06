#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct hdr_transpositions3_k k;
struct hdr_transpositions3_d d;
struct phv_ p;

%%

l4_hdr_transpositions:
    nop.e
    nop
