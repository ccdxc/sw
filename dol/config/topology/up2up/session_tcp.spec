# Flow generation configuration template.

meta:
    id: SESSION_TCP_UP2UP

proto: tcp

entries:
    - entry:
        label: networking
        tracking: False
        responder:
            port : const/42001
        initiator:
            port : const/42002



    - entry:
        label: gft_drop
        tracking: False
        responder: 
            port : const/22222
            flow_info:
                action: drop
        initiator: 
            port : const/33333
            flow_info:
                action: drop

    - entry:
        label: firewall
        tracking: True
        timestamp: True
        responder: 
            port : const/80
            flow_info:
                action  : allow
                state   : established
            tracking_info:
                tcp_seq_num     : const/1000
                tcp_ack_num     : const/5000
                tcp_win_sz      : const/8192
                tcp_win_scale   : const/1
                tcp_mss         : const/1460
        initiator: 
            port : const/40000
            flow_info:
                action  : allow
                state   : established
            tracking_info:
                tcp_seq_num     : const/5000
                tcp_ack_num     : const/1000
                tcp_win_sz      : const/8192
                tcp_win_scale   : const/1
                tcp_mss         : const/1460

