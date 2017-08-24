# Flow generation configuration template.
meta:
    id: SESSION_TCP

proto: tcp

entries:
    - entry:
        label: networking
        tracking: False
        timestamp: False
        responder: 
            port : const/42001
        initiator: 
            port : const/42002

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

    - entry:
        label: span
        tracking: False
        timestamp: False
        responder: 
            port : const/81
            span : ref://store/objects/id=SpanSession0002
        initiator: 
            port : const/41000
            span : ref://store/objects/id=SpanSession0002

    - entry:
        label: tcp-proxy
        tracking: False
        timestamp: False
        responder: 
            port : const/80
        initiator: 
            port : const/47273

