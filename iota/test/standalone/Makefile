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

esx-precheckin: extract-artifacts

apulu-linux: NAPLES_FW_TAR=naples_fw_all_apulu.tgz
apulu-linux: extract-artifacts

apulu-networking: apulu-linux extract-protos-apulu

linux: extract-artifacts

freebsd: extract-artifacts

windows: extract-artifacts
	
regression-rdma: extract-artifacts

regression-rdma-stress: extract-artifacts

regression-storage: extract-artifacts
 
regression-eth: extract-artifacts

regression-infra: extract-artifacts

compatibility: extract-artifacts

linkmgr-25Gb-apulu: NAPLES_FW_TAR=naples_fw_all_apulu.tgz
linkmgr-25Gb-apulu: 


linkmgr-100: extract-naples-fw 
	./regression/linkmgr/linkregression.sh ${TARGET_ID} 100Gb

linkmgr-40: extract-naples-fw
	./regression/linkmgr/linkregression.sh ${TARGET_ID} 40Gb

linkmgr-25: extract-naples-fw
	./regression/linkmgr/linkregression.sh ${TARGET_ID} 25Gb

linkmgr-100-vomero: extract-naples-fw
	./regression/linkmgr/linkregression.sh ${TARGET_ID} 100Gb_vomero

