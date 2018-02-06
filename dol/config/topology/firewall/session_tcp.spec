# Flow generation configuration template.

meta:
    id: SESSION_TCP_FIREWALL

proto: tcp

entries:
    - entry:
        label: firewall
        fte: True
        tracking: False
        responder:
            port : const/42001
        initiator:
            port : const/42002

    - entry:
        label   : perf
        tracking: True
        count   : 64
        initiator:
            port : range/41001/49999
            flow_info:
                action  : allow
                state   : established
            tracking_info:
                tcp_seq_num     : const/1000
                tcp_ack_num     : const/2000
                tcp_win_sz      : const/8192
                tcp_win_scale   : const/1
                tcp_mss         : const/1460
        responder:
            port : range/41001/49999
            flow_info:
                action  : allow
                state   : established
            tracking_info:
                tcp_seq_num     : const/2000
                tcp_ack_num     : const/1000
                tcp_win_sz      : const/8192
                tcp_win_scale   : const/1
                tcp_mss         : const/1460

