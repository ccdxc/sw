# Flow generation configuration template.
meta:
    id: SESSION_TCP

proto: tcp

entries:
    - entry:
        label: networking
        responder: 
            port : const/80
            flow_info:
                action  : allow
                state   : established
            tracking_info:
                seq_num     : 1000
                ack_num     : 5000
                win_sz      : 8192
                win_scale   : 1
                mss         : 16384
        initiator: 
            port : const/42000
            flow_info:
                action  : allow
                state   : established
            tracking_info:
                seq_num     : 1000
                ack_num     : 5000
                win_sz      : 8192
                win_scale   : 1
                mss         : 16384

    - entry:
        label: span
        responder: 
            port : const/81
            span : ref://store/objects/id=SpanSession0002
        initiator: 
            port : const/42001
            span : ref://store/objects/id=SpanSession0002

    - entry:
        label: tcp-proxy
        responder: 
            port : const/80
        initiator: 
            port : const/47273

