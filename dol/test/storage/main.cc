#include <stdio.h>
#include <strings.h>
#include <string>
#include <gflags/gflags.h>

#include "dol/test/storage/tests.hpp"

DEFINE_uint64(hal_port, 50052, "TCP port hal is listening to");

struct {
  int (*test_fn)();
  const std::string test_name;
  bool test_succeded;
} test_suite[] = {
  {&tests::test_run_nvme_pvm_admin_cmd, "NVME->PVM Admin Cmd", false},
  {&tests::test_run_nvme_pvm_read_cmd, "NVME->PVM Read Cmd", false}

  // Always last entry
  {0}
};


int main(int argc, char**argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  if (tests::test_setup() < 0) {
    printf("Setup failed\n");
    return 1;
  }
  printf("Setup completed\n");

  for (int i = 0; test_suite[i].test_fn != nullptr; i++) {
    if (test_suite[i].test_fn() < 0)
      test_suite[i].test_succeded = false;
    else
      test_suite[i].test_succeded = true;
  }

  printf("\nConsolidated Test Report \n");
  printf("--------------------------------------------------------------\n");
  printf("Number\t\tName\t\t\tResult\n");
  printf("--------------------------------------------------------------\n");
  
  for (int i = 0; test_suite[i].test_fn != nullptr; i++) {
    printf("%d\t\t", i+1);
    printf("%s\t", test_suite[i].test_name.c_str());
    printf("%s\n", test_suite[i].test_succeded ? "Success" : "Failure");
  }
  // TODO: Figreout success/failure return
  return 0;
}
