#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct tcp_options_fixup_k k;
struct phv_              p;

%%

tcp_options_fixup:
  seq.e      c1, k.tcp_options_blob_valid, TRUE
  .assert(offsetof(p, tcp_option_unknown_valid) - offsetof(p, tcp_option_mss_valid) == 10)
  phvwr.c1   p.{tcp_option_unknown_valid...tcp_option_mss_valid}, r0
