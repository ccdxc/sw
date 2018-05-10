#include <assert.h>

#include "dol/test/storage/utils.hpp"
#include "dol/test/storage/queues.hpp"
#include "dol/test/storage/rdma.hpp"
#include "dol/test/storage/qstate_if.hpp"

namespace nvme_dp {

// I/O Map dst flags
#define IO_XTS_ENCRYPT                          0x00000001
#define IO_XTS_DECRYPT                          0x00000002
#define IO_DST_REMOTE                           0x00000004


#define IO_BUF_WRITE_ROCE_SQ_WQE_OFFSET         192
#define IO_BUF_READ_ROCE_SQ_WQE_OFFSET          256

#define IO_BUF_SEQ_DB_OFFSET                    2560  // Size == IO_BUF_SEQ_DB_TOTAL_SIZE Size = 512 bytes

#define IO_BUF_SEQ_R2N_DB_OFFSET                384

#define IO_BUF_SEQ_QADDR_OFFSET                 3072  // Size == 64 bytes (supports 8 doorbells * 34 bit address)

#define IO_BUF_SEQ_R2N_QADDR_BIT_OFFSET                 204

#define IO_BUF_NVME_BE_CMD_OFFSET               3896

#define IO_BUF_WRITE_REQ_OFFSET                 4032

#define IO_BUF_DATA_OFFSET                      4096






const static uint32_t	kIOMapEntrySize		 = 1024;
const static uint32_t	kIOMapNumEntries	 = 8;
const static uint32_t	kIOBufMaxDataSize	 = 4096;
const static uint32_t	kIOBufHdrSize	 	 = 4096;
const static uint32_t	kIOBufEntrySize		 = (kIOBufHdrSize + kIOBufMaxDataSize);
const static uint32_t	kIOBufHdrXmitSize	 = (kIOBufHdrSize - IO_BUF_NVME_BE_CMD_OFFSET);
const static uint32_t	kIOBufNumEntriesLog2	 = 3;
const static uint32_t	kIOBufNumEntries	 = NUM_TO_VAL(kIOBufNumEntriesLog2);
const static uint32_t	kIOBFreeListEntrySizeLog2	 = 6;
const static uint32_t	kIOBFreeListEntrySize	 	 = NUM_TO_VAL(kIOBFreeListEntrySizeLog2);
const static uint32_t	kIOBRingStateSizeLog2	 	 = 6;
const static uint32_t	kIOBRingStateSize	 	 = NUM_TO_VAL(kIOBRingStateSizeLog2);


uint32_t IOBufSendLKeyBase = 128;
uint32_t IOBufWriteBackRKeyBase = IOBufSendLKeyBase + kIOBufNumEntries;

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

#define IO_BUF_ADDR(i)		\
    (io_buf_base_addr->pa() + ((i) * kIOBufEntrySize))

int init_iob_ring() {

  // Allocate the I/O buffers to the IOB free list
  for (int i = 0; i < (int) iob_free_list_num_entries-1; i++) {
    iob_free_list_base_addr->write_bit_fields(0, 34, IO_BUF_ADDR(i));
    iob_free_list_base_addr->write_thru();
    iob_free_list_base_addr->line_advance();
  }
  // Reset the line to 0 to get to the start
  iob_free_list_base_addr->line_set(0);

  // Form the IOB ring state pointing to th e IOB free list
  iob_ring_base_addr->write_bit_fields(0, 16, (uint16_t) (iob_free_list_num_entries-1));
  iob_ring_base_addr->write_bit_fields(16, 16, 0);
  iob_ring_base_addr->write_bit_fields(32, 16, (uint16_t) (iob_free_list_num_entries-1));
  iob_ring_base_addr->write_bit_fields(48, 16, (uint16_t) kIOBufNumEntriesLog2);
  iob_ring_base_addr->write_bit_fields(64, 64, iob_free_list_base_addr->pa());
  iob_ring_base_addr->write_bit_fields(128, 16, kIOBFreeListEntrySizeLog2);
  iob_ring_base_addr->write_thru();
  printf("IOB Ring Base setup, PA: %lx \n", iob_ring_base_addr->pa());

  return 0;
}

int setup_io_map() {
  // Populate entry corresponding to NSID 1
  io_map_base_addr->line_set(1);

  // Basic information
  io_map_base_addr->write_bit_fields(0, 64, io_map_base_addr->pa()); // Back pointer to itself
  io_map_base_addr->write_bit_fields(64, 32, 1); // NSID
  io_map_base_addr->write_bit_fields(96, 16, 1); // VFID
  io_map_base_addr->write_bit_fields(112, 32, IO_DST_REMOTE); // dst_flags

  // TBD: Populate R2N lif for testing local target
  
  // ROCE LIF 
  uint16_t lif;
  uint8_t qtype;
  uint32_t qid;
  uint64_t qaddr;
#if 0
  if (rdma_roce_ini_sq_info(&lif, &qtype, &qid, &qaddr) < 0) {
    printf("Can't get RDMA Initiator SQ info \n");
    assert(0);
    return -1;
  }
#endif
  // Fill the Sequencer ROCE descriptor
  lif = queues::get_pvm_lif();
  qtype = SQ_TYPE;
  qid = get_rdma_pvm_roce_init_sq();
  if (qstate_if::get_qstate_addr(lif, qtype, qid, &qaddr) < 0) {
    printf("Can't get PVM's ROCE SQ qaddr \n");
    assert(0);
    return -1;
  }

  io_map_base_addr->write_bit_fields(216, 11, lif);
  io_map_base_addr->write_bit_fields(227, 3, qtype);
  io_map_base_addr->write_bit_fields(230, 24, qid);
  io_map_base_addr->write_bit_fields(254, 34, qaddr);
  
  // NVME backend info
  io_map_base_addr->write_bit_fields(288, 32, 0);
  io_map_base_addr->write_bit_fields(320, 16, queues::nvme_e2e_ssd_handle());
  io_map_base_addr->write_bit_fields(336, 8, 0);

  // Commit to HBM
  io_map_base_addr->write_thru();
  printf("IOB MAP (for NSID 1) setup, PA: %lx \n", io_map_base_addr->pa());

  return 0;
}

int setup_one_io_buffer(int index) {
  // Start with the line set to the index
  io_buf_base_addr->line_set(index);
  // Set the IO buffer address in the IO buffer
  io_buf_base_addr->write_bit_fields(0, 34, io_buf_base_addr->pa());

  // Get the Sequencer ROCE SQ
  //uint32_t seq_pdma_q = queues::get_seq_pdma_sq(index);
  uint32_t seq_roce_q = queues::get_seq_roce_sq(index);
  uint64_t seq_roce_qaddr;
  if (qstate_if::get_qstate_addr(queues::get_seq_lif(), SQ_TYPE, seq_roce_q, &seq_roce_qaddr) < 0) {
    printf("Can't get PVM's Seq ROCE SQ qaddr \n");
    return -1;
  }

  // SendLKey  = base value + index of the IO buffer
  uint32_t send_lkey = IOBufSendLKeyBase + index;
  uint32_t write_back_rkey = IOBufWriteBackRKeyBase + index;

  // Register IO Buffer with ROCE using identity mapping. 
  // No remote access => only LKey (based on base value + offset).
  RdmaMemRegister(io_buf_base_addr->pa(), io_buf_base_addr->pa(), kIOBufEntrySize,
                  send_lkey, write_back_rkey, true);


  // Fill the write wqe
  uint32_t roce_write_wqe_base = IO_BUF_WRITE_ROCE_SQ_WQE_OFFSET * 8;
  uint64_t data_offset_pa = io_buf_base_addr->pa() + IO_BUF_NVME_BE_CMD_OFFSET;
  uint32_t data_len = kIOBufHdrXmitSize + kIOBufMaxDataSize;
  printf("Write data_offset %lx, data len %u \n", data_offset_pa, data_len);
  io_buf_base_addr->write_bit_fields(roce_write_wqe_base, 64, data_offset_pa); // wrid, ptr to actual xmit data
  io_buf_base_addr->write_bit_fields(roce_write_wqe_base+64, 4,  kRdmaSendOpType);  
  io_buf_base_addr->write_bit_fields(roce_write_wqe_base+72, 8,  1); // Num SGEs = 1

  // Store doorbell information of PVM's ROCE CQ in immediate data
  if (kRdmaSendOpType == RDMA_OP_TYPE_SEND_IMM) {
      io_buf_base_addr->write_bit_fields(roce_write_wqe_base+96, 11, queues::get_pvm_lif());
      io_buf_base_addr->write_bit_fields(roce_write_wqe_base+107, 3, CQ_TYPE);
      io_buf_base_addr->write_bit_fields(roce_write_wqe_base+110, 18, get_rdma_pvm_roce_tgt_cq());
  }
  io_buf_base_addr->write_bit_fields(roce_write_wqe_base+192, 32, data_len);  // data len

  // Form the SGE
  io_buf_base_addr->write_bit_fields(roce_write_wqe_base+256, 64,  data_offset_pa); // SGE-va, same as pa
  io_buf_base_addr->write_bit_fields(roce_write_wqe_base+256+64, 32, data_len);
  io_buf_base_addr->write_bit_fields(roce_write_wqe_base+256+64+32, 32, send_lkey);


  // Fill the read wqe
  uint32_t roce_read_wqe_base = IO_BUF_READ_ROCE_SQ_WQE_OFFSET * 8;
  data_offset_pa = io_buf_base_addr->pa() + IO_BUF_NVME_BE_CMD_OFFSET;
  data_len = kIOBufHdrXmitSize;
  printf("Read data_offset %lx, data len %u \n", data_offset_pa, data_len);
  io_buf_base_addr->write_bit_fields(roce_read_wqe_base, 64, data_offset_pa); // wrid, ptr to actual xmit data
  io_buf_base_addr->write_bit_fields(roce_read_wqe_base+64, 4,  kRdmaSendOpType);  
  io_buf_base_addr->write_bit_fields(roce_read_wqe_base+72, 8,  1); // Num SGEs = 1

  // Store doorbell information of PVM's ROCE CQ in immediate data
  if (kRdmaSendOpType == RDMA_OP_TYPE_SEND_IMM) {
      io_buf_base_addr->write_bit_fields(roce_read_wqe_base+96, 11, queues::get_pvm_lif());
      io_buf_base_addr->write_bit_fields(roce_read_wqe_base+107, 3, CQ_TYPE);
      io_buf_base_addr->write_bit_fields(roce_read_wqe_base+110, 18, get_rdma_pvm_roce_tgt_cq());
  }
  io_buf_base_addr->write_bit_fields(roce_read_wqe_base+192, 32, data_len);  // data len

  // Form the SGE
  io_buf_base_addr->write_bit_fields(roce_read_wqe_base+256, 64,  data_offset_pa); // SGE-va, same as pa
  io_buf_base_addr->write_bit_fields(roce_read_wqe_base+256+64, 32, data_len);
  io_buf_base_addr->write_bit_fields(roce_read_wqe_base+256+64+32, 32, send_lkey);

  // Pre-form the (RDMA) write descriptor to point to the data buffer
  uint32_t write_back_wqe_base = IO_BUF_WRITE_REQ_OFFSET * 8;
  data_offset_pa = io_buf_base_addr->pa() + IO_BUF_DATA_OFFSET;
  data_len = kIOBufMaxDataSize;

  // Start the write WQE formation (WRID will be filled by P4+)
  io_buf_base_addr->write_bit_fields(write_back_wqe_base+64, 4, RDMA_OP_TYPE_WRITE);  // op_type
  io_buf_base_addr->write_bit_fields(write_back_wqe_base+72, 8, 1);  // Num SGEs = 1
  io_buf_base_addr->write_bit_fields(write_back_wqe_base+192, 32, (uint32_t) data_len);  // data len

#if 0
  // RDMA will ring the next doorbell with pndx increment,
  // print out this info to make it easy to locate in model.log
  uint64_t db_addr;
  uint64_t db_data;
  queues::get_capri_doorbell_with_pndx_inc(queues::get_seq_lif(), SQ_TYPE, seq_pdma_q, 0,
                                           &db_addr, &db_data);
  printf("write_back_wqe next doorbell db_addr %lx db_data %lx\n", db_addr, db_data);

  // Write WQE: remote side buffer with immediate data as doorbell
  io_buf_base_addr->write_bit_fields(write_back_wqe_base+96, 11, queues::get_seq_lif());
  io_buf_base_addr->write_bit_fields(write_back_wqe_base+107, 3, SQ_TYPE);
  io_buf_base_addr->write_bit_fields(write_back_wqe_base+110, 18, seq_pdma_q);
#endif
  io_buf_base_addr->write_bit_fields(write_back_wqe_base+128, 64, data_offset_pa); // va == pa
  io_buf_base_addr->write_bit_fields(write_back_wqe_base+128+64, 32, (uint32_t) data_len); // len
  io_buf_base_addr->write_bit_fields(write_back_wqe_base+128+64+32, 32, write_back_rkey); // rkey

  // Form the R2N sequencer doorbell
  uint32_t base_db_bit = (IO_BUF_SEQ_DB_OFFSET + IO_BUF_SEQ_R2N_DB_OFFSET) * 8;
  io_buf_base_addr->write_bit_fields(base_db_bit, 11, queues::get_seq_lif());
  io_buf_base_addr->write_bit_fields(base_db_bit+11, 3, SQ_TYPE);
  io_buf_base_addr->write_bit_fields(base_db_bit+14, 24, seq_roce_q);
  io_buf_base_addr->write_bit_fields(base_db_bit+38, 34, seq_roce_qaddr);

  uint32_t base_qaddr_bit = (IO_BUF_SEQ_QADDR_OFFSET * 8);
  io_buf_base_addr->write_bit_fields(base_qaddr_bit+IO_BUF_SEQ_R2N_QADDR_BIT_OFFSET, 34, seq_roce_qaddr);

  // Commit to HBM
  io_buf_base_addr->write_thru();
  printf("IOB buffer %d setup, PA: %lx \n", index, io_buf_base_addr->pa());
  return 0;
}

int setup_io_buffers() {
  // Setup IO buffers (except the last one as it can't be added to the IOB ring)
  for (int i = 0; i < (int) io_buf_num_entries-1; i++) {
    setup_one_io_buffer(i);
  }

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

int config () {

  // Update CQs with ROCE RQ context for buffer posting 
  if (queues::nvme_dp_update_cqs() < 0) {
    printf("Failed to update CQs \n");
    return -1;
  }
  printf("Updated CQs\n");

  //  Setup IO map for  NSID 1
  if (setup_io_map() < 0) {
    printf("Failed to setup IO MAP (for NSID 1)\n");
    return -1;
  }
  printf("Setup IO MAP (for NSID 1)\n");

  //  Setup IO buffers
  if (setup_io_buffers() < 0) {
    printf("Failed to setup IO buffers \n");
    return -1;
  }
  printf("Setup IO buffers \n");

  return 0;
}

}  // namespace nvme_dp

