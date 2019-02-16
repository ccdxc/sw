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
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --management-mode host
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --management-mode network
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --management-mode network --network-mode
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --management-mode network --network-mode host
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --management-mode network --network-mode network
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --management-mode network --network-mode snowflake
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --management-mode network --network-mode host --controllers
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --management-mode network --network-mode host --controllers 2.2.2.2
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --management-mode network --network-mode host --controllers 2.2.2.2/24
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --management-mode network --network-mode network --controllers 2.2.2.2
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --management-mode network --network-mode network --controllers 2.2.2.2/24
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --management-mode network --network-mode inband --controllers 2.2.2.2/24
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --management-mode network --network-mode oob --controllers 2.2.2.2/24
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --management-mode network --network-mode inband
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --management-mode network --network-mode oob
date && time ./penctl.linux update naples --hostname testme --controllers 1.1.1.1/24 --management-mode network --network-mode oob
date && time ./penctl.linux update naples --hostname testme --controllers 1.1.1.1 --management-mode network --network-mode oob
date && time ./penctl.linux update naples --hostname testme --controllers 1.1.1.1/24 --management-mode network --network-mode inband
date && time ./penctl.linux update naples --hostname testme --controllers 1.1.1.1 --management-mode network --network-mode inband

echo "PENCTL GOOD CASES"
echo "STATIC CONFIGURATION"
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --management-mode network --network-mode inband --controllers 2.2.2.2
date && time ./penctl.linux show naples --json
date && time ./penctl.linux update naples --hostname testme --mgmt-ip 1.1.1.1/24 --management-mode network --network-mode oob --controllers 2.2.2.2
date && time ./penctl.linux show naples --json

echo "NO CONFIGURED DHCP SERVER"
date && time ./penctl.linux update naples --hostname testme --management-mode network --network-mode oob
date && time ./penctl.linux show naples --json

echo "CONFIGURED DHCP SERVER"
date && time ./penctl.linux update naples --hostname testme --management-mode network --network-mode inband
date && time ./penctl.linux show naples --json


echo "STARTING LOOP TESTS"
while true; do
	echo "======== MOVE TO OOB DHCP ==========="
	start_d=`date +%s`
	./penctl.linux update naples --hostname testme --management-mode network --network-mode oob
	end_d=`date +%s`
	d=$(($end_d-$start_d))
	date && time ./penctl.linux show naples --json
	echo "TIME TAKEN IS : $d"
	echo

        echo "======== MOVE TO HOST MANAGED ==========="
        start_d=`date +%s`
        ./penctl.linux update naples --hostname testme --management-mode host
        end_d=`date +%s`
        d=$(($end_d-$start_d))
	date && time ./penctl.linux show naples --json
        echo "TIME TAKEN IS : $d"
	echo

        echo "======== MOVE TO INBAND DHCP ==========="
        start_d=`date +%s`
        ./penctl.linux update naples --hostname testme --management-mode network --network-mode inband
        end_d=`date +%s`
        d=$(($end_d-$start_d))
	date && time ./penctl.linux show naples --json
        echo "TIME TAKEN IS : $d"
	echo

        echo "======== MOVE TO HOST MANAGED ==========="
        start_d=`date +%s`
        ./penctl.linux update naples --hostname testme --management-mode host
        end_d=`date +%s`
        d=$(($end_d-$start_d))
	date && time ./penctl.linux show naples --json
        echo "TIME TAKEN IS : $d"
	echo

        echo "======== MOVE TO INBAND STATIC ==========="
        start_d=`date +%s`
        ./penctl.linux update naples --hostname testme --management-mode network --network-mode inband --controllers 4.4.4.4 --mgmt-ip 1.1.1.1/24
        end_d=`date +%s`
        d=$(($end_d-$start_d))
	date && time ./penctl.linux show naples --json
        echo "TIME TAKEN IS : $d"
	echo

        echo "======== MOVE TO HOST MANAGED ==========="
        start_d=`date +%s`
        ./penctl.linux update naples --hostname testme --management-mode host
        end_d=`date +%s`
        d=$(($end_d-$start_d))
	date && time ./penctl.linux show naples --json
        echo "TIME TAKEN IS : $d"
done

exit
