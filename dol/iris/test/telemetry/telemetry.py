#! /usr/bin/python3
# Test Module

import pdb
import infra.api.api as InfraApi
from iris.config.store import Store
import iris.config.objects.span as span
import iris.config.objects.tunnel as tunnel
import iris.config.objects.tenant as tenant
from infra.common.logging import logger as logger

erspan_sessions = []

class spanSessionData:
    localSpanSsns = {
            "SPAN_UPLINK_PC_ING_EG": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "LOCAL", "SPAN_PKT1", "Uplink2", 0),
                (3, "ingress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0),
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink1", 0),
                (2, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink2", 0),
                (3, "egress", "LOCAL", "EG_SPAN_PKT1", "UplinkPc1", 0)
                ],
            "RSPAN_UPLINK_PC_ING_EG": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink2", 0),
                (3, "ingress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0),
                (1, "egress", "RSPAN", "EG_RSPAN_PKT1", "Uplink1", 0),
                (2, "egress", "RSPAN", "EG_RSPAN_PKT1", "Uplink2", 0),
                (3, "egress", "RSPAN", "EG_RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "ERSPAN_UPLINK_PC_ING_EG": [
                (1, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT2", "Uplink2", 0),
                (3, "ingress", "ERSPAN", "ERSPAN_PKT3", "UplinkPc1", 0),
                (1, "egress", "ERSPAN", "EG_ERSPAN_PKT1", "Uplink2", 0),
                (2, "egress", "ERSPAN", "EG_ERSPAN_PKT2", "Uplink2", 0),
                (3, "egress", "ERSPAN", "EG_ERSPAN_PKT3", "UplinkPc1", 0),
                ],
            "XSPAN_UPLINK_PC_ING_EG": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (3, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 0),
                (1, "egress", "ERSPAN", "EG_ERSPAN_PKT1", "Uplink1", 0),
                (2, "egress", "RSPAN", "EG_RSPAN_PKT1", "Uplink1", 0),
                (3, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink1", 0)
                ],
            "XSPAN2_UPLINK_PC_ING_EG": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 0),
                (3, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink1", 0),
                (2, "egress", "ERSPAN", "EG_ERSPAN_PKT2", "Uplink1", 0),
                (3, "egress", "RSPAN", "EG_RSPAN_PKT1", "Uplink1", 0),
                ],
            "XSPAN2_UPLINK_PC_ING_EG_SNAP32": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 36),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 32),
                (3, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 32),
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink1", 32),
                (2, "egress", "ERSPAN", "EG_ERSPAN_PKT2", "Uplink1", 32),
                (3, "egress", "RSPAN", "EG_RSPAN_PKT1", "Uplink1", 32)
                ],
            "XSPAN2_UPLINK_PC_ING_EG_SNAP64": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 64),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 64),
                (3, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 64),
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink1", 64),
                (2, "egress", "ERSPAN", "EG_ERSPAN_PKT2", "Uplink1", 64),
                (3, "egress", "RSPAN", "EG_RSPAN_PKT1", "Uplink1", 64)
                ],
            "XSPAN2_UPLINK_PC_ING_EG_SNAP1000": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 1000),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 1000),
                (3, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 1000),
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink1", 1000),
                (2, "egress", "ERSPAN", "EG_ERSPAN_PKT2", "Uplink1", 1000),
                (3, "egress", "RSPAN", "EG_RSPAN_PKT1", "Uplink1", 1000)
                ],
            "L3_SPAN_UPLINK_PC_ING_EG": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "LOCAL", "SPAN_PKT1", "Uplink2", 0),
                (3, "ingress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0),
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink1", 0),
                (2, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink2", 0),
                (3, "egress", "LOCAL", "EG_SPAN_PKT1", "UplinkPc1", 0)
                ],
            "L3_RSPAN_UPLINK_PC_ING_EG": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink2", 0),
                (3, "ingress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0),
                (1, "egress", "RSPAN", "EG_RSPAN_PKT1", "Uplink1", 0),
                (2, "egress", "RSPAN", "EG_RSPAN_PKT1", "Uplink2", 0),
                (3, "egress", "RSPAN", "EG_RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "L3_ERSPAN_UPLINK_PC_ING_EG": [
                (1, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT2", "Uplink2", 0),
                (3, "ingress", "ERSPAN", "ERSPAN_PKT3", "UplinkPc1", 0),
                (1, "egress", "ERSPAN", "EG_ERSPAN_PKT1", "Uplink2", 0),
                (2, "egress", "ERSPAN", "EG_ERSPAN_PKT2", "Uplink2", 0),
                (3, "egress", "ERSPAN", "EG_ERSPAN_PKT3", "UplinkPc1", 0),
                ],
            "L3_XSPAN_UPLINK_PC_ING_EG": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink2", 0),
                (3, "ingress", "ERSPAN", "ERSPAN_PKT1", "UplinkPc1", 0),
                (1, "egress", "ERSPAN", "EG_ERSPAN_PKT2", "Uplink1", 0),
                (2, "egress", "RSPAN", "EG_RSPAN_PKT1", "Uplink2", 0),
                (3, "egress", "LOCAL", "EG_SPAN_PKT1", "UplinkPc1", 0)
                ],
            "L3_XSPAN2_UPLINK_PC_ING_EG": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink2", 0),
                (3, "ingress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0),
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink1", 0),
                (2, "egress", "ERSPAN", "EG_ERSPAN_PKT2", "Uplink2", 0),
                (3, "egress", "RSPAN", "EG_RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "L3_XSPAN2_UPLINK_PC_ING_EG_SNAP32": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 32),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink2", 32),
                (3, "ingress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 32),
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink1", 32),
                (2, "egress", "ERSPAN", "EG_ERSPAN_PKT2", "Uplink2", 32),
                (3, "egress", "RSPAN", "EG_RSPAN_PKT1", "UplinkPc1", 32)
                ],
            "L3_XSPAN2_UPLINK_PC_ING_EG_SNAP64": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 64),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink2", 64),
                (3, "ingress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 64),
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink1", 64),
                (2, "egress", "ERSPAN", "EG_ERSPAN_PKT2", "Uplink2", 64),
                (3, "egress", "RSPAN", "EG_RSPAN_PKT1", "UplinkPc1", 64)
                ],
            "L3_XSPAN2_UPLINK_PC_ING_EG_SNAP1000": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 1000),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink2", 1000),
                (3, "ingress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 1000),
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink1", 1000),
                (2, "egress", "ERSPAN", "EG_ERSPAN_PKT2", "Uplink2", 1000),
                (3, "egress", "RSPAN", "EG_RSPAN_PKT1", "UplinkPc1", 1000)
                ],
            "SPAN_UPLINK_PC_ING_ONLY": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "LOCAL", "SPAN_PKT1", "Uplink2", 0),
                (3, "ingress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0)
                ],
            "RSPAN_UPLINK_PC_ING_ONLY": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink2", 0),
                (3, "ingress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "ERSPAN_UPLINK_PC_ING_ONLY": [
                (1, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT2", "Uplink2", 0),
                (3, "ingress", "ERSPAN", "ERSPAN_PKT3", "UplinkPc1", 0)
                ],
            "XSPAN_UPLINK_PC_ING_ONLY": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink2", 0),
                (3, "ingress", "ERSPAN", "ERSPAN_PKT1", "UplinkPc1", 0)
                ],
            "XSPAN2_UPLINK_PC_ING_ONLY": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink2", 0),
                (3, "ingress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0)
                ],
            "XSPAN2_UPLINK_PC_ING_ONLY_SNAP32": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 32),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink2", 32),
                (3, "ingress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 32)
                ],
            "XSPAN2_UPLINK_PC_ING_ONLY_SNAP64": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 64),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink2", 64),
                (3, "ingress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 64)
                ],
            "XSPAN2_UPLINK_PC_ING_ONLY_SNAP1000": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 1000),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink2", 1000),
                (3, "ingress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 1000)
                ],
            "SPAN_LOCAL_INGRESS_UPLINK": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink2", 0)
                ],
            "SPAN_LOCAL_INGRESS_PC": [
                (3, "ingress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0)
                ],
            "SPAN_LOCAL_INGRESS_MULTIPLE_UPLINK": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "LOCAL", "SPAN_PKT1", "Uplink2", 0)
                ],
            "SPAN_LOCAL_INGRESS_MULTIPLE_UPLINK_PC": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (3, "ingress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0)
                ],
            "RSPAN_LOCAL_INGRESS_UPLINK": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0)
                ],
            "RSPAN_LOCAL_INGRESS_PC": [
                (3, "ingress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "ERSPAN_LOCAL_INGRESS_UPLINK": [
                (1, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 0)
                ],
            "ERSPAN_LOCAL_INGRESS_PC": [
                (1, "ingress", "ERSPAN", "ERSPAN_PKT1", "UplinkPc1", 0)
                ],
            "RSPAN_LOCAL_INGRESS_MULTIPLE_UPLINK": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink2", 0)
                ],
            "RSPAN_LOCAL_INGRESS_MULTIPLE_UPLINK_PC": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (3, "ingress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "ERSPAN_LOCAL_INGRESS_MULTIPLE_UPLINK": [
                (1, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT2", "Uplink2", 0)
                ],
            "ERSPAN_LOCAL_INGRESS_MULTIPLE_UPLINK_PC": [
                (1, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 0),
                (3, "ingress", "ERSPAN", "ERSPAN_PKT2", "UplinkPc1", 0)
                ],
            "SPAN_RSPAN_ERSPAN_LOCAL_INGRESS_UPLINK": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink2", 0),
                (3, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink2", 0)
                ],
            "SPAN_RSPAN_ERSPAN_LOCAL_INGRESS_PC": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0),
                (3, "ingress", "ERSPAN", "ERSPAN_PKT1", "UplinkPc1", 0)
                ],
            "RSPAN_SPAN_ERSPAN_LOCAL_INGRESS_PC": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0),
                (3, "ingress", "ERSPAN", "ERSPAN_PKT1", "UplinkPc1", 0)
                ],
            "SPAN_LOCAL_EGRESS_UPLINK": [
                (1, "egress", "LOCAL", "SPAN_PKT1", "Uplink1", 0)
                ],
            "SPAN_LOCAL_EGRESS_PC": [
                (3, "egress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0)
                ],
            "SPAN_LOCAL_EGRESS_MULTIPLE_UPLINK": [
                (1, "egress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "egress", "LOCAL", "SPAN_PKT1", "Uplink2", 0)
                ],
            "SPAN_LOCAL_EGRESS_MULTIPLE_UPLINK_PC": [
                (1, "egress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (3, "egress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0)
                ],
            "RSPAN_LOCAL_EGRESS_UPLINK": [
                (1, "egress", "RSPAN", "EG_RSPAN_PKT1", "Uplink1", 0)
                ],
            "RSPAN_LOCAL_EGRESS_PC": [
                (3, "egress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "ERSPAN_LOCAL_EGRESS_UPLINK": [
                (1, "egress", "ERSPAN", "EG_ERSPAN_PKT1", "Uplink1", 0)
                ],
            "ERSPAN_LOCAL_EGRESS_PC": [
                (3, "egress", "ERSPAN", "EG_ERSPAN_PKT1", "UplinkPc1", 0)
                ],
            "RSPAN_LOCAL_EGRESS_MULTIPLE_UPLINK": [
                (1, "egress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (2, "egress", "RSPAN", "RSPAN_PKT1", "Uplink2", 0)
                ],
            "RSPAN_LOCAL_EGRESS_MULTIPLE_UPLINK_PC": [
                (1, "egress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (3, "egress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "ERSPAN_LOCAL_EGRESS_MULTIPLE_UPLINK": [
                (1, "egress", "ERSPAN", "EG_ERSPAN_PKT1", "Uplink1", 0),
                (2, "egress", "ERSPAN", "EG_ERSPAN_PKT2", "Uplink2", 0)
                ],
            "ERSPAN_LOCAL_EGRESS_MULTIPLE_UPLINK_PC": [
                (1, "egress", "ERSPAN", "EG_ERSPAN_PKT1", "Uplink1", 0),
                (3, "egress", "ERSPAN", "EG_ERSPAN_PKT2", "UplinkPc1", 0)
                ],
            "SPAN_RSPAN_ERSPAN_LOCAL_EGRESS_UPLINK": [
                (1, "egress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "egress", "RSPAN", "RSPAN_PKT1", "Uplink2", 0),
                (3, "egress", "ERSPAN", "EG_ERSPAN_PKT1", "UplinkPc1", 0)
                ],
            "SPAN_RSPAN_ERSPAN_LOCAL_EGRESS_PC": [
                (1, "egress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "egress", "RSPAN", "RSPAN_PKT1", "Uplink2", 0),
                (3, "egress", "ERSPAN", "EG_ERSPAN_PKT1", "UplinkPc1", 0)
                ],
            "RSPAN_SPAN_LOCAL_EGRESS_PC": [
                (1, "egress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (3, "egress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0)
                ],
            "RSPAN_SPAN_LOCAL_INGRESS_EGRESS_UPLINK_PC": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (3, "egress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0)
                ],
            "SPAN_RSPAN_LOCAL_INGRESS_EGRESS_UPLINK_PC": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (3, "egress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "SPAN_LOCAL_INGRESS_LOCAL_EP":  [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "AnyLocalEP", 0),
                ],
            "ERSPAN_LOCAL_INGRESS_LOCAL_EP":  [
                (1, "ingress", "ERSPAN", "ERSPAN_PKT1", "AnyLocalEP", 0),
                ],
            "SPAN_LOCAL_EGRESS_LOCAL_EP":  [
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "AnyLocalEP", 0),
                ],
            "ERSPAN_LOCAL_EGRESS_LOCAL_EP":  [
                (1, "egress", "ERSPAN", "EG_ERSPAN_PKT1", "AnyLocalEP", 0),
                ],
            "SPAN_ERSPAN_LOCAL_ING_EG_LOCAL_EP1":  [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "AnyLocalEP", 0),
                ],
            "SPAN_ERSPAN_LOCAL_ING_EG_LOCAL_EP":  [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "AnyLocalEP", 0),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT2", "AnyLocalEP", 0),
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "AnyLocalEP", 0),
                (2, "egress", "ERSPAN", "EG_ERSPAN_PKT2", "AnyLocalEP", 0),
                ],
            "SPAN_ERSPAN_LOCAL_ING_EG_LOCAL_EP_SNAP32":  [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "AnyLocalEP", 32),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT2", "AnyLocalEP", 32),
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "AnyLocalEP", 32),
                (2, "egress", "ERSPAN", "EG_ERSPAN_PKT2", "AnyLocalEP", 32),
                ],
            "SPAN_ERSPAN_LOCAL_ING_EG_LOCAL_EP_SNAP64":  [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "AnyLocalEP", 64),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT2", "AnyLocalEP", 64),
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "AnyLocalEP", 64),
                (2, "egress", "ERSPAN", "EG_ERSPAN_PKT2", "AnyLocalEP", 64),
                ],
            "SPAN_ERSPAN_LOCAL_ING_EG_LOCAL_EP_SNAP1000":  [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "AnyLocalEP", 1000),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT2", "AnyLocalEP", 1000),
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "AnyLocalEP", 1000),
                (2, "egress", "ERSPAN", "EG_ERSPAN_PKT2", "AnyLocalEP", 1000),
                ],
            "SPAN_LOCAL_INGRESS_UPLINK_SNAP32": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 32),
                ],
            "SPAN_LOCAL_INGRESS_UPLINK_SNAP64": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 64),
                ],
            "SPAN_LOCAL_INGRESS_UPLINK_SNAP100": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 100),
                ],
            "SPAN_LOCAL_INGRESS_UPLINK_SNAP1000": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 1000),
                ],
            "SPAN_LOCAL_INGRESS_UPLINK_LOCAL_EP_SNAP64": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "AnyLocalEP", 64),
                ],
            "L3_SPAN_LOCAL_INGRESS_MULTIPLE_UPLINK": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "LOCAL", "SPAN_PKT1", "Uplink2", 0)
                ],
            "L3_SPAN_LOCAL_INGRESS_MULTIPLE_UPLINK_PC": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (3, "ingress", "LOCAL", "SPAN_PKT1", "UplinkPc1", 0)
                ],
            "L3_RSPAN_LOCAL_INGRESS_UPLINK": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0)
                ],
            "L3_RSPAN_LOCAL_INGRESS_PC": [
                (3, "ingress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "L3_RSPAN_LOCAL_INGRESS_MULTIPLE_UPLINK": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink2", 0)
                ],
            "L3_RSPAN_LOCAL_INGRESS_MULTIPLE_UPLINK_PC": [
                (1, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink1", 0),
                (3, "ingress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "L3_ERSPAN_LOCAL_INGRESS_MULTIPLE_UPLINK": [
                (1, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT2", "Uplink2", 0)
                ],
            "L3_ERSPAN_LOCAL_INGRESS_MULTIPLE_UPLINK_PC": [
                (1, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink1", 0),
                (3, "ingress", "ERSPAN", "ERSPAN_PKT2", "UplinkPc1", 0)
                ],
            "L3_RSPAN_LOCAL_EGRESS_UPLINK": [
                (1, "egress", "RSPAN", "EG_RSPAN_PKT1", "Uplink1", 0)
                ],
            "L3_RSPAN_LOCAL_EGRESS_PC": [
                (3, "egress", "RSPAN", "EG_RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "L3_RSPAN_LOCAL_EGRESS_MULTIPLE_UPLINK": [
                (1, "egress", "RSPAN", "EG_RSPAN_PKT1", "Uplink1", 0),
                (2, "egress", "RSPAN", "EG_RSPAN_PKT1", "Uplink2", 0)
                ],
            "L3_SPAN_RSPAN_ERSPAN_LOCAL_INGRESS_UPLINK": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "RSPAN", "RSPAN_PKT1", "Uplink2", 0),
                (3, "ingress", "ERSPAN", "ERSPAN_PKT1", "Uplink2", 0)
                ],
            "L3_SPAN_RSPAN_ERSPAN_LOCAL_INGRESS_PC": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (3, "ingress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0),
                (2, "ingress", "ERSPAN", "ERSPAN_PKT1", "UplinkPc1", 0)
                ],
            "L3_SPAN_LOCAL_EGRESS_UPLINK": [
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink1", 0)
                ],
            "L3_SPAN_LOCAL_EGRESS_MULTIPLE_UPLINK": [
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink1", 0),
                (2, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink2", 0)
                ],
            "L3_SPAN_LOCAL_EGRESS_MULTIPLE_UPLINK_PC": [
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink1", 0),
                (2, "egress", "LOCAL", "EG_SPAN_PKT1", "UplinkPc1", 0)
                ],
            "L3_SPAN_LOCAL_ERSPAN_EGRESS_UPLINK_PC": [
                (1, "egress", "RSPAN", "EG_RSPAN_PKT1", "Uplink1", 0),
                (2, "egress", "ERSPAN", "EG_ERSPAN_PKT1", "Uplink2", 0)
                ],
            "L3_RSPAN_LOCAL_EGRESS_MULTIPLE_UPLINK_PC": [
                (1, "egress", "RSPAN", "EG_RSPAN_PKT1", "Uplink1", 0),
                (3, "egress", "RSPAN", "EG_RSPAN_PKT1", "UplinkPc1", 0)
                ],
            "L3_ERSPAN_LOCAL_EGRESS_MULTIPLE_UPLINK_PC": [
                (1, "egress", "ERSPAN", "EG_ERSPAN_PKT1", "Uplink1", 0),
                (3, "egress", "ERSPAN", "EG_ERSPAN_PKT2", "UplinkPc1", 0)
                ],
            "L3_SPAN_RSPAN_ERSPAN_LOCAL_EGRESS_UPLINK": [
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink1", 0),
                (2, "egress", "RSPAN", "EG_RSPAN_PKT1", "UplinkPc1", 0),
                (3, "egress", "ERSPAN", "EG_ERSPAN_PKT1", "UplinkPc1", 0)
                ],
            "L3_SPAN_INGRESS_EGRESS_UPLINK": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink2", 0)
                ],
            "L3_SPAN_RSPAN_ERSPAN_LOCAL_INGRESS_EGRESS_PC": [
                (1, "ingress", "LOCAL", "SPAN_PKT1", "Uplink1", 0),
                (2, "ingress", "RSPAN", "RSPAN_PKT1", "UplinkPc1", 0),
                (3, "ingress", "ERSPAN", "ERSPAN_PKT1", "UplinkPc1", 0),
                (1, "egress", "LOCAL", "EG_SPAN_PKT1", "Uplink2", 0),
                (2, "egress", "RSPAN", "EG_RSPAN_PKT1", "UplinkPc1", 0),
                (3, "egress", "ERSPAN", "EG_ERSPAN_PKT2", "UplinkPc1", 0)
                ]
    }

    def __init__(self):
        self._cur_erspan_sessions = []
        self._cur_local_erspan_sessions = []
        self._local_ep_dests = []
        self.erspan_count = 0
        self.erspan_sess_map = {}
        self.local_sess_map = {}

    @staticmethod
    def getSessions(case):
        return spanSessionData.localSpanSsns[case]

    def getErspanSession(self, id):
        #sid = (id - 1) % 3
        sid = id - 1
        #if sid <= len(self._cur_erspan_sessions):
        #    return self._cur_erspan_sessions[sid]
        #return None
        return self.erspan_sess_map[sid]

    def setErspanSession(self, id, tnl):
        #pdb.set_trace()
        self.erspan_sess_map[id] = tnl

    def getCurErspanSession(self, id):
        return self._cur_erspan_sessions[(id - 1) % len(self._cur_erspan_sessions)]

    def addErspanSession(self, tnl):
        self._cur_erspan_sessions.append(tnl)

    def clearErspanSession(self):
       self._cur_erspan_sessions = []

    def getLocalErspanSession(self, id):
        sid = (id - 1) % len(self._cur_local_erspan_sessions)
        return self._cur_local_erspan_sessions[sid]

    def addLocalErspanSession(self, tnl):
        self._cur_local_erspan_sessions.append(tnl)

    def clearLocalErspanSession(self):
       self._cur_local_erspan_sessions = []

    def setLocalSession(self, id, sess, direc, spantype, pkt, pktlen, dintf):
        self.local_sess_map[id] = (sess, direc, spantype, pkt, pktlen, dintf)

    def getLocalSession(self, id):
        if id in self.local_sess_map:
            return self.local_sess_map[id]
        return None

    def getLocalSessionBySession(self, sid):
        for key in self.local_sess_map:
            (sess, direc, spantype, pkt, pktlen, obj)  = self.local_sess_map[key]
            if sess == sid:
                return obj
        return None

    def addLocalEpDest(self, ep):
        self._local_ep_dests.append(ep)

    def getLocalEpDest(self, id):
        sid = (id - 1) % len(self._local_ep_dests)
        return self._local_ep_dests[sid]

    def clearLocalEpDest(self):
        self._local_ep_dests = []

    def show(self):
        logger.info("ERSpan Sesssions")
        for key in self.erspan_sess_map:
            tnl = self.erspan_sess_map[key]
            logger.info("  (%d) Dst: %s Src: %s " % (key, tnl.rtep.get(), tnl.ltep.get()))
        logger.info("Local Destinations ", len(self._local_ep_dests))
        for lep in self._local_ep_dests:
            logger.info("  Tenant : %s (%s)" % (lep.tenant, lep.GID()))
            for ipaddr in lep.ipaddrs:
                logger.info("    - Ipaddr    = %s" % ipaddr.get())
        logger.info("ERSPAN Destinations")
        for tnl in self._cur_erspan_sessions:
            logger.info("  (%d) Dst: %s Src: %s " % (key, tnl.rtep.get(), tnl.ltep.get()))
        logger.info("Local Sessions")
        for  key in self.local_sess_map:
            (sess, direc, spantype, pkt, pktlen, dintf) = self.local_sess_map[key]
            logger.info("  (%d) Session: %d Direction: %s Type: %s Pkt:%s Pktlen: %d" % (key, sess, direc, spantype, pkt, pktlen))
            if spantype == "ERSPAN":
                logger.info("  Dst: %s Src: %s " % (dintf.rtep.get(), dintf.ltep.get()))
            else:
                logger.info("  Tenant : %s (%s)" % (lep.tenant, lep.GID()))
                for ipaddr in lep.ipaddrs:
                    logger.info("    - Ipaddr    = %s" % ipaddr.get())

def clearAllSessions():
    spans = Store.objects.GetAllByClass(span.SpanSessionObject)
    for ssn in Store.objects.GetAllByClass(span.SpanSessionObject):
        ssn.Clear()
    erspan_sessions = []
    data.erspan_count = 0
    data.erspan_sess_map = {}
    data.local_sess_map = {}
    return

def setup_span(infra, module, case):
    clearAllSessions()
    ssns = spanSessionData.getSessions(case)
    # setup default ERSPAN tunnels
    count = 0
    for tnl in Store.objects.GetAllByClass(tunnel.TunnelObject):
        if tnl.IsGRE():
            if tnl.local_dest:
                data.addLocalErspanSession(tnl)
            else:
                data.addErspanSession(tnl)
            data.setErspanSession(count, tnl)
            count = count + 1
    count = 1
    lspancount = 1
    for (id, direc, spantype, pkt, intf, pktlen) in ssns:
        lobj = None
        ofset = 0
        if direc == "egress":
            ofset = 3
        id = id + ofset
        sessname = "SpanSession000" + str(id)
        sess = infra.ConfigStore.objects.Get(sessname)
        if spantype == "ERSPAN":
            if intf == "AnyLocalEP":
                tnl = data.getLocalErspanSession(lspancount)
                lobj = tnl
            else:
                tnl = data.getCurErspanSession(count)
            data.setErspanSession(id - 1, tnl)
            logger.info("Setting SpanSession %s(%d) Dst: %s Src: %s " % (sessname, id - 1, tnl.rtep.get(), tnl.ltep.get()))
            sess.erspan_dest = tnl.GetDestIp()
            sess.erspan_src = tnl.GetSrcIp()
            sess.tenant = tnl.tenant
            sess.span_id = 0
            count = count + 1
        dintf = None
        if intf == "AnyLocalEP":
            ten = infra.ConfigStore.objects.Get("Ten0012")
            #for ten in Store.objects.GetAllByClass(tenant.TenantObject):
            eps = ten.GetLocalEps()
            if spantype == "ERSPAN":
                dintf = lobj.rtep
                data.setLocalSession(lspancount, id, direc, spantype, pkt, pktlen, lobj)
            else:
                if lspancount <= len(eps):
                    dintf = eps[lspancount - 1].intf
                    data.addLocalEpDest(eps[lspancount - 1])
                else:
                    dintf = eps[lspancount % len(eps)]
                data.setLocalSession(lspancount, id, direc, spantype, pkt, pktlen, dintf)
            lspancount = lspancount + 1
        else:
            dintf = infra.ConfigStore.objects.Get(intf)
        logger.info("Updating Span Session %s on interface %s with type %s" % (sessname, intf, spantype))
        snaplen = 0
        if pktlen != 0:
            snaplen = pktlen - 14
            if pktlen < 14:
                snaplen = 1
        sess.Update(snaplen, spantype, dintf)
    data.show()
    return

def Setup(infra, module):
    #pdb.set_trace()
    data.clearErspanSession()
    setup_span(infra, module, module.iterator.Get())
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    span_case = tc.module.iterator.Get()
    tc.pvtdata.span_case = span_case
    return

def TestCaseVerify(tc):
    return True

def TestCaseTeardown(tc):
    return

data = spanSessionData()
