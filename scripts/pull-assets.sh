#!/bin/bash

set -e

echo "pulling assets"

for name in $(find minio -name '*.txt' | xargs basename -s .txt)
do
  version=$(grep "${name}" minio/VERSIONS | awk '{ print $2 }')
  asset-pull ${name} ${version} /dev/stdout | tar xvz
done
