core_count=$(grep -c '^processor' /proc/cpuinfo)
if [ $core_count -lt 6 ];then
    echo "*********CORE COUNT IS LESS THAN 6*********"
    cat "/proc/cpuinfo"
    exit 1
fi

echo "ARCH=${ARCH}, PIPELINE=${PIPELINE}, ASIC=${ASIC}"
cd /sw/nic/build/${ARCH}/${PIPELINE}
for d in bin lib out pgm_bin gen gtest_results
do
    if [ -e $d ]; then
        echo "link $d [`readlink $d`] already exist"
    else 
        echo "ln -s ${ASIC}/$d $d"
        ln -s ${ASIC}/$d $d
    fi
done
exit 0
