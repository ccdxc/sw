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


linkmgr-copy-fw: extract-naples-fw
	sshpass -p lab123 scp -o StrictHostKeyChecking=no /sw/nic/naples_fw_.tar diag@192.168.69.245:/home/diag/$${TARGET_ID}.tar

linkmgr-100: linkmgr-copy-fw
	sshpass -p lab123 ssh -o StrictHostKeyChecking=no diag@192.168.69.245 '/home/diag/linkMGR/link_stress_run.py -s /home/diag/linkMGR/setup_6_100G_only.json -t 103 -v /home/diag/${TARGET_ID}.tar'

linkmgr-40: linkmgr-copy-fw
	sshpass -p lab123 ssh -o StrictHostKeyChecking=no diag@192.168.69.245 '/home/diag/linkMGR/link_stress_run.py -s /home/diag/linkMGR/setup_3_40G_only.json -t 103 -v /home/diag/${TARGET_ID}.tar'

linkmgr-25: linkmgr-copy-fw
	sshpass -p lab123 ssh -o StrictHostKeyChecking=no diag@192.168.69.245 '/home/diag/linkMGR/link_stress_run.py -s /home/diag/linkMGR/setup_4_10-25G_only.json -t 103 -v /home/diag/${TARGET_ID}.tar'

linkmgr-100-vomero: linkmgr-copy-fw
	sshpass -p lab123 ssh -o StrictHostKeyChecking=no diag@192.168.69.245 '/home/diag/linkMGR/link_stress_run.py -s /home/diag/linkMGR/setup_4_100g_vomero.json -t 103 -v /home/diag/${TARGET_ID}.tar'

