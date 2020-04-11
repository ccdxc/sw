/******************************************************************************/
/* Egr pipeline                                                                */
/******************************************************************************/

control Policers(inout cap_phv_intr_global_h capri_intrinsic,
            inout cap_phv_intr_p4_h intr_p4,
            inout headers hdr,
            inout metadata_t metadata) {


    @name(".policer_pps")
    action policer_pps_a(bit<1>     entry_valid,
                             bit<16>    pkt_rate,
                             bit<1>     rlimit_en,
                             bit<16>    rlimit_prof,
                             bit<1>     color_aware,
                             bit<1>     rsvd,
                             bit<1>     axi_wr_pend,
                             bit<16>    burst,
                             bit<16>    rate,
                             bit<40>    tbkt) {
    if ((entry_valid == TRUE) && ((tbkt >> 39) == 1)) {
      DROP_PACKET_INGRESS(P4I_DROP_POLICER);
		   
    }
   }

    @name(".policer_pps")
    table policer_pps {
        key = {
            metadata.cntrl.throttle_bw1_id : exact;
        }
        actions  = {
            policer_pps_a;
        }
        size =  POLICER_PPS_SIZE;
        policer = two_color;
        placement = SRAM;
        stage = 5;
    }



    @name(".policer_bw")
      action policer_bw_a(bit<1>     entry_valid,
                             bit<16>    pkt_rate,
                             bit<1>     rlimit_en,
                             bit<16>    rlimit_prof,
                             bit<1>     color_aware,
                             bit<1>     rsvd,
                             bit<1>     axi_wr_pend,
                             bit<16>    burst,
                             bit<16>    rate,
                             bit<40>    tbkt) {
    if ((entry_valid == TRUE) && ((tbkt >> 39) == 1)) {
      DROP_PACKET_INGRESS(P4I_DROP_POLICER);

    }
   }

    @name(".policer_bw")
    table policer_bw {
        key = {
            metadata.cntrl.throttle_bw2_id : exact;
        }
        actions  = {
            policer_bw_a;
        }
        size =  POLICER_BW_SIZE;
        policer = two_color;
        placement = SRAM;
        stage = 5;
    }

    apply {
      if(metadata.cntrl.throttle_bw1_id_valid == TRUE) {
        policer_pps.apply();
      }
      if(metadata.cntrl.throttle_bw2_id_valid == TRUE) {
        policer_bw.apply();
      }
    }
}


