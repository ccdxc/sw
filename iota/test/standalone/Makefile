NAPLES_FW_TAR=naples_fw_all.tgz

extract-naples-fw: 
	cd ../../../ && tar -zxf /sw/$(NAPLES_FW_TAR) -C .
	sync && ls -lrt ../../../nic
	cd ../../../nic && tar -zxf test-utils.tgz  -C .

extract-iota: 
	cd ../../ && cp ../iota.tgz . && tar -zxf iota.tgz -C .

extract-protos-apulu:
	cd ../../../ && tar -zxf /sw/naples-protos-apulu.tgz -C .

extract-artifacts: extract-naples-fw extract-iota

esx: extract-artifacts

esx-regression: extract-artifacts

apulu-linux: NAPLES_FW_TAR=naples_fw_all_apulu.tgz
apulu-linux: extract-artifacts

apulu-networking: apulu-linux extract-protos-apulu

linux: extract-artifacts

freebsd: extract-artifacts
	
regression-rdma: extract-artifacts

regression-rdma-stress: extract-artifacts

regression-storage: extract-artifacts
 
regression-eth: extract-artifacts

regression-infra: extract-artifacts

compatibility: extract-artifacts

linkmgr-25Gb: extract-naples-fw
	ls -lR /sw
	sshpass -h
