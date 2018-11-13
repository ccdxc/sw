#!/bin/bash

set -e

if cmp minio/VERSIONS minio/.VERSIONS.orig
then
    echo "minio/VERSIONS same as current. Skipping pull-assets."
    exit 0
fi

echo "pulling assets"

for fname in $(find minio -name '*.txt') 
do
  name=$(basename $fname .txt)
  version=$(grep "${name}" minio/VERSIONS | awk '{ print $2 }')
  asset-pull ${name} ${version} /dev/stdout | tar xvz
done
cp minio/VERSIONS minio/.VERSIONS.orig
