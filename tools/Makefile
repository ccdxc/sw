# Makefile for dev environment
.PHONY: dev-build-centos72

dev-build-centos73:
	cd packer-templates; packer build -only=virtualbox-iso centos-7.3-x86_64.json
	cd packer-templates; vagrant box add jainvipin/centos73 centos-7.3-x86_64-virtualbox.box

swarm-env-centos73:
	cd packer-templates; SWARM_ENV=1 packer build -only=virtualbox-iso centos-7.3-x86_64.json
	cd packer-templates; vagrant box add jainvipin/docker-centos73 centos-7.3-x86_64-virtualbox.box
