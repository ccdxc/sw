#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "logger.hpp"

const static uint32_t kMinHostMemAllocSize = 64;

#define	NUM_TO_VAL(num)		(1 << (num))

#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

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
  Poller() : timeout(long_poll_interval()) { }
  Poller(int timeout, bool fast_poll=true) : timeout(timeout), fast_poll(fast_poll) { }
  int operator()(std::function<int(void)> poll_func,
                 bool failure_expected=false);
private:
  int timeout; //Default overall timeout
  bool fast_poll = true;
};

}  // namespace utils


#endif  // _UTILS_HPP_
