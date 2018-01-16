// -*- C++ -*-
#ifndef _HOST_MEM_H_
#define _HOST_MEM_H_
#include "pen_mem.h"
class HOST_MEM {
public:
    static pen_mem_base * access(pen_mem_base * p = nullptr)
    {
        static pen_mem_base * _hm = p;
        if (!_hm) {
            _hm = new pen_mem<12>("HOST_MEM", 0x0ull,
                                   0xfffffffffffffull);
        }

        return _hm;
    }
};

class HOST_MEM_CPY : public pen_mem_base {
    pen_mem<12> * _mem;
    HOST_MEM_CPY() {
	_mem = new pen_mem<12>("HOST_MEM_CPY", 0x0ull,
			       0xfffffffffffffull);
    }
    ~HOST_MEM_CPY() {
	delete _mem;
    }
public:    
    virtual bool burst_read(uint64_t addr, unsigned char * data, unsigned int len, bool secure = false, bool reverse_byte_order = false) {
	if (_mem->is_dirty(addr)) {
	    return _mem->burst_read(addr, data, len, secure, reverse_byte_order);
	} else {
	    return HOST_MEM::access()->burst_read(addr, data, len, secure, reverse_byte_order);
	}
    }
    virtual bool burst_write(uint64_t addr, const unsigned char * data, unsigned int len, bool secure = false, bool reverse_byte_order = false) {
	return _mem->burst_write(addr, data, len, secure, reverse_byte_order);
    }
    static pen_mem_base * access()
    {
        static HOST_MEM_CPY * _hmc = nullptr;
        if (!_hmc) {
            _hmc = new HOST_MEM_CPY();
        }
        return _hmc;
    }
};
#endif
