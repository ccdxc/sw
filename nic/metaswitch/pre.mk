# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

export MS_ROOT = $(TOPDIR)/nic/third-party/metaswitch
export GMK_TARGETOS = linuxmt64
export MODE = debug
export MS_LIB_DIR = $(MS_ROOT)/output/$(GMK_TARGETOS)/$(MODE)

export MS_COMPILATION_SWITCH = \
        LINUX_MT_NBASE \
        NBB_64BIT_TYPES_NATIVE \
        NBB_64_BIT_POINTERS

export MS_INCLPATH = \
        code/comn/intface        \
        code/comn/custom         \
        code/comn                \
        buildcfg                 \
        code/comn/ambl           \
	code/comn/fte            \
	code/comn/fte/joins      \
	code/comn/ntlpp          \
	code/comn/autogen/ambl   \
	code/comn/aall           \
	code/stubs/sms           \
	code/stubs/qcft          \
	code/comn/nbase/unix     \
	code/comn/nbase/unixmt   \
	code/comn/nbase/portable \
	code/comn/nbase/base     \
	code/cipr/utils          \
	code/cipr/qrml           \
	code/stubs/li            \
	code/stubs/sck/          \
	code/stubs/l2f/          \
	code/stubs/hals/         \
	code/stubs/smi           \
	code/comn/l2             \
	code/comn/worker         \
	output/$(GMK_TARGETOS)

export MS_LD_LIBS = \
        dcgeolocn dcamx dchm dci3dl dcadll dctpl dchals dcjoinxcpi \
        dcftm dcjoinropi dclim dcjoinbdii dcjoinlapi dcjointci \
        dcjointpi dcjoinsmi dci3filter dcysscam dcyssdef dcpcepsl \
        dcsmi dcsck dcipl dccpfl dcnrm dcjoinnbpi dcl2radv dcnar \
        dcjoinnari dcevpn dcbdpl dcraid dcbgp dcqaql dcamh \
        dcqcft dcla dcl2 dcli dcjoinfri dcfrl dcjoinlipi \
        dcqcrt dccipr dcqrml dcl2rdisc dcpsm dcjoinbfdi \
        dcjoinnhpi dcjoini3 dcjoinnri dcjoinpri dcjoinari \
        dcjointpqi dcl2f dcjoinbdpi dcjoinmai dcjoinmmi \
        dcmal dcbasewo dcfte dchasl dcai3 dccss dccssu \
        dcamb dcsm dchaf dcamkl dcnbase dcregex   \
        pthread rt stdc++
