BR_VER = 2016.11
BR_SRC = $(HOME)/buildroot-$(BR_VER)
BR_TAR = /vagrant/buildroot-$(BR_VER)-aarch64.tar.gz
BR_DIR = `dirname $(BR2_HOST_DIR)`
BR2_HOST_DIR = /buildroot-$(BR_VER)
BR2_DEFCONFIG = /vagrant/br-arm64-defconfig

all: /vagrant build $(BR_TAR)

build: $(BR_SRC)/.config
	# make destination parent dir writable
	[ -w $(BR_DIR) ] || sudo chmod a+w $(BR_DIR)
	make -C $(BR_SRC)

$(BR_SRC)/.config: $(BR_SRC) $(BR2_DEFCONFIG)
	make -C $(BR_SRC) defconfig BR2_DEFCONFIG=$(BR2_DEFCONFIG)

$(BR_SRC):
	(cd $(HOME); tar xfz buildroot-$(BR_VER).tar.gz)

$(BR_TAR):
	tar czf $(BR_TAR) $(BR2_HOST_DIR) --owner=root --group=root

/vagrant:
	@echo Use this Makefile inside the vagrant VM.
	@exit 1
