// TODO Move to dol/test when sanity has build infra for dol.

// Storage DOL Testcases.
// These are involked by the DOL framework.
#include <stdint.h>
#include <stdio.h>

extern "C" {

typedef struct config_info {
  bool dry_run;
  uint64_t hw_lif_id;
  uint64_t q0_addrs[8];
} config_info_t;

void storage_test_config(config_info_t *cfg) {
#if 0
  printf("Got config in storage_test.cc\n");
  printf("\tdryrun = %s\n", cfg->dry_run ? "True" : "False");
  printf("\thw_lif_id = %lu\n", cfg->hw_lif_id);
  for (int i = 0; i < 8; i++)
    printf("\t\ttype:%d q0_addr=%lx\n", i, cfg->q0_addrs[i]);
#endif
}

}  // extern "C"
