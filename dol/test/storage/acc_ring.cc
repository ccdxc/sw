// Accelerator ring
#include <math.h>

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include "gflags/gflags.h"

#include "acc_ring.hpp"
#include "queues.hpp"
#include "tests.hpp"
#include "storage_seq_p4pd.hpp"
#include "nic/utils/host_mem/c_if.h"
#include "nic/model_sim/include/lib_model_client.h"

namespace tests {

/*
 * Constructor
 */
acc_ring_t::acc_ring_t(const char *ring_name,
                       uint64_t cfg_ring_pd_idx,
                       uint32_t ring_size,
                       uint32_t desc_size,
                       uint64_t ring_base_mem_pa,
                       uint32_t pi_size,
                       uint64_t ring_opaque_tag_pa,
                       uint32_t opaque_data_size) :
    ring_name(ring_name),
    cfg_ring_pd_idx(cfg_ring_pd_idx),
    ring_base_mem_pa(ring_base_mem_pa),
    ring_size(ring_size),
    desc_size(desc_size),
    pi_size(pi_size),
    curr_pd_idx(0),
    prev_pd_idx(0),
    curr_seq_qid(0),
    curr_seq_pd_idx(0),
    curr_push_type(ACC_RING_PUSH_INVALID)
{
    assert(ring_size && desc_size && pi_size);
    shadow_pd_idx_mem = new dp_mem_t(1, pi_size);

    ring_base_mem = (uint8_t *)alloc_page_aligned_host_mem(desc_size * ring_size);
    assert(ring_base_mem != nullptr);

    /*
     * If caller passed in a valid ring_base_mem_pa, ring_base_mem above
     * would be used as intermediate descriptor cache for sequencer submission.
     * This case applies when the HW ring was already initialized by HAL.
     *
     * Otherwise, caller would retrieve ring_base_mem_pa from us thru the
     * method ring_base_mem_pa_get() method and use that to configure
     * the HW ring. This case applies when the HW ring is being configured
     * by the DOL caller itself.
     */
    if (!ring_base_mem_pa) {
        ring_base_mem_pa = host_mem_v2p((void *)ring_base_mem);
    }

    /*
     * Ring memory is concurrently updated by multiple P4+ instances in RTL
     * which will be different from how standalone model would update it.
     * Hence, so turn off EOS comparison on the ring to prevent false alarms.
     */
    eos_ignore_addr(ring_base_mem_pa, desc_size * ring_size);
    if (ring_opaque_tag_pa) {
        eos_ignore_addr(ring_opaque_tag_pa, opaque_data_size);
    }

    printf("%s ring_base_mem_pa 0x%lx ring_opaque_tag_pa 0x%lx\n",
           ring_name, ring_base_mem_pa, ring_opaque_tag_pa);
}

/*
 * Format a sequencer descriptor
 */
void
acc_ring_t::seq_desc_pack(dp_mem_t *seq_desc,
                          uint64_t dst_desc_pa,
                          uint16_t batch_size)
{
    STORAGE_SEQ_DESC_DEFINE(desc_action) = {0};
    uint64_t    barco_pndx_shadow_pa = shadow_pd_idx_mem->pa();

    seq_desc->clear();
    STORAGE_SEQ_DESC_ARRAY_SET(desc_action, barco_desc_addr, 
                               dst_desc_pa);
    STORAGE_SEQ_DESC_ARRAY_SET(desc_action, barco_pndx_addr,
                               cfg_ring_pd_idx);
    STORAGE_SEQ_DESC_ARRAY_SET(desc_action, barco_pndx_shadow_addr, 
                               barco_pndx_shadow_pa);
    STORAGE_SEQ_DESC_ARRAY_SET(desc_action, barco_ring_addr, 
                               ring_base_mem_pa);
    STORAGE_SEQ_DESC_SCALAR_SET(desc_action, barco_desc_size,
                                (uint8_t)log2(desc_size));
    STORAGE_SEQ_DESC_SCALAR_SET(desc_action, barco_pndx_size,
                                (uint8_t)log2(pi_size));
    STORAGE_SEQ_DESC_SCALAR_SET(desc_action, barco_ring_size,
                                (uint8_t)log2(ring_size));
    if (batch_size > 1) {
        STORAGE_SEQ_DESC_SCALAR_SET(desc_action, barco_batch_size,
                                    batch_size);
        STORAGE_SEQ_DESC_SCALAR_SET(desc_action, barco_batch_mode, 1);
    }
    STORAGE_SEQ_DESC_PACK(seq_desc->read(), desc_action);
    seq_desc->write_thru();
}

/*
 * Function to handle notification from queues.cc at completion of
 * a batch construction for a given sequencer queue.
 */
void
acc_ring_batch_end_notify(void *user_ctx,
                          dp_mem_t *seq_desc,
                          uint16_t batch_size)
{
    acc_ring_t  *acc_ring = (acc_ring_t *)user_ctx;
    uint64_t    dst_desc_pa;

    /*
     * Indicate batch mode in the given descriptor and the
     * accumulated batch count.
     */
    assert(batch_size);
    dst_desc_pa = *((uint64_t *)seq_desc->read());
    acc_ring->seq_desc_pack(seq_desc, dst_desc_pa, batch_size);
}

/*
 * Push a descriptor into the given queue. Several push types are
 * supported, including batching.
 */
void
acc_ring_t::push(const void *src_desc,
                 acc_ring_push_t push_type,
                 uint32_t seq_qid)
{
    uint8_t     *dst_desc;
    dp_mem_t    *seq_desc;
    bool        new_batch;
    uint32_t    pd_idx;

    switch (push_type) {

    case ACC_RING_PUSH_SEQUENCER:
    case ACC_RING_PUSH_SEQUENCER_BATCH:
        pd_idx = curr_pd_idx;
        curr_pd_idx = (curr_pd_idx + 1) % ring_size;

        dst_desc = &ring_base_mem[pd_idx * desc_size];
        memcpy(dst_desc, src_desc, desc_size);
        curr_seq_qid = seq_qid;

        if (push_type == ACC_RING_PUSH_SEQUENCER_BATCH) {
            seq_desc = queues::seq_sq_batch_consume_entry(curr_seq_qid,
                                   ring_base_mem_pa, &curr_seq_pd_idx,
                                   &new_batch, acc_ring_batch_end_notify,
                                   (void *)this);
            if (new_batch) {

                /*
                 * Hitch a ride on seq_desc to save our dst_desc info
                 * until acc_ring_batch_end_notify() is called.
                 */
                assert(desc_size >= sizeof(uint64_t));
                *((uint64_t *)seq_desc->read()) = host_mem_v2p(dst_desc);
            }

            /*
             * Defer until caller calls post_push()
             */
            curr_push_type = push_type;
            break;
        }

        queues::seq_sq_batch_consume_end(curr_seq_qid);
        seq_desc = queues::seq_sq_consume_entry(curr_seq_qid, &curr_seq_pd_idx);
        seq_desc_pack(seq_desc, host_mem_v2p(dst_desc));
        test_ring_doorbell(queues::get_seq_lif(), SQ_TYPE,
                           curr_seq_qid, 0, curr_seq_pd_idx);
        curr_push_type = ACC_RING_PUSH_INVALID;
        break;

    case ACC_RING_PUSH_HW_DIRECT:
    case ACC_RING_PUSH_HW_DIRECT_BATCH:
    case ACC_RING_PUSH_HW_INDIRECT_BATCH:
        write_mem(ring_base_mem_pa + (curr_pd_idx * desc_size),
                  (uint8_t *)src_desc, desc_size);
        curr_pd_idx = (curr_pd_idx + 1) % ring_size;

        /*
         * Since we didn't go thru sequencer here, ensure the shadow pindex
         * maintains up-to-date value
         */
        *((uint32_t *)shadow_pd_idx_mem->read()) = curr_pd_idx;
        shadow_pd_idx_mem->write_thru();
        if (push_type == ACC_RING_PUSH_HW_DIRECT) {
            write_reg(cfg_ring_pd_idx, curr_pd_idx);
        }

        if ((push_type == ACC_RING_PUSH_HW_DIRECT) ||
            (push_type == ACC_RING_PUSH_HW_INDIRECT_BATCH)) {

            /*
             * ACC_RING_PUSH_HW_INDIRECT_BATCH is a special mode and is used
             * only by xts.cc for hw_daisy_chain where one xts context chains
             * its completion to another, i.e., by having HW write the latter's
             * PI and post_push() is never directly called). Here, we move
             * prev_pd_idx presuming that process will eventually work itself
             * out correctly.
             */
            prev_pd_idx = curr_pd_idx;
            curr_push_type = ACC_RING_PUSH_INVALID;
            break;
        }

        /*
         * Defer until caller calls post_push()
         */
        curr_push_type = push_type;
        break;

    default:
        printf("%s unsupported push_type %d\n", ring_name, push_type);
        assert(0);
        break;
    }
}

/*
 * acc_ring_t is non-reentrant in that the tuple {curr_push_type,
 * curr_seq_qid} serves a single user at a time. When there are multiple
 * users with deferred operations pending, (e.g., as in the case of
 * accelerator scaled tests), each user must re-establish its tuple before
 * calling post_push().
 */
void
acc_ring_t::reentrant_tuple_set(acc_ring_push_t push_type,
                                uint32_t seq_qid)
{
    curr_push_type = push_type;
    curr_seq_qid = seq_qid;
    queues::seq_sq_consumed_entry_get(curr_seq_qid,
                                      &curr_seq_pd_idx);
}

/*
 * DOL tests use a mixture of HW_DIRECT and sequencer push so a resync of
 * shadow to real PI may be called for. Real applications (such as storage
 * driver) always use the sequencer approach and resync would not be
 * needed in that case.
 */
void
acc_ring_t::resync(void)
{
    uint32_t    curr_shadow;

    curr_shadow = *((uint32_t *)shadow_pd_idx_mem->read_thru()) % ring_size;
    printf("%s resync shadow %u to PI %u\n", ring_name,
           curr_shadow, curr_pd_idx);
    curr_pd_idx = curr_shadow;
    prev_pd_idx = curr_shadow;
}

/*
 * Execute any deferred push() on the given acc_ring. The argument
 * 'push_amount', applicable only to ACC_RING_PUSH_HW_DIRECT_BATCH, controls
 * how many entries to push.
 */
void
acc_ring_t::post_push(uint32_t push_amount)
{
    uint32_t    push_pd_idx;
    uint32_t    curr_amount;

    switch (curr_push_type) {

    case ACC_RING_PUSH_SEQUENCER_BATCH:
        queues::seq_sq_batch_consume_end(curr_seq_qid);
        test_ring_doorbell(queues::get_seq_lif(), SQ_TYPE, 
                           curr_seq_qid, 0, curr_seq_pd_idx);
        curr_push_type = ACC_RING_PUSH_INVALID;
        break;

    case ACC_RING_PUSH_HW_DIRECT_BATCH:

        /*
         * If push_amount is zero, push whatever remains in the batch.
         * Otherwise, constraint the push to the indicated amount.
         */
        curr_amount = (curr_pd_idx < prev_pd_idx ? 
                       curr_pd_idx + ring_size :
                       curr_pd_idx) - prev_pd_idx;
        if (curr_amount) {
            push_pd_idx = curr_pd_idx;
            if (push_amount) {
                push_amount = std::min(curr_amount, push_amount);
                push_pd_idx = (prev_pd_idx + push_amount) % ring_size;
            }
            write_reg(cfg_ring_pd_idx, push_pd_idx);
            prev_pd_idx = push_pd_idx;
            if (push_pd_idx == curr_pd_idx) {
                curr_push_type = ACC_RING_PUSH_INVALID;
            }
            break;
        }

        /*
         * Fall through!!!
         */

    default:
        printf("%s nothing to do for curr_push_type %d\n",
               ring_name, curr_push_type);
        curr_push_type = ACC_RING_PUSH_INVALID;
        break;
    }
}

}  // namespace tests
