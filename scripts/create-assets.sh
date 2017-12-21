#!/bin/bash

cat <<EOF
This will upload assets to the server from
your working copy at the current versions.

Please do not run this script unless you know what you're doing.

If you really wanted to run it, pass UPLOAD=1 to this command's environment.
EOF

if [ "x$UPLOAD" = "x" ]
then
  echo "Not doing it"
  exit 1
fi

for name in $(find minio -name '*.txt' | xargs basename -s .txt)
do
  version=$(grep "${name}" minio/VERSIONS | awk '{ print $2 }')
  if ! tar cvz $(cat minio/${name}.txt) | asset-upload ${name} ${version} /dev/stdin
  then
    echo +++ ${name}/${version} already uploaded or errored. Attempting to continue...
  fi
done
