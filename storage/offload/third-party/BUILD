# Indirection for all external dependencies.
package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "spdk",
#    srcs = glob(["spdk/export/x86_64/lib/*.lo"]),
    srcs = [
        "spdk/export/x86_64/lib/libspdk_event.lo",
        "spdk/export/x86_64/lib/libspdk_conf.lo",
        "spdk/export/x86_64/lib/libspdk_env_dpdk.lo",
        "spdk/export/x86_64/lib/libspdk_trace.lo",
        "spdk/export/x86_64/lib/libspdk_rpc.lo",
        "spdk/export/x86_64/lib/libspdk_jsonrpc.lo",
        "spdk/export/x86_64/lib/libspdk_util.lo",
        "spdk/export/x86_64/lib/libspdk_json.lo",
        "spdk/export/x86_64/lib/libspdk_log.lo",
        "spdk/export/x86_64/lib/librte_bus_pci.lo",
        "spdk/export/x86_64/lib/librte_eal.lo",
        "spdk/export/x86_64/lib/librte_mempool.lo",
        "spdk/export/x86_64/lib/librte_mempool_ring.lo",
        "spdk/export/x86_64/lib/librte_pci.lo",
        "spdk/export/x86_64/lib/librte_ring.lo",
    ],
    hdrs = glob(["spdk/export/include/**/*.h"]),
    includes = [
        "spdk/export/include",
    ],
    alwayslink = 1,
)

cc_library(
    name = "yaml",
    srcs = [
        "libyaml/export/x86_64/lib/libyaml.so",
    ],
    hdrs = [ "libyaml/export/include/yaml.h" ],
    includes = [
        "libyaml/export/include",
    ],
    alwayslink = 1,
)

cc_library(
    name = "sim_crypto",
    srcs = glob(["freebsd-crypto/src/*.c"]),
    hdrs = glob(["freebsd-crypto/src/*.h"]),
    deps = [ "//storage/offload:osal" ],
    alwayslink = 1,
)
