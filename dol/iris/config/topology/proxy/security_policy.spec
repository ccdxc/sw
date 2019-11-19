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
                        dst_port : 9080
                apps:
                    - app : "HTTP"
            - rule:
                action :  ALLOW
                log    :  True
                svcs:
                    - svc:
                        proto    : 0
