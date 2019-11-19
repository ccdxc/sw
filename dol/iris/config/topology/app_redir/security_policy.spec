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
                    - svc:
                        proto    : 6
                        dst_port : 1433
                    - svc:
                        proto    : 6
                        dst_port : 1521
                    - svc:
                        proto    : 17
                        dst_port : 111
                    - svc:
                        proto    : 17
                        dst_port : 1048
                    - svc:
                        proto    : 17
                        dst_port : 2049
                apps:
                    - app : "MYSQL"
                    - app : "DNS"
                    - app : "HTTP"
                    - app : "MONGODB"
                    - app : "CASSANDRA"
                    - app : "POSTGRESQL"
                    - app : "MSSQL"
                    - app : "ORACLE"
                    - app : "NFS"
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
                        dst_port : 19696
                    - svc:
                        proto    : 6
                        dst_port : 23761
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
