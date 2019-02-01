#ifndef CAP_BASE_API_H
#define CAP_BASE_API_H

#include "LogMsg.h"

class cap_base_api {

public:
   string sknob_base;
   string name;
   int chip_id;
   int num_insts;

   cap_base_api(string _nm, string _sk_base, int _chip_id = 0, int _num_insts = 1) {
      name = _nm;
      sknob_base = _sk_base;
      chip_id = _chip_id;
      num_insts = _num_insts;
   } 

   virtual void init() = 0;
   virtual void eos_cnt() = 0;
   virtual void eos_int() = 0;
   virtual void eos_sta() = 0;
   virtual void eos() = 0;

};

#endif // CAP_BASE_API_H
