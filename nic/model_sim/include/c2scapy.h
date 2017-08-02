//
// Singleton to handle interface to Scapy
//

#ifndef C2SCAPY
#define C2SCAPY

#include <Python.h>
#include "LogMsg.h"

using namespace std;

class C2Scapy {
 public:
  static C2Scapy & Access() {
    // lazy instantiation & guaranteed destruction
    static C2Scapy c2scapy_if;
    return (c2scapy_if);
  }

  void DecodeEthPkt(char *pkt_str);

  void CreatePkt(char *scapy_fmt_str, string & pkt_bytes);

  void GetHeaderFieldsInfo(string header_name, string func, string & fields_str);

  void SetRandomSeed(int seed);

  int IntRetCharArg(const char *func_name, const char *in_str);

  char *CharRetCharArg(const char *func_name, const char *in_str);

 private:
  int my_seed;

  PyObject *py_module_;

  C2Scapy();
  ~C2Scapy();

  // don't implementso that nobody can get another instance
  C2Scapy(C2Scapy const&);
  void operator=(C2Scapy const&);
  
};

#endif
