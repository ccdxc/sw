# Qos class configuration spec
meta:
    id: QOS_CLASS

entries:
    - entry:
        group: DEFAULT
        
    - entry:
        group: USER_DEFINED_1
        marking:
            dot1q_pcp_en : True
            ip_dscp_en : True

    - entry:
        group: USER_DEFINED_2
        marking:
            dot1q_pcp_en : True
            ip_dscp_en : False 

    - entry:
        group: INTERNAL_RX_PROXY_NO_DROP
        
    - entry:
        group: INTERNAL_RX_PROXY_DROP

    - entry:
        group: INTERNAL_TX_PROXY_NO_DROP
        
    - entry:
        group: INTERNAL_TX_PROXY_DROP

    - entry:
        group: INTERNAL_ADMIN
