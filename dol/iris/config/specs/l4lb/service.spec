# Flow generation configuration template.
meta:
    id: L4LB

entries:
    - entry:
        label       : nat
        proto       : tcp
        port        : const/80
        mode        : nat
        backends:
            - port    : const/8080
              count   : const/2
              remote  : False

    - entry:
        label       : nat
        proto       : tcp
        port        : const/80
        mode        : nat
        backends:
            - port    : const/8080
              count   : const/2
              remote  : True
              tnnled  : False

    - entry:
        label       : nat
        proto       : tcp
        port        : const/80
        mode        : nat
        backends:
            - port    : const/8080
              count   : const/2
              remote  : True
              tnnled  : True

    - entry:
        label       : twice-nat
        proto       : tcp
        port        : const/443
        mode        : twice_nat
        snat_ips    : ipstep/201.0.0.1/0.0.0.1/100
        snat_ipv6s  : ipv6step/3101::0001/0000::0001/100
        snat_ports  : range/49152/65535
        backends:
            - port  : const/8443
              count : const/2
              remote: False

    - entry:
        label       : twice-nat
        proto       : tcp
        port        : const/443
        mode        : twice_nat
        snat_ips    : ipstep/201.0.0.1/0.0.0.1/100
        snat_ipv6s  : ipv6step/3101::0001/0000::0001/100
        snat_ports  : range/49152/65535
        backends:
            - port  : const/8443
              count : const/2
              remote: True
              tnnled: False

    - entry:
        label       : twice-nat
        proto       : tcp
        port        : const/443
        mode        : twice_nat
        snat_ips    : ipstep/201.0.0.1/0.0.0.1/100
        snat_ipv6s  : ipv6step/3101::0001/0000::0001/100
        snat_ports  : range/49152/65535
        backends:
            - port  : const/8443
              count : const/2
              remote: True
              tnnled: True 

    - entry:
        label       : nat
        proto       : udp
        port        : const/21
        mode        : nat
        backends:
            - port  : const/2021
              count : const/2
              remote: False

    - entry:
        label       : nat
        proto       : udp
        port        : const/21
        mode        : nat
        backends:
            - port  : const/2021
              count : const/2
              remote: True
              tnnled: False

    - entry:
        label       : nat
        proto       : udp
        port        : const/21
        mode        : nat
        backends:
            - port  : const/2021
              count : const/2
              remote: True
              tnnled: True 

    - entry:
        label       : twice-nat
        proto       : udp
        port        : const/22
        mode        : twice_nat
        snat_ips    : ipstep/201.1.0.1/0.0.0.1/100
        snat_ipv6s  : ipv6step/3101::0001/0000::0001/100
        snat_ports  : range/49152/65535
        backends:
            - port  : const/2022
              count : const/2
              remote: False

    - entry:
        label       : twice-nat
        proto       : udp
        port        : const/22
        mode        : twice_nat
        snat_ips    : ipstep/201.1.0.1/0.0.0.1/100
        snat_ipv6s  : ipv6step/3101::0001/0000::0001/100
        snat_ports  : range/49152/65535
        backends:
            - port  : const/2022
              count : const/2
              remote: True
              tnnled: False

    - entry:
        label       : twice-nat
        proto       : udp
        port        : const/22
        mode        : twice_nat
        snat_ips    : ipstep/201.1.0.1/0.0.0.1/100
        snat_ipv6s  : ipv6step/3101::0001/0000::0001/100
        snat_ports  : range/49152/65535
        backends:
            - port  : const/2022
              count : const/2
              remote: True
              tnnled: True

    # IP Only Backends
    - entry:
        label       : nat
        proto       : tcp
        port        : const/81
        mode        : nat
        backends:
            - port  : const/0
              count : const/2
              remote: False

    - entry:
        label       : nat
        proto       : tcp
        port        : const/81
        mode        : nat
        backends:
            - port  : const/0
              count : const/2
              remote: True
              tnnled: False

    - entry:
        label       : nat
        proto       : tcp
        port        : const/81
        mode        : nat
        backends:
            - port  : const/0
              count : const/2
              remote: True
              tnnled: True

    - entry:
        label       : twice-nat
        proto       : tcp
        port        : const/443
        mode        : twice_nat
        snat_ips    : ipstep/201.0.0.1/0.0.0.1/100
        snat_ipv6s  : ipv6step/3101::0001/0000::0001/100
        snat_ports  : None
        backends:
            - port  : const/0
              count : const/2
              remote: False

    - entry:
        label       : twice-nat
        proto       : tcp
        port        : const/443
        mode        : twice_nat
        snat_ips    : ipstep/201.0.0.1/0.0.0.1/100
        snat_ipv6s  : ipv6step/3101::0001/0000::0001/100
        snat_ports  : None
        backends:
            - port  : const/0
              count : const/2
              remote: True
              tnnled: False

    - entry:
        label       : twice-nat
        proto       : tcp
        port        : const/443
        mode        : twice_nat
        snat_ips    : ipstep/201.0.0.1/0.0.0.1/100
        snat_ipv6s  : ipv6step/3101::0001/0000::0001/100
        snat_ports  : None
        backends:
            - port  : const/0
              count : const/2
              remote: True
              tnnled: True

    - entry:
        label       : nat
        proto       : udp
        port        : const/23
        mode        : nat
        backends:
            - port  : const/0
              count : const/2
              remote: False
    - entry:
        label       : nat
        proto       : udp
        port        : const/23
        mode        : nat
        backends:
            - port  : const/0
              count : const/2
              remote: True
              tnnled: False

    - entry:
        label       : nat
        proto       : udp
        port        : const/23
        mode        : nat
        backends:
            - port  : const/0
              count : const/2
              remote: True
              tnnled: True

    - entry:
        label       : twice-nat
        proto       : udp
        port        : const/22
        mode        : twice_nat
        snat_ips    : ipstep/201.1.0.1/0.0.0.1/100
        snat_ipv6s  : ipv6step/3101::0001/0000::0001/100
        snat_ports  : None
        backends:
            - port  : const/0
              count : const/2
              remote: False

    - entry:
        label       : twice-nat
        proto       : udp
        port        : const/22
        mode        : twice_nat
        snat_ips    : ipstep/201.1.0.1/0.0.0.1/100
        snat_ipv6s  : ipv6step/3101::0001/0000::0001/100
        snat_ports  : None
        backends:
            - port  : const/0
              count : const/2
              remote: True
              tnnled: False

    - entry:
        label       : twice-nat
        proto       : udp
        port        : const/22
        mode        : twice_nat
        snat_ips    : ipstep/201.1.0.1/0.0.0.1/100
        snat_ipv6s  : ipv6step/3101::0001/0000::0001/100
        snat_ports  : None
        backends:
            - port  : const/0
              count : const/2
              remote: True
              tnnled: True
