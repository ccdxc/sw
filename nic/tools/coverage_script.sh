export ASIC_TOOLS_DIR=/home/asic/tools
mkdir -p $ASIC_TOOLS_DIR
mount -o bind /asictools $ASIC_TOOLS_DIR

make pull-assets
#Skipping asm instruction stats as GFT will not generate instruction data.
#Ignore job run errors for now.
tools/run-coverage --run-time 120 -s gft/gtest -s gft -s gft_rdma -c hal_coverage.json --skip-asm-ins-stats -i
run_status=$?
if [ $run_status -ne 0 ];then
    echo "Coverage run failed...."
    exit run_status
fi
chmod 777 coverage/ -R
exit $?
