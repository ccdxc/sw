NAPLES_FW_TAR=naples_fw_all.tgz

extract-artifacts: 
	cd ../../../ && tar -zxf /sw/$(NAPLES_FW_TAR) -C .
	sync && ls -lrt ../../../nic
	cd ../../../nic && tar -zxf test-utils.tgz  -C .
	cd ../../ && cp ../iota.tgz . && tar -zxf iota.tgz -C .

esx: extract-artifacts
	cd /sw/iota && cat /warmd.json && ./iota.py --testsuite hostpin_vm --testbed /warmd.json

esx-regression: extract-artifacts
	cd /sw/iota && cat /warmd.json && ./iota.py --testsuite hostpin_vm --testbed /warmd.json \
	          --regression --skip-sanity

apulu-linux: NAPLES_FW_TAR=naples_fw_all_apulu.tgz
apulu-linux: extract-artifacts
	cd /sw/iota && cat /warmd.json && ./iota.py --pipeline apulu --testsuite apulu --testbed /warmd.json \
               --testbundle mnicclassic

linux: extract-artifacts
	cd /sw/iota && cat /warmd.json && ./iota.py --testsuite classic --testbed /warmd.json \
               --testbundle ethtool,connectivity,filters,subifs,rx_mode,mnicclassic,oob1g,ionicdrv,penctl,mode_change,post_diag_check,asic_tools,obfl_check,cpldupgrade,hpe_ilo

freebsd: extract-artifacts
	cd /sw/iota && cat /warmd.json && ./iota.py --testsuite classic --testbed /warmd.json
	
regression-rdma: extract-artifacts
	cd /sw/iota && cat /warmd.json && ./iota.py --testsuite classic --testbed /warmd.json \
	           --regression --testbundle mtu,rdma_reg

regression-rdma-stress: extract-artifacts
	cd /sw/iota && cat /warmd.json && ./iota.py --testsuite classic --testbed /warmd.json \
	           --regression --testbundle mtu,rdma_stress

regression-storage: extract-artifacts
	cd /sw/iota && cat /warmd.json && ./iota.py --testsuite classic --testbed /warmd.json \
	           --regression --testbundle storage 
 
regression-eth: extract-artifacts
	cd /sw/iota && cat /warmd.json && ./iota.py --testsuite classic --testbed /warmd.json \
	           --regression --testbundle ethtool,connectivity,filters,subifs,rx_mode,mnicclassic,oob1g,ionicdrv,penctl,ionicdrv_regr

regression-infra: extract-artifacts
	cd /sw/iota && cat /warmd.json && ./iota.py --testsuite classic --testbed /warmd.json \
	        	--regression --testbundle connectivity,mode_change,post_diag_check,asic_tools,obfl_check

