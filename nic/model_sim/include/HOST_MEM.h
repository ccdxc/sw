// -*- C++ -*-
#ifndef _HOST_MEM_H_
#define _HOST_MEM_H_
#include "pen_mem.h"
class HOST_MEM : public pen_mem<12> {
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

class HOST_MEM_CPY : public pen_mem<12> {
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
#endif
