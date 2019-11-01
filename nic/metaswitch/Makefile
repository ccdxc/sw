include pre.mk

default: configure
	make -C $(MS_ROOT) -j32

configure: 
	cd $(MS_ROOT); ./configure --dynamic --platform $(GMK_TARGETOS) --mode $(MODE); cd -
