# Makefile for dev environment
.PHONY: dev-build-centos72

dev-build-centos72:
	cd packer-templates; packer build -only=virtualbox-iso centos-7.2-x86_64.json
	cd packer-templates; vagrant box add jainvipin/centos72 centos-7.2-x86_64-virtualbox.box
