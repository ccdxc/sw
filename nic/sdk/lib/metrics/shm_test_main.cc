// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ev++.h>
#include "shm.hpp"

using namespace std;
using namespace sdk;
using namespace sdk::metrics;

const int32_t kshmTestMemSize = (1024 * 1024);
const string kshmName = "test_shm";

int main(int argc, char **argv) {
    int c;
    bool    client     = true;
    bool    server     = false;
    int32_t mem_size   = kshmTestMemSize;
    string  shm_name   = kshmName;
    int     num_allocs = 1;
    bool    dump_meta  = false;
    int     alloc_len  = 1;
    ShmUptr shm_ptr(new Shm());
    error         err;

    while ((c = getopt (argc, argv, "csz:n:a:dl:")) != -1) {
        switch (c){
        case 'c':
            client = true;
            server = false;
            break;
        case 's':
            client = false;
            server = true;
            break;
        case 'z':
            mem_size = atoi(optarg);
            break;
        case 'n':
            shm_name = optarg;
            break;
        case 'a':
            num_allocs = atoi(optarg);
            break;
        case 'd':
            dump_meta = true;
            break;
        case 'l':
            alloc_len = atoi(optarg);
            break;
        case '?':
        default:
            LogError("Invalid arguments");
            abort ();
          }
      }

      if (client) {
          LogInfo("Running in client mode");
          err = shm_ptr->MemMap(shm_name, mem_size);
      } else if (server) {
          LogInfo("Running in server mode");
          err = shm_ptr->MemMap(shm_name, mem_size);
      }
      if (err.IsNotOK()) {
          LogError("Memory mapping shared memory {} of size {} failed. Err: {}", shm_name, mem_size, err);
      }

      LogInfo("Memory mapped region at {}", shm_ptr->GetBase());
      LogInfo("Allocating and freeing {} objects", num_allocs);

      for (int i = 0; i < num_allocs; i++) {
          void *ptr = shm_ptr->Alloc(alloc_len);
          if (ptr == NULL) {
              LogError("Error allocating memory");
          }

          err = shm_ptr->Free(ptr);
          if (err.IsNotOK()) {
              LogError("Error freeing memory. Err: {}", err);
          }
      }

      // dump metadata
      if (dump_meta) {
          shm_ptr->DumpMeta();
      }
}
