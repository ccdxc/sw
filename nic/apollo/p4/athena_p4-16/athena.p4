#include <capri_model.p4>


#define DROP_PACKET_INGRESS(x) \
    metadata.cntrl.drop_reason[x:x] = 1; \
    capri_intrinsic.drop = 1;

#define DROP_PACKET_EGRESS(x) \
    metadata.cntrl.egress_drop_reason[x:x] = 1; \
    capri_intrinsic.drop = 1;

#define SET_BIT_VALUE(m, x, v) \
    m[x:x] = v;

//#include "poseidon16_table_size.h"
#include "common/capri.h"
//#include "common/defines.h"
#include "athena_defines.h"
#include "athena_16_defines.h"
#include "athena_table_sizes.h"
//#include "poseidon16_defines.h"
#include "externs.p4"
#include "metadata.p4"
#include "headers.p4"
#include "parser.p4"
#include "egress_parser.p4"
//#include "vport.p4"
#include "nicvid.p4"
#include "key.p4"
#include "offloads.p4"
#include "dnat.p4"
#include "flow.p4"
#include "policer.p4"
#include "config_verify.p4"
#include "inter_pipe.p4"
#include "session.p4"
//#include "validate.p4"
//#include "epoch.p4"
//#include "flow_timestamp.p4"
//#include "egress_flow_info.p4"
//#include "egress_stats_fixup.p4"
#include "stats.p4"
//#include "histogram.p4"
//#include "output_mapping.p4"
#include "checksum.p4"
#include "nacl.p4"
//#include "flow_log_key.p4"
//#include "flow_log.p4"
#include "conntrack.p4"


control AthenaIngressPipeline(inout cap_phv_intr_global_h intr_global,
                            inout cap_phv_intr_p4_h intr_p4,
                            inout headers hdr,
                            inout metadata_t metadata) {
    apply {
      vnic.apply(intr_global, intr_p4, hdr, metadata);
      key_init.apply(intr_global, intr_p4, hdr, metadata);
      dnat_lookup.apply(intr_global, intr_p4, hdr, metadata);
      offloads.apply(intr_global, intr_p4, hdr, metadata);
      flow_lookup.apply(intr_global, intr_p4, hdr, metadata);
      ingress_inter_pipe.apply(intr_global, intr_p4, hdr, metadata);
      p4i_statistics.apply(intr_global, intr_p4, hdr, metadata);
    }
}


control AthenaEgressPipeline(inout cap_phv_intr_global_h intr_global,
                           inout cap_phv_intr_p4_h intr_p4,
                           inout headers hdr,
                           inout metadata_t metadata) {
    apply {
      
      session_info_lookup.apply(intr_global, intr_p4, hdr, metadata);
      config_verify.apply(intr_global, intr_p4, hdr, metadata);
      nacl_lookup.apply(intr_global, intr_p4, hdr, metadata);
      update_checksums.apply(intr_global, intr_p4, hdr, metadata);
      egress_inter_pipe.apply(intr_global, intr_p4, hdr, metadata);
      conntrack_state_update.apply(intr_global, intr_p4, hdr, metadata);
      p4e_statistics.apply(intr_global, intr_p4, hdr, metadata);
    }
}
IngressPipelineP4(AthenaIngressParser(), AthenaIngressPipeline(),
                  AthenaIngressDeparser()) ingress_pipeline;
EgressPipelineP4(AthenaEgressParser(), AthenaEgressPipeline(),
                 AthenaEgressDeparser()) egress_pipeline;
CapriP4(ingress_pipeline, egress_pipeline) main;
