// -*- C++ -*-
#ifndef _HBM_H_
#define _HBM_H_

#include "pen_mem.h"

class HBM : public pen_mem<12> {
public:
    static pen_mem<12> * access()
    {
        //static pen_mem<12> * _hbm = nullptr;
        //if (!_hbm) {
        //    _hbm = new pen_mem<12>("HBM", 0x10000000ull,
        //                           0x10000000ull+0xffffffffull);
        // }
        //
        //return _hbm;
        
        static unique_ptr< pen_mem<12> > _hbm(new pen_mem<12>("HBM", 0x10000000ull, 0x10000000ull+0xffffffffull));
        return _hbm.get();
    }
};
#endif
