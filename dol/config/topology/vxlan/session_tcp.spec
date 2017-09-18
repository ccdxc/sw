# Flow generation configuration template.
meta:
    id: VXLANTOPO_SESSION_TCP

proto: tcp

entries:
    - entry:
        label: networking
        tracking: False
        timestamp: False
        responder: 
            port : const/1
            flow_info:
                eg_qos:    
                    cos_rw  : const/1
                    cos     : const/4
                    dscp_rw : const/1
                    dscp    : const/1
        initiator: 
            port : const/4
            flow_info:
                eg_qos:    
                    cos_rw  : const/1
                    cos     : const/3
                    dscp_rw : const/1
                    dscp    : const/2

    - entry:
        label: gft_drop
        tracking: False
        timestamp: False
        responder: 
            port : const/12345
            flow_info:
                action  : drop
        initiator: 
            port : const/54321
            flow_info:
                action  : drop
