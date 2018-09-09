/*
 * Copyright 2017-2018 Pensando Systems, Inc.  All rights reserved.
 *
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

/*
 * ionic_utilities.h --
 *
 * Definitions of utilities functions
 */

#ifndef _IONIC_UTILITIES_H_
#define _IONIC_UTILITIES_H_

#include <vmkapi.h>

#define IONIC_INIT_WORLD_EVENT_ID(x) *(x) = (vmk_VA)(x)

#define IONIC_CONTAINER_OF(ptr, type, member) ({                           \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);               \
        (type *)( (char *)__mptr - vmk_offsetof(type,member) );})

#define IONIC_WARN_ON(condition) ({                                        \
      int __ret_warn_on = !!(condition);                                   \
      if (VMK_UNLIKELY(__ret_warn_on)) {                                   \
         VMK_ASSERT(VMK_FALSE);                                            \
      }                                                                    \
      VMK_UNLIKELY(__ret_warn_on);                                         \
})

#define IONIC_MAX(a, b) ({                                                 \
        typeof(a) _var1 = (a);                                             \
        typeof(b) _var2 = (b);                                             \
        _var1 > _var2 ? _var1 : _var2; })

#define IONIC_MIN(a, b) ({                                                 \
        typeof(a) _var1 = (a);                                             \
        typeof(b) _var2 = (b);                                             \
        _var1 < _var2 ? _var1 : _var2; })

//#define IONIC_ALIGN(ptr,n)                        (((ptr)+(n)-1) & ~((n)-1))  
#define IONIC_ALIGN(x,a)                            (((x)+(a)-1)&~((x)+(a)-1-(x)))

inline int
ionic_ilog2(vmk_uint64 num);

inline vmk_Bool
ionic_is_power_of_2(unsigned long n);

inline void
ionic_en_pkt_release(vmk_PktHandle *pkt,
                     vmk_NetPoll netpoll);
inline vmk_Bool
ionic_is_eth_addr_equal(vmk_EthAddress addr1,
                        vmk_EthAddress addr2);

void
ionic_hex_dump(char *desc,
               void *addr,
               vmk_uint32 len);

#endif /* End of _IONIC_UTILITIES_H_ */
