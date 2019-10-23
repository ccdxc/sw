TOPDIR = ..

MS_ROOT = $(TOPDIR)/third-party/metaswitch
include ${MS_ROOT}/buildcfg/build_config.mk
MS_LIB_DIR = $(MS_ROOT)/output/$(GMK_TARGETOS)/$(MODE)
DLD_FLAGS = -Wl,-rpath-link=$(MS_LIB_DIR)

MS_TARGET = $(MS_LIB_DIR)/dcsi

CXX = g++
CPPFLAGS += -std=c++11 -pthread -g -c -Wall -Werror -O0
INC_DIRS = -Istubs/common/ -Istubs/hals/ -Istubs/mgmt/
LD_FLAGS = -Lstubs/common/lib -Lstubs/hals/lib -Lstubs/mgmt/lib

OBJ_DIR = obj
BIN_DIR = bin

PDSA_BIN = $(BIN_DIR)/pdsa

install: $(OBJ_DIR) $(BIN_DIR) $(PDSA_BIN)
$(OBJ_DIR):	
	mkdir -p $(OBJ_DIR)
$(BIN_DIR):	
	mkdir -p $(BIN_DIR)

SRC_FILES = $(wildcard *.cc)
OBJ_FILES = $(SRC_FILES:%.cc=$(OBJ_DIR)/%.o)
DEP_FILES = $(OBJ_FILES:.o=.d)
$(PDSA_BIN): $(OBJ_FILES) $(MS_TARGET) PDSA_STUBS
	@echo Building binary
	$(CXX) -g -o $@ $(CPPFLAGS) $(INC_DIRS) $< $(LD_FLAGS) $(DLD_FLAGS) -lpdsahal

CXX = g++
CPPFLAGS = -I/usr/local/include -std=c++11 \
        	$(INC_DIRS) -fpic -pthread -g -Wall -Werror -O0
$(OBJ_DIR)/%.o: %.cc
	@echo Compiling $@
	$(CXX) $(CPPFLAGS) -c $< -o $@
include $(DEP_FILES)

$(OBJ_DIR)/%.d: %.cc
	@echo Building dependency $@
	@echo $(CPP) $(CPPFLAGS) $< -MM -MT $(@:.d=.o) >$@
	@$(CPP) $(CPPFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: PDSA_STUBS
PDSA_STUBS: 				
	$(MAKE) -C stubs/hals

MS_BUILD_CFG = $(MS_ROOT)/buildcfg/build_config.mk

$(MS_TARGET): $(MS_BUILD_CFG)
	$(MAKE) -C $(MS_ROOT) 

$(MS_BUILD_CFG): 
	cd $(MS_ROOT); ./configure --dynamic --platform linuxmt64 --mode release; cd -

.PHONY: clean
clean:
	$(MAKE) clean -C stubs/hals		 
	rm -f $(PDSA_BIN) 

.PHONY: cleandep
cleandep:
	$(MAKE) cleandep -C stubs/hals		 
	rm -f $(DEP_FILES)
