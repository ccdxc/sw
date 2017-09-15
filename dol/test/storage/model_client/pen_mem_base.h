#ifndef _PEN_MEM_BASE_H_
#define _PEN_MEM_BASE_H_
class pen_mem_base {
public:
    pen_mem_base() {}
    virtual ~pen_mem_base() {}
    virtual bool burst_read(uint64_t addr, unsigned char * data, unsigned int len, bool secure = false, bool reverse_byte_order = false) = 0;
    virtual bool burst_write(uint64_t addr, const unsigned char * data, unsigned int len, bool secure = false, bool reverse_byte_order = false) = 0;
    virtual bool can_block_write() { return true; }
};

#endif
