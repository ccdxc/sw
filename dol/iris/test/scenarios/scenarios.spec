# This file contains the list of scenarios.
features:
    - feature:
        name        : L2UC_IP
        featureid   : BRIDGING
        scenarios   :
            - ref://scenarios/labels/sources
            - ref://scenarios/labels/destinations
        multipliers:
            - ref://scenarios/labels/ip_family
            - ref://scenarios/labels/ip_protos
            - ref://scenarios/labels/encaps

    - feature:
        name        : L2UC_MAC
        featureid   : BRIDGING
        scenarios   :
            - ref://scenarios/labels/sources
            - ref://scenarios/labels/destinations
        multipliers:
            - ref://scenarios/labels/mac_family
            - ref://scenarios/labels/encaps

