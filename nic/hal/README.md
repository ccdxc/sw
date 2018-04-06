HAL heap memory usage analysis instructions:

0) create a workspace on build servers (srv6 for example)

1) build "hal_perf" binary doing "make PROFILING=1 -j8 nic" (under sw/nic)

2) HEAPPROFILE=halperf.hprof PROFILING=1 ./tools/tools/start-hal-mock.sh (from nic/hal directory)
   (for more options, see https://gperftools.github.io/gperftools/heapprofile.html)
   Once HAL is up and running, you should see files prefixed with halperf.hprof like
   halperf.hprof.0001.heap, halperf.hprof2.0002.heap and so on

3) For each file generated in step 2, do the following
   /tool/toolchain/gperftools-2.6.3/external/bin/pprof --svg ../bazel-bin/nic/hal/hal_perf halperf.hprof.0001.heap  > /tmp/mem_out1.svg
   Each svg file has information about the memory allocation

   NOTE:
   once you kill hal, a final .heap file will be generated with all consolidated
   information, which is helpful as it gives full overview
