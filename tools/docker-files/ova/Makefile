IMAGE=registry.test.pensando.io:5000/pensando/ova_build:1.0

default venice-ova:
	$(MAKE) clean-venice-ova
	ln ../../../bin/pen-install.iso .
	docker run --rm -v $(CURDIR):/t -w /t $(IMAGE) /t/entrypoint-default.sh
venice-apulu-ova:
	$(MAKE) clean-venice-apulu-ova
	ln ../../../bin/pen-install.apulu.iso pen-install.iso
	docker run --rm -v $(CURDIR):/t -w /t $(IMAGE) /t/entrypoint-default.sh apulu
venice-ova-old:
	rm -fr pen-install.iso output-venice-livecd
	ln ../../../bin/pen-install.iso .
	docker run --rm -v $(CURDIR):/t -w /t $(IMAGE) /t/entrypoint.sh
clean-venice-ova:
	rm -fr pen-install.iso output output-venice-livecd *.qcow2 *.ova *.save *.ovf *.nvram *.vmdk
clean-venice-apulu-ova:
	rm -fr pen-install.iso output-apulu output-venice-livecd *.qcow2 *.ova *.save *.ovf *.nvram *.vmdk
install_box:
	@if [ ! -x /usr/local/bin/box ]; then echo "Installing box, sudo is required"; curl -sSL box-builder.sh | sudo bash; fi
deps: install_box
	box -t $(IMAGE) box-deps.rb

