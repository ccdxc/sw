extract-artifacts: 
	cd ../../../ && tar -zxf /sw/naples_fw_all.tgz -C .
	sync && ls -lrt ../../../nic
	cd ../../../nic && tar -zxf test-utils.tgz  -C .
	cd ../../ && cp ../iota.tgz . && tar -zxf iota.tgz -C .

esx: extract-artifacts
	cd /sw/iota && cat /warmd.json && ./iota.py --testsuite hostpin_vm --testbed /warmd.json
