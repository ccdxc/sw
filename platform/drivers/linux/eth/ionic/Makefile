obj-$(CONFIG_IONIC) := ionic.o
obj-$(CONFIG_IONIC_MNIC) := ionic_mnic.o

ionic-y := ionic_main.o ionic_bus_pci.o ionic_dev.o \
	ionic_lif.o ionic_txrx.o ionic_adminq.o ionic_debugfs.o
ionic_mnic-y := ionic_main.o ionic_bus_platform.o ionic_dev.o \
	ionic_lif.o ionic_txrx.o ionic_adminq.o ionic_debugfs.o

ccflags-y := -DDEBUGFS_TEST_API
