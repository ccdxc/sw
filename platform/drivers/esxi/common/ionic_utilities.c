/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 */

/*
 * ionic_utilities.c --
 *
 * Implement utilities functions 
 */

#include "ionic.h"

/*
 ******************************************************************************
 *
 * ionic_ilog2 --
 *
 *    Log of base 2 of a number
 *
 *  Parameters:
 *     num - IN
 *
 *  Results:
 *     Return the log of base 2 of num
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

inline int
ionic_ilog2(vmk_uint64 num)                                       // IN
{
        int val = 0;

        while (num >>= 1) {
                ++val;
        }

        return val;
}


/*
 ******************************************************************************
 *
 * ionic_is_power_of_2 --
 *
 *    Check if a given value is a power of 2
 *
 *  Parameters:
 *     n - IN
 *
 *  Results:
 *     VMK_TRUE if the value is power of 2, otherwise VMK_FALSE
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

inline vmk_Bool
ionic_is_power_of_2(unsigned long n)                              // IN
{
        return (n != 0 && ((n & (n - 1)) == 0));
}


/*
 *****************************************************************************
 *
 * ionic_en_pkt_release
 *
 *     Releases a packet based on system and packet state.
 *
 *  Parameters:
 *     pkt     - IN (pointer to the packet handler that need to be  released)
 *     netpoll - IN (NetPoll used to release the given packet handler)
 *
 *  Results:
 *     None
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

inline void
ionic_en_pkt_release(vmk_PktHandle *pkt,                          // IN
                     vmk_NetPoll netpoll)                         // IN
{
        VMK_ASSERT(pkt);

        if (vmk_SystemCheckState(VMK_SYSTEM_STATE_PANIC)) {
                vmk_PktReleasePanic(pkt);
        } else if (netpoll) {
                vmk_NetPollQueueCompPkt(netpoll, pkt);
        } else {
                vmk_PktRelease(pkt);
        }   
}


/*
 ******************************************************************************
 *
 * ionic_is_eth_addr_equal
 *
 *     Check if the two given eth addresses are equal
 *
 *  Parameters:
 *     addr1  - IN (first ethernet address)
 *     addr2  - IN (second ethernet address)
 *
 *  Results:
 *     VMK_TRUE if equal, VMK_FALSE otherwise
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

inline vmk_Bool
ionic_is_eth_addr_equal(vmk_EthAddress addr1,                     // IN
                        vmk_EthAddress addr2)                     // IN
{
        if ((addr1[0] ^ addr2[0]) | (addr1[1] ^ addr2[1]) | (addr1[2] ^ addr2[2]) |
            (addr1[3] ^ addr2[3]) | (addr1[4] ^ addr2[4]) | (addr1[5] ^ addr2[5])) {
                return VMK_FALSE;
        }   

        return VMK_TRUE;
}


/*
 ******************************************************************************
 *
 * ionic_is_eth_addr_zero
 *
 *     Check if the given ethernet address is zero
 *
 *  Parameters:
 *     addr - IN (ethernet address to check)
 *
 *  Results:
 *     VMK_TRUE if ethernet address is zero
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

inline vmk_Bool
ionic_is_eth_addr_zero(vmk_EthAddress addr)                       // IN
{
        static vmk_EthAddress zero_addr = {0};
        return ionic_is_eth_addr_equal(addr, zero_addr);
}


/*
 ******************************************************************************
 *
 * ionic_hex_dump 
 *
 *    Hexdumping a given range of memory 
 *
 *  Parameters:
 *     desc - IN (Description of this dumping object)
 *     addr - IN (starting  address to dump)
 *     len  - IN (Length of this memory)
 *
 *  Results:
 *     None 
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

void
ionic_hex_dump(char *desc,                                         // IN
               void *addr,                                         // IN
               vmk_uint32 len)                                     // IN
{
        vmk_uint32 i, offset;
        vmk_ByteCount out_len;
        unsigned char *pc = (unsigned char*)addr;
        char hexdump[IONIC_HEX_DUMP_SIZE];

        if (desc != NULL) {
                ionic_print ("%s, len: %d", desc, len);
        }

        if (len == 0) {
                ionic_print("Length is zero\n");
                return;
        }
        if (len < 0) {
                ionic_print("Length is negative: %i\n",len);
                return;
        }

        offset = 0;
        out_len = 0;
        vmk_Memset(hexdump,
                   0,
                   sizeof(char) * IONIC_HEX_DUMP_SIZE);

        for (i = 0; i < len; i++) {
                if ((i % 16) == 0 && i != 0) {
                        ionic_print ("%s", hexdump);
                        vmk_Memset(hexdump, 0, offset);
                        offset = 0;
                }

                vmk_StringFormat(hexdump + offset,
                                 IONIC_HEX_DUMP_SIZE - offset,
                                 &out_len,
                                 " %02x",
                                 pc[i]);
                                 
                offset += out_len;                         

                if (i == len - 1) {
                        ionic_print ("%s", hexdump);
                }

        }
}
