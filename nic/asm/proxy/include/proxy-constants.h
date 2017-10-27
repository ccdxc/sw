#ifndef __PROXY_CONSTANTS_H
#define __PROXY_CONSTANTS_H

#define NIC_DESC_ENTRY_0_OFFSET              64         /* &((nic_desc_t *)0)->entry[0]*/
#define NIC_DESC_ENTRY_L_OFFSET              96         /* &((nic_desc_t *)0)->entry[1]*/
#define NIC_DESC_ENTRY_SIZE                  16         /* sizeof(nic_desc_entry_t) */
#define NIC_DESC_ENTRY_SIZE_SHIFT            3          /* sizeof(nic_desc_entry_t) = 8B */
#define NIC_DESC_ENTRY_NEXT_ADDR_OFFSET      112
#define NIC_SERQ_ENTRY_SIZE                  8
#define NIC_SERQ_ENTRY_SIZE_SHIFT            3          /* for 8B */
#define NIC_SESQ_ENTRY_SIZE                  8
#define NIC_SESQ_ENTRY_SIZE_SHIFT            3          /* for 8B */

#endif // __PROXY_CONSTANTS_H
