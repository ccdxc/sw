meta:
    id: SECURITY_POLICY_PROXY

policies:
    - policy:
        id         : SEC_POLICY_ALLOW11
        default    : True
        in_rules:
            - rule:
                action :  ALLOW
                log    :  True
                svcs:
                    - svc:
                        proto    : 6
                        dst_port : 3307
                apps:
                    - app : "MYSQL"

    - policy:
        id         : SEC_POLICY_ALLOW12
        default    : False
        in_rules:
            - rule:
                action :  ALLOW
                log    :  True
                svcs:
                    - svc:
                        proto    : 6
                        dst_port : 3307
                apps:
                    - app : "MYSQL"
    - policy:
        id         : SEC_POLICY_ALLOW13
        default    : False
        in_rules:
            - rule:
                action :  ALLOW
                log    :  True
                svcs:
                    - svc:
                        proto    : 0
                        dst_port : 0
