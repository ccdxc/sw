default:
	rm -fr pen-install.iso
	ln ../../../bin/pen-install.iso .
	docker run --rm -v $(PWD):/t -w /t centos:7.3.1611 /t/entrypoint.sh
