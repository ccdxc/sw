rxlpm2_res_handler:
    /* Disable further stages for LPM1 */
    phvwr            p.p4_to_rxdma_lpm2_enable, FALSE
    /* Is this pass 0? */
    seq              c1, k.lpm_metadata_recirc_count, 0
    /* If so, write DPORT ClassId for SACL0, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_dport_classid0, res_reg
    /* Is this pass 1? */
    seq              c1, k.lpm_metadata_recirc_count, 1
    /* If so, write DIP ClassId for SACL0, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_dip_classid0, res_reg
    /* Is this pass 2? */
    seq              c1, k.lpm_metadata_recirc_count, 2
    /* If so, write DPORT ClassId for SACL1, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_dport_classid1, res_reg
    /* Is this pass 3? */
    seq              c1, k.lpm_metadata_recirc_count, 3
    /* If so, write DIP ClassId for SACL1, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_dip_classid1, res_reg
    /* Is this pass 4? */
    seq              c1, k.lpm_metadata_recirc_count, 4
    /* If so, write DPORT ClassId for SACL2, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_dport_classid2, res_reg
    /* Is this pass 5? */
    seq              c1, k.lpm_metadata_recirc_count, 5
    /* If so, write DIP ClassId for SACL2, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_dip_classid2, res_reg
    /* Is this pass 6? */
    seq              c1, k.lpm_metadata_recirc_count, 6
    /* If so, write DPORT ClassId for SACL3, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_dport_classid3, res_reg
    /* Is this pass 7? */
    seq              c1, k.lpm_metadata_recirc_count, 7
    /* If so, write DIP ClassId for SACL3, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_dip_classid3, res_reg
    /* Is this pass 8? */
    seq              c1, k.lpm_metadata_recirc_count, 8
    /* If so, write DPORT ClassId for SACL4, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_dport_classid4, res_reg
    /* Is this pass 9? */
    seq              c1, k.lpm_metadata_recirc_count, 9
    /* If so, write DIP ClassId for SACL4, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_dip_classid4, res_reg
    /* Is this pass 10? */
    seq              c1, k.lpm_metadata_recirc_count, 10
    /* If so, write DPORT ClassId for SACL5, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_dport_classid5, res_reg
    /* Is this pass 11? */
    seq              c1, k.lpm_metadata_recirc_count, 11
    /* If so, write DIP ClassId for SACL5, Return*/
    phvwr.c1.e       p.rx_to_tx_hdr_dip_classid5, res_reg
    nop.e
    nop
