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

#ifndef _KERNEL_UPGRADE_HPP_
#define _KERNEL_UPGRADE_HPP_

namespace upgrade {
namespace kernel {
    const string KEXEC_EXE = "/sbin/kexec";
    const string KEXEC_CMD_LINE = "";

    typedef enum {
        K_FAIL = 0,
        K_PASS = 1,
    } kernel_upgrade_res ;

    kernel_upgrade_res kernel_preupgrade_check();

    kernel_upgrade_res kernel_load_new(string new_kernel,
                                       string new_initrd);

    kernel_upgrade_res kernel_unload_new(string new_kernel,
                                         string new_initrd);
    kernel_upgrade_res kernel_execute();

    kernel_upgrade_res kernel_execute_debug();

    kernel_upgrade_res kernel_upgrade(string new_kernel,
                                      string new_initrd);

    kernel_upgrade_res kernel_upgrade_debug(string new_kernel,
                                            string new_initrd);

    kernel_upgrade_res kernel_postupgrade_check();
} // namespace kernel
} // namespace upgrade

#endif
