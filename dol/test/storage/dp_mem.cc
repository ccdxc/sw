#include "dol/test/storage/utils.hpp"
#include "dol/test/storage/hal_if.hpp"
#include "dol/test/storage/dp_mem.hpp"
#include "nic/model_sim/include/lib_model_client.h"

namespace dp_mem {

/*
 * DataPath memory
 */
dp_mem_t::dp_mem_t(uint32_t num_entries,
                   uint32_t entry_size,
                   bool page_aligned) :
    num_entries(num_entries),
    entry_size(entry_size),
    total_size(num_entries * entry_size),
    curr_entry_idx(0),
    is_member(false)
{
    int         alloc_rc;

    /*
     * total_size may equal 0 which would indicate a member creation,
     * see member_find().
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
	std::unordered_map<uint64_t, dp_mem_t*>::iterator member_it;

    /*
     * Iterate and delete members
     */
    member_it = member_map.begin();
	while (member_it != member_map.end()) {
        delete member_it->second;
        member_it++;
    }
    member_map.clear();
    
    /*
     * There are no methods to free HBM memory but at least we can free
     * allocated local memory
     */
    if (!is_member) {
        delete[] cache;
    }
}


/*
 * Set current location index
 */
dp_mem_t *
dp_mem_t::entry_set(uint32_t entry_idx)
{
    if (entry_idx >= num_entries) {
        return nullptr;
    }

    curr_entry_idx = entry_idx;
    return this;
}


/*
 * Clear memory at current cache location, but not its corresponding
 * datapath memory (see clear_thru()).
 */
void
dp_mem_t::clear(void)
{
    memset(cache_entry_addr(), 0, entry_size);
}


/*
 * Clear memory at current cache location and its corresponding
 * datapath memory.
 */
void
dp_mem_t::clear_thru(void)
{
    clear();
    write_thru();
}


/*
 * Return pointer to current cache location, allowing caller to
 * modify its content.
 */
uint8_t *
dp_mem_t::read(void)
{
    return cache_entry_addr();
}


/*
 * Transfer corresponding datapath memory into current cache location.
 */
uint8_t *
dp_mem_t::read_thru(void)
{
    read_mem(hbm_entry_addr(), cache_entry_addr(), entry_size);
    return read();
}


/*
 * Update bit fields at the current cache location (but not its corresponding
 * datapath memory).
 */
void
dp_mem_t::write_bit_fields(uint32_t start_bit_offset,
                           uint32_t size_in_bits,
                           uint64_t value)
{
    uint32_t    byte_offset = start_bit_offset / BITS_PER_BYTE;
    uint32_t    byte_size = (size_in_bits + BITS_PER_BYTE - 1) / BITS_PER_BYTE;

    if ((byte_offset + byte_size) > entry_size) {
        printf("%s start_bit_offset %u size_in_bits %u too large\n",
               __FUNCTION__, start_bit_offset, size_in_bits);
        return;
    }

    utils::write_bit_fields(cache_entry_addr(), start_bit_offset, size_in_bits, value);
}


/*
 * Write current value in current cache location to corresponding datapath memory.
 */
void
dp_mem_t::write_thru(void)
{
    write_mem(hbm_entry_addr(), cache_entry_addr(), entry_size);
}


/*
 * Return physical address of the current datapath memory location.
 */
uint64_t
dp_mem_t::pa(void)
{
    return hbm_entry_addr();
}


/*
 * Return virtual address of the current datapath memory location
 * (which is same as physical address)
 */
uint64_t
dp_mem_t::va(void)
{
    return pa();
}


/*
 * Find/create a new dp_mem_t which addresses a portion of the same
 * cache and datapath memory at the current location.
 */
dp_mem_t *
dp_mem_t::member_find(uint32_t byte_offset,
                      uint32_t byte_size)
{
    dp_mem_t    *member;
    uint64_t    member_key;
	std::pair<uint64_t, dp_mem_t*> member_elem;
	std::unordered_map<uint64_t, dp_mem_t*>::const_iterator member_it;

    if ((byte_offset + byte_size) > entry_size) {
        printf("%s byte_offset %u plus size %u exceeds entry_size %u\n",
               __FUNCTION__, byte_offset, byte_size, entry_size);
        return nullptr;
    }

    member_key = ((uint64_t)(byte_offset + (curr_entry_idx * entry_size)) << 32) |
                 byte_size;
    member_it = member_map.find(member_key);
    if (member_it == member_map.end()) {
        member = new dp_mem_t(0, 0);
        member->num_entries = 1;
        member->entry_size = byte_size;
        member->total_size = byte_size;
        member->is_member = true;
        member->hbm_addr = hbm_entry_addr() + byte_offset;
        member->cache = cache_entry_addr() + byte_offset;

        member_elem = std::make_pair(member_key, member);
        member_map.insert(member_elem);

    } else {
        member = member_it->second;
    }

    return member;
}


/*
 * Return address of the current datapath memory location.
 */
uint64_t 
dp_mem_t::hbm_entry_addr(void)
{
    return hbm_addr + (curr_entry_idx * entry_size);
}


/*
 * Return address of the current cache memory location.
 */
uint8_t *
dp_mem_t::cache_entry_addr(void)
{
    return &cache[curr_entry_idx * entry_size];
}

} // namespace dp_mem