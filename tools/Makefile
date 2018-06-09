# Makefile for dev environment
.PHONY: dev-build-centos74

dev-build-centos74:
	@pens-packer-scripts/packer-fixup.sh
	cd packer-templates; packer build -only=virtualbox-iso pens-centos-7.4-x86_64.json
	cd packer-templates; vagrant box add --name venice/centos74 centos-7.4-x86_64-virtualbox.box
	# To test use:
	#vagrant box add --name vramakrishnan/centos73 packer-templates/centos-7.3-x86_64-virtualbox.box
	# and then Upload image manually to https://app.vagrantup.com/

swarm-env-centos73:
	cd packer-templates; SWARM_ENV=1 packer build -only=virtualbox-iso centos-7.3-x86_64.json
	cd packer-templates; vagrant box add jainvipin/docker-centos73 centos-7.3-x86_64-virtualbox.box
