
## Makefile Infra

We are using the GNU Makefile infra to build the files. The dependencies will be taken care of in infra for most of the cases.
For cases where the user has to take care are specified in the subsequent sections.

There are multiple targets which the Makefile infra supports.

1. Program
2. Static Library (.a)
3. Shared/Dynamic Library (.so)
4. Sub dirs
5. Build Objects
6. Custom

The top-most Makefile will have the directories where the "Make" has to go to do Makes. 

Currently we dont support more than one target per directory. There is a workaround which we will be explained in the subsequent
section.

### 1. Program

Makefile will build a binary from given SOURCES (.cc). Currently this is being used in the Makefile for GTESTs.

Example: utils/indexer/test/Makefile

### 2. Library (.a)

This is to build .a libraries which we are not using in our repo.

### 3. Shared/Dynamic Library (.so)

Makefile will build Shared libraries from given SOURCES (.cc). Currently this is being used to form PI utils and PD utils libraries.

Example: nic/utils/indexer/Makefile

### 4. Sub dirs

This is used to prevent the upstream directory to not specify every directory in its Makefile. 
Currently this is being used in PI utils directory to build all the utils.

Example: nic/utils/Makefile 

### 5. Build Objects

Use this target if the Makefile is building only Objects and not a library or binary. This is being used in PD utils specific directories.

Example: nic/hal/pd/utils/directmap/Makefile

### 6. Custom

Use this is you dont want to migrate to new Makefile Infra and continue using the existing Makefile.

Example: nic/hal/Makefile


### 7. Handle Multiple Targets in a Makefile

Provide one target to Makefile Infra and add other targets as LPOSTALL or LPREALL depending on the order you want the targets.
This is being used today in pd utils to build the libpdutils.so and libpdutils_stub.so.

Example: nic/hal/pd/utils/Makefile

### 8. x86_64 and aarch64 builds

Use the below commands to build both x86 and arm binaries in same workspace

# x86_64 build
TEST_TMPDIR=<path_to_bazel_x86_64_binaries> make nic

# aarch64 build for haps
TEST_TMPDIR=<path_to_bazel_aarch64_binaries> make PLATFORM=haps ARCH=aarch64 nic

# aarch64 build for HW
TEST_TMPDIR=<path_to_bazel_aarch64_binaries> make ARCH=aarch64 nic

