# Flow generation configuration template.
meta:
    id: SESSION_MAC_AGENT_UP2UP

proto: None
entries:
    - entry:
        label: networking
        initiator:
            ethertype: 0xCCC0
            flow_info:
                eg_qos:    
                    cos_rw  : const/1
                    cos     : const/5
        responder:
            ethertype: 0xCCC0

