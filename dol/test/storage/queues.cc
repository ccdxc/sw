#include <stdio.h>
#include <strings.h>

#include "dol/test/storage/hal_if.hpp"
#include "dol/test/storage/qstate_if.hpp"
#include "dol/test/storage/nvme.hpp"
#include "dol/test/storage/queues.hpp"
#include "dol/test/storage/host_mem/c_if.h"
#include "dol/test/storage/model_client/lib_model_client.h"

#define	NUM_TO_VAL(num)		(1 << (num))

const static uint32_t	kNvmeNumSQs		 = 3;
const static uint32_t	kNvmeNumCQs		 = 3;
const static uint32_t	kPvmNumSQs		 = 7;
const static uint32_t	kPvmNumCQs		 = 5;
const static uint32_t	kPvmNumHQs		 = 0; // 2^0 => 1 queue
const static uint32_t	kPvmNumEQs		 = 1;
const static uint32_t	kPvmNumNvmeSQs		 = 1;
const static uint32_t	kPvmNumNvmeCQs		 = 1;
const static uint32_t	kDefaultEntrySize	 = 6; // Default is 64 bytes
const static uint32_t	kPvmNvmeSQEntrySize	 = 7; // PVM SQ is 128 bytes (NVME command + PVM header)
const static uint32_t	kNvmeCQEntrySize	 = 4; // NVME CQ is 16 bytes
const static uint32_t	kNvmeNumEntries		 = 6;
const static uint32_t	kPvmNumEntries		 = 6;
const static char	*kNvmeSqHandler		 = "storage_tx_nvme_sq_handler.bin";
const static char	*kPvmCqHandler		 = "storage_tx_pvm_cq_handler.bin";
const static uint32_t	kDefaultTotalRings	 = 1;
const static uint32_t	kDefaultHostRings	 = 1;
const static uint32_t	kDefaultNoHostRings	 = 0;
const static uint32_t	kNvmeBeTotalRings	 = 3;
const static uint32_t	kNvmeBeHostRings	 = 0;

