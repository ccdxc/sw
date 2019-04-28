export ASIC_TOOLS_DIR=/home/asic/tools
mkdir -p $ASIC_TOOLS_DIR
mount -o bind /asictools $ASIC_TOOLS_DIR

cd /sw && make pull-assets && cd nic
/home/asic/tools/eda/bullseye/bin/cov01 -1
export PATH=$PATH:/home/asic/tools/eda/bullseye/bin
tools/run-coverage --run-time 120 -j apollo/.job.yml -c apollo_coverage.json -i --skip-asm-ins-stats
run_status=$?
sudo chmod 777 coverage/ -R
if [ $run_status -ne 0 ];then
    echo "Coverage run failed...."
    exit run_status
fi
exit $?
