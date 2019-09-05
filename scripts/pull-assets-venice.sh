#!/bin/bash

set -e

if cmp minio/VERSIONS-VENICE minio/.VERSIONS-VENICE.orig
then
    echo "minio/VERSIONS-VENICE same as current. Skipping pull-assets."
    exit 0
fi

echo "pulling assets"
for fname in $(find minio -name '*.txt') 
do
  name=$(basename $fname .txt)
  entry=$(grep "${name}" minio/VERSIONS-VENICE | awk '{ print $1 }')
  if [ "$entry" != "" ]; then
    version=$(grep "${name}" minio/VERSIONS-VENICE | awk '{ print $2 }')
    echo asset-pull ${name} ${version}
    asset-pull ${name} ${version} /dev/stdout | tar xz
  fi
done
cp minio/VERSIONS-VENICE minio/.VERSIONS-VENICE.orig
