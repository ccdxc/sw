core_count=$(grep -c '^processor' /proc/cpuinfo)
if [ $core_count -lt 6 ];then
    echo "*********CORE COUNT IS LESS THAN 6*********"
    cat "/proc/cpuinfo"
    exit 1
fi
exit 0
