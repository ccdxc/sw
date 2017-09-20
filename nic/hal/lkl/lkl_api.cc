#include <string>
#include <unistd.h>
#include <getopt.h>
#include <hal.hpp>

extern "C" {
#include <lkl.h>
#include <lkl_host.h>
int lkl_start_kernel(struct lkl_host_operations *lkl_ops,
                    const char *cmd_line, ...);
}

namespace hal {

int lkl_init(void) {
    int ret;
    HAL_TRACE_DEBUG("Starting LKL\n");
    ret = lkl_start_kernel(&lkl_host_ops, "mem=16M loglevel=8");
    if (ret) {
        HAL_TRACE_DEBUG("LKL could not be started: %s\n", lkl_strerror(ret));
        return HAL_RET_ERR;
    }
    return HAL_RET_OK;
}

}
