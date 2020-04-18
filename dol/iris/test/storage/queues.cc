#include <stdio.h>
#include <strings.h>
#include <byteswap.h>
#include <math.h>

#include "dol/iris/test/storage/hal_if.hpp"
#include "dol/iris/test/storage/utils.hpp"
#include "dol/iris/test/storage/qstate_if.hpp"
#include "dol/iris/test/storage/nvme.hpp"
#include "dol/iris/test/storage/rdma.hpp"
#include "dol/iris/test/storage/nvme_dp.hpp"
#include "dol/iris/test/storage/queues.hpp"
#include "dol/iris/test/storage/xts.hpp"
#include "dol/iris/test/storage/compression.hpp"
#include "dol/iris/test/storage/ssd.hpp"
#include "dol/iris/test/storage/nicmgr_if.hpp"
#ifdef ELBA
#include "third-party/asic/elba/model/elb_top/elb_top_csr_defines.h"
#include "third-party/asic/elba/model/elb_wa/elb_wa_csr_define.h"
#else
#include "third-party/asic/capri/model/cap_top/cap_top_csr_defines.h"
#include "third-party/asic/capri/model/cap_wa/cap_wa_csr_define.h"
#endif

// NOTE CAREFULLY: BEGIN: When adding queues please ensure, calc_total_queues()
// accurate reflects all the counts

      static uint32_t	NvmeNumSQs		 = 3;

      static uint32_t	NvmeNumCQs		 = 3;

      static uint32_t	PvmNumSQs;                    // log2(Sum total of all PVM SQs)
const static uint32_t	kPvmNumNvmeSQs		 = 1;
const static uint32_t	kPvmNumR2nSQs		 = 1;
const static uint32_t	kPvmNumNvmeBeSQs	 = 4;
const static uint32_t	kPvmNumSsdSQs	 	 = 4;

      static uint32_t	PvmNumCQs;                    // log2(Sum total of all PVM CQs)
const static uint32_t	kPvmNumNvmeCQs		 = 1;
const static uint32_t	kPvmNumR2nCQs		 = 0; // 2^0 => 1 queue
const static uint32_t	kPvmNumNvmeBeCQs	 = 4;

      static uint32_t	SeqNumSQs;                    // log2(Sum total of all PVM CQs)
      static uint32_t	SeqNumPdmaSQs		 = 3; // pdma test may modify at run time
const static uint32_t	kSeqNumR2nSQs		 = 3;
      static uint32_t	kSeqNumXtsSQs		 = 3; // acc_scale test may modify at run time
      static uint32_t	kSeqNumXtsStatusSQs	 = 4; //    "    "
      static uint32_t	kSeqNumCompSQs		 = 4; // acc_scale test may modify at run time
      static uint32_t	kSeqNumCompStatusSQs = 4; //    "    "
const static uint32_t	kSeqNumRoceSQs		 = 5;


      static uint32_t	ArmNumQs;                    // log2(Sum total of all ARM Qs)
const static uint32_t	kArmTotalQs		 = 6; // absolute value and not log2 value

const static uint32_t	SeqNumAdminQs	 = 0; // log2(number of admin queues)

// NOTE CAREFULLY: END


const static uint32_t	kDefaultQstateEntrySize = 6; // Default qstate is 64 bytes
const static uint32_t	kSeqQstateEntrySize      = STORAGE_SEQ_CB_SIZE_SHFT;
const static uint32_t	kDefaultEntrySize	 = 6; // Default desc size is 64 bytes
const static uint32_t	kSeqCompStatusSQEntrySize = 7; // Seq compression status SQ is 128 bytes
const static uint32_t	kSeqXtsStatusSQEntrySize = 7; // Seq XTS status SQ is 128 bytes
const static uint32_t	kPvmNvmeSQEntrySize	 = 7; // PVM SQ is 128 bytes (NVME command + PVM header)
const static uint32_t	kNvmeCQEntrySize	 = 4; // NVME CQ is 16 bytes
const static uint32_t	kNvmeNumEntries		 = 6;
const static uint32_t	kPvmNumEntries		 = 6;
      static uint32_t   kSeqNumAccEntries = 6; // accelerator scale test may modify at run time
const static uint32_t	kSeqNumEntries	 	 = 6;
const static uint32_t	kArmNumEntries		 = 6;
const static uint32_t	kSeqAdminNumEntries	 = 6;
const static uint32_t	kSeqAdminQEntrySize	 = 6;

const static char	*kNvmePvmSqHandler	 = "storage_tx_nvme_sq_handler.bin";
const static char	*kPvmCqHandler		 = "storage_tx_pvm_cq_handler.bin";
const static char	*kR2nSqHandler		 = "storage_tx_r2n_sq_handler.bin";
const static char	*kNvmeBeCqHandler	 = "storage_tx_nvme_be_cq_handler.bin";
const static char	*kSeqPdmaSqHandler	 = "storage_tx_seq_pdma_entry_handler.bin";
const static char	*kSeqR2nSqHandler	 = "storage_tx_seq_r2n_entry_handler.bin";
const static char	*kSeqXtsSqHandler	 = STORAGE_SEQ_PGM_NAME_SQ_GEN;
const static char	*kSeqXtsStatusDesc0SqHandler = STORAGE_SEQ_PGM_NAME_CRYPTO_STATUS0;
const static char	*kSeqXtsStatusDesc1SqHandler = STORAGE_SEQ_PGM_NAME_CRYPTO_STATUS1;
const static char	*kPvmRoceSqHandler	 = "storage_tx_pvm_roce_sq_wqe_process.bin";
const static char	*kPvmRoceCqHandler	 = "storage_tx_roce_cq_handler.bin";
const static char	*kSeqCompSqHandler	 = STORAGE_SEQ_PGM_NAME_SQ_GEN;
const static char	*kSeqCompStatusDesc0SqHandler = STORAGE_SEQ_PGM_NAME_CPDC_STATUS0;
const static char	*kSeqCompStatusDesc1SqHandler = STORAGE_SEQ_PGM_NAME_CPDC_STATUS1;
const static char	*kArmQTimeoutHandler		 = "storage_nvme_timeout_iob_addr.bin";
const static char	*kArmQFreeHandler		 = "storage_nvme_free_iob_addr.bin";

const static uint32_t	kDefaultTotalRings	 = 1;
const static uint32_t	kDefaultHostRings	 = 1;
const static uint32_t	kDefaultNoHostRings	 = 0;
const static uint32_t	kNvmeBeTotalRings	 = 3;


const static uint32_t	kDbAddrHost		 = 0x400000;
const static uint32_t	kDbAddrNvme		 = 0xC00000;
#ifdef ELBA
const static uint32_t	kDbAddrLocal		 = ELB_ADDR_BASE_DB_WA_OFFSET +
                                                   ELB_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS;
#else
const static uint32_t	kDbAddrLocal		 = CAP_ADDR_BASE_DB_WA_OFFSET +
                                                   CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS;
#endif
const static uint32_t	kDbAddrUpdate		 = 0xB;
const static uint32_t	kDbAddrInc		 = ((3 << 2) + 1); // plus sched_wr_eval bit
const static uint32_t	kDbQidShift		 = 24;
const static uint32_t	kDbRingShift		 = 16;
const static uint32_t	kDbUpdateShift		 = 17;
const static uint32_t	kDbLifShift		 = 6;
const static uint32_t	kDbTypeShift		 = 3;
const static uint32_t	kNvmeDbQidShift		 = 16;
const static uint32_t	kNvmeDbTypeShift	 = 2;
const static uint32_t	kNvmeDbLifShift	 	 = 5;
const static uint32_t	kNvmeDbUpdateShift	 = 16;

// Special SSD marked for running E2e traffic with queues from
// NvmeSsd class
const static int	kE2eSsdhandle		 = 2;


// Offset of the Cndx in the Qstate
const static uint32_t	kQstateCndxOffset	 = 10;

const static uint32_t	kHbmSsdBitmapSize	 = (16 * 4096);

// BDFs for NVME, Sequencer and PVM LIF
const static uint32_t	kNvmeLifBdf		= 0xA;
const static uint32_t	kPvmLifBdf		= 0xB;
const static uint32_t	kSeqLifBdf		= 0xC;
const static uint32_t	kArmLifBdf		= 0xD;

namespace queues {

// Special SSD marked for running E2e traffic with queues from
// NvmeSsd class
std::unique_ptr<storage_test::NvmeSsd> nvme_e2e_ssd;

uint64_t ssd_cndx_addr[NUM_TO_VAL(kPvmNumNvmeBeSQs)];

uint64_t pvm_last_sq;
uint64_t pvm_last_cq;

uint64_t storage_hbm_ssd_bm_addr;

accel_ring_t nicmgr_accel_ring_tbl[ACCEL_RING_ID_MAX];

/*
 * Sequencer queue batching support: see seq_sq_batch_consume_entry()
 */
class queue_batch_t
{
public:
    queue_batch_t() :
        batch_desc(nullptr),
        user_ctx(nullptr),
        end_notify_fn(nullptr),
        batch_id(0),
        batch_size(0),
        batch_pd_idx(0)
    {
    }

