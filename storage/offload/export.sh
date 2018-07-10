cd /sw/storage/offload
rm offload_sim.zip
zip -P katana offload_sim README Makefile.bsd include/BUILD include/*.h third-party/freebsd-crypto/BUILD third-party/freebsd-crypto/src/*.h third-party/freebsd-crypto/src/*.c src/osal/BUILD src/osal/*.h src/osal/linux/kernel/*.c src/osal/linux/kernel/sim/*.c src/osal/linux/uspace/*.c src/osal/linux/uspace/sim/*.c src/osal/test/osal_test.c src/sim/BUILD src/sim/pnso_sim_api.c src/sim/sim*.h src/sim/sim*.c src/sim/lzrw.h src/sim/lzrw1-a.c
