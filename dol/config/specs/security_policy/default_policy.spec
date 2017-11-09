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
#                    - svc:
#                        proto    : 17
#                        dst_port : 111
#                        alg      : SUN_RPC
#                    - svc:
#                        proto    : 6
#                        dst_port : 111
#                        alg      : SUN_RPC
            - rule:
                action :  ALLOW
                log    :  True
                svcs:
                    - svc:
                        proto    : 0
                        dst_port : 0


    - policy:
        id         : SEC_POLICY_ALLOW2
        default    : False
        in_rules:
            - rule:
                action :  ALLOW
                log    :  True
                svcs:
                    - svc:
                        proto    : 0
                        dst_port : 0

    - policy:
        id         :  SEC_POLICY_ALLOW3
        default    :  False
        in_rules:
            - rule:
                action :  ALLOW
                log    :  True
                svcs:
                    - svc:
                        proto    : 0
                        dst_port : 0

    - policy:
        id          : SEC_POLICY_ALLOW4
        default     : False
        in_rules:
            - rule:
                action :  ALLOW
                log    :  True
                svcs:
                    - svc:
                        proto    : 0
                        dst_port : 0

    - policy:
        id          : SEC_POLICY_ALLOW5
        default     : False
        in_rules:
            - rule:
                action  : ALLOW
                log     : True
                svcs:
                    - svc:
                        proto    : 0
                        dst_port : 0

    - policy:
        id         : SEC_POLICY_ALLOW6
        default    : False
        in_rules:
            - rule:
                action :  ALLOW
                log    :  True
                svcs:
                    - svc:
                        proto    : 0
                        dst_port : 0

