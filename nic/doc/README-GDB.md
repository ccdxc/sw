Steps to run GDB in nic dev container:
1) cd /sw/nic
2) /tool/toolchain/aarch64-1.1/bin/aarch64-linux-gnu-gdb build/aarch64/iris/bin/hal <core.hal>
3) set sysroot /tool/toolchain/aarch64-1.1/aarch64-linux-gnu/libc
4) set solib-search-path buildroot/output/target/lib/:build/aarch64/iris/lib/:/tool/toolchain/aarch64-1.1/aarch64-linux-gnu/libc/usr/lib:/tool/toolchain/aarch64-1.1/aarch64-linux-gnu/libc/lib

# For gdb on Naples
# Copy libs to /data/libs since there is not enough space in /nic. Start gdb with hal and core file
# Run below gdb cmds to set the path
set solib-search-path /data/libs/:/nic/lib/:/lib64:/usr/lib64:/lib:.
set solib-absolute-prefix /data/libs:target:
