#ifndef _DP_MEM_HPP_
#define _DP_MEM_HPP_

#include <utility>
#include <unordered_map>

#ifndef BITS_PER_BYTE
#define BITS_PER_BYTE       8
#endif

namespace dp_mem {

/*
 * DataPath memory
 */
class dp_mem_t
{
public:

    dp_mem_t(uint32_t num_entries,
             uint32_t entry_size,
             bool page_aligned = false);

    ~dp_mem_t();

    // method functions
    dp_mem_t *entry_set(uint32_t entry_idx);

    void clear(void);
    void clear_thru(void);
    uint8_t *read(void);
    uint8_t *read_thru(void);
    void write_bit_fields(uint32_t start_bit_offset,
                          uint32_t size_in_bits,
                          uint64_t value);
    void write_thru(void);
    dp_mem_t *member_find(uint32_t byte_offset,
                          uint32_t byte_size);
    uint64_t va(void);
    uint64_t pa(void);

private:

    // method functions
    uint64_t hbm_entry_addr(void);
    uint8_t *cache_entry_addr(void);

    uint8_t         *cache;
    uint64_t        hbm_addr;
    uint32_t        num_entries;
    uint32_t        entry_size;
    uint32_t        total_size;
    uint32_t        curr_entry_idx;
    bool            is_member;

    /*
     * Members are elements that address some portions of the same
     * cache and datapath memory.
     */
	std::unordered_map<uint64_t, dp_mem_t*> member_map;
};

}  // namespace dp_mem


#endif  // _DP_MEM_HPP_
