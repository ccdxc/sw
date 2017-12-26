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
                    - svc:
                        proto    : 17
                        dst_port : 53
                    - svc:
                        proto    : 6
                        dst_port : 12380
                    - svc:
                        proto    : 6
                        dst_port : 27017
                apps:
                    - app : "MYSQL"
                    - app : "DNS"
                    - app : "HTTP"
                    - app : "MONGO"

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
                    - svc:
                        proto    : 17
                        dst_port : 53
                    - svc:
                        proto    : 6
                        dst_port : 12380
                    - svc:
                        proto    : 6
                        dst_port : 27017
                apps:
                    - app : "MYSQL"
                    - app : "DNS"
                    - app : "HTTP"
                    - app : "MONGO"

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
