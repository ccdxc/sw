#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "logger.hpp"
#ifdef __x86_64__
#include "nic/utils/host_mem/c_if.h"
#include "lib_model_client.h"
#endif


const static uint32_t kMinHostMemAllocSize = 64;

#define	NUM_TO_VAL(num)		(1 << (num))

#ifndef htonll
#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif

/*
 * lib_model_client interfaces
 */
#ifdef __x86_64__
#define EOS_IGNORE_ADDR(addr, size)     eos_ignore_addr(addr, size)
#define REGISTER_MEM_ADDR(addr)         register_mem_addr(addr)
#define TESTCASE_BEGIN(tcid, loopid)    testcase_begin(tcid, loopid);
#define TESTCASE_END(tcid, loopid)      testcase_end(tcid, loopid);
#define CONFIG_DONE()                   config_done()
#define EXIT_SIMULATION()               exit_simulation()

#define ALLOC_HOST_MEM(size)            alloc_host_mem(size)
#define ALLOC_PAGE_ALIGNED_HOST_MEM(size) alloc_page_aligned_host_mem(size)
#define ALLOC_SPEC_ALIGNED_HOST_MEM(size, align) alloc_spec_aligned_host_mem(size, align)
#define FREE_HOST_MEM(addr)             free_host_mem(addr)
#define HOST_MEM_V2P(addr)              host_mem_v2p(addr)
#define HOST_MEM_P2V(addr)              host_mem_p2v(addr)
#else
#define EOS_IGNORE_ADDR(addr, size)
#define REGISTER_MEM_ADDR(addr)
#define TESTCASE_BEGIN(tcid, loopid)    (void)loopid
#define TESTCASE_END(tcid, loopid)      (void)loopid
#define CONFIG_DONE()
#define EXIT_SIMULATION()

/*
 * On ARM, host_mem is not to be used. The following macros only
 * serve to silence compiler errors and tests that use host_mem
 * will not work.
 */
#define ALLOC_HOST_MEM(size)            malloc(size)
#define ALLOC_PAGE_ALIGNED_HOST_MEM(size) malloc(size)
#define FREE_HOST_MEM(addr)             free(addr)
#define HOST_MEM_V2P(addr)              ((uint64_t)0)
#define HOST_MEM_P2V(addr)              NULL
#endif

extern uint64_t poll_interval(void);
extern uint64_t long_poll_interval(void);

namespace utils {

extern const uint32_t kUtilsPageSize;
extern uint64_t storage_hbm_addr;
extern uint32_t storage_hbm_size;

void write_bit_fields(void *ptr, unsigned start_bit_offset,
                      unsigned size_in_bits, uint64_t value);

void dump(uint8_t *buf, int size=64);

void dump1K(uint8_t *buf);

int hbm_buf_init();

int hbm_addr_alloc(uint32_t size, uint64_t *alloc_ptr);

int hbm_addr_alloc_page_aligned(uint32_t size, uint64_t *alloc_ptr);
int hbm_addr_alloc_spec_aligned(uint32_t size, uint64_t *alloc_ptr, uint32_t spec_align_size);
uint32_t roundup_to_pow_2(uint32_t x);

/*
 * namespace utils version of Poller, for use by offload
 */
class Poller {
public:
#ifdef __x86_64__
  Poller() : timeout(long_poll_interval()) { }
#else
  Poller() : timeout(10) { }
#endif
  Poller(int timeout, bool fast_poll=true) : timeout(timeout), fast_poll(fast_poll) { }
  int operator()(std::function<int(void)> poll_func,
                 bool failure_expected=false);
private:
  int timeout; //Default overall timeout
  bool fast_poll = true;
};

}  // namespace utils


#endif  // _UTILS_HPP_
