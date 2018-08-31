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
                       uint64_t shadow_pd_idx_pa,
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
    shadow_pd_idx_mem = shadow_pd_idx_pa ?
                         new dp_mem_t((uint8_t *)shadow_pd_idx_pa, 1, pi_size) :
                         new dp_mem_t(1, pi_size);
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
 * Function to handle notification from queues.cc at completion of
 * a batch construction for a given sequencer queue.
 */
void
acc_ring_batch_end_notify(void *user_ctx,
                          dp_mem_t *seq_desc_container,
                          uint16_t batch_size)
{
    queues::seq_desc_t  *seq_desc;

    /*
     * Indicate batch mode in the given descriptor and the
     * accumulated batch count.
     */
    assert(batch_size);
    if (batch_size) {
        seq_desc = (queues::seq_desc_t *)seq_desc_container->read();
        seq_desc->acc_batch_size = htons(batch_size);
        seq_desc->acc_batch_mode = true;
    }
    seq_desc_container->write_thru();
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
    uint8_t             *cache_desc;
    dp_mem_t            *seq_desc_container;
    bool                update_seq_desc;
    bool                ring_wrapped;

    switch (push_type) {

    case ACC_RING_PUSH_SEQUENCER:
        cache_desc = ring_cache_desc_fill(src_desc, &ring_wrapped);
        queues::seq_sq_batch_consume_end(seq_qid);
        seq_desc_container = queues::seq_sq_consume_entry(seq_qid,
                                                          &curr_seq_pd_idx);
        seq_desc_fill(seq_desc_container, cache_desc);
        seq_desc_container->write_thru();
        test_ring_doorbell(queues::get_seq_lif(), SQ_TYPE,
                           seq_qid, 0, curr_seq_pd_idx);
        curr_seq_qid = seq_qid;
        curr_push_type = ACC_RING_PUSH_INVALID;
        break;

    case ACC_RING_PUSH_SEQUENCER_BATCH:
        cache_desc = ring_cache_desc_fill(src_desc, &ring_wrapped);
        seq_desc_container = queues::seq_sq_batch_consume_entry(seq_qid,
                                     ring_base_mem_pa, &curr_seq_pd_idx,
                                     &update_seq_desc, acc_ring_batch_end_notify,
                                     (void *)this);
        if (update_seq_desc) {
            seq_desc_fill(seq_desc_container, cache_desc);
        }

        /*
         * If intermediate descriptor cache index wraps, end the current batch
         * since P4+ doesn't know the cache base and wouldn't know how to
         * break up the source portion of the transfer. 
         */
        if (ring_wrapped) {
            queues::seq_sq_batch_consume_end(seq_qid);
        }

        /*
         * Defer until caller calls post_push()
         */
        curr_seq_qid = seq_qid;
        curr_push_type = push_type;
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

/*
 * Utility to fill an intermediate descriptor in the 
 * ring_base_mem cache.
 */
uint8_t *
acc_ring_t::ring_cache_desc_fill(const void *src_desc,
                                 bool *ring_wrapped)
{
    uint8_t     *cache_desc;
    uint32_t    pd_idx;

    pd_idx = curr_pd_idx;
    curr_pd_idx = (curr_pd_idx + 1) % ring_size;

    cache_desc = &ring_base_mem[pd_idx * desc_size];
    memcpy(cache_desc, src_desc, desc_size);

    *ring_wrapped = curr_pd_idx < pd_idx;
    return cache_desc;
}

/*
 * Utility to fill an sequencer descriptor.
 */
void
acc_ring_t::seq_desc_fill(dp_mem_t *seq_desc_container,
                          uint8_t *cache_desc)
{
    queues::seq_desc_t  *seq_desc;

    seq_desc_container->clear();
    seq_desc = (queues::seq_desc_t *)seq_desc_container->read();
    seq_desc->acc_desc_addr = htonll(host_mem_v2p(cache_desc));
    seq_desc->acc_pndx_addr = htonll(cfg_ring_pd_idx);
    seq_desc->acc_pndx_shadow_addr = htonll(shadow_pd_idx_mem->pa());
    seq_desc->acc_ring_addr = htonll(ring_base_mem_pa);
    seq_desc->acc_desc_size = (uint8_t)log2(desc_size);
    seq_desc->acc_pndx_size = (uint8_t)log2(pi_size);
    seq_desc->acc_ring_size = (uint8_t)log2(ring_size);
}

}  // namespace tests
