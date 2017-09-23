#!/bin/bash

for name in $(find minio -name '*.txt' | xargs basename -s .txt)
do
  version=$(grep "${name}" minio/VERSIONS | awk '{ print $2 }')
  tar cvz $(cat minio/${name}.txt) | asset-upload ${name} ${version} /dev/stdin
done
