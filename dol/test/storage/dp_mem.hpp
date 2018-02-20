#ifndef _DP_MEM_HPP_
#define _DP_MEM_HPP_

#include <utility>
#include <unordered_map>

#ifndef BITS_PER_BYTE
#define BITS_PER_BYTE       8
#endif

namespace dp_mem {

/*
 * For convenience, dp_mem_t can manage either HBM or host memory
 * with its method functions.
 */
typedef enum {
    DP_MEM_TYPE_HBM,
    DP_MEM_TYPE_HOST_MEM,
    DP_MEM_TYPE_LAST
} dp_mem_type_t;


typedef enum {
    DP_MEM_ALIGN_NONE,
    DP_MEM_ALIGN_PAGE
} dp_mem_align_t;


/*
 * DataPath memory
 */
class dp_mem_t
{
public:

    dp_mem_t(uint32_t num_lines,
             uint32_t line_size,
             dp_mem_align_t mem_align = DP_MEM_ALIGN_NONE,
             dp_mem_type_t mem_type = DP_MEM_TYPE_HBM);

    ~dp_mem_t();

    // method functions
    uint32_t line_get(void);
    uint32_t next_line_get(void);
    uint32_t next_line_set(void);
    void line_set(uint32_t line);
    uint32_t line_advance(void);

    void clear(void);
    void clear_thru(void);
    uint8_t *read(void);
    uint8_t *read_thru(void);
    void write_bit_fields(uint32_t start_bit_offset,
                          uint32_t size_in_bits,
                          uint64_t value);
    void write_thru(void);
    dp_mem_t *fragment_find(uint32_t frag_offset,
                            uint32_t frag_size);
    uint64_t va(void);
    uint64_t pa(void);

    // simple inline methods
    bool is_mem_type_hbm(void)
    {
        return mem_type == DP_MEM_TYPE_HBM;
    }

    bool is_mem_type_host_mem(void)
    {
        return mem_type == DP_MEM_TYPE_HOST_MEM;
    }

    uint32_t line_size_get(void)
    {
        return line_size;
    }

private:

    // method functions
    uint64_t hbm_line_addr(void);
    uint8_t *cache_line_addr(void);

    dp_mem_type_t   mem_type;
    uint8_t         *cache;
    uint64_t        hbm_addr;
    uint32_t        num_lines;
    uint32_t        line_size;
    uint32_t        total_size;
    uint32_t        curr_line;
    uint32_t        next_line;

    /*
     * Fragments are elements that address some portions of the same
     * cache and datapath memory line.
     */
    std::unordered_map<uint64_t, dp_mem_t*> fragments_map;
    uint64_t        fragment_key;
    dp_mem_t        *fragment_parent;
};

}  // namespace dp_mem


#endif  // _DP_MEM_HPP_
