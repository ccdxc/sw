include pre.mk

default: configure
	make -C $(MS_ROOT) -j28

configure: 
	cd $(MS_ROOT); ./configure --static --platform $(GMK_TARGETOS) --mode $(MODE); cd -
