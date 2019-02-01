#ifndef DPI_REGISTRY_H
#define DPI_REGISTRY_H

#include <atomic>
#include <mutex>
#include <string>
#include <functional>
#include <vector>
#include <map>

using namespace std;

typedef function<void()> cb;

typedef enum {SoftReset, InitStart, InitDone, Config, EOSInt, EOSCnt, EOS} api_step_e;
typedef map<api_step_e, vector<cb> > dpi_funcptr_map;

class DPIRegistry {
public:
   dpi_funcptr_map dpi_fns;

   static DPIRegistry* access();
   void add_fn(api_step_e which_step, cb fnptr) {
      dpi_fns[which_step].push_back(fnptr);
   }
   void exec_fns(api_step_e which_step) {
      for (auto& f : dpi_fns[which_step])
         f();
   }
private:
   DPIRegistry() {}
   static atomic<DPIRegistry*> pinstance;
   static mutex m_;
};

#endif
