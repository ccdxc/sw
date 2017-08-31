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
            - port  : const/8080
              count : const/0
              remote: False
            - port  : const/8080
              count : const/2
              remote: True

    - entry:
        label       : twice-nat
        proto       : tcp
        port        : const/443
        mode        : twice_nat
        snat_ips    : ipstep/201.0.0.1/0.0.0.1/16
        snat_ports  : range/49152/65535
        backends:
            - port  : const/8443
              count : const/0
              remote: False
            - port  : const/8443
              count : const/2
              remote: True

    - entry:
        label       : nat
        proto       : udp
        port        : const/21
        mode        : nat
        backends:
            - port  : const/2021
              count : const/0
              remote: False
            - port  : const/2021
              count : const/2
              remote: True

    - entry:
        label       : twice-nat
        proto       : udp
        port        : const/22
        mode        : twice_nat
        snat_ips    : ipstep/201.1.0.1/0.0.0.1/16
        snat_ports  : range/49152/65535
        backends:
            - port  : const/2022
              count : const/0
              remote: False
            - port  : const/2022
              count : const/2
              remote: True
