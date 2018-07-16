#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo "Invalid number of arguments"
    echo " $0 <protobuf file directory>"
    exit 1
fi
container=$1

while read -r line || [[ -n "$line" ]];
do
  pkg=$(echo $line | awk '{ print $2 }');
  echo "++--- generating docs for package ${pkg}";
  [[ -z "${pkg// }" ]] && continue;
  [[ "${pkg}" = "bookstore"  ]] && continue;
  docker run --user `id -u`:`id -g` --rm -v${PWD}:/import/src/github.com/pensando/sw -v${PWD}/bin/cbin:/import/bin -w /import/src/github.com/pensando/sw ${container} bash -c "spectacle -q -1 /import/src/github.com/pensando/sw/api/generated/${pkg}/swagger/svc_*.json -t /import/src/github.com/pensando/sw/api/docs/generated/${pkg}" 2&> /dev/null
done < ${PWD}/api/generated/pkgmanifest;
cp -r api/docs tools/docker-files/apigw;
