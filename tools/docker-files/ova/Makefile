default venice-ova:
	$(MAKE) clean-venice-ova
	ln ../../../bin/pen-install.iso .
	docker run --rm -v $(PWD):/t -w /t registry.test.pensando.io:5000/centos:7.3.1611 /t/entrypoint-default.sh
venice-ova-old:
	rm -fr pen-install.iso output-venice-livecd
	ln ../../../bin/pen-install.iso .
	docker run --rm -v $(PWD):/t -w /t registry.test.pensando.io:5000/centos:7.3.1611 /t/entrypoint.sh
clean-venice-ova:
	rm -fr pen-install.iso output output-venice-livecd *.qcow2 *.ova *.save *.ovf *.nvram *.vmdk
