TOPDIR = ..
MS_ROOT = $(TOPDIR)/third-party/metaswitch

LD_FLAGS = -Lcommon/lib -Lhals/lib -Lmgmt/lib
DLD_FLAGS = -Wl,-rpath-link=$(MS_LIB_DIR)

PDSA_LIB = stubs/hals/lib/libpdsahal.so

#PDSA_LIB += stubs/common/lib/libpdsacommon.so \
			stubs/mgmt/lib/libpdsamgmt.so


PDSA_BIN = pdsa

include ${MS_ROOT}/buildcfg/build_config.mk
MS_LIB_DIR = $(MS_ROOT)/output/$(GMK_TARGETOS)/$(MODE)

MS_TARGET = $(MS_LIB_DIR)/dcsi

CXX = g++
CPPFLAGS += -std=c++11 -pthread -g -c -Wall -Werror -O0
INC_DIRS = -Istubs/common/ -Istubs/hals/ -Istubs/mgmt/

$(PDSA_BIN): pdsa_main.cc $(MS_TARGET)
	$(MAKE) -C stubs/hals		 
	@echo Building binary
	$(CXX) -g -o $@ $(CPPFLAGS) $(INC_DIRS) $< $(LD_FLAGS) -lpdsa

MS_BUILD_CFG = ${MS_ROOT}/buildcfg/build_config.mk

$(MS_TARGET): ${MS_BUILD_CFG}
	$(MAKE) -C $(MS_ROOT) 

$(MS_BUILD_CFG): 
	cd $(MS_ROOT); ./configure --dynamic --platform linuxmt64 --mode release; cd -

.PHONY: clean all
clean:
	$(MAKE) clean -C stubs/hals		 
	rm -f $(PDSA_BIN) 