    dp_mem_t                    *batch_desc;
    void                        *user_ctx;
    seq_sq_batch_end_notify_t   end_notify_fn;
    uint64_t                    batch_id;
    uint16_t                    batch_size;
    uint16_t                    batch_pd_idx;
};

typedef struct queues_ {
  dp_mem_t *mem;
  uint16_t entry_size;
  uint16_t num_entries;
  uint16_t batch_limit;

  /*
   * Not all sequencer queues are used by DOL with batching so
   * we allocate batch structure only on demand.
   */
  queue_batch_t *batch;
} queues_t;

// NVME Submission, Completion queues
queues_t *nvme_sqs = NULL;
queues_t *nvme_cqs = NULL;

// Sequencer Submission queues
queues_t *seq_sqs = NULL;

// PVM Submission, Completion queues
queues_t *pvm_sqs = NULL;
queues_t *pvm_cqs = NULL;

// ARM queues
queues_t *arm_qs = NULL;

// Admin queues
queues_t *admin_qs = NULL;

uint64_t nvme_lif, seq_lif, pvm_lif, arm_lif;

uint32_t host_nvme_sq_base;
uint32_t pvm_nvme_sq_base;
uint32_t pvm_r2n_tgt_sq_base;
uint32_t pvm_r2n_init_sq_base;
uint32_t pvm_nvme_be_sq_base;
uint32_t pvm_ssd_sq_base;
uint32_t pvm_seq_pdma_sq_base;
uint32_t pvm_seq_r2n_sq_base;
uint32_t pvm_r2n_host_sq_base;
uint32_t pvm_seq_xts_sq_base;
uint32_t pvm_seq_xts_status_sq_base;
uint32_t pvm_seq_roce_sq_base;
uint32_t pvm_seq_comp_sq_base;
uint32_t pvm_seq_comp_status_sq_base;
uint32_t host_nvme_cq_base;
uint32_t pvm_nvme_cq_base;
uint32_t pvm_r2n_cq_base;
uint32_t pvm_nvme_be_cq_base;

#define MAX_ARM_FREE_IOB_QS	3
uint32_t arm_q_base;
uint32_t arm_sq;
uint32_t arm_cq;
uint32_t arm_timeout_q;
uint32_t arm_free_iob_q[MAX_ARM_FREE_IOB_QS];

void *pndx_data_va;
uint64_t pndx_data_pa;
void *cndx_data_va;
uint64_t cndx_data_pa;

// Forward declaration with default mem_type
int queue_init(queues_t *queue, uint16_t num_entries, uint16_t entry_size,
               dp_mem_align_t mem_align = DP_MEM_ALIGN_SPEC,
               dp_mem_type_t mem_type = DP_MEM_TYPE_HBM,
               uint16_t batch_limit = 0);
int seq_queue_setup(queues_t *q_ptr, uint32_t qid,
                    storage_seq_qgroup_t qgroup, char *pgm_bin,
                    uint16_t total_rings, uint16_t host_rings,
                    uint16_t num_entries, uint16_t entry_size,
                    dp_mem_type_t mem_type = DP_MEM_TYPE_HBM,
                    uint16_t batch_limit = 0,
                    char *desc1_pgm_bin = nullptr);
int storage_tx_queue_setup(queues_t *q_ptr, uint32_t qid, char *pgm_bin,
                           uint16_t total_rings, uint16_t host_rings,
                           dp_mem_type_t mem_type = DP_MEM_TYPE_HBM);

static uint32_t log_2(uint32_t x);

void lif_params_init(hal_if::lif_params_t *params, uint16_t type,
                     uint16_t num_entries, uint16_t num_queues) {
  params->type[type].valid = true;
  params->type[type].queue_size = num_entries; // you get 2^num_entries
  params->type[type].num_queues = num_queues;  // you get 2^num_queues
}

int nvme_e2e_ssd_handle() {
  return kE2eSsdhandle;
}

int nvme_e2e_ssd_sq_init(queues_t *queue, uint16_t num_entries, uint16_t entry_size) {
  // NOTE: Assumes that nvme_e2e SSD SQ is initated with 64 entries x 64 bytes size
  // TODO: Fix to make this consistent or assert on the values
  storage_test::SsdWorkingParams params;
  nvme_e2e_ssd->GetWorkingParams(&params);
  queue->mem = params.subq;
  queue->mem->line_set(0);
  queue->entry_size = entry_size;
  queue->num_entries = num_entries;
  return 0;
}

int nvme_e2e_ssd_cq_init(queues_t *queue, uint16_t num_entries, uint16_t entry_size) {
  // NOTE: Assumes that nvme_e2e SSD SQ is initated with 64 entries x 16 bytes size
  // TODO: Fix to make this consistent or assert on the values
  storage_test::SsdWorkingParams params;
  nvme_e2e_ssd->GetWorkingParams(&params);
  queue->mem = params.compq;
  queue->mem->line_set(0);
  queue->entry_size = entry_size;
  queue->num_entries = num_entries;
  return 0;
}

void nvme_e2e_ssd_db_init(uint64_t db_addr, uint64_t db_data) {
  nvme_e2e_ssd->EnableInterrupt(db_addr, db_data);
}

int queue_init(queues_t *queue, uint16_t num_entries, uint16_t entry_size,
               dp_mem_align_t mem_align,
               dp_mem_type_t mem_type, uint16_t batch_limit) {
  memset(queue, 0, sizeof(*queue));
  queue->mem = new dp_mem_t(num_entries, entry_size, mem_align,
                            mem_type, entry_size);
  queue->entry_size = entry_size;
  queue->num_entries = num_entries;
  queue->batch_limit = batch_limit;
  return 0;
}

int queue_pre_init(queues_t *queue, dp_mem_t *mem, uint16_t num_entries,
                   uint16_t entry_size, uint16_t batch_limit) {
  memset(queue, 0, sizeof(*queue));
  queue->mem = mem;
  queue->mem->line_set(0);
  queue->entry_size = entry_size;
  queue->num_entries = num_entries;
  queue->batch_limit = batch_limit;
  return 0;
}

dp_mem_t *queue_consume_entry(queues_t *queue, uint16_t *index) {
  if (!queue->mem || !index) return nullptr;
  queue->mem->line_set(queue->mem->next_line_get());
  *index = queue->mem->next_line_set();
  return queue->mem;
}


dp_mem_t *queue_consumed_entry_get(queues_t *queue, uint16_t *index) {
  if (!queue->mem || !index) return nullptr;
  *index = queue->mem->next_line_get();
  return queue->mem;
}

void seq_queue_pdma_num_set(uint64_t& num_pdma_queues) {

    // Make adjustment for the number of seq SQs needed for PDMA testing.
    // Note num_pdma_queues denotes 2 ^ num_pdma_queues
    if (num_pdma_queues < SeqNumPdmaSQs) {
        num_pdma_queues = SeqNumPdmaSQs;
    }

    SeqNumPdmaSQs = num_pdma_queues;
}

void seq_queue_acc_sub_num_set(uint64_t& acc_scale_submissions,
                               uint64_t& acc_scale_chain_replica,
                               uint32_t acc_scale_test_max,
                               uint32_t acc_scale_test_num_true_chaining_tests) {
    uint32_t max_acc_entries;
    uint32_t max_queues;
    uint32_t max_status_queues;

    // acc_scale_tests_max_chains is the only entity not a power of 2.
    max_queues = log_2(NUM_TO_VAL(acc_scale_chain_replica) * acc_scale_test_max);

    // Make adjustment for the number of seq SQs needed for accelerator
    // scale testing.
    //
    // Note that both acc_scale_submissions and acc_scale_chain_replica
    // are in power of 2. Total is bumped up to ensure CI != PI
    // during submission
    max_acc_entries = log_2(NUM_TO_VAL(acc_scale_submissions)) + 1;
    max_status_queues = log_2(NUM_TO_VAL(acc_scale_submissions) *
                              (NUM_TO_VAL(acc_scale_chain_replica) *
                               acc_scale_test_num_true_chaining_tests));

    kSeqNumAccEntries = std::max(kSeqNumAccEntries, max_acc_entries);
    kSeqNumCompSQs = std::max(kSeqNumCompSQs, max_queues);
    kSeqNumCompStatusSQs = std::max(kSeqNumCompStatusSQs, max_status_queues);
    kSeqNumXtsSQs = std::max(kSeqNumXtsSQs, max_queues);
    kSeqNumXtsStatusSQs = std::max(kSeqNumXtsStatusSQs, max_status_queues);
}

int seq_admin_queue_setup(queues_t *q_ptr, uint16_t num_entries,
                          uint16_t entry_size, dp_mem_type_t mem_type)
{

  int   ret;

  if (run_nicmgr_tests) {

      /*
       * Allocate twice the number of entries as the nicmgr will
       * implicitly divide the adminQ space, half for submission and
       * half for completion.
       */
      if (queue_init(q_ptr, NUM_TO_VAL(num_entries + 1),
                     NUM_TO_VAL(entry_size), DP_MEM_ALIGN_PAGE,
                     mem_type) < 0) {
        printf("Unable to allocate memory for Seq AdminQ\n");
        return -1;
      }

      ret = nicmgr_if::nicmgr_if_lif_init(seq_lif);
      if (ret == 0) {
          ret = nicmgr_if::nicmgr_if_lif_reset(seq_lif);
      }
#if 0
      /*
       * lib_driver is going away so we no longer use it
       * to drive the adminQ in DOL.
       */
      if (ret == 0) {
          ret = nicmgr_if::nicmgr_if_admin_queue_init(seq_lif, num_entries,
                                                      q_ptr->mem->pa());
      }
#endif

      if (ret) {
          printf("Failed to setup Seq AdminQ state \n");
          return -1;
      }
  }
  return 0;
}

int seq_queue_setup(queues_t *q_ptr, uint32_t qid,
                    storage_seq_qgroup_t qgroup, char *pgm_bin,
                    uint16_t total_rings, uint16_t host_rings,
                    uint16_t num_entries, uint16_t entry_size,
                    dp_mem_type_t mem_type, uint16_t batch_limit,
                    char *desc1_pgm_bin)
{

  int   ret;

  // Initialize the queue in the DOL enviroment
  if (queue_init(q_ptr, NUM_TO_VAL(num_entries), NUM_TO_VAL(entry_size),
                 DP_MEM_ALIGN_SPEC, mem_type, batch_limit) < 0) {
    printf("Unable to allocate memory for Seq SQ %d\n", qid);
    return -1;
  }
  printf("Initialized Seq SQ %d \n", qid);

  // Setup the queue state in Capri:
  if (run_nicmgr_tests) {
      ret = nicmgr_if::nicmgr_if_seq_queue_init(seq_lif, qgroup,
                                                qid, total_rings, host_rings,
                                                num_entries, q_ptr->mem->pa(),
                                                entry_size);

  } else {
      ret = qstate_if::setup_seq_q_state(seq_lif, SQ_TYPE, qgroup, qid,
                                         pgm_bin, total_rings, host_rings,
                                         num_entries, q_ptr->mem->pa(),
                                         entry_size, desc1_pgm_bin);
  }

  if (ret) {
      printf("Failed to setup Seq SQ %d state \n", qid);
      return -1;
  }
  return 0;
}

int storage_tx_queue_setup(queues_t *q_ptr, uint32_t qid, char *pgm_bin,
                           uint16_t total_rings, uint16_t host_rings,
                           dp_mem_type_t mem_type) {

  // Initialize the queue in the DOL enviroment
  if (queue_init(q_ptr, NUM_TO_VAL(kSeqNumEntries), NUM_TO_VAL(kDefaultEntrySize),
                 DP_MEM_ALIGN_SPEC, mem_type) < 0) {
    printf("Unable to allocate host memory for PVM Seq SQ %d\n", qid);
    return -1;
  }
  printf("Initialized PVM Seq SQ %d \n", qid);

  // Setup the queue state in Capri:
  // qgroup below is a dontcare in this case
  if (qstate_if::setup_seq_q_state(seq_lif, SQ_TYPE, STORAGE_SEQ_QGROUP_CPDC,
                                   qid, pgm_bin, total_rings, host_rings,
                                   kSeqNumEntries, q_ptr->mem->pa(),
                                   kDefaultEntrySize) < 0) {
    printf("Failed to setup PVM Storage Tx SQ %d state \n", qid);
    return -1;
  }
  return 0;
}

static uint32_t
log_2(uint32_t x)
{
  uint32_t log = 0;
  while (NUM_TO_VAL(log) < (int) x) {
    log++;
  }
  return log;
}

static void
calc_total_queues()
{
  uint32_t count;

  // No dynamic calculation for NvmeNumSQs, NvmeNumCQs

  // Get the total count and log2 to nearest power of 2
  count = NUM_TO_VAL(kPvmNumNvmeSQs) +
          NUM_TO_VAL(kPvmNumR2nSQs) +            // target R2N SQ in P4+
          NUM_TO_VAL(kPvmNumNvmeBeSQs) +
          NUM_TO_VAL(kPvmNumSsdSQs) +
          NUM_TO_VAL(kPvmNumR2nSQs) +            // target R2N SQ in host
          NUM_TO_VAL(kPvmNumR2nSQs);             // initiator R2N SQ
  PvmNumSQs = log_2(count);
  printf("PVM SQS %u \n", PvmNumSQs);

  // Get the total count and log2 to nearest power of 2
  count = NUM_TO_VAL(kPvmNumNvmeCQs) +
          NUM_TO_VAL(kPvmNumR2nCQs) +
          NUM_TO_VAL(kPvmNumNvmeBeCQs);
  PvmNumCQs = log_2(count);
  printf("PVM CQS %u \n", PvmNumCQs);

  // Get the total count and log2 to nearest power of 2
  count = NUM_TO_VAL(SeqNumPdmaSQs) +
          NUM_TO_VAL(kSeqNumR2nSQs) +
          NUM_TO_VAL(kSeqNumXtsSQs) +
          NUM_TO_VAL(kSeqNumXtsStatusSQs) +
          NUM_TO_VAL(kSeqNumCompSQs) +
          NUM_TO_VAL(kSeqNumCompStatusSQs) +
          NUM_TO_VAL(kSeqNumRoceSQs);
  SeqNumSQs = log_2(count);
  printf("Seq SQS %u \n", SeqNumSQs);

  ArmNumQs = log_2(kArmTotalQs);
  printf("ARM QS %u \n", ArmNumQs);

  printf("Admin QS %u \n", SeqNumAdminQs);
}

static void
alloc_queues()
{
  if ((nvme_sqs = (queues_t *) malloc(sizeof(queues_t) * NUM_TO_VAL(NvmeNumSQs))) == NULL) {
    printf("can't allocate nvme_sqs \n");
    exit(1);
  }
  if ((nvme_cqs = (queues_t *) malloc(sizeof(queues_t) * NUM_TO_VAL(NvmeNumCQs))) == NULL) {
    printf("can't allocate nvme_cqs n");
    exit(1);
  }
  if ((pvm_sqs = (queues_t *) malloc(sizeof(queues_t) * NUM_TO_VAL(PvmNumSQs))) == NULL) {
    printf("can't allocate pvm_sqs n");
    exit(1);
  }
  if ((pvm_cqs = (queues_t *) malloc(sizeof(queues_t) * NUM_TO_VAL(PvmNumCQs))) == NULL) {
    printf("can't allocate pvm_cqs n");
    exit(1);
  }
  if ((seq_sqs = (queues_t *) malloc(sizeof(queues_t) * NUM_TO_VAL(SeqNumSQs))) == NULL) {
    printf("can't allocate seq_sqs n");
    exit(1);
  }
  if ((arm_qs = (queues_t *) malloc(sizeof(queues_t) * NUM_TO_VAL(ArmNumQs))) == NULL) {
    printf("can't allocate arm_qs n");
    exit(1);
  }
  if ((admin_qs = (queues_t *) malloc(sizeof(queues_t) * NUM_TO_VAL(SeqNumAdminQs))) == NULL) {
    printf("can't allocate admin_qs n");
    exit(1);
  }
}

int resources_init() {

  // Calculate the total queues for all queue types
  calc_total_queues();

  // Allocate all queues
  alloc_queues();

  // Allocatge HBM address for storage
  nvme_e2e_ssd.reset(new storage_test::NvmeSsd());

  // Allocate storage SSD bitmap address
  if (utils::hbm_addr_alloc(kHbmSsdBitmapSize, &storage_hbm_ssd_bm_addr) < 0) {
    printf("Can't allocate SSD bitmap address \n");
    return -1;
  }

  // Create the just one global 32 bit pndx counter for all SSDs, NVME driver,
  // R2N/PVM SQ etc.  // This is needed as a dummy for the P4+ program to write
  // the pndx to a valid host address. The SSD emulation layer implements this
  // for the E2E cases.
  pndx_data_va = ALLOC_HOST_MEM(kMinHostMemAllocSize);
  if (pndx_data_va == nullptr) {
    printf("Unable to allocate host memory for p_ndx\n");
    return -1;
  }
  memset(pndx_data_va, 0, kMinHostMemAllocSize);
  pndx_data_pa = HOST_MEM_V2P(pndx_data_va);

  cndx_data_va = ALLOC_HOST_MEM(kMinHostMemAllocSize);
  if (cndx_data_va == nullptr) {
    printf("Unable to allocate host memory for c_ndx\n");
    return -1;
  }
  memset(cndx_data_va, 0, kMinHostMemAllocSize);
  cndx_data_pa = HOST_MEM_V2P(cndx_data_va);


  return 0;
}


int lifs_setup() {

  // Create NVME, Sequencer and PVM LIFs
  hal_if::lif_params_t nvme_lif_params, seq_lif_params, pvm_lif_params;

  // Let nicmgr create all the rserved LIFs first
  if (run_nicmgr_tests) {
      uint32_t nicmgr_num_seq_queues;
      uint32_t num_seq_queues = (uint32_t)NUM_TO_VAL(SeqNumSQs);

      if (nicmgr_if::nicmgr_if_init()) {
          return -1;
      }
      if (nicmgr_if::nicmgr_if_reset()) {
          return -1;
      }
      if (nicmgr_if::nicmgr_if_identify()) {
          return -1;
      }
      if (nicmgr_if::nicmgr_if_lif_identify(&seq_lif, &nicmgr_num_seq_queues,
                                            nicmgr_accel_ring_tbl,
                                            sizeof(nicmgr_accel_ring_tbl))) {
          return -1;
      }
      if (nicmgr_num_seq_queues < num_seq_queues) {
          printf("ERROR: nicmgr_num_seq_queues %u is less than SeqNumSQs %u\n",
                 nicmgr_num_seq_queues, num_seq_queues);
          assert(nicmgr_num_seq_queues >= num_seq_queues);
      }
  }

  bzero(&nvme_lif_params, sizeof(nvme_lif_params));
  lif_params_init(&nvme_lif_params, SQ_TYPE, kDefaultQstateEntrySize, NvmeNumSQs);
  lif_params_init(&nvme_lif_params, CQ_TYPE, kDefaultQstateEntrySize, NvmeNumCQs);

  if (hal_if::create_lif(&nvme_lif_params, &nvme_lif) < 0) {
    printf("can't create nvme lif \n");
    return -1;
  }
  printf("NVME LIF %u/%lu created\n", nvme_lif_params.sw_lif_id, nvme_lif);

  if (hal_if::set_lif_bdf(nvme_lif, kNvmeLifBdf) < 0) {
    printf("Can't set NVME LIF %lu BDF %u \n", nvme_lif, kNvmeLifBdf);
    return -1;
  }
  printf("Successfully set NVME LIF %lu BDF %u \n", nvme_lif, kNvmeLifBdf);

  bzero(&pvm_lif_params, sizeof(pvm_lif_params));
  lif_params_init(&pvm_lif_params, SQ_TYPE, kDefaultQstateEntrySize, PvmNumSQs);
  lif_params_init(&pvm_lif_params, CQ_TYPE, kDefaultQstateEntrySize, PvmNumCQs);

  if (hal_if::create_lif(&pvm_lif_params, &pvm_lif) < 0) {
    printf("can't create PVM lif \n");
    return -1;
  }
  printf("PVM LIF %u/%lu created\n", pvm_lif_params.sw_lif_id, pvm_lif);

  if (hal_if::set_lif_bdf(pvm_lif, kPvmLifBdf) < 0) {
    printf("Can't set PVM LIF %lu BDF %u \n", pvm_lif, kPvmLifBdf);
    return -1;
  }
  printf("Successfully set PVM LIF %lu BDF %u \n", pvm_lif, kPvmLifBdf);

  if (!run_nicmgr_tests) {
      bzero(&seq_lif_params, sizeof(seq_lif_params));
      lif_params_init(&seq_lif_params, SQ_TYPE, kSeqQstateEntrySize, SeqNumSQs);
      if (hal_if::create_lif(&seq_lif_params, &seq_lif) < 0) {
        printf("can't create Sequencer lif \n");
        return -1;
      }
      printf("Sequencer LIF %u/%lu created\n", seq_lif_params.sw_lif_id, seq_lif);
  }

  if (hal_if::set_lif_bdf(seq_lif, kSeqLifBdf) < 0) {
    printf("Can't set Sequencer LIF %lu BDF %u \n", seq_lif, kSeqLifBdf);
    return -1;
  }
  printf("Successfully set Sequencer LIF %lu BDF %u \n", seq_lif, kSeqLifBdf);

  // Create ARM LIF
  hal_if::lif_params_t arm_lif_params;

  bzero(&arm_lif_params, sizeof(arm_lif_params));
  lif_params_init(&arm_lif_params, SQ_TYPE, kArmNumEntries, ArmNumQs);

  if (hal_if::create_lif(&arm_lif_params, &arm_lif) < 0) {
    printf("can't create arm lif \n");
    return -1;
  }
  printf("ARM LIF %u/%lu created\n", arm_lif_params.sw_lif_id, arm_lif);

  if (hal_if::set_lif_bdf(arm_lif, kArmLifBdf) < 0) {
    printf("Can't set ARM LIF %lu BDF %u \n", arm_lif, kArmLifBdf);
    return -1;
  }
  printf("Successfully set ARM LIF %lu BDF %u \n", arm_lif, kArmLifBdf);

  return 0;
}


int
nvme_pvm_queues_setup() {
  int i;

  // Initialize NVME SQs
  host_nvme_sq_base = 0; // first queue
  for (i = 0; i < (int) NUM_TO_VAL(NvmeNumSQs); i++) {
    // Initialize the queue in the DOL enviroment
    if (queue_init(&nvme_sqs[i], NUM_TO_VAL(kNvmeNumEntries),
                   NUM_TO_VAL(kDefaultEntrySize)) < 0) {
      printf("Unable to allocate host memory for NVME SQ %d\n", i);
      return -1;
    }
    printf("Initialized NVME SQ %d \n", i);

    // Setup the queue state in Capri
    if (qstate_if::setup_q_state(nvme_lif, SQ_TYPE, i, (char *) kNvmePvmSqHandler,
                                 kDefaultTotalRings, kDefaultHostRings,
                                 kNvmeNumEntries, nvme_sqs[i].mem->pa(),
                                 kDefaultEntrySize, true, pvm_lif, SQ_TYPE,
                                 (i%2), 0, 0, storage_hbm_ssd_bm_addr, 0, 0, 0) < 0) {
      printf("Failed to setup NVME SQ %d state \n", i);
      return -1;
    }
  }


  // Initialize NVME CQs
  host_nvme_cq_base = 0; // first queue
  for (i = 0; i < (int) NUM_TO_VAL(NvmeNumCQs); i++) {
    // Initialize the queue in the DOL enviroment
    if (queue_init(&nvme_cqs[i], NUM_TO_VAL(kNvmeNumEntries),
                   NUM_TO_VAL(kNvmeCQEntrySize)) < 0) {
      printf("Unable to allocate host memory for NVME CQ %d\n", i);
      return -1;
    }
    printf("Initialized NVME CQ %d \n", i);

    // Setup the queue state in Capri:
    // 1. no dst queues for these as these are in host
    // 2. no program address for these as these are in host
    // 3. The push address is that of the SSD's SQ's PI
    if (qstate_if::setup_pci_q_state(nvme_lif, CQ_TYPE, i,
                                     kDefaultTotalRings, kDefaultHostRings,
                                     kNvmeNumEntries, nvme_cqs[i].mem->pa(),
                                     kNvmeCQEntrySize, pndx_data_pa,
                                     0, 0, 0) < 0) {
      printf("Failed to setup NVME CQ %d state \n", i);
      return -1;
    }
  }
  return 0;
}

int
nvme_dp_update_cqs() {
  int i;

  // ROCE LIF
  uint16_t lif;
  uint8_t qtype;
  uint32_t qid;
  uint64_t qaddr;
  uint64_t base_pa;

  if (rdma_roce_ini_rq_info(&lif, &qtype, &qid, &qaddr, &base_pa) < 0) {
    printf("Can't get RDMA Initiator RQ info \n");
    assert(0);
    return -1;
  }

  for (i = 0; i < (int) NUM_TO_VAL(NvmeNumCQs); i++) {
    if (qstate_if::update_nvme_cq_state(nvme_lif, CQ_TYPE, i,
                                        lif, qtype, qid, qaddr, base_pa) < 0) {
      printf("Failed to update NVME CQ %d state \n", i);
      return -1;
    }
  }

  return 0;
}

int
nvme_dp_queues_setup() {
  int i;

  // Initialize NVME CQs first
  host_nvme_cq_base = 0; // first queue
  for (i = 0; i < (int) NUM_TO_VAL(NvmeNumCQs); i++) {
    // Initialize the queue in the DOL enviroment
    if (queue_init(&nvme_cqs[i], NUM_TO_VAL(kNvmeNumEntries),
                   NUM_TO_VAL(kNvmeCQEntrySize), DP_MEM_ALIGN_SPEC,
                   DP_MEM_TYPE_HOST_MEM) < 0) {
      printf("Unable to allocate host memory for NVME CQ %d\n", i);
      return -1;
    }
    printf("Initialized NVME CQ %d \n", i);

    // Setup the queue state in Capri:
    // 1. no dst queues for these as these are in host
    // 2. no program address for these as these are in host
    // 3. The push address is that of the SSD's SQ's PI
    if (qstate_if::setup_nvme_cq_state(nvme_lif, CQ_TYPE, i,
                                       kDefaultTotalRings, kDefaultHostRings,
                                       kNvmeNumEntries, nvme_cqs[i].mem->pa(),
                                       kNvmeCQEntrySize, 0, 0, 0, 0) < 0) {
      printf("Failed to setup NVME CQ %d state \n", i);
      return -1;
    }
  }

  // Initialize NVME SQs
  host_nvme_sq_base = 0; // first queue
  for (i = 0; i < (int) NUM_TO_VAL(NvmeNumSQs); i++) {
    // Initialize the queue in the DOL enviroment
    if (queue_init(&nvme_sqs[i], NUM_TO_VAL(kNvmeNumEntries),
                   NUM_TO_VAL(kDefaultEntrySize), DP_MEM_ALIGN_SPEC,
                   DP_MEM_TYPE_HOST_MEM) < 0) {
      printf("Unable to allocate host memory for NVME SQ %d\n", i);
      return -1;
    }
    printf("Initialized NVME SQ %d \n", i);

    // Setup the queue state in Capri
    if (qstate_if::setup_nvme_sq_state(nvme_lif, SQ_TYPE, i, NULL,
                                       kDefaultTotalRings, kDefaultHostRings,
                                       kNvmeNumEntries, nvme_sqs[i].mem->pa(),
                                       kDefaultEntrySize, 0, i, nvme_lif, CQ_TYPE,
                                       i, arm_lif, SQ_TYPE, arm_q_base,
                                       nvme_dp::get_io_map_base_addr(),
                                       nvme_dp::get_io_map_num_entries(),
                                       nvme_dp::get_iob_ring_base_addr()) < 0) {
      printf("Failed to setup NVME SQ %d state \n", i);
      return -1;
    }
  }

  return 0;
}

int
pvm_queues_setup() {
  int i, j;

  // Initialize PVM SQs for processing commands from NVME VF only
  // Note: i is overall index across PVM SQs, j iterates the loop
  pvm_nvme_sq_base = 0; // first queue
  for (j = 0, i = 0; j < (int) NUM_TO_VAL(kPvmNumNvmeSQs); j++, i++) {
    // Initialize the queue in the DOL enviroment
    if (queue_init(&pvm_sqs[i], NUM_TO_VAL(kPvmNumEntries),
                   NUM_TO_VAL(kPvmNvmeSQEntrySize)) < 0) {
      printf("Unable to allocate host memory for PVM NVME SQ %d\n", i);
      return -1;
    }
    printf("Initialized PVM NVME SQ %d \n", i);

    // Setup the queue state in Capri:
    // 1. no dst queues for these as these are in host
    // 2. no program address for these as these are in host
    // 3. The push address is that of the SSD's SQ's PI
    if (qstate_if::setup_pci_q_state(pvm_lif, SQ_TYPE, i,
                                     kDefaultTotalRings, kDefaultHostRings,
                                     kPvmNumEntries, pvm_sqs[i].mem->pa(),
                                     kPvmNvmeSQEntrySize, pndx_data_pa,
                                     0, 0, 0) < 0) {
      printf("Failed to setup PVM NVME SQ %d state \n", i);
      return -1;
    }
  }

  // Initialize PVM SQs for processing R2N commands
  // Note: Not incrementing nvme_be_q in for loop as the SSD handle is added to this
  //       by R2N module in datapath.
  // Save the R2N queue number
  uint32_t nvme_be_q = i + NUM_TO_VAL(kPvmNumR2nSQs);
  pvm_r2n_tgt_sq_base = i;
  for (j = 0; j < (int) NUM_TO_VAL(kPvmNumR2nSQs); j++, i++) {
    // Initialize the queue in the DOL enviroment
    if (queue_init(&pvm_sqs[i], NUM_TO_VAL(kPvmNumEntries),
                   NUM_TO_VAL(kDefaultEntrySize)) < 0) {
      printf("Unable to allocate host memory for PVM R2N SQ %d\n", i);
      return -1;
    }
    printf("Initialized PVM R2N SQ %d \n", i);

    // Setup the queue state in Capri:
    if (qstate_if::setup_q_state(pvm_lif, SQ_TYPE, i, (char *) kR2nSqHandler,
                                 kDefaultTotalRings, kDefaultHostRings,
                                 kPvmNumEntries, pvm_sqs[i].mem->pa(),
                                 kDefaultEntrySize, true, pvm_lif, SQ_TYPE,
                                 nvme_be_q, 0, 0, storage_hbm_ssd_bm_addr,
                                 kPvmNumNvmeBeSQs, kDefaultEntrySize, 0) < 0) {
      printf("Failed to setup PVM R2N SQ %d state \n", i);
      return -1;
    }
  }

  // Initialize PVM SQs for processing NVME backend commands
  // Note: Incrementing ssd_q in the for loop as the NVME backend corresponds 1:1
  //       with the SSD
  uint32_t ssd_q = i + NUM_TO_VAL(kPvmNumNvmeBeSQs);
  pvm_nvme_be_sq_base = i;
  for (j = 0; j < (int) NUM_TO_VAL(kPvmNumNvmeBeSQs); j++, i++, ssd_q++) {
    // Initialize the queue in the DOL enviroment
    if (queue_init(&pvm_sqs[i], NUM_TO_VAL(kPvmNumEntries),
                   NUM_TO_VAL(kDefaultEntrySize)) < 0) {
      printf("Unable to allocate host memory for PVM NVME backend SQ %d\n", i);
      return -1;
    }
    printf("Initialized PVM NVME backend SQ %d \n", i);

    // Setup the queue state in Capri:
    if (qstate_if::setup_pri_q_state(pvm_lif, SQ_TYPE, i,
                                     kNvmeBeTotalRings, kDefaultNoHostRings,
                                     kPvmNumEntries, pvm_sqs[i].mem->pa(),
                                     kDefaultEntrySize, true, pvm_lif, SQ_TYPE,
                                     ssd_q, 0, 0, storage_hbm_ssd_bm_addr,
                                     (j == kE2eSsdhandle)) < 0) {
      printf("Failed to setup PVM NVME backend SQ %d state \n", i);
      return -1;
    }
  }

  // Initialize PVM SQs for processing SSD commands
  pvm_ssd_sq_base = i;
  for (j = 0; j < (int) NUM_TO_VAL(kPvmNumSsdSQs); j++, i++) {
    // Physical address to update pindex
    uint64_t pi_pa;

    // For the special E2E SSD, use NvmeSsd class to initialize the queue
    if  (j == kE2eSsdhandle) {
      // Initialize the queue in the DOL enviroment
      if (nvme_e2e_ssd_sq_init(&pvm_sqs[i], NUM_TO_VAL(kPvmNumEntries),
                               NUM_TO_VAL(kDefaultEntrySize)) < 0) {
        printf("Unable to allocate host memory for PVM E2E SSD SQ %d\n", i);
        return -1;
      }
      printf("Initialized PVM SSD E2E SQ %d \n", i);

      // Get the PI's physical address from the SSD emulation layer
      storage_test::SsdWorkingParams params;
      nvme_e2e_ssd->GetWorkingParams(&params);
      pi_pa = params.subq_pi_pa;
      ssd_cndx_addr[j] = params.compq_ci_pa;
      printf("j %d ci_pa %lx cndx_addr %lx pi_pa %lx \n", j, params.compq_ci_pa, ssd_cndx_addr[j], params.subq_pi_pa);

      // Initialize the doorbell of the CQ
      uint64_t db_addr;
      uint64_t db_data;
      uint32_t qid = (int) NUM_TO_VAL(kPvmNumNvmeCQs) + (int) NUM_TO_VAL(kPvmNumR2nCQs) + kE2eSsdhandle;
      queues::get_host_doorbell(pvm_lif, CQ_TYPE, qid, 0, 0, &db_addr, &db_data);
      nvme_e2e_ssd_db_init(db_addr, db_data);
      printf("Initialized backend doorbell for SSD %d \n", i);

    } else {
      // Initialize the queue in the DOL enviroment
      if (queue_init(&pvm_sqs[i], NUM_TO_VAL(kPvmNumEntries),
                     NUM_TO_VAL(kDefaultEntrySize)) < 0) {
        printf("Unable to allocate host memory for PVM SSD SQ %d\n", i);
        return -1;
      }
      printf("Initialized PVM SSD SQ %d \n", i);

      // Store the default value of the PI's physical address
      pi_pa = pndx_data_pa;
      ssd_cndx_addr[j] = cndx_data_pa;
    }

    // Setup the queue state in Capri:
    // 1. no dst queues for these as these are in host
    // 2. no program address for these as these are in host
    // 3. The push address is that of the SSD's SQ's PI
    if (qstate_if::setup_pci_q_state(pvm_lif, SQ_TYPE, i,
                                     kDefaultTotalRings, kDefaultNoHostRings,
                                     kPvmNumEntries, pvm_sqs[i].mem->pa(),
                                     kDefaultEntrySize, pi_pa,
                                     0, 0, 0) < 0) {
      printf("Failed to setup PVM SSD SQ %d state \n", i);
      return -1;
    }
  }

  // Initialize PVM SQs for processing R2N commands from the Sequencer.
  // This is strictly to avoid queue sharing between PVM and P4+ code.
  // Note: This is different from the Sequencer R2N entry handler queue
  // created above.
  pvm_r2n_host_sq_base = i;
  for (j = 0; j < (int) NUM_TO_VAL(kPvmNumR2nSQs); j++, i++) {
    // Initialize the queue in the DOL enviroment
    if (queue_init(&pvm_sqs[i], NUM_TO_VAL(kPvmNumEntries),
                   NUM_TO_VAL(kDefaultEntrySize)) < 0) {
      printf("Unable to allocate host memory for PVM R2N SQ for Seq %d\n", i);
      return -1;
    }
    printf("Initialized PVM R2N SQ %d for Seq \n", i);

    // Setup the queue state in Capri:
    if (qstate_if::setup_q_state(pvm_lif, SQ_TYPE, i, (char *) kR2nSqHandler,
                                 kDefaultTotalRings, kDefaultHostRings,
                                 kPvmNumEntries, pvm_sqs[i].mem->pa(),
                                 kDefaultEntrySize, true, pvm_lif, SQ_TYPE,
                                 nvme_be_q, 0, 0, storage_hbm_ssd_bm_addr,
                                 kPvmNumNvmeBeSQs, kDefaultEntrySize, 0) < 0) {
      printf("Failed to setup PVM R2N SQ %d state for Seq \n", i);
      return -1;
    }
  }

  // Initialize Initiator R2N SQs for processing NVME status
  // Save the R2N queue number
  pvm_r2n_init_sq_base = i;
  for (j = 0; j < (int) NUM_TO_VAL(kPvmNumR2nSQs); j++, i++) {
    // Initialize the queue in the DOL enviroment
    if (queue_init(&pvm_sqs[i], NUM_TO_VAL(kPvmNumEntries),
                   NUM_TO_VAL(kDefaultEntrySize)) < 0) {
      printf("Unable to allocate host memory for Initiator R2N SQ %d\n", i);
      return -1;
    }
    printf("Initialized Initiator R2N SQ %d \n", i);

    // Setup the queue state in Capri:
    if (qstate_if::setup_init_r2n_q_state(pvm_lif, SQ_TYPE, i,
                                          kDefaultTotalRings, kDefaultHostRings,
                                          kPvmNumEntries, pvm_sqs[i].mem->pa(),
                                          kDefaultEntrySize) < 0) {
      printf("Failed to setup Inititator R2N SQ %d state \n", i);
      return -1;
    }
  }

  // Save PVM's last SQ
  pvm_last_sq = i;
  printf("PVM's last saved SQ %lu \n", pvm_last_sq);

  // Initialize PVM CQs for processing commands from NVME VF only
  // Note: i is overall index across PVM CQs, j iterates the loop
  pvm_nvme_cq_base = 0; // first queue
  for (j = 0, i = 0; j < (int) NUM_TO_VAL(kPvmNumNvmeCQs); j++, i++) {
    // Initialize the queue in the DOL enviroment
    if (queue_init(&pvm_cqs[i], NUM_TO_VAL(kPvmNumEntries),
                   NUM_TO_VAL(kDefaultEntrySize)) < 0) {
      printf("Unable to allocate host memory for PVM NVME CQ %d\n", i);
      return -1;
    }
    printf("Initialized PVM NVME CQ %d \n", i);

    // Setup the queue state in Capri:
    // 1. no dst queues for these as these are in host
    // 2. no program address for these as these are in host
    if (qstate_if::setup_q_state(pvm_lif, CQ_TYPE, i, (char *) kPvmCqHandler,
                                 kDefaultTotalRings, kDefaultHostRings,
                                 kPvmNumEntries, pvm_cqs[i].mem->pa(),
                                 kDefaultEntrySize, false, 0, 0,
                                 0, 0, 0, storage_hbm_ssd_bm_addr, 0, 0, 0) < 0) {
      printf("Failed to setup PVM NVME CQ %d state \n", i);
      return -1;
    }
  }

  // Initialize PVM CQs for processing R2N commands
  pvm_r2n_cq_base = i;
  for (j = 0; j < (int) NUM_TO_VAL(kPvmNumR2nCQs); j++, i++) {
    // Initialize the queue in the DOL enviroment
    if (queue_init(&pvm_cqs[i], NUM_TO_VAL(kPvmNumEntries),
                   NUM_TO_VAL(kDefaultEntrySize)) < 0) {
      printf("Unable to allocate host memory for PVM R2N CQ %d\n", i);
      return -1;
    }
    printf("Initialized PVM R2N CQ %d \n", i);

    // Setup the queue state in Capri:
    // 1. no dst queues for these as these are in host
    // 2. no program address for these as these are in host
    // 3. The push address is that of the SSD's SQ's PI
    if (qstate_if::setup_pci_q_state(pvm_lif, CQ_TYPE, i,
                                     kDefaultTotalRings, kDefaultNoHostRings,
                                     kPvmNumEntries, pvm_cqs[i].mem->pa(),
                                     kDefaultEntrySize, pndx_data_pa,
                                     0, 0, 0) < 0) {
      printf("Failed to setup PVM NVME SQ %d state \n", i);
      return -1;
    }
  }

  // Initialize PVM CQs for processing NVME backend commands
  // Note: Incrementing ssd_q in the for loop as the NVME backend corresponds 1:1
  //       with the SSD
  pvm_nvme_be_cq_base = i;
  for (j = 0; j < (int) NUM_TO_VAL(kPvmNumNvmeBeCQs); j++, i++) {
    // For the special E2E SSD, use NvmeSsd class to initialize the queue
    if  (j == kE2eSsdhandle) {
      // Initialize the queue in the DOL enviroment
      if (nvme_e2e_ssd_cq_init(&pvm_cqs[i], NUM_TO_VAL(kPvmNumEntries),
                               NUM_TO_VAL(kNvmeCQEntrySize)) < 0) {
        printf("Unable to allocate host memory for PVM E2E SSD CQ %d\n", i);
        return -1;
      }
      printf("Initialized PVM NVME backend E2E CQ %d \n", i);
    } else {
      // Initialize the queue in the DOL enviroment
      if (queue_init(&pvm_cqs[i], NUM_TO_VAL(kPvmNumEntries),
                     NUM_TO_VAL(kNvmeCQEntrySize)) < 0) {
        printf("Unable to allocate host memory for PVM NVME backend CQ %d\n", i);
        return -1;
      }
      printf("Initialized PVM NVME backend CQ %d \n", i);
    }

    // Setup the queue state in Capri:
    if (qstate_if::setup_q_state(pvm_lif, CQ_TYPE, i, (char *) kNvmeBeCqHandler,
                                 kDefaultTotalRings, kDefaultHostRings,
                                 kPvmNumEntries, pvm_cqs[i].mem->pa(),
                                 kNvmeCQEntrySize, true, pvm_lif, CQ_TYPE,
                                 get_pvm_r2n_cq(0), 0, 0, storage_hbm_ssd_bm_addr, 0, 0,
                                 ssd_cndx_addr[j]) < 0) {
      printf("Failed to setup PVM NVME backend CQ %d state \n", i);
      return -1;
    }
  }

  pvm_last_cq = i;
  printf("PVM's last saved CQ %lu \n", pvm_last_cq);

  return 0;
}

int
seq_queues_setup() {
  int i, j;

  // Initialize storage_seq AdminQ
  if (seq_admin_queue_setup(&admin_qs[0], kSeqAdminNumEntries,
                            kSeqAdminQEntrySize, DP_MEM_TYPE_HOST_MEM) < 0) {
    printf("Failed to setup Seq AdminQ state\n");
    return -1;
  }

  // Initialize PVM SQs for processing Sequencer commands for PDMA
  pvm_seq_pdma_sq_base = 0;
  for (i = 0, j = 0; j < (int) NUM_TO_VAL(SeqNumPdmaSQs); j++, i++) {
    if (storage_tx_queue_setup(&seq_sqs[i], i, (char *) kSeqPdmaSqHandler,
                               kDefaultTotalRings, kDefaultHostRings) < 0) {
      printf("Failed to setup PVM Seq PDMA queue %d \n", i);
      return -1;
    }
    printf("Setup PVM Seq PDMA queue %d \n", i);
  }

  // Initialize PVM SQs for processing Sequencer commands for R2N
  pvm_seq_r2n_sq_base = i;
  for (j = 0; j < (int) NUM_TO_VAL(kSeqNumR2nSQs); j++, i++) {
    if (storage_tx_queue_setup(&seq_sqs[i], i, (char *) kSeqR2nSqHandler,
                               kDefaultTotalRings, kDefaultNoHostRings) < 0) {
      printf("Failed to setup PVM Seq R2n queue %d \n", i);
      return -1;
    }
    printf("Setup PVM Seq R2n queue %d \n", i);
  }

  // Initialize PVM SQs for processing Sequencer commands for XTS
  pvm_seq_xts_sq_base = i;
  for (j = 0; j < (int) NUM_TO_VAL(kSeqNumXtsSQs); j++, i++) {
    // Initialize the queue in the DOL enviroment
    if (seq_queue_setup(&seq_sqs[i], i, STORAGE_SEQ_QGROUP_CRYPTO,
                        (char *)kSeqXtsSqHandler,
                        kDefaultTotalRings, kDefaultHostRings,
                        kSeqNumAccEntries, kDefaultEntrySize,
                        DP_MEM_TYPE_HOST_MEM,
                        QUEUE_BATCH_LIMIT(kXtsDescSize)) < 0) {
      printf("Failed to setup Seq Xts SQ %d state \n", i);
      return -1;
    }

    printf("Setup PVM Seq Xts queue %d \n", i);
  }

  // Initialize PVM SQs for processing Sequencer commands for post XTS
  pvm_seq_xts_status_sq_base = i;
  for (j = 0; j < (int) NUM_TO_VAL(kSeqNumXtsStatusSQs); j++, i++) {
    // Initialize the queue in the DOL enviroment
    if (seq_queue_setup(&seq_sqs[i], i, STORAGE_SEQ_QGROUP_CRYPTO_STATUS,
                        (char *)kSeqXtsStatusDesc0SqHandler,
                        kDefaultTotalRings, kDefaultHostRings,
                        kSeqNumEntries, kSeqXtsStatusSQEntrySize,
                        DP_MEM_TYPE_HOST_MEM, 0,
                        (char *)kSeqXtsStatusDesc1SqHandler) < 0) {
      printf("Failed to setup XTS Status SQ %d state \n", i);
      return -1;
    }

    printf("Setup PVM Seq XTS Status queue %d \n", i);
  }

  // Initialize PVM SQs for processing Sequencer commands for ROCE
  pvm_seq_roce_sq_base = i;
  for (j = 0; j < (int) NUM_TO_VAL(kSeqNumRoceSQs); j++, i++) {
    if (storage_tx_queue_setup(&seq_sqs[i], i, (char *) kSeqR2nSqHandler,
                               kDefaultTotalRings, kDefaultHostRings) < 0) {
      printf("Failed to setup PVM Seq ROCE queue %d \n", i);
      return -1;
    }
    printf("Setup PVM Seq ROCE queue %d \n", i);
  }

  // Initialize PVM SQs for processing Sequencer commands for compression
  pvm_seq_comp_sq_base = i;
  for (j = 0; j < (int) NUM_TO_VAL(kSeqNumCompSQs); j++, i++) {
    // Initialize the queue in the DOL enviroment
    if (seq_queue_setup(&seq_sqs[i], i, STORAGE_SEQ_QGROUP_CPDC,
                        (char *)kSeqCompSqHandler,
                        kDefaultTotalRings, kDefaultHostRings,
                        kSeqNumAccEntries, kDefaultEntrySize,
                        DP_MEM_TYPE_HOST_MEM,
                        QUEUE_BATCH_LIMIT(sizeof(tests::cp_desc_t))) < 0) {
      printf("Failed to setup Seq Comp SQ %d state \n", i);
      return -1;
    }

    printf("Setup PVM Seq Compression queue %d \n", i);
  }

  // Initialize PVM SQs for processing Sequencer commands for post compression
  pvm_seq_comp_status_sq_base = i;
  for (j = 0; j < (int) NUM_TO_VAL(kSeqNumCompStatusSQs); j++, i++) {
    // Initialize the queue in the DOL enviroment
    if (seq_queue_setup(&seq_sqs[i], i, STORAGE_SEQ_QGROUP_CPDC_STATUS,
                        (char *)kSeqCompStatusDesc0SqHandler,
                        kDefaultTotalRings, kDefaultHostRings,
                        kSeqNumEntries, kSeqCompStatusSQEntrySize,
                        DP_MEM_TYPE_HOST_MEM, 0,
                        (char *)kSeqCompStatusDesc1SqHandler) < 0) {
      printf("Failed to setup Comp Status SQ %d state \n", i);
      return -1;
    }

    printf("Setup PVM Seq Compression Status queue %d \n", i);
  }

  if (run_nicmgr_tests) {
    if (nicmgr_if::nicmgr_if_seq_queue_init_complete()) {
      printf("Failed Seq queue init complete\n");
      return -1;
    }
  }

  return 0;
}

int
arm_queue_init(int qid, char *pgm_bin, bool dst_valid, uint16_t dst_lif,
               uint8_t dst_qtype, uint32_t dst_qid) {

  // Validate the qid passed in
  if (qid >= (int) kArmTotalQs) {
    printf("qid passed in %d >= Total Arm qs %d \n", qid, kArmTotalQs);
    return -1;
  }

  // Initialize the queue in the DOL enviroment
  if (queue_init(&arm_qs[qid], NUM_TO_VAL(kArmNumEntries),
                 NUM_TO_VAL(kDefaultEntrySize)) < 0) {
    printf("Unable to allocate memory for ARM Q %d\n", qid);
    return -1;
  }
  printf("Initialized ARM Q %d \n", qid);

  // Setup the queue state in Capri
  if (qstate_if::setup_arm_q_state(arm_lif, SQ_TYPE, qid, pgm_bin,
                                   kDefaultTotalRings, kDefaultHostRings,
                                   kArmNumEntries, arm_qs[qid].mem->pa(),
                                   kDefaultEntrySize, dst_valid, dst_lif,
                                   dst_qtype, dst_qid, 0, 0, 0, 0,
                                   nvme_dp::get_iob_ring_base_addr()) < 0) {
    printf("Failed to setup ARM Q %d state \n", qid);
    return -1;
  }
  return 0;
}

int
arm_queues_setup() {

  arm_q_base = 0;

  arm_sq = 0;
  arm_cq = 1;
  arm_timeout_q = 2;
  uint32_t arm_free_iob_q_base = 3;
  for (int i = 0; i < MAX_ARM_FREE_IOB_QS; i++) {
    arm_free_iob_q[i] = arm_free_iob_q_base + i;
  }

  if (arm_queue_init(arm_sq, NULL, false, 0, 0, 0) < 0) {
    printf("Failed to setup ARM SQ \n");
    return -1;
  }

  // TODO: Add completion queue handler

  if (arm_queue_init(arm_timeout_q, (char *) kArmQTimeoutHandler, false, 0, 0, 0) < 0) {
    printf("Failed to setup ARM timeout Q \n");
    return -1;
  }

  for (int i = 0; i < MAX_ARM_FREE_IOB_QS; i++) {
    if (arm_queue_init(arm_free_iob_q[i], (char *) kArmQFreeHandler, false, 0, 0, 0) < 0) {
      printf("Failed to setup ARM timeout Q \n");
      return -1;
    }
  }
  return 0;
}


int
pvm_roce_sq_init(uint16_t rsq_lif, uint16_t rsq_qtype, uint32_t rsq_qid,
                 uint16_t rrq_lif, uint16_t rrq_qtype, uint32_t rrq_qid,
                 dp_mem_t *mem, uint32_t num_entries, uint32_t entry_size,
                 uint64_t rrq_base_pa, uint8_t post_buf) {

    uint32_t i = pvm_last_sq;
    // Initialize the queue in the DOL enviroment
    if (queue_pre_init(&pvm_sqs[i], mem, NUM_TO_VAL(num_entries),
                       NUM_TO_VAL(entry_size), 0) < 0) {
      printf("Unable to pre init PVM ROCE SQ %d\n", i);
      return -1;
    }
    printf("Initialized PVM ROCE SQ %d \n", i);

    // Setup the queue state in Capri:
    if (qstate_if::setup_roce_sq_state(pvm_lif, SQ_TYPE, i, (char *) kPvmRoceSqHandler,
                                       kDefaultTotalRings, kDefaultHostRings,
                                       num_entries, pvm_sqs[i].mem->pa(),
                                       entry_size, true, rrq_lif, rrq_qtype, rrq_qid,
                                       rsq_lif, rsq_qtype, rsq_qid, rrq_base_pa,
                                       post_buf) < 0) {
      printf("Failed to setup PVM ROCE SQ %d state \n", i);
      return -1;
    }
    pvm_last_sq++;
    return i;
}

int
pvm_roce_cq_init(uint16_t rcq_lif, uint16_t rcq_qtype, uint32_t rcq_qid,
                 dp_mem_t *mem, uint32_t num_entries, uint32_t entry_size,
                 uint64_t xlate_addr) {

    uint32_t i = pvm_last_cq;
    // Initialize the queue in the DOL enviroment
    if (queue_pre_init(&pvm_cqs[i], mem, NUM_TO_VAL(num_entries),
                       NUM_TO_VAL(entry_size), 0) < 0) {
      printf("Unable to pre init PVM ROCE CQ %d\n", i);
      return -1;
    }
    printf("Initialized PVM ROCE CQ %d \n", i);

    // Setup the queue state in Capri:
    if (qstate_if::setup_roce_cq_state(pvm_lif, CQ_TYPE, i, (char *) kPvmRoceCqHandler,
                                       kDefaultTotalRings, kDefaultHostRings,
                                       num_entries, pvm_cqs[i].mem->pa(), entry_size,
                                       xlate_addr, rcq_lif, rcq_qtype, rcq_qid) < 0) {
      printf("Failed to setup PVM ROCE CQ %d state \n", i);
      return -1;
    }
    pvm_last_cq++;
    return i;
}

dp_mem_t *nvme_sq_consume_entry(uint16_t qid, uint16_t *index) {
  if (qid >= NUM_TO_VAL(NvmeNumSQs)) return nullptr;
  return queue_consume_entry(&nvme_sqs[qid], index);
}

dp_mem_t *pvm_sq_consume_entry(uint16_t qid, uint16_t *index) {
  if (qid >= NUM_TO_VAL(PvmNumSQs)) return nullptr;
  return queue_consume_entry(&pvm_sqs[qid], index);
}

dp_mem_t *seq_sq_consume_entry(uint16_t qid, uint16_t *index) {
  if (qid >= NUM_TO_VAL(SeqNumSQs)) return nullptr;
  seq_sq_batch_consume_end(qid);
  return queue_consume_entry(&seq_sqs[qid], index);
}

dp_mem_t * seq_sq_consumed_entry_get(uint16_t qid, uint16_t *index) {
  if (qid >= NUM_TO_VAL(SeqNumSQs)) return nullptr;
  return queue_consumed_entry_get(&seq_sqs[qid], index);
}

/*
 * Batching support consumes a single sequencer queue descriptor for multiple
 * requests where, presumably, the user would store in that descriptor
 * the start address of a vector of subordinate descriptors (e.g. Barco
 * descriptors) and a count.
 *
 * With just a single sequencer descriptor and a single doorbell ring,
 * sequencer P4+ code would transfer the multiple subordinate descriptors,
 * e.g., to Barco, and wake up Barco in one shot.
 */
dp_mem_t *
seq_sq_batch_consume_entry(uint16_t qid,
                           uint64_t batch_id,
                           uint16_t *index,
                           bool *new_batch,
                           seq_sq_batch_end_notify_t end_notify_fn,
                           void *user_ctx)
{
    queues_t        *queue;
    queue_batch_t   *batch;
    dp_mem_t        *queue_mem;

    assert(batch_id && (qid < NUM_TO_VAL(SeqNumSQs)));
    *new_batch = false;

    /*
     * See if starting a new batch
     */
    queue = &seq_sqs[qid];
    queue_mem = queue->mem;
    batch = queue->batch;
    if (!batch) {
        batch = queue->batch = new queue_batch_t();
    }
    if (batch->batch_id == 0) {
        batch->batch_desc = queue_consume_entry(queue, &batch->batch_pd_idx)->
                                                fragment_find(0, queue->entry_size);
        batch->batch_id = batch_id;
        batch->end_notify_fn = end_notify_fn;
        batch->user_ctx = user_ctx;
        batch->batch_size = 0;
        *new_batch = true;
    }

    /*
     * Continue adding to existing batch
     */
    if (batch_id == batch->batch_id) {
        *index = batch->batch_pd_idx;

        /*
         * Enforce batch limit if any
         */
        batch->batch_size++;
        if (queue->batch_limit && (batch->batch_size >= queue->batch_limit)) {
            seq_sq_batch_consume_end(qid);
        }

    } else {

        /*
         * End current batch and switch to new one
         */
        seq_sq_batch_consume_end(qid);

        /*
         * Caution: recursion!!!
         */
        seq_sq_batch_consume_entry(qid, batch_id, index,
                                   new_batch, end_notify_fn, user_ctx);
    }

    return queue_mem;
}

/*
 * Complete the current batch, if any, and invoke the user callback.
 */
void
seq_sq_batch_consume_end(uint16_t qid)
{
    queues_t        *queue;
    queue_batch_t   *batch;

    assert(qid < NUM_TO_VAL(SeqNumSQs));
    queue = &seq_sqs[qid];
    batch = queue->batch;
    if (batch && batch->batch_id) {
        assert(batch->batch_size);
        (*batch->end_notify_fn)(batch->user_ctx, batch->batch_desc, batch->batch_size);
        batch->batch_id = 0;
    }
}

dp_mem_t *nvme_cq_consume_entry(uint16_t qid, uint16_t *index) {
  if (qid >= NUM_TO_VAL(NvmeNumCQs)) return nullptr;
  return queue_consume_entry(&nvme_cqs[qid], index);
}

dp_mem_t *pvm_cq_consume_entry(uint16_t qid, uint16_t *index) {
  if (qid >= NUM_TO_VAL(PvmNumCQs)) return nullptr;
  return queue_consume_entry(&pvm_cqs[qid], index);
}

uint16_t get_nvme_lif() {
  return (uint16_t) nvme_lif;
}

uint16_t get_pvm_lif() {
  return (uint16_t) pvm_lif;
}

uint16_t get_seq_lif() {
  return (uint16_t) seq_lif;
}

uint16_t get_arm_lif() {
  return (uint16_t) arm_lif;
}

uint32_t get_nvme_bdf() {
  return kNvmeLifBdf;
}

uint32_t get_host_nvme_sq(uint32_t offset) {
  assert((int) offset < NUM_TO_VAL(NvmeNumSQs));
  return (host_nvme_sq_base + offset);
}

uint32_t get_pvm_nvme_sq(uint32_t offset) {
  assert((int) offset < NUM_TO_VAL(kPvmNumNvmeSQs));
  return (pvm_nvme_sq_base + offset);
}

uint32_t get_pvm_r2n_tgt_sq(uint32_t offset) {
  assert((int) offset < NUM_TO_VAL(kPvmNumR2nSQs));
  return (pvm_r2n_tgt_sq_base + offset);
}

uint32_t get_pvm_r2n_init_sq(uint32_t offset) {
  assert((int) offset < NUM_TO_VAL(kPvmNumR2nSQs));
  return (pvm_r2n_init_sq_base + offset);
}

uint32_t get_pvm_r2n_host_sq(uint32_t offset) {
  assert(offset < NUM_TO_VAL(kPvmNumR2nSQs));
  return (pvm_r2n_host_sq_base + offset);
}

uint32_t get_pvm_nvme_be_sq(uint32_t offset) {
  assert((int) offset < NUM_TO_VAL(kPvmNumNvmeBeSQs));
  return (pvm_nvme_be_sq_base + offset);
}

uint32_t get_pvm_ssd_sq(uint32_t offset) {
  assert((int) offset < NUM_TO_VAL(kPvmNumSsdSQs));
  return (pvm_ssd_sq_base + offset);
}

uint32_t get_seq_pdma_sq(uint32_t offset) {
  assert(offset < (uint32_t)NUM_TO_VAL(SeqNumPdmaSQs));
  return (pvm_seq_pdma_sq_base + offset);
}

uint32_t get_seq_r2n_sq(uint32_t offset) {
  assert(offset < NUM_TO_VAL(kSeqNumR2nSQs));
  return (pvm_seq_r2n_sq_base + offset);
}

uint32_t get_seq_xts_sq(uint32_t offset) {
  assert((int)offset < NUM_TO_VAL(kSeqNumXtsSQs));
  return (pvm_seq_xts_sq_base + offset);
}

uint32_t get_seq_xts_status_sq(uint32_t offset) {
  assert((int)offset < NUM_TO_VAL(kSeqNumXtsStatusSQs));
  return (pvm_seq_xts_status_sq_base + offset);
}

uint32_t get_seq_roce_sq(uint32_t offset) {
  assert(offset < NUM_TO_VAL(kSeqNumRoceSQs));
  return (pvm_seq_roce_sq_base + offset);
}

uint32_t get_seq_comp_sq(uint32_t offset) {
  assert((int)offset < NUM_TO_VAL(kSeqNumCompSQs));
  return (pvm_seq_comp_sq_base + offset);
}

uint32_t get_seq_comp_status_sq(uint32_t offset) {
  assert((int)offset < NUM_TO_VAL(kSeqNumCompStatusSQs));
  return (pvm_seq_comp_status_sq_base + offset);
}

uint32_t get_host_nvme_cq(uint32_t offset) {
  assert((int) offset < NUM_TO_VAL(NvmeNumCQs));
  return (host_nvme_cq_base + offset);
}

uint32_t get_pvm_nvme_cq(uint32_t offset) {
  assert(offset < NUM_TO_VAL(kPvmNumNvmeCQs));
  return (pvm_nvme_cq_base + offset);
}

uint32_t get_pvm_r2n_cq(uint32_t offset) {
  assert(offset < NUM_TO_VAL(kPvmNumR2nCQs));
  return (pvm_r2n_cq_base + offset);
}

uint32_t get_pvm_nvme_be_cq(uint32_t offset) {
  assert(offset < NUM_TO_VAL(kPvmNumNvmeBeCQs));
  return (pvm_nvme_be_cq_base + offset);
}

void ring_nvme_e2e_ssd() {
  printf("*********** RINGING SSD DOORBELL for testing *********** \n");
  storage_test::SsdWorkingParams params;
  nvme_e2e_ssd->GetWorkingParams(&params);
  *((uint32_t *) params.subq_pi_va) =   (*((uint32_t *) params.subq_pi_va)) + 1;
}

void get_nvme_doorbell(uint16_t lif, uint8_t qtype, uint32_t qid,
                       uint8_t ring, uint16_t index,
                       uint64_t *db_addr, uint64_t *db_data) {

  if (!db_addr || !db_data) return;

  *db_data = ((uint64_t)qid << kNvmeDbQidShift) | index;
  *db_addr = kDbAddrNvme |  ((uint64_t)kDbAddrUpdate << kNvmeDbUpdateShift) |
             ((uint64_t)lif << kNvmeDbLifShift) | ((uint64_t)qtype << kNvmeDbTypeShift);
}

void get_host_doorbell(uint16_t lif, uint8_t qtype, uint32_t qid,
                       uint8_t ring, uint16_t index,
                       uint64_t *db_addr, uint64_t *db_data) {

  if (!db_addr || !db_data) return;

  *db_data = ((uint64_t)qid << kDbQidShift) | ((uint64_t)ring << kDbRingShift) | index;
  *db_addr = kDbAddrHost |  ((uint64_t)kDbAddrUpdate << kDbUpdateShift) |
             ((uint64_t)lif << kDbLifShift) | ((uint64_t)qtype << kDbTypeShift);
}

void get_capri_doorbell(uint16_t lif, uint8_t qtype, uint32_t qid,
                        uint8_t ring, uint16_t index,
                        uint64_t *db_addr, uint64_t *db_data) {

  if (!db_addr || !db_data) return;

  *db_data = ((uint64_t)qid << kDbQidShift) | ((uint64_t)ring << kDbRingShift) | index;
  *db_addr = kDbAddrLocal |  ((uint64_t)kDbAddrUpdate << kDbUpdateShift) |
             ((uint64_t)lif << kDbLifShift) | ((uint64_t)qtype << kDbTypeShift);
}

void get_capri_doorbell_with_pndx_inc(uint16_t lif, uint8_t qtype, uint32_t qid,
                                      uint8_t ring, uint64_t *db_addr, uint64_t *db_data) {

  if (!db_addr || !db_data) return;

  *db_data = ((uint64_t)qid << kDbQidShift) | ((uint64_t)ring << kDbRingShift);
  *db_addr = kDbAddrLocal |  ((uint64_t)kDbAddrInc << kDbUpdateShift) |
             ((uint64_t)lif << kDbLifShift) | ((uint64_t)qtype << kDbTypeShift);
}

void queues_shutdown() {
  storage_test::NvmeSsd *ssd_ptr = nvme_e2e_ssd.release();
  delete ssd_ptr;
  nicmgr_if::nicmgr_if_fini();
  EXIT_SIMULATION();
}

}  // namespace queues
