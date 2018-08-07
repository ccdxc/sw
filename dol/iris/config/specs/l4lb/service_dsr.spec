# Flow generation configuration template.
meta:
    id: L4LB_DSR

entries:
    # TCP
    - entry:
        label       : nat_dsr
        proto       : tcp
        port        : const/80
        mode        : nat_dsr
        backends:
            - port    : const/0
              count   : const/2
              remote  : False

    - entry:
        label       : nat_dsr
        proto       : tcp
        port        : const/80
        mode        : nat_dsr
        backends:
            - port    : const/8880
              count   : const/2
              remote  : False

    - entry:
        label       : nat_dsr
        proto       : tcp
        port        : const/80
        mode        : nat_dsr
        backends:
            - port    : const/0
              count   : const/2
              remote  : True
              tnnled  : False

    - entry:
        label       : nat_dsr
        proto       : tcp
        port        : const/80
        mode        : nat_dsr
        backends:
            - port    : const/8880
              count   : const/2
              remote  : True
              tnnled  : False

    - entry:
        label       : nat_dsr
        proto       : tcp
        port        : const/80
        mode        : nat_dsr
        backends:
            - port    : const/0
              count   : const/2
              remote  : True
              tnnled  : True

    - entry:
        label       : nat_dsr
        proto       : tcp
        port        : const/80
        mode        : nat_dsr
        backends:
            - port    : const/8880
              count   : const/2
              remote  : True
              tnnled  : True

    # UDP
    - entry:
        label       : nat_dsr
        proto       : udp
        port        : const/21
        mode        : nat_dsr
        backends:
            - port    : const/0
              count   : const/2
              remote  : False

    - entry:
        label       : nat_dsr
        proto       : udp
        port        : const/21
        mode        : nat_dsr
        backends:
            - port    : const/2021
              count   : const/2
              remote  : False

    - entry:
        label       : nat_dsr
        proto       : udp
        port        : const/21
        mode        : nat_dsr
        backends:
            - port    : const/0
              count   : const/2
              remote  : True
              tnnled  : False

    - entry:
        label       : nat_dsr
        proto       : udp
        port        : const/21
        mode        : nat_dsr
        backends:
            - port    : const/2021
              count   : const/2
              remote  : True
              tnnled  : False

    - entry:
        label       : nat_dsr
        proto       : udp
        port        : const/21
        mode        : nat_dsr
        backends:
            - port    : const/0
              count   : const/2
              remote  : True
              tnnled  : True

    - entry:
        label       : nat_dsr
        proto       : udp
        port        : const/21
        mode        : nat_dsr
        backends:
            - port    : const/2021
              count   : const/2
              remote  : True
              tnnled  : True



