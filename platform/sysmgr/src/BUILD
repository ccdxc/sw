package(default_visibility = ["//visibility:public"])

# Compile the sysmgr binary
cc_binary(
    name = "sysmgr",
    srcs = glob(["runsv.c", "strerr.h", "taia.h", "tai.h", "uint64.h", "sig.h", 
        "env.h", "coe.h", "ndelay.h", "fifo.h", "open.h", "lock.h", "iopause.h",
        "fd.h", "buffer.h", "fmt.h", "byte.h", "wait.h", "error.h",
        "haswaitp.h", "hassgprm.h", "select.h", "hasmkffo.h", "hasflock.h",
        "hassgact.h", "str.h",
        "taia_uint.c", "taia_less.c", "taia_add.c", "error.c", "wait_nohang.c",
        "sig_block.c", "iopause.c", "taia_now.c", "fifo.c", "open_read.c",
        "open_write.c", "lock_ex.c", "open_append.c", "lock_exnb.c", "coe.c",
        "sig_catch.c", "sig.c", "ndelay_on.c", "fd_copy.c", "fmt_ulong.c",
        "wait_pid.c", "byte_copy.c", "open_trunc.c", "taia_pack.c",
        "tai_pack.c", "buffer_put.c", "buffer.c", "buffer_write.c",
        "strerr_die.c", "buffer_2.c", "str_len.c", "taia_sub.c",
        "taia_approx.c", "taia_frac.c", "strerr_sys.c", "error_str.c"]),
    linkopts = [
        "-lpthread",
    ],
)
