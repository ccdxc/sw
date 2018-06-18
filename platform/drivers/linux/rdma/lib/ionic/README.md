# IONIC User Space RDMA Driver

## Build and Install

### Rdma-Core and Build Dependencies

Build in an environment matching where libraries will be installed.  I use
sshfs to mount the system where sources are located, and then build and install
on QEMU.  It's slow, but the build should be infrequent, and incremental if
there are changes.

```sh
sudo apt-get install build-essential cmake gcc libudev-dev libnl-3-dev \
	libnl-route-3-dev ninja-build pkg-config valgrind ```

git clone https://github.com/linux-rdma/rdma-core.git
cd rdma-core
```

### Add ionic provider to Rdma-Core

Reference the ionic user space driver sources from the providers dir.

```sh
ln -s /path/to/sw/platform/drivers/linux/rdma/lib/ionic providers/ionic
ln -s /path/to/sw/platform/drivers/linux/rdma/drv/ionic/uapi/rdma/ionic-abi.h \
	kernel-headers/rdma/ionic-abi.h
```

Edit the CMakeLists.txt, adding ionic, and (optional, recommended) removing all
of the other providers (or comment them out).

```diff
diff --git a/CMakeLists.txt b/CMakeLists.txt
index e63f4cf56367..6247d12de235 100644
--- a/CMakeLists.txt
+++ b/CMakeLists.txt
@@ -430,28 +430,7 @@ add_subdirectory(libibverbs/man)
 add_subdirectory(librdmacm)
 add_subdirectory(librdmacm/man)

-# Providers
-if (HAVE_COHERENT_DMA)
-add_subdirectory(providers/bnxt_re)
-add_subdirectory(providers/cxgb3) # NO SPARSE
-add_subdirectory(providers/cxgb4) # NO SPARSE
-add_subdirectory(providers/hns)
-add_subdirectory(providers/i40iw) # NO SPARSE
-add_subdirectory(providers/mlx4)
-add_subdirectory(providers/mlx4/man)
-add_subdirectory(providers/mlx5)
-add_subdirectory(providers/mlx5/man)
-add_subdirectory(providers/mthca)
-add_subdirectory(providers/nes) # NO SPARSE
-add_subdirectory(providers/ocrdma)
-add_subdirectory(providers/qedr)
-add_subdirectory(providers/vmw_pvrdma)
-endif()
-
-add_subdirectory(providers/hfi1verbs)
-add_subdirectory(providers/ipathverbs)
-add_subdirectory(providers/rxe)
-add_subdirectory(providers/rxe/man)
+add_subdirectory(providers/ionic)

 # Binaries
 add_subdirectory(ibacm) # NO SPARSE
```

Also edit CMakeLists.txt in kernel-headers.

```diff
diff --git a/kernel-headers/CMakeLists.txt b/kernel-headers/CMakeLists.txt
index 04b9896..fdcb8e3 100644
--- a/kernel-headers/CMakeLists.txt
+++ b/kernel-headers/CMakeLists.txt
@@ -63,6 +63,7 @@ rdma_kernel_provider_abi(
   rdma/hns-abi.h
   rdma/i40iw-abi.h
   rdma/ib_user_verbs.h
+  rdma/ionic-abi.h
   rdma/mlx4-abi.h
   rdma/mlx5-abi.h
   rdma/mthca-abi.h
```

### Build and Run In-place

Run `build.sh`.  Run programs directly from the build/bin dir.

Do not attempt to install.

### Build and Install

Do not run build.sh if you intend to install after the build.

```sh
mkdir -p build
cd build
cmake -GNinja ..
ninja
ninja install
```
