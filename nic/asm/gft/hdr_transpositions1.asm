#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct hdr_transpositions1_k k;
struct hdr_transpositions1_d d;
struct phv_ p;

%%

hdr_transpositions:
    nop.e
    nop
