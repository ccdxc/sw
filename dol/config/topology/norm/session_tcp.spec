# Flow generation configuration template.

meta:
    id: SESSION_TCP_NORM

proto: tcp

entries:
    - entry:
        label: norm
        tracking: False
        timestamp: False
        responder:
            port : const/42001
        initiator:
            port : const/42002

    - entry:
        label: norm_split_handshake
        tracking: True
        timestamp: False
        responder: 
            port : const/40001
            flow_info:
                action  : allow
                state   : init
            tracking_info:
                tcp_ack_num     : const/0
        initiator: 
            port : const/40002
            flow_info:
                action  : allow
                state   : syn_rcvd
            tracking_info:
                tcp_seq_num     : const/1000
                tcp_win_sz      : const/8192

    - entry:
        label: norm_data_len_gt_mss
        tracking: True
        timestamp: False
        responder: 
            port : const/40003
            flow_info:
                action  : allow
                state   : established
            tracking_info:
                tcp_seq_num     : const/1000
                tcp_ack_num     : const/5000
                tcp_win_sz      : const/8192
                tcp_win_scale   : const/1
                tcp_mss         : const/1000
        initiator: 
            port : const/40004
            flow_info:
                action  : allow
                state   : established
            tracking_info:
                tcp_seq_num     : const/5000
                tcp_ack_num     : const/1000
                tcp_win_sz      : const/8192
                tcp_win_scale   : const/1
                tcp_mss         : const/1000

    - entry:
        label: norm_data_len_gt_wsize
        tracking: True
        timestamp: False
        responder: 
            port : const/40005
            flow_info:
                action  : allow
                state   : established
            tracking_info:
                tcp_seq_num     : const/900
                tcp_ack_num     : const/5000
                tcp_win_sz      : const/1000
                tcp_win_scale   : const/0
                tcp_mss         : const/1500
        initiator: 
            port : const/40006
            flow_info:
                action  : allow
                state   : established
            tracking_info:
                tcp_seq_num     : const/4000
                tcp_ack_num     : const/1000
                tcp_win_sz      : const/400
                tcp_win_scale   : const/0
                tcp_mss         : const/1500

    - entry:
        label: norm_ts
        tracking: True
        timestamp: True
        responder: 
            port : const/40007
            flow_info:
                action  : allow
                state   : established
            tracking_info:
                tcp_seq_num     : const/1000
                tcp_ack_num     : const/5000
                tcp_win_sz      : const/1000
                tcp_win_scale   : const/1
                tcp_mss         : const/1500
        initiator: 
            port : const/40008
            flow_info:
                action  : allow
                state   : established
            tracking_info:
                tcp_seq_num     : const/5000
                tcp_ack_num     : const/1000
                tcp_win_sz      : const/1000
                tcp_win_scale   : const/1
                tcp_mss         : const/1500
    
    - entry:
        label: norm_non_ts
        tracking: True
        timestamp: False
        responder: 
            port : const/40009
            flow_info:
                action  : allow
                state   : established
            tracking_info:
                tcp_seq_num     : const/1000
                tcp_ack_num     : const/5000
                tcp_win_sz      : const/1000
                tcp_win_scale   : const/1
                tcp_mss         : const/1500
        initiator: 
            port : const/40010
            flow_info:
                action  : allow
                state   : established
            tracking_info:
                tcp_seq_num     : const/5000
                tcp_ack_num     : const/1000
                tcp_win_sz      : const/1000
                tcp_win_scale   : const/1
                tcp_mss         : const/1500

    - entry:
        label: norm_tcp_non_syn
        tracking: False
        timestamp: False
        fte: True
        responder:
            port : const/40011
        initiator:
            port : const/40012

