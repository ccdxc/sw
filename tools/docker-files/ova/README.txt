Target: venice-ova-old

This is the old way of building the Venice OVA using packer tool and template venice-livecd.json. This build target hard coded in the json to use the ESXi ae-s6 to build the OVA. 

Target: venice-ova

This is the new way (and default) of building the Venice OVA using buildit tool. Once the OVA is built, it will insert the OVA properties and convert to a new OVA. At the same time it will convert the vmdk in the OVA to a qcow2 for KVM. The output artifact of the build will be kept in the output/ directory.

Executatable required:

packer executable obtained from https://releases.hashicorp.com/packer/1.4.3/packer_1.4.3_linux_amd64.zip
VMware-ovftool-4.3.0-13981069-lin.x86_64.bundle obtained from Kangwarn from vmware website
qemu-img (can do by yum -y install qemu-img)
buildit tool
