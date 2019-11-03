rxlpm1_res_handler:
    /* Disable further stages for LPM2 */
    phvwr            p.p4_to_rxdma_lpm1_enable, FALSE
    /* Is this pass 0? */
    seq              c1, k.lpm_metadata_recirc_count, 0
    /* If so, write SPORT ClassId for SACL0, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_sport_classid0, res_reg
    /* Is this pass 1? */
    seq              c1, k.lpm_metadata_recirc_count, 1
    /* If so, write SIP ClassId for SACL0, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_sip_classid0, res_reg
    /* Is this pass 2? */
    seq              c1, k.lpm_metadata_recirc_count, 2
    /* If so, write SPORT ClassId for SACL1, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_sport_classid1, res_reg
    /* Is this pass 3? */
    seq              c1, k.lpm_metadata_recirc_count, 3
    /* If so, write SIP ClassId for SACL1, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_sip_classid1, res_reg
    /* Is this pass 4? */
    seq              c1, k.lpm_metadata_recirc_count, 4
    /* If so, write SPORT ClassId for SACL2, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_sport_classid2, res_reg
    /* Is this pass 5? */
    seq              c1, k.lpm_metadata_recirc_count, 5
    /* If so, write SIP ClassId for SACL2, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_sip_classid2, res_reg
    /* Is this pass 6? */
    seq              c1, k.lpm_metadata_recirc_count, 6
    /* If so, write SPORT ClassId for SACL3, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_sport_classid3, res_reg
    /* Is this pass 7? */
    seq              c1, k.lpm_metadata_recirc_count, 7
    /* If so, write SIP ClassId for SACL3, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_sip_classid3, res_reg
    /* Is this pass 8? */
    seq              c1, k.lpm_metadata_recirc_count, 8
    /* If so, write SPORT ClassId for SACL4, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_sport_classid4, res_reg
    /* Is this pass 9? */
    seq              c1, k.lpm_metadata_recirc_count, 9
    /* If so, write SIP ClassId for SACL4, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_sip_classid4, res_reg
    /* Is this pass 10? */
    seq              c1, k.lpm_metadata_recirc_count, 10
    /* If so, write SPORT ClassId for SACL5, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_sport_classid5, res_reg
    /* Is this pass 11? */
    seq              c1, k.lpm_metadata_recirc_count, 11
    /* If so, write SIP ClassId for SACL5, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_sip_classid5, res_reg
    nop.e
    nop
