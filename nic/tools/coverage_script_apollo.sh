export ASIC_TOOLS_DIR=/home/asic/tools
mkdir -p $ASIC_TOOLS_DIR
mount -o bind /asictools $ASIC_TOOLS_DIR

cd /usr/src/github.com/pensando/sw && make pull-assets && cd nic
/home/asic/tools/eda/bullseye/bin/cov01 -1
export PATH=$PATH:/home/asic/tools/eda/bullseye/bin
tools/run-coverage --run-time 120 -s artemis/aarch64_build  -s artemis/valgrind -s apollo/aarch64_container -s apollo/x86_64_container -j apollo/.job.yml -c apollo_coverage.json -i --skip-asm-ins-stats
run_status=$?
sudo chmod 777 coverage/ -R
if [ $run_status -ne 0 ];then
    echo "Coverage run failed...."
    exit run_status
fi
exit $?
