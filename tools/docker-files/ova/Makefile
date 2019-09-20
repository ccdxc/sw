default:
	rm -fr pen-install.iso output-venice-livecd
	ln ../../../bin/pen-install.iso .
	docker run --rm -v $(PWD):/t -w /t registry.test.pensando.io:5000/centos:7.3.1611 /t/entrypoint.sh
