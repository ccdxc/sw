#include "bm_allocator.hpp"
#include <stdio.h>
#include <assert.h>

int
main()
{
  hal::Bitmap bm(22);

  assert(bm.IsBitSet(21) == false);
  assert(bm.IsBitClear(21) == true);
  assert(bm.IsBitClear(22) == true);
  assert(bm.IsBitSet(22) == false);

  assert(bm.AreBitsClear(4, 16) == true);
  bm.SetBits(2, 19);
  assert(bm.AreBitsSet(2, 19) == true);
  bm.ResetBits(12, 1);
  assert(bm.AreBitsSet(2, 19) == false);
  bm.ResetBits(15, 1);
  assert(bm.AreBitsClear(15, 1) == true);
  assert(bm.IsBitSet(0) == false);
  assert(bm.IsBitSet(1) == false);
  assert(bm.IsBitSet(2) == true);
  assert(bm.IsBitSet(12) == false);
  assert(bm.IsBitSet(11) == true);
  assert(bm.IsBitSet(13) == true);
  assert(bm.IsBitSet(14) == true);
  assert(bm.IsBitSet(15) == false);
  assert(bm.IsBitSet(16) == true);

  hal::BMAllocator bma(4096);

  assert(bma.Alloc(20) == 0);
  assert(bma.Alloc(20) == 20);
  bma.Free(0, 20);
  assert(bma.Alloc(50) == 40);
  assert(bma.Alloc(5000) == -1);
  bma.Free(20, 20);
  bma.Free(40, 50);
  assert(bma.Alloc(4096) == 0);
  bma.Free(0, 4096);
  assert(bma.Alloc(1024) == 0);
  assert(bma.Alloc(1024) == 1024);
  assert(bma.Alloc(1024) == 2048);
  bma.Free(0, 1024);
  bma.Free(1024, 1024);
  assert(bma.Alloc(2048) == 0);

  printf("Bitmap allocator tests passed.\n");

  return 0;
}
