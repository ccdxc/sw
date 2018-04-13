#include "dol/test/storage/utils.hpp"
#include "dol/test/storage/queues.hpp"

namespace nvme_dp {

const static uint32_t	kIOMapEntrySize		 = 1024;
const static uint32_t	kIOMapNumEntries	 = 8;
const static uint32_t	kIOBufEntrySize		 = 8192;
const static uint32_t	kIOBufNumEntries	 = 8;
const static uint32_t	kIOBFreeListEntrySize	 = 64;
const static uint32_t	kIOBRingStateSize	 = 64;

dp_mem_t *io_map_base_addr;
uint32_t io_map_num_entries;
dp_mem_t *io_buf_base_addr;
uint32_t io_buf_num_entries;
dp_mem_t *iob_free_list_base_addr;
uint32_t iob_free_list_num_entries;
dp_mem_t *iob_ring_base_addr;

int alloc_io_resources() {

  io_map_num_entries = kIOMapNumEntries;
  if ((io_map_base_addr = new dp_mem_t(io_map_num_entries, kIOMapEntrySize, 
                                       DP_MEM_ALIGN_PAGE, DP_MEM_TYPE_HBM)) < 0) {
    printf("can't alloc IO MAP entry \n");
    return -1;
  }
  printf("IO MAP PA: %lx \n", io_map_base_addr->pa());

  io_buf_num_entries = kIOBufNumEntries;
  if ((io_buf_base_addr = new dp_mem_t(io_buf_num_entries, kIOBufEntrySize, 
                                       DP_MEM_ALIGN_PAGE, DP_MEM_TYPE_HBM)) < 0) {
    printf("can't alloc IO Buf entry \n");
    return -1;
  }
  printf("IO Buf entry PA: %lx \n", io_buf_base_addr->pa());

  iob_free_list_num_entries = io_buf_num_entries;
  if ((iob_free_list_base_addr = new dp_mem_t(iob_free_list_num_entries,
                                              kIOBFreeListEntrySize,
                                              DP_MEM_ALIGN_PAGE,
                                              DP_MEM_TYPE_HBM)) < 0) {
    printf("can't alloc IOB free list entry \n");
    return -1;
  }
  printf("IOB Free List PA: %lx \n", iob_free_list_base_addr->pa());

  if ((iob_ring_base_addr = new dp_mem_t(1, kIOBRingStateSize,
                                         DP_MEM_ALIGN_PAGE, DP_MEM_TYPE_HBM)) < 0) {
    printf("can't alloc IOB ring base state \n");
    return -1;
  }
  printf("IOB Ring Base PA: %lx \n", iob_ring_base_addr->pa());

  return 0;
}

uint64_t get_io_map_base_addr() {
  return io_map_base_addr->pa();
}

uint64_t get_io_map_num_entries() {
  return io_map_num_entries;
}

uint64_t get_io_buf_base_addr() {
  return io_buf_base_addr->pa();
}

uint64_t get_iob_ring_base_addr() {
  return iob_ring_base_addr->pa();
}

#define IOB_FREE_LIST_ADDR(i)	\
    ((uint64_t *) (iob_free_list_base_addr->read() + ((i) * kIOBFreeListEntrySize)))

#define IO_BUF_ADDR(i)		\
    (io_buf_base_addr->pa() + ((i) * kIOBufEntrySize))

int init_iob_ring() {

  // Allocate the I/O buffers to the IOB free list
  for (int i = 0; i < (int) iob_free_list_num_entries-1; i++) {
    *IOB_FREE_LIST_ADDR(i) = IO_BUF_ADDR(i);
  }
  iob_free_list_base_addr->write_thru();

  // Form the IOB ring state pointing to th e IOB free list
  iob_ring_base_addr->write_bit_fields(0, 16, (uint16_t) (iob_free_list_num_entries-1));
  iob_ring_base_addr->write_bit_fields(16, 16, 0);
  iob_ring_base_addr->write_bit_fields(32, 16, (uint16_t) (iob_free_list_num_entries-1));
  iob_ring_base_addr->write_bit_fields(48, 16, (uint16_t) iob_free_list_num_entries);
  iob_ring_base_addr->write_bit_fields(64, 64, iob_free_list_base_addr->pa());
  iob_ring_base_addr->write_bit_fields(128, 16, kIOBFreeListEntrySize);
  iob_ring_base_addr->write_thru();
  printf("IOB Ring Base PA: %lx \n", iob_ring_base_addr->pa());

  return 0;
}

int test_setup () {

  // Allocate resources 
  if (alloc_io_resources() < 0) {
    printf("Failed to allocate resources \n");
    return -1;
  }

  // Initialize IOB ring
  if (init_iob_ring() < 0) {
    printf("Failed to initialize IOB ring \n");
    return -1;
  }

  // Initialize resources
  if (queues::resources_init() < 0) {
    printf("Failed to initialize resources \n");
    return -1;
  }
  printf("Initialized resources \n");

  // Setup LIFs
  if (queues::lifs_setup() < 0) {
    printf("Failed to setup LIFs\n");
    return -1;
  }
  printf("Setup LIFs \n");

  // Setup ARM queues. Do this first up as nvme_dp_queues_setup
  // depends on the queues that are setup in this API.
  if (queues::arm_queues_setup() < 0) {
    printf("Failed to setup ARM queues \n");
    return -1;
  }
  printf("Setup ARM queues  \n");

  // Setup NVME datapath queues in P4+
  if (queues::nvme_dp_queues_setup() < 0) {
    printf("Failed to setup NVME datapath queues \n");
    return -1;
  }
  printf("Setup NVME datapath queues  \n");

  // Setup PVM queues
  if (queues::pvm_queues_setup() < 0) {
    printf("Failed to setup PVM queues \n");
    return -1;
  }
  printf("Setup PVM queues  \n");

  // Setup Sequencer queues
  if (queues::seq_queues_setup() < 0) {
    printf("Failed to setup Sequencer queues \n");
    return -1;
  }
  printf("Setup Sequeuncer queues \n");

  return 0;
}

}  // namespace nvme_dp

