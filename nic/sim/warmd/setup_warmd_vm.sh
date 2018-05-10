#! /bin/bash
set -e

export NAPLES_DIR="/home/vm/naples"

rm -rf $NAPLES_DIR

mkdir -p ${NAPLES_DIR}
mkdir -p ${NAPLES_DIR}/logs/
cd ${NAPLES_DIR}
printf "Extracting NAPLES image"
cp /home/vm/naples-release-v1.tgz ${NAPLES_DIR}
tar xvzf naples-release-v1.tgz 2>&1 > logs/extract.log
printf "......... SUCCESS\n"

cp /home/vm/*.sh ${NAPLES_DIR}

printf "Stopping Existing NAPLES container (if any)"
${NAPLES_DIR}/stop-naples-docker.sh
printf "......... SUCCESS\n"

printf "Starting NAPLES container"
${NAPLES_DIR}/start-naples-docker.sh
printf "......... SUCCESS\n"

printf "Starting VM Bootstrap"
${NAPLES_DIR}/warmd_bootstrap.sh
printf "......... SUCCESS\n"