namespace queues {

typedef struct queues_ {
  void *vaddr;
  uint64_t paddr;
  uint16_t index; // p_ndx for tx queue, c_ndx for rx queue
  uint16_t entry_size;
  uint16_t num_entries;
} queues_t;

// NVME Submission, Completion queues
queues_t nvme_sqs[NUM_TO_VAL(kNvmeNumSQs)];
queues_t nvme_cqs[NUM_TO_VAL(kNvmeNumCQs)];

// PVM Submission, Completion, Host(for R2N), Error queues
queues_t pvm_sqs[NUM_TO_VAL(kPvmNumSQs)];
queues_t pvm_cqs[NUM_TO_VAL(kPvmNumCQs)];
queues_t pvm_rqs[NUM_TO_VAL(kPvmNumHQs)];
queues_t pvm_eqs[NUM_TO_VAL(kPvmNumEQs)];

uint64_t nvme_lif, pvm_lif;

void lif_params_init(hal_if::lif_params_t *params, uint16_t type,
                     uint16_t num_entries, uint16_t num_queues) {
  params->type[type].valid = true; 
  params->type[type].queue_size = num_entries; // you get 2^num_entries
  params->type[type].num_queues = num_queues;  // you get 2^num_queues 
}

int queue_init(queues_t *queue, uint16_t num_entries, uint16_t entry_size) {
  queue->vaddr = (uint8_t *)alloc_host_mem(num_entries * entry_size);
  if (queue->vaddr == nullptr) {
    printf("Unable to allocate host memory for queue\n");
    return -1;
  }
  queue->paddr = host_mem_v2p(queue->vaddr);
  queue->index = 0;
  queue->entry_size = entry_size;
  queue->num_entries = num_entries;
  return 0;
}

void *queue_consume_entry(queues_t *queue, uint16_t *index) {
  if (!queue->vaddr || !index) return nullptr;
  uint16_t curr_index = queue->index;
  queue->index = ((queue->index + 1)  % queue->num_entries);
  *index = queue->index;
  return (void *) ((uint8_t *) queue->vaddr + (curr_index * queue->entry_size));
}

int queues_setup() {
  // Create NVME and PVM LIFs
  hal_if::lif_params_t nvme_lif_params, pvm_lif_params;

  bzero(&nvme_lif_params, sizeof(nvme_lif_params));
  lif_params_init(&nvme_lif_params, SQ_TYPE, kNvmeNumEntries, kNvmeNumSQs);
  lif_params_init(&nvme_lif_params, CQ_TYPE, kNvmeNumEntries, kNvmeNumCQs);

  if (hal_if::create_lif(&nvme_lif_params, &nvme_lif) < 0) {
    printf("can't create nvme lif \n");
    return -1;
  }
  printf("NVME LIF created\n");

  bzero(&pvm_lif_params, sizeof(pvm_lif_params));
  lif_params_init(&pvm_lif_params, SQ_TYPE, kPvmNumEntries, kPvmNumSQs);
  lif_params_init(&pvm_lif_params, CQ_TYPE, kPvmNumEntries, kPvmNumCQs);
  lif_params_init(&pvm_lif_params, HQ_TYPE, kPvmNumEntries, kPvmNumHQs);
  lif_params_init(&pvm_lif_params, EQ_TYPE, kPvmNumEntries, kPvmNumEQs);

  if (hal_if::create_lif(&pvm_lif_params, &pvm_lif) < 0) {
    printf("can't create pvm lif \n");
    return -1;
  }
  printf("PVM LIF created\n");

  // Initialize NVME SQs
  for (int i = 0; i < (int) NUM_TO_VAL(kNvmeNumSQs); i++) {
    // Initialize the queue in the DOL enviroment
    if (queue_init(&nvme_sqs[i], NUM_TO_VAL(kNvmeNumEntries),
                   NUM_TO_VAL(kDefaultEntrySize)) < 0) {
      printf("Unable to allocate host memory for NVME SQ %d\n", i);
      return -1;
    }
    printf("Initialized NVME SQ %d \n", i);

    // Setup the queue state in Capri
    if (qstate_if::setup_q_state(nvme_lif, SQ_TYPE, i, (char *) kNvmeSqHandler, 
                                 kDefaultTotalRings, kDefaultHostRings, 
                                 kNvmeNumEntries, nvme_sqs[i].paddr,
                                 kDefaultEntrySize, true, pvm_lif, SQ_TYPE, 
                                 (i%2), 0, 0, 0, 0, 0) < 0) {
      printf("Failed to setup NVME SQ %d state \n", i);
      return -1;
    }
    printf("Setup NVME SQ %d state paddr %lx \n", i, nvme_sqs[i].paddr);
  }

  // Initialize NVME CQs
  for (int i = 0; i < (int) NUM_TO_VAL(kNvmeNumCQs); i++) {
    // Initialize the queue in the DOL enviroment
    if (queue_init(&nvme_cqs[i], NUM_TO_VAL(kNvmeNumEntries),
                   NUM_TO_VAL(kDefaultEntrySize)) < 0) {
      printf("Unable to allocate host memory for NVME CQ %d\n", i);
      return -1;
    }
    printf("Initialized NVME CQ %d \n", i);

    // Setup the queue state in Capri
    if (qstate_if::setup_q_state(nvme_lif, CQ_TYPE, i, NULL, 
                                 kDefaultTotalRings, kDefaultHostRings, 
                                 kNvmeNumEntries, nvme_cqs[i].paddr,
                                 kDefaultEntrySize, false, 0, 0, 
                                 0, 0, 0, 0, 0, 0) < 0) {
      printf("Failed to setup NVME CQ %d state \n", i);
      return -1;
    }
    printf("Setup NVME CQ %d state paddr %lx \n", i, nvme_cqs[i].paddr);
  }

  // Initialize PVM SQs for processing commands from NVME VF only
  for (int i = 0; i < (int) NUM_TO_VAL(kPvmNumNvmeSQs); i++) {
    // Initialize the queue in the DOL enviroment
    if (queue_init(&pvm_sqs[i], NUM_TO_VAL(kPvmNumEntries),
                   NUM_TO_VAL(kPvmNvmeSQEntrySize)) < 0) {
      printf("Unable to allocate host memory for PVM SQ %d\n", i);
      return -1;
    }
    printf("Initialized PVM SQ %d \n", i);

    // Setup the queue state in Capri:
    // 1. no dst queues for these as these are in host
    // 2. no program address for these as these are in host
    if (qstate_if::setup_q_state(pvm_lif, SQ_TYPE, i, NULL, 
                                 kDefaultTotalRings, kDefaultHostRings, 
                                 kPvmNumEntries, pvm_sqs[i].paddr,
                                 kPvmNvmeSQEntrySize, false, 0, 0,
                                 0, 0, 0, 0, 0, 0) < 0) {
      printf("Failed to setup PVM SQ %d state \n", i);
      return -1;
    }
    printf("Setup PVM SQ %d state paddr %lx \n", i, pvm_sqs[i].paddr);
  }

  // Initialize PVM CQs for processing commands from NVME VF only
  for (int i = 0; i < (int) NUM_TO_VAL(kPvmNumNvmeCQs); i++) {
    // Initialize the queue in the DOL enviroment
    if (queue_init(&pvm_cqs[i], NUM_TO_VAL(kPvmNumEntries),
                   NUM_TO_VAL(kDefaultEntrySize)) < 0) {
      printf("Unable to allocate host memory for PVM CQ %d\n", i);
      return -1;
    }
    printf("Initialized PVM CQ %d \n", i);

    // Setup the queue state in Capri:
    // 1. no dst queues for these as these are supplied in PVM status
    if (qstate_if::setup_q_state(pvm_lif, CQ_TYPE, i, (char *) kPvmCqHandler, 
                                 kDefaultTotalRings, kDefaultHostRings, 
                                 kPvmNumEntries, pvm_cqs[i].paddr,
                                 kDefaultEntrySize, false, 0, 0,
                                 0, 0, 0, 0, 0, 0) < 0) {
      printf("Failed to setup PVM CQ %d state \n", i);
      return -1;
    }
    printf("Setup PVM CQ %d state paddr %lx \n", i, pvm_cqs[i].paddr);
  }

  return 0;
}

void *nvme_sq_consume_entry(uint16_t qid, uint16_t *index) {
  if (qid >= NUM_TO_VAL(kNvmeNumSQs)) return nullptr;
  return queue_consume_entry(&nvme_sqs[qid], index);
}

void *pvm_sq_consume_entry(uint16_t qid, uint16_t *index) {
  if (qid >= NUM_TO_VAL(kPvmNumSQs)) return nullptr;
  return queue_consume_entry(&pvm_sqs[qid], index);
}

void *nvme_cq_consume_entry(uint16_t qid, uint16_t *index) {
  if (qid >= NUM_TO_VAL(kNvmeNumCQs)) return nullptr;
  return queue_consume_entry(&nvme_cqs[qid], index);
}

void *pvm_cq_consume_entry(uint16_t qid, uint16_t *index) {
  if (qid >= NUM_TO_VAL(kPvmNumCQs)) return nullptr;
  return queue_consume_entry(&pvm_cqs[qid], index);
}

uint16_t get_nvme_lif() {
  return (uint16_t) nvme_lif;
}

uint16_t get_pvm_lif() {
  return (uint16_t) pvm_lif;
}

}  // namespace queues
