#ifndef _ATHENA_DEFINES_H_
#define _ATHENA_DEFINES_H_


/*****************************************************************************/
/* User L3 rewrite types                                                     */
/*****************************************************************************/
#define L3REWRITE_NONE                          0
#define L3REWRITE_SNAT                          1
#define L3REWRITE_DNAT                          2


/*****************************************************************************/
/* Encap types                                                               */
/*****************************************************************************/
#define REWRITE_ENCAP_NONE                      0
#define REWRITE_ENCAP_L2                        1
#define REWRITE_ENCAP_MPLSOUDP                  2
#define REWRITE_ENCAP_MPLSOGRE                  3


/*****************************************************************************/
/* P4E packet types                                                          */
/*****************************************************************************/
#define P4E_PACKET_NORMAL                       0
#define P4E_PACKET_OVERLAY_IPV4                 1
#define P4E_PACKET_OVERLAY_IPV6                 2

#endif /* _ATHENA_DEFINES_H_ */
