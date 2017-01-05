# Makefile for dev environment
.PHONY: dev dev-ssh dev-clean dev-test

dev:
	scripts/bringup-dev.sh

dev-build-centos72:
	cd packer-templates; packer build -only=virtualbox-iso centos-7.2-x86_64.json
	cd packer-templates; vagrant box add jainvipin/centos72 centos-7.2-x86_64-virtualbox.box
	
dev-ssh:
	cd dev && vagrant ssh node1

dev-clean:
	scripts/cleanup-dev.sh

dev-testsetup:
	scripts/CI-setup.sh

dev-testrun:
	scripts/CI.sh

dev-testcleanup:
	scripts/CI-cleanup.sh

dev-test:
	scripts/CI-setup.sh
	scripts/CI.sh
	scripts/CI-cleanup.sh
