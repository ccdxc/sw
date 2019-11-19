meta:
    id: SECURITY_POLICY_DEFAULT

policies:
    - policy:
        id          : SEC_POLICY_ALLOW1
        default     : True
        in_rules:
            - rule:
                action  :  ALLOW
                log     :  True
                svcs:
                    - svc:
                        proto    : 17
                        dst_port : 69
                        alg      : TFTP
                    - svc:
                        proto    : 17
                        dst_port : 111
                        alg      : SUN_RPC
                    - svc:
                        proto    : 6
                        dst_port : 111
                        alg      : SUN_RPC
                    - svc:
                        proto    : 17
                        dst_port : 135
                        alg      : MSFT_RPC
                    - svc:
                        proto    : 6
                        dst_port : 135
                        alg      : MSFT_RPC
                    - svc:
                        proto    : 6
                        dst_port : 21
                        alg      : FTP
            - rule:
                action :  ALLOW
                log    :  True
                svcs:
                    - svc:
                        proto    : 0
