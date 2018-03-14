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
    bool          use_cpy;
    bool          cow; //Copy On Write
    HOST_MEM_CPY() {
	_mem = new pen_mem<12>("HOST_MEM_CPY", 0x0ull,
			       0xfffffffffffffull);
        use_cpy = false;
	cow     = false;
    }
    ~HOST_MEM_CPY() {
	delete _mem;
    }
public:    
    virtual bool burst_read(uint64_t addr, unsigned char * data, unsigned int len, bool secure = false, bool reverse_byte_order = false) {
	addr = ((1ull<<52)-1)&addr;
	if (_mem->is_dirty(addr) || use_cpy) {
	    return _mem->burst_read(addr, data, len, secure, reverse_byte_order);
	} else {
	    return HOST_MEM::access()->burst_read(addr, data, len, secure, reverse_byte_order);
	}
    }
    virtual bool burst_write(uint64_t addr, const unsigned char * data, unsigned int len, bool secure = false, bool reverse_byte_order = false) {
	addr = ((1ull<<52)-1)&addr;
	if (cow && ((addr & 0x3f) != 0 || (len & 0x3f) != 0)) {
	  auto aligned_addr  = (addr & (~0x3full));
	  uint32_t num_flits = (len+63)/64;
	  uint8_t  buf[64];
	  for (uint32_t i = 0; i < num_flits; i++) {
	    // Don't copy if it is already there.
	    if (!_mem->is_dirty(aligned_addr)) {
	      HOST_MEM::access()->burst_read(aligned_addr+(64*i), buf, 64, secure, reverse_byte_order); 
	      _mem->burst_write(aligned_addr+(64*i), buf, 64, secure, reverse_byte_order);
	    }
	  }
	}
	return _mem->burst_write(addr, data, len, secure, reverse_byte_order);
    }
    static HOST_MEM_CPY * access()
    {
        static HOST_MEM_CPY * _hmc = nullptr;
        if (!_hmc) {
            _hmc = new HOST_MEM_CPY();
        }
        return _hmc;
    }
    pen_mem<12> * get_mem(void) {
	return _mem;
    }
   void set_use_cpy(bool cpy) {
     use_cpy = cpy;
    }
   void set_cow(bool _cow) {
     cow = _cow;
    }
};
#endif
