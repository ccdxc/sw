# TX and RX NMPR definitions
meta:
    id: PROXY_SERVICE

entries:
    - entry:
        name        : SERVICE_LIF
        type        : TCP_PROXY
    - entry:
        name        : SERVICE_LIF
        type        : TLS_PROXY
    - entry:
        name        : SERVICE_LIF
        type        : IPSEC_PROXY
    - entry:
        name        : SERVICE_LIF
        type        : APP_REDIR
    - entry:
        name        : SERVICE_LIF
        type        : P4PT
    - entry:
        name        : SERVICE_LIF
        type        : APP_REDIR_PROXY_TCP
    - entry:
        name        : SERVICE_LIF
        type        : APP_REDIR_SPAN
    - entry:
        name        : SERVICE_LIF
        type        : APP_REDIR_PROXY_TCP_SPAN
#    - entry:
#        name        : SERVICE_LIF
#        type        : CPU_PROXY
