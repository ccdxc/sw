#include "dol/test/storage/utils.hpp"
#include "dol/test/storage/hal_if.hpp"
#include "dol/test/storage/dp_mem.hpp"
#include "nic/model_sim/include/lib_model_client.h"

namespace dp_mem {

/*
 * DataPath memory
 */
dp_mem_t::dp_mem_t(uint32_t num_lines,
                   uint32_t line_size,
                   bool page_aligned) :
    cache(nullptr),
    hbm_addr(0),
    num_lines(num_lines),
    line_size(line_size),
    total_size(num_lines * line_size),
    curr_line(0),
    is_fragment(false)
{
    int         alloc_rc;

    /*
     * total_size may equal 0 which would indicate a fragment creation,
     * see fragment_find().
     */
    if (total_size) {
        alloc_rc = page_aligned ? 
                   utils::hbm_addr_alloc_page_aligned(total_size, &hbm_addr) :
                   utils::hbm_addr_alloc(total_size, &hbm_addr);
        if (alloc_rc < 0) {
          printf("%s unable to allocate HBM memory size %u\n",
                 __FUNCTION__, total_size);
          assert(alloc_rc >= 0);
        }

        cache = new (std::nothrow) uint8_t[total_size];
        if (!cache) {
            printf("%s unable to allocate host memory size %u\n",
                   __FUNCTION__, total_size);
            assert(cache);
        }

        memset(cache, 0, total_size);
    }
}


dp_mem_t::~dp_mem_t()
{
	std::unordered_map<uint64_t, dp_mem_t*>::iterator fragment_it;

    /*
     * Iterate and delete fragments
     */
    fragment_it = fragments_map.begin();
	while (fragment_it != fragments_map.end()) {
        delete fragment_it->second;
        fragment_it++;
    }
    fragments_map.clear();
    
    /*
     * There are no methods to free HBM memory but at least we can free
     * allocated local memory
     */
    if (!is_fragment && cache) {
        delete[] cache;
    }
}


/*
 * Set current line index
 */
dp_mem_t *
dp_mem_t::line_set(uint32_t line)
{
    if (line >= num_lines) {
        return nullptr;
    }

    curr_line = line;
    return this;
}


/*
 * Clear memory at current cache line, but not its corresponding
 * datapath memory (see clear_thru()).
 */
void
dp_mem_t::clear(void)
{
    memset(cache_line_addr(), 0, line_size);
}


/*
 * Clear memory at current cache line and its corresponding
 * datapath memory.
 */
void
dp_mem_t::clear_thru(void)
{
    clear();
    write_thru();
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
    read_mem(hbm_line_addr(), cache_line_addr(), line_size);
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
        printf("%s start_bit_offset %u size_in_bits %u too large\n",
               __FUNCTION__, start_bit_offset, size_in_bits);
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
    write_mem(hbm_line_addr(), cache_line_addr(), line_size);
}


/*
 * Return physical address of the current datapath memory line.
 */
uint64_t
dp_mem_t::pa(void)
{
    return hbm_line_addr();
}


/*
 * Return virtual address of the current datapath memory line
 * (which is same as physical address)
 */
uint64_t
dp_mem_t::va(void)
{
    return pa();
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
    uint64_t    fragment_key;
	std::pair<uint64_t, dp_mem_t*> fragment_elem;
	std::unordered_map<uint64_t, dp_mem_t*>::const_iterator fragment_it;

    if ((frag_offset + frag_size) > line_size) {
        printf("%s frag_offset %u plus size %u exceeds line_size %u\n",
               __FUNCTION__, frag_offset, frag_size, line_size);
        assert((frag_offset + frag_size) <= line_size);
        return nullptr;
    }

    fragment_key = ((uint64_t)(frag_offset + (curr_line * line_size)) << 32) |
                   frag_size;
    fragment_it = fragments_map.find(fragment_key);
    if (fragment_it == fragments_map.end()) {
        fragment = new dp_mem_t(0, 0);
        fragment->num_lines = 1;
        fragment->line_size = frag_size;
        fragment->total_size = frag_size;
        fragment->is_fragment = true;
        fragment->hbm_addr = hbm_line_addr() + frag_offset;
        fragment->cache = cache_line_addr() + frag_offset;

        fragment_elem = std::make_pair(fragment_key, fragment);
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