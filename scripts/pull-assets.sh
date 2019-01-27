#!/bin/bash

set -e

check_model_sim()
{
  if [[ -d nic/model_sim/libs && -n "$(ls -A nic/model_sim/libs)" ]] ; then
    mkdir -p nic/sdk/model_sim/libs
    mv nic/model_sim/libs/* nic/sdk/model_sim/libs
    rm -rf nic/model_sim
  fi
}

if cmp minio/VERSIONS minio/.VERSIONS.orig
then
    echo "minio/VERSIONS same as current. Skipping pull-assets."
    check_model_sim
    exit 0
fi

echo "pulling assets"

for fname in $(find minio -name '*.txt') 
do
  name=$(basename $fname .txt)
  version=$(grep "${name}" minio/VERSIONS | awk '{ print $2 }')
  asset-pull ${name} ${version} /dev/stdout | tar xvz
done
check_model_sim
cp minio/VERSIONS minio/.VERSIONS.orig
