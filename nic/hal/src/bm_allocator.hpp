#ifndef _BM_ALLOCATOR_HPP_
#define _BM_ALLOCATOR_HPP_

#include <stdint.h>
#include <map>
#include <mutex>

namespace hal {

// A class similar to bitset but optimized for multi-bit operations.
class Bitmap {
 public:
  Bitmap(uint32_t size);
  ~Bitmap();

  bool IsBitSet(uint32_t ndx);
  bool IsBitClear(uint32_t ndx);
  void SetBits(uint32_t offset, uint32_t size);
  void ResetBits(uint32_t offset, uint32_t size);
  bool AreBitsSet(uint32_t offset, uint32_t size);
  bool AreBitsClear(uint32_t offset, uint32_t size);

 private:
  void BitSet(uint32_t offset);
  void BitReset(uint32_t offset);

  uint8_t *bmp_;
  uint32_t size_;
};

// BMAllocator - Simple bitmap based allocator.
// A simple allocator which only deals in terms of bits.
// It is upto the user to decide what does 1-bit mean
// e.g. 1KB, 16KB or 64B.
class BMAllocator {
 public:
  // Construct with how many total bits to track.
  BMAllocator(uint32_t total_bits);

  // Allocate a continous stretch of 'n' bits.
  // Returns the bit offset.
  // Returns -1 if the allocation fails.
  int Alloc(uint32_t n);

  // Reserve a specific range. Return error if the
  // range is not free.
  // Returns
  //   'start' - in case of success.
  //   -1      - in case of error.
  int CheckAndReserve(uint32_t offset, uint32_t size);

  // Frees a previous allocation.
  void Free(uint32_t offset, uint32_t allocation_length);

 private:
  std::mutex lk_;
  Bitmap bitmap_;
  uint32_t total_bits_;
  uint32_t current_ptr_;
};

}  // namespace hal

#endif  // _BM_ALLOCATOR_HPP_
