#include "nic/p4/common/defines.h"

/*****************************************************************************/
/* IP types                                                                  */
/*****************************************************************************/
#define IPTYPE_IPV4                     0
#define IPTYPE_IPV6                     1

/*****************************************************************************/
/* MPLS tag depth                                                            */
/*****************************************************************************/
#define MPLS_DEPTH                      2

/*****************************************************************************/
/* Key types                                                                 */
/*****************************************************************************/
#define KEY_TYPE_NONE                   0
#define KEY_TYPE_IPV4                   1
#define KEY_TYPE_IPV6                   2
#define KEY_TYPE_MAC                    3

/*****************************************************************************/
/* Lifs                                                                      */
/*****************************************************************************/
#define HOSTPORT_LIF                    0
#define SWITCHPORT_LIF                  1

/*****************************************************************************/
/* Packet direction                                                          */
/*****************************************************************************/
#define RX_FROM_SWITCH                  0
#define TX_FROM_HOST                    1

/*****************************************************************************/
/* IP Types                                                                  */
/*****************************************************************************/
#define IP_TYPE_OVERLAY     0
#define IP_TYPE_PUBLIC      1

/*****************************************************************************/
/* Encap Types towards switch                                                */
/*****************************************************************************/
#define GW_ENCAP            0
#define VNIC_ENCAP          1

/*****************************************************************************/
/* drop reasons - these are bit positions to be used in ASM                  */
/*****************************************************************************/
#define DROP_FLOW_HIT                           0

