export ASIC_TOOLS_DIR=/home/asic/tools
mkdir -p $ASIC_TOOLS_DIR
mount -o bind /asictools $ASIC_TOOLS_DIR

make pull-assets
tools/run-coverage --run-time 120 --modellogs -s gft/gtest -c hal_coverage.json
run_status=$?
if [ $run_status -ne 0];then
    echo "Coverage run failed...."
    exit run_status
fi
chmod 777 coverage/ -R
exit $?
