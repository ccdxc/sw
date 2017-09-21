#include "bm_allocator.hpp"

#include <assert.h>
#include <strings.h>

namespace hal {

Bitmap::Bitmap(uint32_t size) {
  assert(size);
  size_ = size;
  uint32_t alloc_size = ((size + 7) & ~7U) >> 3;
  bmp_ = new uint8_t[alloc_size];
  bzero(bmp_, alloc_size);
}

Bitmap::~Bitmap() {
  delete[] bmp_;
}

bool Bitmap::IsBitSet(uint32_t ndx) {
  if (ndx >= size_)
    return false;

  return (bmp_[ndx >> 3] & (((uint8_t)1) << (ndx & 7))) != 0;
}

bool Bitmap::IsBitClear(uint32_t ndx) {
  return !IsBitSet(ndx);
}

void Bitmap::BitSet(uint32_t offset) {
  bmp_[offset >> 3] |= ((uint8_t)1) << (offset & 7);
}

void Bitmap::BitReset(uint32_t offset) {
  bmp_[offset >> 3] &= ~(((uint8_t)1) << (offset & 7));
}

bool Bitmap::AreBitsSet(uint32_t offset, uint32_t size) {
  if (size == 0)
    return true;
  if (offset >= size_)
    return true;
  if ((offset + size) >= size_)
    size = size_ - offset;
  while (true) {
    while ((offset & 7) || (size < 8)) {
      if (IsBitClear(offset++))
        return false;
      size--;
      if (size == 0)
        return true;
    }
    while (size >= 8) {
      if (bmp_[offset >> 3] != 0xFF)
        return false;
      offset += 8;
      size -= 8;
    }
    if (size == 0)
      break;
  }
  return true;
}

bool Bitmap::AreBitsClear(uint32_t offset, uint32_t size) {
  if (size == 0)
    return true;
  if (offset >= size_)
    return true;
  if ((offset + size) >= size_)
    size = size_ - offset;
  while (true) {
    while ((offset & 7) || (size < 8)) {
      if (IsBitSet(offset++))
        return false;
      size--;
      if (size == 0)
        return true;
    }
    while (size >= 8) {
      if (bmp_[offset >> 3] != 0)
        return false;
      offset += 8;
      size -= 8;
    }
    if (size == 0)
      break;
  }
  return true;
}

void Bitmap::SetBits(uint32_t offset, uint32_t size) {
  if (size == 0)
    return;
  if (offset >= size_)
    return;
  if ((offset + size) >= size_)
    size = size_ - offset;
  while (true) {
    while ((offset & 7) || (size < 8)) {
      BitSet(offset++);
      size--;
      if (size == 0)
        return;
    }
    while (size >= 8) {
      bmp_[offset >> 3] = 0xFF;
      offset += 8;
      size -= 8;
    }
    if (size == 0)
      return;
  }
}

void Bitmap::ResetBits(uint32_t offset, uint32_t size) {
  if (size == 0)
    return;
  if (offset >= size_)
    return;
  if ((offset + size) >= size_)
    size = size_ - offset;
  while (true) {
    while ((offset & 7) || (size < 8)) {
      BitReset(offset++);
      size--;
      if (size == 0)
        return;
    }
    while (size >= 8) {
      bmp_[offset >> 3] = 0;
      offset += 8;
      size -= 8;
    }
    if (size == 0)
      return;
  }
}

void BMAllocator::Free(uint32_t offset, uint32_t allocation_length) {
  std::lock_guard<std::mutex> lk(lk_);

  bitmap_.ResetBits(offset, allocation_length);
}

int BMAllocator::Alloc(uint32_t n, uint32_t align) {
  std::lock_guard<std::mutex> lk(lk_);

  if (n == 0)
    return -1;

  // Bring current_ptr to the required align.
  uint32_t r = current_ptr_ % align;
  current_ptr_ = r ? current_ptr_ + (align - r) : current_ptr_;

  // If there is not enough left, reset the curptr.
  if ((current_ptr_ + n) >= total_bits_)
    current_ptr_ = 0;

  uint32_t offset = 0;
  uint32_t alloced_length = 0;
  bool found_offset = false;
  bool try_again;
  do {
    try_again = false;
    for(uint32_t ndx = current_ptr_; ndx < total_bits_;) {
      if (found_offset) {
        // Allocation mode.
        if (bitmap_.IsBitClear(ndx)) {
          ndx++;
          alloced_length++;
          if (alloced_length == n)
            break;
        } else {
          // Not enough bits in this stretch. Reset and enter
          // back into search mode.
          found_offset = false;
          alloced_length = 0;
          uint32_t r = ndx % align;
          ndx = r ? ndx + (align - r) : ndx;
          if (ndx >= total_bits_)
            break;
        }
      } else {
        // Search mode. Looking for a potential point to start.
        if (bitmap_.IsBitClear(ndx)) {
          found_offset = true;
          offset = ndx;
          alloced_length = 1;
          if (n == 1) {
            // For 1-bit allocations, no need to enter allocation mode.
            break;
          }
          ndx++;
        } else {
          ndx += align;
        }
      }  // Search mode.
    }  // For bits from current_ptr_
    if (found_offset && (n == alloced_length)) {
      // We got what we wanted, go on to state update.
      break;
    }
    if (current_ptr_ == 0) {
      // Failed, even when we searched from beginning.
      return -1;
    }
    // Setup to search from beginning.
    current_ptr_ = 0;
    found_offset = 0;
    alloced_length = 0;
    try_again = true;
  } while (try_again);

  // At this point the allocation has succeeded.
  // Update state and get out.
  bitmap_.SetBits(offset, alloced_length);
  current_ptr_ = offset + alloced_length;
  if (current_ptr_ >= total_bits_)
    current_ptr_ = 0;
  return offset;
}

int BMAllocator::CheckAndReserve(uint32_t offset, uint32_t size) {
  std::lock_guard<std::mutex> lk(lk_);

  if (!bitmap_.AreBitsClear(offset, size))
    return -1;

  bitmap_.SetBits(offset, size);
  return (int)offset;
}

BMAllocator::BMAllocator(uint32_t total_bits) : bitmap_(total_bits) {
  total_bits_ = total_bits;
  current_ptr_ = 0;
}

}  // namespace hal
