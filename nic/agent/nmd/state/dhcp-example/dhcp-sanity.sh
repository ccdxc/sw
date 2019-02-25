#!/bin/bash
export NAPLES_URL="http://169.254.0.1"

echo "PENCTL CLI CASES"
date && time ./penctl.linux update
date && time ./penctl.linux mode
date && time ./penctl.linux update naples
date && time ./penctl.linux mode update
date && time ./penctl.linux update naples --hostname
date && time ./penctl.linux update naples --hostname testme
date && time ./penctl.linux update naples --hostname testme --mgmt-ip
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --managed-by host
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --managed-by network
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --managed-by network --management-network
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --managed-by network --management-network host
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --managed-by network --management-network network
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --managed-by network --management-network snowflake
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --managed-by network --management-network host --controllers
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --managed-by network --management-network host --controllers 2.2.2.2
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --managed-by network --management-network host --controllers 2.2.2.2/24
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --managed-by network --management-network network --controllers 2.2.2.2
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --managed-by network --management-network network --controllers 2.2.2.2/24
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --managed-by network --management-network inband --controllers 2.2.2.2/24
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --managed-by network --management-network oob --controllers 2.2.2.2/24
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --managed-by network --management-network inband
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --managed-by network --management-network oob
date && time ./penctl.linux update naples --hostname testme --controllers 1.1.1.1/24 --managed-by network --management-network oob
date && time ./penctl.linux update naples --hostname testme --controllers 1.1.1.1 --managed-by network --management-network oob
date && time ./penctl.linux update naples --hostname testme --controllers 1.1.1.1/24 --managed-by network --management-network inband
date && time ./penctl.linux update naples --hostname testme --controllers 1.1.1.1 --managed-by network --management-network inband

echo "PENCTL GOOD CASES"
echo "STATIC CONFIGURATION"
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --managed-by network --management-network inband --controllers 2.2.2.2
date && time ./penctl.linux show naples --json
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --managed-by network --management-network oob --controllers 2.2.2.2
date && time ./penctl.linux show naples --json

echo "NO CONFIGURED DHCP SERVER"
date && time ./penctl.linux update naples --hostname testme --managed-by network --management-network oob
date && time ./penctl.linux show naples --json

echo "CONFIGURED DHCP SERVER"
date && time ./penctl.linux update naples --hostname testme --managed-by network --management-network inband
date && time ./penctl.linux show naples --json


echo "STARTING LOOP TESTS"
while true; do
	echo "======== MOVE TO OOB DHCP ==========="
	start_d=`date +%s`
	./penctl.linux update naples --hostname testme --managed-by network --management-network oob
	end_d=`date +%s`
	d=$(($end_d-$start_d))
	date && time ./penctl.linux show naples --json
	echo "TIME TAKEN IS : $d"
	echo

        echo "======== MOVE TO HOST MANAGED ==========="
        start_d=`date +%s`
        ./penctl.linux update naples --hostname testme --managed-by host
        end_d=`date +%s`
        d=$(($end_d-$start_d))
	date && time ./penctl.linux show naples --json
        echo "TIME TAKEN IS : $d"
	echo

        echo "======== MOVE TO INBAND DHCP ==========="
        start_d=`date +%s`
        ./penctl.linux update naples --hostname testme --managed-by network --management-network inband
        end_d=`date +%s`
        d=$(($end_d-$start_d))
	date && time ./penctl.linux show naples --json
        echo "TIME TAKEN IS : $d"
	echo

        echo "======== MOVE TO HOST MANAGED ==========="
        start_d=`date +%s`
        ./penctl.linux update naples --hostname testme --managed-by host
        end_d=`date +%s`
        d=$(($end_d-$start_d))
	date && time ./penctl.linux show naples --json
        echo "TIME TAKEN IS : $d"
	echo

        echo "======== MOVE TO INBAND STATIC ==========="
        start_d=`date +%s`
        ./penctl.linux update naples --hostname testme --managed-by network --management-network inband --controllers 4.4.4.4 --mgmt-ip 1.1.1.1/24
        end_d=`date +%s`
        d=$(($end_d-$start_d))
	date && time ./penctl.linux show naples --json
        echo "TIME TAKEN IS : $d"
	echo

        echo "======== MOVE TO HOST MANAGED ==========="
        start_d=`date +%s`
        ./penctl.linux update naples --hostname testme --managed-by host
        end_d=`date +%s`
        d=$(($end_d-$start_d))
	date && time ./penctl.linux show naples --json
        echo "TIME TAKEN IS : $d"
done

exit
