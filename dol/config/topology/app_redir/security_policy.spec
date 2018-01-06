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
                        proto    : 6
                        dst_port : 3306
                    - svc:
                        proto    : 17
                        dst_port : 53
                    - svc:
                        proto    : 6
                        dst_port : 12380
                    - svc:
                        proto    : 6
                        dst_port : 27017
                    - svc:
                        proto    : 6
                        dst_port : 9042
                    - svc:
                        proto    : 6
                        dst_port : 5432
                apps:
                    - app : "MYSQL"
                    - app : "DNS"
                    - app : "HTTP"
                    - app : "MONGO"
                    - app : "CASSANDRA"
                    - app : "POSTGRES"
            - rule:
                action :  ALLOW
                log    :  True
                svcs:
                    - svc:
                        proto    : 6
                        dst_port : 3307
                    - svc:
                        proto    : 6
                        dst_port : 2501
                    - svc:
                        proto    : 6
                        dst_port : 19694
                    - svc:
                        proto    : 6
                        dst_port : 23763
                    - svc:
                        proto    : 6
                        dst_port : 23765
                    - svc:
                        proto    : 6
                        dst_port : 23767
            - rule:
                action :  DENY
                log    :  True
                svcs:
                    - svc:
                        proto    : 0
                        dst_port : 0


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
                        dst_port : 3306
                    - svc:
                        proto    : 17
                        dst_port : 53
                    - svc:
                        proto    : 6
                        dst_port : 12380
                    - svc:
                        proto    : 6
                        dst_port : 27017
                    - svc:
                        proto    : 6
                        dst_port : 9042
                    - svc:
                        proto    : 6
                        dst_port : 5432
                apps:
                    - app : "MYSQL"
                    - app : "DNS"
                    - app : "HTTP"
                    - app : "MONGO"
                    - app : "CASSANDRA"
                    - app : "POSTGRES"
            - rule:
                action :  ALLOW
                log    :  True
                svcs:
                    - svc:
                        proto    : 6
                        dst_port : 3307
                    - svc:
                        proto    : 6
                        dst_port : 2501
                    - svc:
                        proto    : 6
                        dst_port : 19694
                    - svc:
                        proto    : 6
                        dst_port : 23763
                    - svc:
                        proto    : 6
                        dst_port : 23765
                    - svc:
                        proto    : 6
                        dst_port : 23767
            - rule:
                action :  DENY
                log    :  True
                svcs:
                    - svc:
                        proto    : 0
                        dst_port : 0

