/*****************************************************************************/
/* Session tracking                                                          */
/*****************************************************************************/
header_type tcp_scratch_metadata_t {
    fields{
        iflow_tcp_state                 : 4;
        iflow_tcp_seq_num               : 32;
        iflow_tcp_ack_num               : 32;
        iflow_tcp_mss                   : 16;
        iflow_tcp_win_size              : 16;
        iflow_tcp_win_scale             : 4;
        iflow_tcp_exceptions            : 16;
        iflow_tcp_win_scale_option_sent : 1;
        rflow_tcp_flags                 : 8;
        rflow_tcp_state                 : 4;
        rflow_tcp_seq_num               : 32;
        rflow_tcp_ack_num               : 32;
        rflow_tcp_mss                   : 16;
        rflow_tcp_win_size              : 16;
        rflow_tcp_win_scale             : 4;
        rflow_tcp_exceptions            : 16;
        tcp_flags                       : 8;
        tcp_mss                         : 16;
        tcp_data_len                    : 16;
        tcp_rcvr_win_size               : 32;
        tcp_seq_num_hi                  : 32;
        tcp_exceptions                  : 16;
    }
}

@pragma scratch_metadata
metadata tcp_scratch_metadata_t tcp_scratch;

action session_track_info(iflow_tcp_state, iflow_tcp_seq_num, iflow_tcp_ack_num,
                          iflow_tcp_win_size, iflow_tcp_win_scale,
                          iflow_tcp_mss, iflow_tcp_exceptions,
                          iflow_tcp_win_scale_option_sent,
                          rflow_tcp_state, rflow_tcp_seq_num, rflow_tcp_ack_num,
                          rflow_tcp_win_size, rflow_tcp_win_scale,
                          rflow_tcp_mss, rflow_tcp_exceptions) {

    modify_field(tcp_scratch.iflow_tcp_state, iflow_tcp_state);
    modify_field(tcp_scratch.iflow_tcp_seq_num, iflow_tcp_seq_num);
    modify_field(tcp_scratch.iflow_tcp_ack_num, iflow_tcp_ack_num);
    modify_field(tcp_scratch.iflow_tcp_win_size, iflow_tcp_win_size);
    modify_field(tcp_scratch.iflow_tcp_win_scale, iflow_tcp_win_scale);
    modify_field(tcp_scratch.iflow_tcp_mss, iflow_tcp_mss);
    modify_field(tcp_scratch.iflow_tcp_exceptions, iflow_tcp_exceptions);
    modify_field(tcp_scratch.iflow_tcp_win_scale_option_sent,
                 iflow_tcp_win_scale_option_sent);
    modify_field(tcp_scratch.rflow_tcp_state, rflow_tcp_state);
    modify_field(tcp_scratch.rflow_tcp_seq_num, rflow_tcp_seq_num);
    modify_field(tcp_scratch.rflow_tcp_ack_num, rflow_tcp_ack_num);
    modify_field(tcp_scratch.rflow_tcp_win_size, rflow_tcp_win_size);
    modify_field(tcp_scratch.rflow_tcp_win_scale, rflow_tcp_win_scale);
    modify_field(tcp_scratch.rflow_tcp_mss, rflow_tcp_mss);
    modify_field(tcp_scratch.rflow_tcp_exceptions, rflow_tcp_exceptions);

    modify_field(tcp_scratch.tcp_exceptions, 0);
    if (ipv4_1.valid == TRUE) {
        modify_field(tcp_scratch.tcp_data_len,
                     (ipv4_1.totalLen - ((ipv4_1.ihl + tcp.dataOffset) * 4)));
    } else {
        modify_field(tcp_scratch.tcp_data_len,
                     (ipv6_1.payloadLen - (tcp.dataOffset) * 4));
    }
    modify_field(tcp_scratch.tcp_seq_num_hi,
                 (tcp.seqNo + tcp_scratch.tcp_data_len - 1));

    if (p4e_i2e.flow_role == TCP_FLOW_INITIATOR) {
        modify_field(tcp_scratch.tcp_rcvr_win_size,
                     (tcp_scratch.rflow_tcp_win_size <<
                      tcp_scratch.rflow_tcp_win_scale));
        modify_field(tcp_scratch.tcp_mss, tcp_scratch.rflow_tcp_mss);

        if ((tcp_scratch.iflow_tcp_state == FLOW_STATE_ESTABLISHED) and
            (tcp_scratch.rflow_tcp_state == FLOW_STATE_ESTABLISHED) and
            ((tcp.flags == TCP_FLAG_ACK) or
             (tcp.flags == (TCP_FLAG_ACK|TCP_FLAG_PSH))) and
            (tcp_scratch.tcp_rcvr_win_size != 0) and
            (((tcp_scratch.tcp_data_len != 0) and
              (tcp.seqNo == tcp_scratch.iflow_tcp_seq_num) and
              (tcp_scratch.tcp_seq_num_hi <
               (tcp_scratch.rflow_tcp_ack_num + tcp_scratch.tcp_rcvr_win_size))) or
             ((tcp_scratch.tcp_data_len == 0) and
              ((tcp_scratch.rflow_tcp_ack_num - 1) <= tcp.seqNo) and
              (tcp.seqNo <
               (tcp_scratch.rflow_tcp_ack_num + tcp_scratch.tcp_rcvr_win_size))))) {

            if ((tcp.ackNo > tcp_scratch.iflow_tcp_ack_num) and
                (tcp.ackNo <= tcp_scratch.rflow_tcp_seq_num)) {
                modify_field(tcp_scratch.iflow_tcp_ack_num, tcp.ackNo);
                modify_field(tcp_scratch.iflow_tcp_win_size, tcp.window);
            }
            if (tcp_scratch.tcp_seq_num_hi > tcp_scratch.iflow_tcp_seq_num) {
                // Only if we are seeing a new high sequence number than we
                // saved already
                modify_field(tcp_scratch.iflow_tcp_seq_num,
                             tcp_scratch.tcp_seq_num_hi + 1);
            }
            // Exit the action routine
        }

        // If SYN or FIN is set in a packet then we can increment the
        // tcp_seq_num_hi by 1 here
        if (((tcp.flags & TCP_FLAG_SYN) == TCP_FLAG_SYN) or
            ((tcp.flags & TCP_FLAG_FIN) == TCP_FLAG_FIN)) {
            add_to_field(tcp_scratch.tcp_seq_num_hi, 1);
        }

        if ((tcp_scratch.tcp_data_len != 0) and
            (tcp_scratch.tcp_rcvr_win_size != 0)) {
            if ((tcp.seqNo == tcp_scratch.iflow_tcp_seq_num) and
                (tcp_scratch.tcp_seq_num_hi <
                 (tcp_scratch.rflow_tcp_ack_num + tcp_scratch.tcp_rcvr_win_size))) {
                // goto INITIATOR_TCP_STATE_TRANSITION:
            }
            if ((tcp.seqNo >= tcp_scratch.rflow_tcp_ack_num) and
                (tcp_scratch.tcp_seq_num_hi <
                 (tcp_scratch.rflow_tcp_ack_num + tcp_scratch.tcp_rcvr_win_size))) {
                // Non-overlapping within the window but not the next one we
                // are expecting, possible re-ordering of segments happens
                // in between
                bit_or(tcp_scratch.tcp_exceptions,
                       tcp_scratch.tcp_exceptions, TCP_PACKET_REORDER);
                // goto INITIATOR_TCP_STATE_TRANSITION:
            }
            if ((tcp.seqNo < tcp_scratch.rflow_tcp_ack_num) and
                (tcp_scratch.tcp_seq_num_hi < tcp_scratch.rflow_tcp_ack_num)) {
                // Full retransmit of packet we have seen before, still
                // acceptable. We shouldn't drop this packet as we might have
                // seen the ack but  sender might not have rcvd the ack as it
                // could be dropped after us. We will have to update the ack #
                // and window fromt the packet.
                bit_or(tcp_scratch.tcp_exceptions,
                       tcp_scratch.tcp_exceptions, TCP_FULL_REXMIT);
                // Update state (ack and window) and exit.
                // goto INITIATOR_TCP_SESSION_UPDATE:
            }

            if ((tcp.seqNo < tcp_scratch.rflow_tcp_ack_num) and
                (tcp_scratch.tcp_seq_num_hi <
                 (tcp_scratch.rflow_tcp_ack_num + tcp_scratch.tcp_rcvr_win_size))) {
                // Left side overlap, still acceptable
                bit_or(tcp_scratch.tcp_exceptions,
                       tcp_scratch.tcp_exceptions, TCP_PARTIAL_OVERLAP);
                // goto INITIATOR_TCP_STATE_TRANSITION:
            }

            // Either segment is overlapping on the right or fully outside the
            // window to the right or engulfing the whole window size and
            // exceeding the window size - drop
            bit_or(tcp_scratch.tcp_exceptions,
                   tcp_scratch.tcp_exceptions, TCP_OUT_OF_WINDOW);
            egress_drop(P4E_DROP_TCP_OUT_OF_WINDOW);
            // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
       }

        // This case is also handled in best packet case but it assumes that
        // no flags other than ACK+PSH were set.
        if ((tcp_scratch.tcp_data_len == 0) and
            (tcp_scratch.tcp_rcvr_win_size != 0)) {
            // Initiator flow can also be installed in INIT state after flow
            // miss is sent to VPP which will reinject the packet and this time
            // we will hit the flow that is installed. This is to avoid tcp
            // session tracking to see the VPP injected packet as a SYN REXMIT.
            if ((tcp_scratch.iflow_tcp_state == FLOW_STATE_INIT) and
                (tcp.flags == TCP_FLAG_SYN)) {
                modify_field(tcp_scratch.iflow_tcp_state,
                             FLOW_STATE_TCP_SYN_RCVD);
                if (control_metadata.tcp_option_ws_valid == TRUE) {
                    modify_field(tcp_scratch.iflow_tcp_win_scale_option_sent, 1);
                    modify_field(tcp_scratch.iflow_tcp_win_scale,
                                 tcp_option_ws.value);
                }
                if (control_metadata.tcp_option_mss_valid == TRUE) {
                    modify_field(tcp_scratch.iflow_tcp_mss,
                                 tcp_option_mss.value);
                }
                // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
            }
            if ((tcp_scratch.iflow_tcp_state == FLOW_STATE_TCP_SYN_RCVD) and
                ((tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_ACK)) == TCP_FLAG_SYN) and
                (tcp.seqNo + 1 == tcp_scratch.iflow_tcp_seq_num)) {
                bit_or(tcp_scratch.tcp_exceptions,
                       tcp_scratch.tcp_exceptions, TCP_SYN_REXMIT);
                // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
            }
            // Before we do the RFC check, we need to make sure that we received
            // ack from the responder side, only then rflow_tcp_ack_num will be
            // valid
            if (tcp_scratch.rflow_tcp_state <= FLOW_STATE_TCP_SYN_RCVD) {
                // This will be hit when simultanoes open is done and initiator
                // is responding with an ACK to responder
                // goto INITIATOR_TCP_STATE_TRANSITION:
            }
            if (((tcp_scratch.rflow_tcp_ack_num -1) <= tcp.seqNo) and
                (tcp.seqNo < (tcp_scratch.rflow_tcp_ack_num +
                              tcp_scratch.tcp_rcvr_win_size))) {
                // goto INITIATOR_TCP_STATE_TRANSITION:
            } else {
                bit_or(tcp_scratch.tcp_exceptions,
                       tcp_scratch.tcp_exceptions, TCP_OUT_OF_WINDOW);
                egress_drop(P4E_DROP_TCP_OUT_OF_WINDOW);
               // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
            }
        }

        // if segment length is > 0 but receiver window size is 0
        // this is not actually a security breach, may be the sender
        // didn't receive the window advertisement with size 0 yet, simply
        // retransmit, but increment a counter
        if ((tcp_scratch.tcp_data_len != 0) and
            (tcp_scratch.tcp_rcvr_win_size == 0)) {
            bit_or(tcp_scratch.tcp_exceptions,
                   tcp_scratch.tcp_exceptions, TCP_WIN_ZERO_DROP);
            egress_drop(P4E_DROP_TCP_WIN_ZERO);
            // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
        }

        // Per RFC: if both segment length and receive window are zero,
        // packets that are allowed are empty ACK or RST or URG pkts
        if ((tcp_scratch.tcp_data_len == 0) and
            (tcp_scratch.tcp_rcvr_win_size == 0)) {
            // SYN Packets will not hit this case because in pre established
            // state window size will be non-zero
            if ((tcp.seqNo == tcp_scratch.rflow_tcp_ack_num) or
                (tcp.seqNo == (tcp_scratch.rflow_tcp_ack_num - 1))) {
                // goto INITIATOR_TCP_STATE_TRANSITION:
            } else {
                bit_or(tcp_scratch.tcp_exceptions,
                       tcp_scratch.tcp_exceptions, TCP_OUT_OF_WINDOW);
                egress_drop(P4E_DROP_TCP_OUT_OF_WINDOW);
                // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
            }
        }

        // Default case is to proceed to TCP State Transitions.
        // All invalid sequence number should have been taken care above this.

        // INITIATOR_TCP_STATE_TRANSITION:
        // TCP connection state tracking

        // A reordered frame, retransmit or partial window frome in established
        // state with no SYN, FIN and RST should go through fast processing
        if ((tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_FIN|TCP_FLAG_RST) == 0) and
            (tcp_scratch.iflow_tcp_state == FLOW_STATE_ESTABLISHED)) {
            // goto INITIATOR_TCP_SESSION_UPDATE
        }

        // RST Handling
        if ((tcp.flags & TCP_FLAG_RST) == TCP_FLAG_RST) {
            if (tcp_scratch.iflow_tcp_state != FLOW_STATE_RESET) {
                // Goal here is to reset the connection only if the sequence
                // number matches the next expected sequence number from the
                // point of view of the receiver who is receiving the reset.
                // For all other resets with sequence number in window, we
                // assuming that the challenge ack rfc is supported and we
                // eventually get the reset with right sequence number.
                // RFC 5961
                if (tcp_scratch.rflow_tcp_state < FLOW_STATE_TCP_ACK_RCVD) {
                    modify_field(tcp_scratch.iflow_tcp_state, FLOW_STATE_RESET);
                    modify_field(tcp_scratch.rflow_tcp_state, FLOW_STATE_RESET);
                    // Mark the packet to redirect to CPU.
                    // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
                }
                // If we really want to add a knob to support end hosts which
                // support and which don't support challenge ack then we can
                // add here and handle accordingly
                // RFC 5961
                if (tcp.seqNo == tcp_scratch.rflow_tcp_ack_num) {
                    modify_field(tcp_scratch.iflow_tcp_state, FLOW_STATE_RESET);
                    modify_field(tcp_scratch.rflow_tcp_state, FLOW_STATE_RESET);
                    // Mark the packet to redirect to CPU.
                    // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
                } else {
                    // We will let it go and let the Challege ack take care of
                    // it to send the reset with the right seq number
                    // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
                }
            } else {
                // Duplicate RST, Let it go.
                // Not adding a exception here for now.
            }
            // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
        }

        if ((tcp_scratch.iflow_tcp_state >= FLOW_STATE_ESTABLISHED) and
            ((tcp.flags & TCP_FLAG_SYN) == TCP_FLAG_SYN)) {
            bit_or(tcp_scratch.tcp_exceptions,
                   tcp_scratch.tcp_exceptions, TCP_UNEXPECTED_PKT);
            egress_drop(P4E_DROP_TCP_UNEXPECTED_PKT);
            // INITIATOR_TCP_SESSION_STATE_INFO_EXIT:
        }

        // If we receive FIN in any pre-established state,
        if (tcp_scratch.iflow_tcp_state < FLOW_STATE_ESTABLISHED) {
            if ((tcp.flags & TCP_FLAG_FIN) == TCP_FLAG_FIN) {
                bit_or(tcp_scratch.tcp_exceptions,
                       tcp_scratch.tcp_exceptions, TCP_UNEXPECTED_PKT);
                egress_drop(P4E_DROP_TCP_UNEXPECTED_PKT);
                // INITIATOR_TCP_SESSION_STATE_INFO_EXIT:
            }
        }

        // we will do a switch case based on iflow_tcp_state for reducing the
        // number of instructions executed.
        if (tcp_scratch.iflow_tcp_state == FLOW_STATE_TCP_SYN_RCVD)  {
            // INITIATOR_TCP_SESSION_STATE_INFO_EXIT:
        }

        if (tcp_scratch.iflow_tcp_state == FLOW_STATE_TCP_SYN_RCVD)  {
            if ((tcp_scratch.rflow_tcp_state == FLOW_STATE_TCP_SYN_ACK_RCVD) and
                ((tcp.flags & TCP_FLAG_ACK) == TCP_FLAG_ACK) and
                (tcp.ackNo == tcp_scratch.rflow_tcp_seq_num)) {
                modify_field(tcp_scratch.iflow_tcp_state, FLOW_STATE_ESTABLISHED);
                modify_field(tcp_scratch.rflow_tcp_state, FLOW_STATE_ESTABLISHED);
                modify_field(tcp_scratch.iflow_tcp_seq_num,
                             tcp_scratch.tcp_seq_num_hi + 1);
                modify_field(tcp_scratch.iflow_tcp_ack_num, tcp.ackNo);
                modify_field(tcp_scratch.iflow_tcp_win_size, tcp.window);
                // We could have Data along with this ACK.
                // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
            }
            if ((tcp_scratch.rflow_tcp_state == FLOW_STATE_TCP_SYN_RCVD) and
                ((tcp.flags & TCP_FLAG_ACK) == TCP_FLAG_ACK) and
                (tcp.ackNo == tcp_scratch.rflow_tcp_seq_num)) {
                modify_field(tcp_scratch.iflow_tcp_state,
                             FLOW_STATE_TCP_SYN_ACK_RCVD);
                modify_field(tcp_scratch.iflow_tcp_ack_num, tcp.ackNo);
                modify_field(tcp_scratch.iflow_tcp_win_size, tcp.window);
                // goto INITIATOR_TCP_SESSION_UPDATE
            }
            // I don't think we need to handle FIN here right ?
            // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
        }

        if (tcp_scratch.iflow_tcp_state == FLOW_STATE_TCP_SYN_ACK_RCVD)  {
            // Do we have to handle FIN in this case ?
            // If initator is in this state then no packet from initiator
            // will the state to established, It should be a ACK packet
            // from responder which can move to established state which is
            // is handled later in responder section.
            // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
        }

        if (tcp_scratch.iflow_tcp_state == FLOW_STATE_TCP_ACK_RCVD)  {
            // Not a valid state for initiator
            // Should we drop the packet and have a default exception.
            // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
        }

        if (tcp_scratch.iflow_tcp_state == FLOW_STATE_ESTABLISHED)  {
            if (((tcp.flags & TCP_FLAG_FIN) == TCP_FLAG_FIN) and
                (tcp_scratch.rflow_tcp_state != FLOW_STATE_FIN_RCVD)) {
                modify_field (tcp_scratch.iflow_tcp_state, FLOW_STATE_FIN_RCVD);
                // goto INITIATOR_TCP_SESSION_UPDATE
            }
            if (((tcp.flags & TCP_FLAG_FIN) == TCP_FLAG_FIN) and
                (tcp_scratch.rflow_tcp_state == FLOW_STATE_FIN_RCVD)) {
                modify_field (tcp_scratch.iflow_tcp_state, FLOW_STATE_BIDIR_FIN_RCVD);
                modify_field (tcp_scratch.rflow_tcp_state, FLOW_STATE_BIDIR_FIN_RCVD);
                // goto INITIATOR_TCP_SESSION_UPDATE
            }
        }

        if ((tcp_scratch.iflow_tcp_state == FLOW_STATE_FIN_RCVD) or
            (tcp_scratch.iflow_tcp_state == FLOW_STATE_BIDIR_FIN_RCVD)) {
            // We should drop any data beyond the FIN sequence number which
            // we should have cached as part of iflow_tcp_seq_num
            // Below check cannot be >= as all the subsequent packets after FIN
            // which can be ACK only packets will have a sequence number of
            // FIN sequence + 1 which is what we saved in rflow_tcp_seq_num
            if (tcp_scratch.tcp_seq_num_hi >= tcp_scratch.iflow_tcp_seq_num) {
                bit_or(tcp_scratch.tcp_exceptions,
                       tcp_scratch.tcp_exceptions, TCP_DATA_AFTER_FIN);
                egress_drop(P4E_DROP_TCP_DATA_AFTER_FIN);
                // INITIATOR_TCP_SESSION_STATE_INFO_EXIT:
            }
            // goto INITIATOR_TCP_SESSION_UPDATE
        }

        if (tcp_scratch.iflow_tcp_state == FLOW_STATE_RESET) {
            // No point letting any packets go through other than RESET itself
            // in case the earlier one is lost. This is handled as part of RST
            // FLAG Handling.

            // Here we need to handle any other packets that are coming
            // when we are reset state. we can just drop them
            bit_or(tcp_scratch.tcp_exceptions,
                   tcp_scratch.tcp_exceptions, TCP_NON_RST_PKT_AFTER_RST);
            egress_drop(P4E_DROP_TCP_NON_RST_PKT_AFTER_RST);
            // INITIATOR_TCP_SESSION_STATE_INFO_EXIT:
        }

        // INITIATOR_TCP_SESSION_UPDATE:
        // Making sure that some random ack number is not updated as this
        // can completely throw us off
        if ((tcp.ackNo > tcp_scratch.iflow_tcp_ack_num) and
            (tcp.ackNo <= tcp_scratch.rflow_tcp_seq_num)) {
            modify_field(tcp_scratch.iflow_tcp_ack_num, tcp.ackNo);
            modify_field(tcp_scratch.iflow_tcp_win_size, tcp.window);
        }

        if (tcp_scratch.tcp_seq_num_hi >= tcp_scratch.iflow_tcp_seq_num) {
            // Only if we are seeing a new high sequence number than we saved
            // already
            modify_field(tcp_scratch.iflow_tcp_seq_num,
                         tcp_scratch.tcp_seq_num_hi + 1);
        }

        // INITIATOR_TCP_SESSION_STATE_INFO_EXIT:
        bit_or(tcp_scratch.iflow_tcp_exceptions,
               tcp_scratch.iflow_tcp_exceptions, tcp_scratch.tcp_exceptions);
        // Exit
    } else {
        modify_field(tcp_scratch.tcp_rcvr_win_size,
                     (tcp_scratch.iflow_tcp_win_size <<
                      tcp_scratch.iflow_tcp_win_scale));
        modify_field(tcp_scratch.tcp_mss, tcp_scratch.iflow_tcp_mss);
        modify_field(tcp_scratch.tcp_seq_num_hi,
                     (tcp.seqNo + tcp_scratch.tcp_data_len - 1));

        if ((tcp_scratch.iflow_tcp_state == FLOW_STATE_ESTABLISHED) and
            (tcp_scratch.rflow_tcp_state == FLOW_STATE_ESTABLISHED) and
            ((tcp.flags == TCP_FLAG_ACK) or
             (tcp.flags == (TCP_FLAG_ACK|TCP_FLAG_PSH))) and
            (tcp_scratch.tcp_rcvr_win_size != 0) and
            (((tcp_scratch.tcp_data_len != 0) and
              (tcp.seqNo == tcp_scratch.rflow_tcp_seq_num) and
              (tcp_scratch.tcp_seq_num_hi <
               (tcp_scratch.iflow_tcp_ack_num + tcp_scratch.tcp_rcvr_win_size))) or
             ((tcp_scratch.tcp_data_len == 0) and
              ((tcp_scratch.iflow_tcp_ack_num - 1) <= tcp.seqNo) and
              (tcp.seqNo <
               (tcp_scratch.iflow_tcp_ack_num + tcp_scratch.tcp_rcvr_win_size))))) {

            if ((tcp.ackNo > tcp_scratch.rflow_tcp_ack_num) and
                (tcp.ackNo <= tcp_scratch.iflow_tcp_seq_num)) {
                modify_field(tcp_scratch.iflow_tcp_ack_num, tcp.ackNo);
                modify_field(tcp_scratch.iflow_tcp_win_size, tcp.window);
            }
            if (tcp_scratch.tcp_seq_num_hi >  tcp_scratch.rflow_tcp_seq_num) {
                // Only if we are seeing a new high sequence number than we
                // saved already
                modify_field(tcp_scratch.rflow_tcp_seq_num,
                             tcp_scratch.tcp_seq_num_hi + 1);
            }
            // Exit the action routine
        }

        // If SYN or FIN is set in a packet then we can increment the
        // tcp_seq_num_hi by 1 here. This will make sure that all seq number
        // checks take into account and do correct boundary checks like out of
        // window.
        if (((tcp.flags & TCP_FLAG_FIN) == TCP_FLAG_FIN) or
            ((tcp.flags & TCP_FLAG_SYN) == TCP_FLAG_SYN)) {
            add_to_field (tcp_scratch.tcp_seq_num_hi, 1);
        }

        if ((tcp_scratch.tcp_data_len != 0) and
            (tcp_scratch.tcp_rcvr_win_size != 0)) {
            if ((tcp.seqNo == tcp_scratch.rflow_tcp_seq_num) and
                (tcp_scratch.tcp_seq_num_hi <
                 (tcp_scratch.iflow_tcp_ack_num + tcp_scratch.tcp_rcvr_win_size))) {
                // goto RESPONDER_TCP_STATE_TRANSITION:
            }
            if ((tcp.seqNo < tcp_scratch.iflow_tcp_ack_num) and
                (tcp_scratch.tcp_seq_num_hi < tcp_scratch.iflow_tcp_ack_num)) {
                // full retransmit of packet we have seen before, still
                // acceptable We shouldn't drop this packet as we might have
                // seen the ack but sender might not have rcvd the ack as it
                // could be dropped after us. We will have to update the ack #
                // and window fromt the packet.
                bit_or(tcp_scratch.tcp_exceptions,
                       tcp_scratch.tcp_exceptions, TCP_FULL_REXMIT);
                // Update state (ack and window) and exit.
                // goto RESPONDER_TCP_SESSION_UPDATE:
            }

            if ((tcp.seqNo < tcp_scratch.iflow_tcp_ack_num) and
                (tcp_scratch.tcp_seq_num_hi <
                 (tcp_scratch.iflow_tcp_ack_num + tcp_scratch.tcp_rcvr_win_size))) {
                // left side overlap, still acceptable
                bit_or(tcp_scratch.tcp_exceptions,
                       tcp_scratch.tcp_exceptions, TCP_PARTIAL_OVERLAP);
                // goto RESPONDER_TCP_STATE_TRANSITION:
            }

            if ((tcp.seqNo >= tcp_scratch.iflow_tcp_ack_num) and
                (tcp_scratch.tcp_seq_num_hi <
                 (tcp_scratch.iflow_tcp_ack_num + tcp_scratch.tcp_rcvr_win_size))) {
                // non-overlapping within the window but not the next one we
                // are expecting, possible re-ordering of segments happens in
                // between
                bit_or(tcp_scratch.tcp_exceptions,
                       tcp_scratch.tcp_exceptions, TCP_PACKET_REORDER);
                // goto RESPONDER_TCP_STATE_TRANSITION:
            }

            // either segment is overlapping on the right or fully outside the
            // window to the right or engulfing the whole window size and
            // exceeding the window sz - drop
            bit_or(tcp_scratch.tcp_exceptions,
                   tcp_scratch.tcp_exceptions, TCP_OUT_OF_WINDOW);
            egress_drop(P4E_DROP_TCP_OUT_OF_WINDOW);
            // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
        }

        // This case is also handled in best packet case but it assumed that
        // no flags other than ACK+PSH were set.
        if ((tcp_scratch.tcp_data_len == 0) and
            (tcp_scratch.tcp_rcvr_win_size != 0)) {
            // How do we let the first packet coming from Responder go to TCP
            // State Transiton logic. Only valid value at this point in the
            // session is initiator TCP Seq Number.
            if ((tcp_scratch.rflow_tcp_state < FLOW_STATE_ESTABLISHED) and
                (tcp_scratch.iflow_tcp_state < FLOW_STATE_ESTABLISHED)) {
                if (((tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_ACK)) ==
                     TCP_FLAG_SYN|TCP_FLAG_ACK) and
                     (tcp.ackNo == tcp_scratch.iflow_tcp_seq_num)) {
                    // This is the gneuine SYN_ACK from responder. Most common
                    // case when opening a new connection
                    // goto RESPONDER_TCP_STATE_TRANSITION:
                }
                // From RFC: 793
                // In all states except SYN-SENT, all reset (RST) segments are
                // validated by checking their SEQ-fields. A reset is valid if
                // its sequence number is in the window.
                // In the SYN-SENT state (a RST received in response to an
                // initial SYN), the RST is acceptable if the ACK field
                // acknowledges the SYN.
                if (((tcp.flags & TCP_FLAG_RST) == TCP_FLAG_RST) and
                    (tcp.ackNo != tcp_scratch.iflow_tcp_seq_num)) {
                    bit_or(tcp_scratch.tcp_exceptions,
                           tcp_scratch.tcp_exceptions,
                           TCP_RST_WITH_INVALID_ACK_NUM);
                    egress_drop(P4E_DROP_TCP_RST_WITH_INVALID_ACK_NUM);
                }
                if (((tcp.flags & TCP_FLAG_ACK) == TCP_FLAG_ACK) and
                    (tcp.ackNo == tcp_scratch.iflow_tcp_seq_num)) {
                    // This will be true for any genuine packet that is sent by
                    // responder in response to initiator SYN. This could a
                    // packet FIM or RST.
                    // goto RESPONDER_TCP_STATE_TRANSITION:
                }
                if ((tcp.flags & TCP_FLAG_SYN) == TCP_FLAG_SYN) {
                    // If this is a simultaneous open then only SYN flag will
                    // be set and there is nothing much to verify.
                    // goto RESPONDER_TCP_STATE_TRANSITION:
                }
                // For rest of the cases, we will fall through the RFC check
                // for seq numbers and any illegitimate packet will be dropped.
            }

            if (((tcp_scratch.iflow_tcp_ack_num - 1) <= tcp.seqNo) and
                (tcp.seqNo <
                 (tcp_scratch.iflow_tcp_ack_num + tcp_scratch.tcp_rcvr_win_size))) {
                // goto RESPONDER_TCP_STATE_TRANSITION:
            } else {
                bit_or(tcp_scratch.tcp_exceptions,
                       tcp_scratch.tcp_exceptions, TCP_OUT_OF_WINDOW);
                egress_drop(P4E_DROP_TCP_OUT_OF_WINDOW);
                // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
            }
        }

        // if segment length is > 0 but receiver window size is 0
        // this is not actually a security breach, may be the sender
        // didn't receive the window advertisement with size 0 yet, simply
        // retransmit, but increment a counter
        if ((tcp_scratch.tcp_data_len != 0) and
            (tcp_scratch.tcp_rcvr_win_size == 0)) {
            bit_or(tcp_scratch.tcp_exceptions,
                   tcp_scratch.tcp_exceptions, TCP_WIN_ZERO_DROP);
            egress_drop(P4E_DROP_TCP_WIN_ZERO);
            // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
        }

        // Per RFC: if both segment length and receive window are zero, only
        // packets that are allowed are empty ACK or RST or URG pkts
        // How is FIN prevented here ?
        if ((tcp_scratch.tcp_data_len == 0) and
            (tcp_scratch.tcp_rcvr_win_size == 0)) {
            // SYN Packets will not hit this case because in pre established
            // state window size will be non-zero
            if ((tcp.seqNo == tcp_scratch.iflow_tcp_ack_num) or
                (tcp.seqNo == (tcp_scratch.iflow_tcp_ack_num - 1))) {
                // goto RESPONDER_TCP_STATE_TRANSITION:
            } else {
                bit_or(tcp_scratch.tcp_exceptions,
                       tcp_scratch.tcp_exceptions, TCP_OUT_OF_WINDOW);
                egress_drop(P4E_DROP_TCP_OUT_OF_WINDOW);
                // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
            }
        }

        // Default case is to proceed to TCP State Transitions.
        // All invalid sequence number should have been taken care above this.

        // RESPONDER_TCP_STATE_TRANSITION:
        // TCP connection state tracking

        // A reordered frame, retransmit or partial window frome in established
        // state with no SYN, FIN and RST should go through fast processing
        if (((tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_FIN|TCP_FLAG_RST)) == 0) and
            (tcp_scratch.iflow_tcp_state == FLOW_STATE_ESTABLISHED)) {
            // goto INITIATOR_TCP_SESSION_UPDATE
        }
        // RST Handling
        if ((tcp.flags & TCP_FLAG_RST) == TCP_FLAG_RST) {
            if (tcp_scratch.rflow_tcp_state != FLOW_STATE_RESET) {
                // Goal here is to reset the connection only if the sequence
                // number matches the next expected sequence number from the
                // point of view of the receiver who is receiving the reset.
                // For all other resets with sequence number in window, we
                // assuming that the challenge ack rfc is supported and we
                // eventually get the reset with right sequence number.
                // RFC 5961
                if (tcp_scratch.iflow_tcp_state < FLOW_STATE_TCP_ACK_RCVD) {
                    modify_field(tcp_scratch.iflow_tcp_state, FLOW_STATE_RESET);
                    modify_field(tcp_scratch.rflow_tcp_state, FLOW_STATE_RESET);
                    // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
                }
                // If we really want to add a knob to support end hosts which
                // support and which don't support challenge ack then we can
                // add here and handle accordingly
                // RFC 5961
                if (tcp.seqNo == tcp_scratch.iflow_tcp_ack_num) {
                    modify_field(tcp_scratch.iflow_tcp_state, FLOW_STATE_RESET);
                    modify_field(tcp_scratch.rflow_tcp_state, FLOW_STATE_RESET);
                    // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
                } else {
                    // We will let it go and let the Challege ack take care of
                    // it to send the reset with the right seq number
                    // goto INITIATOR_TCP_SESSION_STATE_INFO_EXIT
                }
            } else {
                // Duplicate RST, Let it go.
                // Not adding a exception here for now.
            }
            // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
        }
        if ((tcp_scratch.rflow_tcp_state >= FLOW_STATE_ESTABLISHED) and
            (tcp.flags & TCP_FLAG_SYN == TCP_FLAG_SYN)) {
            bit_or(tcp_scratch.tcp_exceptions,
                   tcp_scratch.tcp_exceptions, TCP_UNEXPECTED_PKT);
            egress_drop(P4E_DROP_TCP_UNEXPECTED_PKT);
            // RESPONDER_TCP_SESSION_STATE_INFO_EXIT:
        }

        // If we receive FIN in any pre-established state,
        if (tcp_scratch.rflow_tcp_state < FLOW_STATE_ESTABLISHED) {
            if ((tcp.flags & TCP_FLAG_FIN) == TCP_FLAG_FIN) {
                bit_or(tcp_scratch.tcp_exceptions,
                       tcp_scratch.tcp_exceptions, TCP_UNEXPECTED_PKT);
                egress_drop(P4E_DROP_TCP_UNEXPECTED_PKT);
                // RESPONDER_TCP_SESSION_STATE_INFO_EXIT:
            }
        }

        // we will do a switch case based on rflow_tcp_state for reducing the
        // number of instructions executed.
        if (tcp_scratch.rflow_tcp_state == FLOW_STATE_INIT) {
            // Initator flow has to be in SYN_RCVD state.
            if ((tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_ACK)) ==
                (TCP_FLAG_SYN|TCP_FLAG_ACK) and
                (tcp.ackNo == tcp_scratch.iflow_tcp_seq_num)) {
                modify_field(tcp_scratch.rflow_tcp_state,
                             FLOW_STATE_TCP_SYN_ACK_RCVD);
                if ((tcp_scratch.iflow_tcp_win_scale_option_sent == TRUE) and
                    (control_metadata.tcp_option_ws_valid == TRUE)) {
                    modify_field (tcp_scratch.rflow_tcp_win_scale,
                                  tcp_option_ws.value);
                } else {
                    modify_field (tcp_scratch.iflow_tcp_win_scale, 0);
                }
                if (control_metadata.tcp_option_mss_valid == TRUE) {
                    modify_field (tcp_scratch.rflow_tcp_mss,
                                  tcp_option_mss.value);
                }
                // We will have to update the responder flows seq and ack
                // directly here as the commmon code will check for greater
                // than and it might not do the right stuff as the initial
                // values are zero when the session entry is programmed.
                modify_field(tcp_scratch.rflow_tcp_seq_num,
                             tcp_scratch.tcp_seq_num_hi + 1);
                modify_field(tcp_scratch.rflow_tcp_ack_num, tcp.ackNo);
                modify_field(tcp_scratch.rflow_tcp_win_size, tcp.window);
                // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
            }
            if (((tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_ACK)) == TCP_FLAG_ACK) and
                (tcp.ackNo == tcp_scratch.iflow_tcp_seq_num)) {
                bit_or(tcp_scratch.tcp_exceptions,
                       tcp_scratch.tcp_exceptions, TCP_SPLIT_HANDSHAKE_DETECTED);
                modify_field(tcp_scratch.rflow_tcp_state,
                             FLOW_STATE_TCP_ACK_RCVD);
                modify_field(tcp_scratch.rflow_tcp_ack_num, tcp.ackNo);
                modify_field(tcp_scratch.rflow_tcp_win_size, tcp.window);
                // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
            }
            if (((tcp.flags & (TCP_FLAG_SYN|TCP_FLAG_ACK)) == TCP_FLAG_SYN) and
                (tcp.ackNo == 0)) {
                // This is simultaneous open case, for now we are treating it
                // under the bucket of split handshake.
                bit_or(tcp_scratch.tcp_exceptions,
                       tcp_scratch.tcp_exceptions, TCP_SPLIT_HANDSHAKE_DETECTED);
                modify_field(tcp_scratch.rflow_tcp_state, FLOW_STATE_TCP_SYN_RCVD);
                if ((tcp_scratch.iflow_tcp_win_scale_option_sent == TRUE) and
                    (control_metadata.tcp_option_ws_valid == TRUE)) {
                    modify_field (tcp_scratch.rflow_tcp_win_scale,
                                  tcp_option_ws.value);
                } else {
                    modify_field (tcp_scratch.iflow_tcp_win_scale, 0);
                }
                if (control_metadata.tcp_option_mss_valid == TRUE) {
                    modify_field (tcp_scratch.rflow_tcp_mss,
                                  tcp_option_mss.value);
                }
                // We will have to update the responder flows seq and ack
                // directly here as the commmon code will check for greater
                // than and it might not do the right stuff as the initial
                // values are zero when the session entry is programmed.
                // Not updating window as ACK bit is not set.
                modify_field(tcp_scratch.rflow_tcp_seq_num,
                             tcp_scratch.tcp_seq_num_hi + 1);
                // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
            }
            // Drop any other packet for now.
            bit_or(tcp_scratch.tcp_exceptions,
                   tcp_scratch.tcp_exceptions, TCP_INVALID_RESPONDER_FIRST_PKT);
            egress_drop(P4E_DROP_TCP_INVALID_RESPONDER_FIRST_PKT);
        }

        if (tcp_scratch.rflow_tcp_state == FLOW_STATE_TCP_SYN_RCVD)  {
            if ((tcp_scratch.iflow_tcp_state == FLOW_STATE_TCP_SYN_RCVD) and
                ((tcp.flags & TCP_FLAG_ACK) == TCP_FLAG_ACK) and
                (tcp.ackNo == tcp_scratch.iflow_tcp_seq_num)) {
                modify_field(tcp_scratch.rflow_tcp_state,
                             FLOW_STATE_TCP_SYN_ACK_RCVD);
                // goto RESPONDER_TCP_SESSION_UPDATE
            }
            if ((tcp_scratch.iflow_tcp_state == FLOW_STATE_TCP_SYN_ACK_RCVD) and
                ((tcp.flags & TCP_FLAG_ACK) == TCP_FLAG_ACK) and
                (tcp.ackNo == tcp_scratch.iflow_tcp_seq_num)) {
                modify_field(tcp_scratch.iflow_tcp_state,
                             FLOW_STATE_ESTABLISHED);
                modify_field(tcp_scratch.rflow_tcp_state,
                             FLOW_STATE_ESTABLISHED);
                // We could have Data along with this ACK.
                // goto RESPONDER_TCP_SESSION_UPDATE
            }
            // I don't think we need to handle FIN here right ?
            // We are not dropping the packet as there could be SYN Retransmit
            // If we have to strict, we can check for SYN also and drop rest
            // of the packets.
            // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
        }

        if (tcp_scratch.rflow_tcp_state == FLOW_STATE_TCP_ACK_RCVD)  {
            // iflow_tcp_state should be in FLOW_STATE_TCP_SYN_RCVD only
            if ((tcp.flags & TCP_FLAG_SYN) == TCP_FLAG_SYN) {
                modify_field(tcp_scratch.rflow_tcp_state,
                             FLOW_STATE_TCP_SYN_RCVD);
                if ((tcp_scratch.iflow_tcp_win_scale_option_sent == TRUE) and
                    (control_metadata.tcp_option_ws_valid == TRUE)) {
                    modify_field (tcp_scratch.rflow_tcp_win_scale,
                                  tcp_option_ws.value);
                } else {
                    modify_field (tcp_scratch.iflow_tcp_win_scale, 0);
                }
                if (control_metadata.tcp_option_mss_valid == TRUE) {
                    modify_field (tcp_scratch.rflow_tcp_mss, tcp_option_mss.value);
                }
                // We will have to update the responder flows seq and ack
                // directly here as the We will have to update the responder
                // flows seq and ack directly here as the  commmon code will
                // check for greater than and it might not do the right stuff
                // as the initial values are zero when the session entry is
                // programmed.
                // Not updating window as ACK bit is not set.
                modify_field(tcp_scratch.rflow_tcp_seq_num,
                             tcp_scratch.tcp_seq_num_hi + 1);
                // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
            }
            // We are not dropping the packet as there could be ACKs Retransmit
            // If we have to strict, we can check for SYN also and drop rest
            // of the packets.
            // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
        }

        if (tcp_scratch.rflow_tcp_state == FLOW_STATE_TCP_SYN_ACK_RCVD)  {
            // No packet received from Responder will help to move out of this
            // state.
            // Do we have to handle FIN in this case ?
            // goto RESPONDER_TCP_SESSION_STATE_INFO_EXIT
        }

        if (tcp_scratch.rflow_tcp_state == FLOW_STATE_ESTABLISHED)  {
            if (((tcp.flags & TCP_FLAG_FIN) == TCP_FLAG_FIN) and
                (tcp_scratch.iflow_tcp_state != FLOW_STATE_FIN_RCVD)) {
                modify_field (tcp_scratch.rflow_tcp_state, FLOW_STATE_FIN_RCVD);
                // goto RESPONDER_TCP_SESSION_UPDATE
            }
            if (((tcp.flags & TCP_FLAG_FIN) == TCP_FLAG_FIN) and
                (tcp_scratch.iflow_tcp_state == FLOW_STATE_FIN_RCVD)) {
                modify_field (tcp_scratch.rflow_tcp_state,
                              FLOW_STATE_BIDIR_FIN_RCVD);
                modify_field (tcp_scratch.iflow_tcp_state,
                              FLOW_STATE_BIDIR_FIN_RCVD);
                // goto RESPONDER_TCP_SESSION_UPDATE
            }
        }

        if ((tcp_scratch.rflow_tcp_state == FLOW_STATE_FIN_RCVD) or
            (tcp_scratch.rflow_tcp_state == FLOW_STATE_BIDIR_FIN_RCVD)) {
            // We should drop any data beyond the FIN Sequence number whiich
            // we should have cached as part of rflow_tcp_seq_num
            // Below check cannot be >= as all the subsequent packets after FIN
            // which can be ACK only packets will have a sequence number of
            // FIN sequence + 1 which is what we saved in rflow_tcp_seq_num
            if (tcp_scratch.tcp_seq_num_hi >= tcp_scratch.rflow_tcp_seq_num) {
                bit_or(tcp_scratch.tcp_exceptions,
                       tcp_scratch.tcp_exceptions, TCP_DATA_AFTER_FIN);
                egress_drop(P4E_DROP_TCP_DATA_AFTER_FIN);
                // RESPONDER_TCP_SESSION_STATE_INFO_EXIT:
            }
            // goto RESPONDER_TCP_SESSION_UPDATE
        }

        if (tcp_scratch.rflow_tcp_state == FLOW_STATE_RESET) {
            // No point letting any packets go through other than RESET itself
            // in case the earlier one is lost. This is handled as part of RST
            // FLAG Handling.

            // Here we need to handle any other packets that are coming
            // when we are reset state. we can just drop them
            bit_or(tcp_scratch.tcp_exceptions,
                   tcp_scratch.tcp_exceptions, TCP_NON_RST_PKT_AFTER_RST);
            egress_drop(P4E_DROP_TCP_NON_RST_PKT_AFTER_RST);
            // RESPONDER_TCP_SESSION_STATE_INFO_EXIT:
        }

        // RESPONDER_TCP_SESSION_UPDATE:
        // Making sure that some random ack number is not updated as this
        // can completely throw us off
        if ((tcp.ackNo > tcp_scratch.rflow_tcp_ack_num) and
            (tcp.ackNo <= tcp_scratch.iflow_tcp_seq_num)) {
            modify_field(tcp_scratch.rflow_tcp_ack_num, tcp.ackNo);
            modify_field(tcp_scratch.rflow_tcp_win_size, tcp.window);
        }

        if (tcp_scratch.tcp_seq_num_hi >=  tcp_scratch.rflow_tcp_seq_num) {
            // Only if we are seeing a new high sequence number than we
            // saved already
            modify_field(tcp_scratch.rflow_tcp_seq_num,
                         tcp_scratch.tcp_seq_num_hi + 1);
        }

        // RESPONDER_TCP_SESSION_STATE_INFO_EXIT:
        bit_or(tcp_scratch.rflow_tcp_exceptions,
               tcp_scratch.rflow_tcp_exceptions, tcp_scratch.tcp_exceptions);
        // Exit
    }
}

@pragma stage 2
@pragma hbm_table
@pragma table_write
@pragma index_table
@pragma capi_bitfields_struct
table session_track {
    reads {
        p4e_i2e.session_id  : exact;
    }
    actions {
        session_track_info;
    }
    size : SESSION_TABLE_SIZE;
}
