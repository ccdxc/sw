export ASIC_TOOLS_DIR=/home/asic/tools
mkdir -p $ASIC_TOOLS_DIR
mount -o bind /asictools $ASIC_TOOLS_DIR

cd /sw && make pull-assets && cd nic
#Skipping asm instruction stats as GFT will not generate instruction data.
#Ignore job run errors for now.
tools/run-coverage --run-time 120 -s gft/gtest -s agent -s codesync -s dol/gft/rdma -s dol/gft -s apollo/gtest -s dol/gft/rdma_l2l -s storage -s storage/nvme -s storage/combined -s storage/nvme_perf -s e2e/naples-sim -s codesync -s venice -s basetopo -s perf -s arm/build/haps -s arm/build  -s delphi -s storage/perf -s dol/basetopo -c hal_coverage.json -c hal_hw_coverage.json --skip-asm-ins-stats -i -k
run_status=$?
if [ $run_status -ne 0 ];then
    echo "Coverage run failed...."
    exit run_status
fi
sudo chmod 777 coverage/ -R
exit $?
