#!/bin/bash

if [ -z $RELEASE ]
then
  echo "RELEASE is not set, return"
  exit 0
fi

echo "Get Naples firmware..."
tar -zxf naples_fw_all_apulu.tgz
if [ $? -ne 0 ]; then
  echo "Failed to untar naples firmware"
  exit 1
fi

mv nic/naples_fw*.tar /go/src/github.com/pensando/sw/nic
rm -fr nic platform tools storage

cd /go/src/github.com/pensando/sw

echo "Get Venice Base ISO..."
fname=$(find minio -name 'venice_base_iso.txt')
name=$(basename $fname .txt)
entry=$(grep "${name}" minio/VERSIONS-VENICE | awk '{ print $1 }')
if [ "$entry" != "" ]; then
  version=$(grep "${name}" minio/VERSIONS-VENICE | awk '{ print $2 }')
  echo asset-pull ${name} ${version}
  asset-pull ${name} ${version} /dev/stdout | tar xz
else
  echo "Failed to find base ISO info"
  exit 1
fi

if [ $? -ne 0 ]; then
  echo "Failed to pull venice base ISO"
  exit 1
fi

cp /sw/bin/venice.apulu.tgz bin/
if [ $? -ne 0 ]; then
  exit 1
fi

echo "Make Apulu Venice ISO..."
make venice-apulu-iso
if [ $? -ne 0 ]; then
  exit 1
fi

echo "Make apulu bundle image..."
make bundle-apulu-image
if [ $? -ne 0 ]; then
  exit 1
fi

echo "Get ova tools..."
fname=$(find minio -name 'ova.txt')
name=$(basename $fname .txt)
entry=$(grep "${name}" minio/VERSIONS | awk '{ print $1 }')
if [ "$entry" != "" ]; then
    version=$(grep "${name}" minio/VERSIONS | awk '{ print $2 }')
    echo asset-pull ${name} ${version}
    asset-pull ${name} ${version} /dev/stdout | tar xz
else
  echo "Failed to find ova tools info"
  exit 1
fi

echo "Make Apulu ova image..."
make apulu-ova
if [ $? -ne 0 ]; then
  exit 1
fi

echo "Copy artifacts..."
mkdir -p /sw/output/apulu-bundle
ls -al bin/
mv tools/docker-files/ova/output-apulu/venice.ova /sw/output/psm.apulu.ova
mv tools/docker-files/ova/output-apulu/venice.qcow2 /sw/output/psm.apulu.qcow2
mv apulu-bundle/bundle.tar /sw/output/apulu-bundle/
mv bin/venice.apulu.tgz /sw/output/psm.apulu.tgz
mv bin/pen-install.apulu.iso /sw/output/

mkdir -p /sw/output/pxe/tftpboot/pxelinux.cfg
mv bin/pxe/tftpboot/initrd0.img /sw/output/pxe/tftpboot/
mv bin/pxe/tftpboot/vmlinuz0 /sw/output/pxe/tftpboot/
mv bin/pxe/tftpboot/pxelinux.0 /sw/output/pxe/tftpboot/
mv bin/pxe/tftpboot/pxelinux.cfg/default /sw/output/pxe/tftpboot/pxelinux.cfg/

mkdir -p /sw/output/psm-install
mv bin/venice-install/initrd0.img /sw/output/psm-install
mv bin/venice-install/squashfs.img /sw/output/psm-install
mv tools/docker-files/vinstall/PEN-VERSION /sw/output/psm-install

cd /sw/output
find . -type f -print0 | while IFS= read -r -d $'\0' file;
  do asset-push builds hourly $RELEASE "$file" ;
  if [ $? -ne 0 ]; then
    exit 1
  fi
done
