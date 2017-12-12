meta:
    id: SECURITY_POLICY_APP_REDIR

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
                        proto    : 0
                        dst_port : 0
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
                        proto    : 0
                        dst_port : 0
                apps:
                    - app : "DNS"
