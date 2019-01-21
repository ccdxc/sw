#ifndef __PROXY_CONSTANTS_H
#define __PROXY_CONSTANTS_H

#define NIC_PAGE_HDR_SIZE                    0         /* sizeof(nic_page_hdr_t) */

#define CACHE_LINE_SIZE                     64
#define NIC_PAGE_SIZE                       (128 * CACHE_LINE_SIZE)
#define NIC_PAGE_SIZE_SHFT                  13
#define NIC_PAGE_HEADROOM                   (17 * CACHE_LINE_SIZE)
#define NIC_CPU_HDR_SIZE                    328
#define NIC_CPU_HDR_SIZE_BYTES              (NIC_CPU_HDR_SIZE/8)

#define NIC_DESC_ENTRY_TCP_FLAGS_OFFSET      56
#define NIC_DESC_ENTRY_0_OFFSET              64         /* &((nic_desc_t *)0)->entry[0]*/
#define NIC_DESC_ENTRY_L_OFFSET              96         /* &((nic_desc_t *)0)->entry[1]*/
#define NIC_DESC_ENTRY_SIZE                  16         /* sizeof(nic_desc_entry_t) */
#define NIC_DESC_ENTRY_SIZE_SHIFT            3          /* sizeof(nic_desc_entry_t) = 8B */
#define NIC_DESC_ENTRY_NEXT_ADDR_OFFSET      112
#define NIC_SERQ_ENTRY_SIZE                  32
#define NIC_SERQ_ENTRY_SIZE_SHIFT            5          /* for 32B */
#define NIC_SESQ_ENTRY_SIZE                  8
#define NIC_SESQ_ENTRY_SIZE_SHIFT            3          /* for 8B */
#define NIC_L7Q_ENTRY_SIZE                   8
#define NIC_L7Q_ENTRY_SIZE_SHIFT             3          /* for 8B */

#define PKT_DESC_AOL_OFFSET                  64

// L7 Redirect Type
#define L7_PROXY_TYPE_NONE                   0
#define L7_PROXY_TYPE_REDIR                  1
#define L7_PROXY_TYPE_SPAN                   2

#endif // __PROXY_CONSTANTS_H
