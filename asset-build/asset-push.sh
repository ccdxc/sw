#!/bin/bash

if [ -z $RELEASE ]
then
  echo "RELEASE is not set, return"
else
  echo "Get Naples firmware..."
  tar xvzf naples_fw_all.tgz
  mv nic/naples_fw*.tar /go/src/github.com/pensando/sw/nic
  rm -fr nic platform tools storage
  cd /go/src/github.com/pensando/sw

  echo "Get Venice Base ISO..."
  for fname in $(find minio -name 'venice_base_iso.txt')
  do
    name=$(basename $fname .txt)
    entry=$(grep "${name}" minio/VERSIONS-VENICE | awk '{ print $1 }')
    if [ "$entry" != "" ]; then
      version=$(grep "${name}" minio/VERSIONS-VENICE | awk '{ print $2 }')
      echo asset-pull ${name} ${version}
      asset-pull ${name} ${version} /dev/stdout | tar xz
    fi
  done

  echo "Make Venice ISO..."
  cp /sw/bin/venice.tgz bin/
  make venice-iso

  echo "Make bundle image..."
  make bundle-image

  echo "Copy artifacts..."
  cp /sw/build_iris_sim.tar.gz /sw/output
  mkdir -p /sw/output/bundle
  mv /go/src/github.com/pensando/sw/bin/bundle/bundle.tar /sw/output/bundle/
  mv /go/src/github.com/pensando/sw/bin/pen-install.iso /sw/output/
  mkdir -p /sw/output/pxe/tftpboot/pxelinux.cfg
  mv /go/src/github.com/pensando/sw/bin/pxe/tftpboot/initrd0.img /sw/output/pxe/tftpboot/
  mv /go/src/github.com/pensando/sw/bin/pxe/tftpboot/vmlinuz0 /sw/output/pxe/tftpboot/
  mv /go/src/github.com/pensando/sw/bin/pxe/tftpboot/pxelinux.0 /sw/output/pxe/tftpboot/
  mv /go/src/github.com/pensando/sw/bin/pxe/tftpboot/pxelinux.cfg/default /sw/output/pxe/tftpboot/pxelinux.cfg/
  cd /sw/output
  find . -type f -print0 | while IFS= read -r -d $'\0' file;
    do asset-push builds hourly $RELEASE "$file" ;
  done
fi
