#include "dol/test/storage/utils.hpp"
#include "dol/test/storage/hal_if.hpp"
#include "dol/test/storage/dp_mem.hpp"
#include "nic/utils/host_mem/c_if.h"
#include "nic/model_sim/include/lib_model_client.h"

namespace dp_mem {

/*
 * DataPath memory
 */
dp_mem_t::dp_mem_t(uint32_t num_lines,
                   uint32_t spec_line_size,
                   dp_mem_align_t mem_align,
                   dp_mem_type_t mem_type,
                   uint32_t spec_align_size) :
    mem_type(mem_type),
    cache(nullptr),
    hbm_addr(0),
    num_lines(num_lines),
    line_size(spec_line_size),
    total_size(num_lines * line_size),
    curr_line(0),
    next_line(0),
    fragment_key(0),
    fragment_parent(nullptr)
{
    int         alloc_rc;

    /*
     * total_size may equal 0 which would indicate a fragment creation,
     * see fragment_find().
     */
    if (total_size) {

        /*
         * ASIC/RTL wants every new line in host memory to be aligned
         * on at least a 64-byte boundary
         */
        if (mem_align == DP_MEM_ALIGN_PAGE) {
            spec_align_size = utils::kUtilsPageSize;
        }
        if ((mem_type == DP_MEM_TYPE_HOST_MEM) && 
            (mem_align != DP_MEM_ALIGN_PAGE)) {

            mem_align = DP_MEM_ALIGN_SPEC;
            spec_align_size = std::max(kMinHostMemAllocSize,
                                       spec_align_size);
            /*
             * When there's more than one line, dp_mem enforces the same
             * alignment on every line so the line size must be
             * rounded up if necessary.
             */
            if (num_lines > 1) {
                line_size = utils::roundup_to_pow_2(spec_line_size);
                total_size = num_lines * line_size;
            }
        }

        /*
         * Validate alignment
         */
        if (mem_align != DP_MEM_ALIGN_NONE) {

            /*
             * align_spec_size should be a power of 2
             */
            if (!spec_align_size || (spec_align_size & (spec_align_size - 1))) {
                printf("%s spec_align_size %u is not a valid power of 2\n", 
                       __FUNCTION__, spec_align_size);
                assert(spec_align_size && !(spec_align_size & (spec_align_size - 1)));
            }

            if (num_lines > 1) {
                assert((line_size & (line_size - 1)) == 0);
                if (spec_align_size > line_size) {
                    assert((spec_align_size & (line_size - 1)) == 0);
                }
            }
        }

        switch (mem_type) {

        case DP_MEM_TYPE_HBM:
            alloc_rc = mem_align == DP_MEM_ALIGN_NONE ? 
                       utils::hbm_addr_alloc(total_size, &hbm_addr) :
                       utils::hbm_addr_alloc_spec_aligned(total_size, &hbm_addr,
                                                          spec_align_size);
            if (alloc_rc < 0) {
                printf("%s unable to allocate HBM memory size %u\n",
                       __FUNCTION__, total_size);
                assert(alloc_rc >= 0);
            }

            cache = new (std::nothrow) uint8_t[total_size];
            if (!cache) {
                printf("%s unable to allocate cache size %u\n",
                       __FUNCTION__, total_size);
                assert(cache);
            }

            all_lines_clear_thru();
            break;

        case DP_MEM_TYPE_HOST_MEM:

            /*
             * cache is the same as the requested host mem
             */
            cache = mem_align == DP_MEM_ALIGN_NONE ? 
                    (uint8_t *)alloc_host_mem(total_size) :
                    (uint8_t *)alloc_spec_aligned_host_mem(total_size,
                                                           spec_align_size);
            if (!cache) {
                printf("%s unable to allocate host memory size %u\n",
                       __FUNCTION__, total_size);
                assert(cache);
            }

            memset(cache, 0, total_size);
            break;

        default:
            printf("%s invalid memory type %u\n",
                   __FUNCTION__, mem_type);
            assert((mem_type == DP_MEM_TYPE_HBM) ||
                   (mem_type == DP_MEM_TYPE_HOST_MEM));
            break;
        }
    }
}


dp_mem_t::~dp_mem_t()
{
    std::unordered_map<uint64_t, dp_mem_t*>::iterator fragment_it;
    dp_mem_t    *fragment;

    /*
     * Iterate and delete fragments
     */
    fragment_it =  fragments_map.begin();
    while (fragment_it != fragments_map.end()) {
        fragment = fragment_it->second;
        fragment_it = fragments_map.erase(fragment_it);
        delete fragment;
    }
    
    /*
     * Take self out of parent's map
     */
    if (fragment_parent) {
        fragment_parent->fragments_map.erase(fragment_key);
    } else {

        /*
         * There are no methods to free HBM memory but at least we can free
         * allocated local memory
         */
        if (cache) {
            if (is_mem_type_host_mem()) {
                free_host_mem(cache);
            } else {
                delete[] cache;
            }
        }
    }
}


/*
 * Return current line index
 */
uint32_t
dp_mem_t::line_get(void)
{
    return curr_line;
}


/*
 * Return the next line index as it was last computed.
 */
uint32_t
dp_mem_t::next_line_get(void)
{
    return next_line;
}


/*
 * Set current line index (next line index is also reset to same value).
 */
void
dp_mem_t::line_set(uint32_t line)
{
    assert(line < num_lines);
    curr_line = line;
    next_line = line;
}


/*
 * Set only the next line index (the current line index remains unchanged).
 */
uint32_t
dp_mem_t::next_line_set(void)
{
    assert(num_lines);
    next_line = (curr_line + 1) % num_lines;
    return next_line;
}


/*
 * Advance current line index (next line index is also reset to same value).
 */
uint32_t
dp_mem_t::line_advance(void)
{
    next_line_set();
    curr_line = next_line;
    return curr_line;
}


/*
 * Fill memory at current cache line with a repeated byte value,
 * but not its corresponding datapath memory (see fill_thru()).
 */
void
dp_mem_t::fill(uint8_t fill_byte)
{
    memset(cache_line_addr(), fill_byte, line_size);
}


/*
 * Clear memory at current cache line and its corresponding
 * datapath memory.
 */
void
dp_mem_t::fill_thru(uint8_t fill_byte)
{
    fill(fill_byte);
    write_thru();
}


/*
 * Clear memory at current cache line, but not its corresponding
 * datapath memory (see clear_thru()).
 */
void
dp_mem_t::clear(void)
{
    fill(0);
}


/*
 * Clear memory at current cache line and its corresponding
 * datapath memory.
 */
void
dp_mem_t::clear_thru(void)
{
    fill_thru(0);
}


/*
 * Clear memory at all cache lines and their corresponding
 * datapath memory.
 */
void
dp_mem_t::all_lines_clear_thru(void)
{
    uint32_t    save_curr_line = curr_line;

    for (curr_line = 0; curr_line < num_lines; curr_line++) {
        clear_thru();
    }
    curr_line = save_curr_line;
}


/*
 * Return pointer to current cache line, allowing caller to
 * modify its content.
 */
uint8_t *
dp_mem_t::read(void)
{
    return cache_line_addr();
}


/*
 * Transfer corresponding datapath memory into current cache line.
 */
uint8_t *
dp_mem_t::read_thru(void)
{
    uint8_t     *curr_cache_addr;
    uint64_t    curr_read_addr;
    uint32_t    curr_read_size;
    uint32_t    total_read_size;

    if (is_mem_type_hbm()) {

        /*
         * read_mem has an upper size limit so break up into multiple 
         * reads as necessary
         */
        if (line_size <= DP_HBM_WRITE_READ_UPPER_LIMIT) {
            read_mem(hbm_line_addr(), cache_line_addr(), line_size);
            return read();
        }

        curr_cache_addr = cache_line_addr();
        curr_read_addr = hbm_line_addr();
        total_read_size = line_size;
        while (total_read_size) {
            curr_read_size = total_read_size > DP_HBM_WRITE_READ_UPPER_LIMIT ?
                             DP_HBM_WRITE_READ_UPPER_LIMIT : total_read_size;
            read_mem(curr_read_addr, curr_cache_addr, curr_read_size);
            curr_cache_addr += curr_read_size;
            curr_read_addr += curr_read_size;
            total_read_size -= curr_read_size;
        }
    }
    return read();
}


/*
 * Update bit fields at the current cache line (but not its corresponding
 * datapath memory).
 */
void
dp_mem_t::write_bit_fields(uint32_t start_bit_offset,
                           uint32_t size_in_bits,
                           uint64_t value)
{
    uint32_t    byte_offset = start_bit_offset / BITS_PER_BYTE;
    uint32_t    byte_size = (size_in_bits + BITS_PER_BYTE - 1) / BITS_PER_BYTE;

    if ((byte_offset + byte_size) > line_size) {
        printf("%s start_bit_offset %u size_in_bits %u > line_size %u\n",
               __FUNCTION__, start_bit_offset, size_in_bits, line_size);
        assert((byte_offset + byte_size) <= line_size);
        return;
    }

    utils::write_bit_fields(cache_line_addr(), start_bit_offset,
                            size_in_bits, value);
}


/*
 * Write current value in current cache line to corresponding datapath memory.
 */
void
dp_mem_t::write_thru(void)
{
    uint8_t     *curr_cache_addr;
    uint64_t    curr_write_addr;
    uint32_t    curr_write_size;
    uint32_t    total_write_size;

    if (is_mem_type_hbm()) {

        /*
         * write_mem has an upper size limit so break up into multiple 
         * writes as necessary
         */
        if (line_size <= DP_HBM_WRITE_READ_UPPER_LIMIT) {
            write_mem(hbm_line_addr(), cache_line_addr(), line_size);
            return;
        }

        curr_cache_addr = cache_line_addr();
        curr_write_addr = hbm_line_addr();
        total_write_size = line_size;
        while (total_write_size) {
            curr_write_size = total_write_size > DP_HBM_WRITE_READ_UPPER_LIMIT ? 
                              DP_HBM_WRITE_READ_UPPER_LIMIT : total_write_size;
            write_mem(curr_write_addr, curr_cache_addr, curr_write_size);
            curr_cache_addr += curr_write_size;
            curr_write_addr += curr_write_size;
            total_write_size -= curr_write_size;
        }
    }
}


/*
 * Return physical address of the current datapath memory line.
 */
uint64_t
dp_mem_t::pa(void)
{
    if (is_mem_type_hbm()) {
        return hbm_line_addr();
    }

    return host_mem_v2p(cache_line_addr());
}


/*
 * Return virtual address of the current datapath memory line.
 * For HBM, it is the same as physical address.
 */
uint64_t
dp_mem_t::va(void)
{
    if (is_mem_type_hbm()) {
        return hbm_line_addr();
    }

    return (uint64_t)cache_line_addr();
}


/*
 * Find/create a new dp_mem_t which addresses a portion of the same
 * cache and datapath memory at the current line.
 */
dp_mem_t *
dp_mem_t::fragment_find(uint32_t frag_offset,
                        uint32_t frag_size)
{
    dp_mem_t    *fragment;
    uint64_t    local_key;
	std::pair<uint64_t, dp_mem_t*> fragment_elem;
	std::unordered_map<uint64_t, dp_mem_t*>::const_iterator fragment_it;

    if ((frag_offset + frag_size) > line_size) {
        printf("%s frag_offset %u plus size %u exceeds line_size %u\n",
               __FUNCTION__, frag_offset, frag_size, line_size);
        assert((frag_offset + frag_size) <= line_size);
        return nullptr;
    }

    local_key = ((uint64_t)(frag_offset + (curr_line * line_size)) << 32) |
                frag_size;
    fragment_it = fragments_map.find(local_key);
    if (fragment_it == fragments_map.end()) {
        fragment = new dp_mem_t(0, 0, DP_MEM_ALIGN_NONE, mem_type);
        fragment->num_lines = 1;
        fragment->line_size = frag_size;
        fragment->total_size = frag_size;
        if (is_mem_type_hbm()) {
            fragment->hbm_addr = hbm_line_addr() + frag_offset;
        }
        fragment->cache = cache_line_addr() + frag_offset;

        fragment->fragment_key = local_key;
        fragment->fragment_parent = this;
        fragment_elem = std::make_pair(local_key, fragment);
        fragments_map.insert(fragment_elem);

    } else {
        fragment = fragment_it->second;
    }

    return fragment;
}


/*
 * Return address of the current datapath memory line..
 */
uint64_t 
dp_mem_t::hbm_line_addr(void)
{
    assert(is_mem_type_hbm());
    return hbm_addr + (curr_line * line_size);
}


/*
 * Return address of the current cache memory line.
 */
uint8_t *
dp_mem_t::cache_line_addr(void)
{
    return &cache[curr_line * line_size];
}

} // namespace dp_mem
