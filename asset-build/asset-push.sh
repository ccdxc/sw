#!/bin/bash

if [ -z $RELEASE ]
then
  echo "RELEASE is not set, return"
  exit 1
fi

echo "Get Naples firmware..."
tar -zxf naples_fw_all.tgz
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

echo "Make Venice ISO..."
cp /sw/bin/venice.tgz bin/
if [ $? -ne 0 ]; then
  exit 1
fi

make venice-iso
if [ $? -ne 0 ]; then
  exit 1
fi

echo "Make bundle image..."
make bundle-image
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

echo "Make ova image..."
make ova
if [ $? -ne 0 ]; then
  exit 1
fi

echo "Copy artifacts..."
mkdir -p /sw/output/bundle
cp /sw/build_iris_sim.tar.gz /sw/output
mv bin/venice.tgz /sw/output
mv tools/docker-files/ova/output/venice.ova /sw/output
mv tools/docker-files/ova/output/venice.qcow2 /sw/output
mv bin/bundle/bundle.tar /sw/output/bundle/
mv bin/pen-install.iso /sw/output/
mkdir -p /sw/output/pxe/tftpboot/pxelinux.cfg
mv bin/pxe/tftpboot/initrd0.img /sw/output/pxe/tftpboot/
mv bin/pxe/tftpboot/vmlinuz0 /sw/output/pxe/tftpboot/
mv bin/pxe/tftpboot/pxelinux.0 /sw/output/pxe/tftpboot/
mv bin/pxe/tftpboot/pxelinux.cfg/default /sw/output/pxe/tftpboot/pxelinux.cfg/
cd /sw/output
find . -type f -print0 | while IFS= read -r -d $'\0' file;
  do asset-push builds hourly $RELEASE "$file" ;
  if [ $? -ne 0 ]; then
    exit 1
  fi
done
