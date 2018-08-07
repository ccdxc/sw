/* ***************************************************************************
 * {C} Copyright 2018 Pensando Systems Inc. All rights reserved.
 *
 * FILENAME : kernel_upgrade.hpp
 * 
 * DESCRIPTION :
 *          This is the header file which contains all the function declaration
 *          to aid in kernel upgrade. Currently, they are wrappers around
 *          the kexec-tools application. We have tried to add the basic APIs
 *          which we think will be required from an upgrade perspective.
 *
 * **************************************************************************/
#include <string>
#include "kernel_upgrade.hpp"

namespace upgrade {
namespace kernel {

using namespace std;

const string KEXEC_EXE = "/sbin/kexec";

kernel_upgrade_res
kernel_preupgrade_check() {
    return K_PASS;
}

kernel_upgrade_res
kernel_load_new(string new_kernel,
                string new_initrd) {

}

kernel_upgrade_res
kernel_unload_new(string new_kernel,
                  string new_initrd) {

}

kernel_upgrade_res
kernel_execute() {
}

kernel_upgrade_res
kernel_upgrade(string new_kernel,
               string new_initrd) {
}

kernel_upgrade_res
kernel_postupgrade_check() {
}
     
} // namespace kernel
} // namespace upgrade
