# Makefile for dev environment
.PHONY: dev-build-centos74

dev-build-centos74:
	@pens-packer-scripts/packer-fixup.sh
	cd packer-templates; packer build -only=virtualbox-iso pens-centos-7.4-x86_64.json
	cd packer-templates; vagrant box add --name venice/centos74 centos-7.4-x86_64-virtualbox.box
	# To test use:
	#vagrant box add --name vramakrishnan/centos73 packer-templates/centos-7.3-x86_64-virtualbox.box
	# and then Upload image manually to https://app.vagrantup.com/

turin-centos74:
	TURIN=1 pens-packer-scripts/packer-fixup.sh
	cd packer-templates; packer build -only=virtualbox-iso pens-centos-7.4-x86_64.json
	cd packer-templates; vagrant box add --name turin/centos74 centos-7.4-x86_64-virtualbox.box
