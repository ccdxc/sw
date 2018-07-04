from "ubuntu:16.04"

REGISTRY = "registry.test.pensando.io:5000"

LINUX_SRC_DIR = "/usr/src/linux"
USER_SRC_DIR = "/usr/src/"

run "apt -y update"
run "apt -y upgrade"
run "apt -y install build-essential git libssl-dev bc"

#Clone the Linux kernel source code from kernel.org at tag v4.14
inside USER_SRC_DIR do
    run "git clone -b 'v4.14' --single-branch --depth 1 https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git"
end

#Create a branch linux_4.14.0
inside LINUX_SRC_DIR do
    run "git checkout -b linux_4.14.0"

    #Copy the .config which has the kernel configuration
    #Prerequisites is to have this qemu_kernel.config available at /mnt/qemu-linux-config/ location
    copy "qemu_kernel.config", "#{LINUX_SRC_DIR}/.config"

    #Compile the vmlinux image
    run "echo \"Compiling the Linux kernel ...\""
    run "make -j$(grep -c processor /proc/cpuinfo)"
end

tag "#{REGISTRY}/pensando/linux-src:4.14.0"

