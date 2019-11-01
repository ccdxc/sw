TOPDIR = ..

MS_ROOT = $(TOPDIR)/third-party/metaswitch
include ${MS_ROOT}/buildcfg/build_config.mk
MS_LIB_DIR = $(MS_ROOT)/output/$(GMK_TARGETOS)/$(MODE)
DLD_FLAGS = -Wl,-rpath-link=$(MS_LIB_DIR)

MS_TARGET = $(MS_LIB_DIR)/dcsi

CXX = g++
CPPFLAGS += -std=c++11 -pthread -g -c -Wall -Werror -O0
INC_DIRS = -Istubs/common/ -Istubs/hals/ -Istubs/mgmt/ -Istubs
LD_PATHS = -Lstubs/common/lib -Lstubs/hals/lib -Lstubs/mgmt/lib
PDSA_LD_LIBS  = -lpegasusmgmt -lpdsacmn -lpdsahal
PEGASUS_LD_LIBS  = -lpegasusmgmt

OBJ_DIR = obj
BIN_DIR = bin

PDSA_BIN = $(BIN_DIR)/pdsa
PEGASUS_BIN = $(BIN_DIR)/pegasus

install: $(OBJ_DIR) $(BIN_DIR) $(PDSA_BIN) $(PEGASUS_BIN)
$(OBJ_DIR):	
	mkdir -p $(OBJ_DIR)
$(BIN_DIR):	
	mkdir -p $(BIN_DIR)

SRC_FILES = $(wildcard *.cc)
OBJ_FILES = $(SRC_FILES:%.cc=$(OBJ_DIR)/%.o)
DEP_FILES = $(OBJ_FILES:.o=.d)

$(PDSA_BIN): $(OBJ_FILES) $(MS_TARGET) PDSA_STUBS MGMT_STUBS
	@echo Building PDSA binary
	$(CXX) -g -DNAPLES_BUILD -o $@ $(CPPFLAGS) $(INC_DIRS) $< $(LD_PATHS) $(DLD_FLAGS) $(PDSA_LD_LIBS)

$(PEGASUS_BIN): $(OBJ_FILES) $(MS_TARGET) MGMT_STUBS
	@echo Building Pegasus binary
	$(CXX) -g -DVENICE_BUILD -o $@ $(CPPFLAGS) $(INC_DIRS) $< $(LD_PATHS) $(DLD_FLAGS) $(PEGASUS_LD_LIBS)

CXX = g++
CPPFLAGS = -I/usr/local/include -std=c++11 \
        	$(INC_DIRS) -fpic -pthread -g -Wall -Werror -O0
$(OBJ_DIR)/%.o: %.cc
	@echo Compiling $@
	$(CXX) $(CPPFLAGS) -c $< -o $@

include $(DEP_FILES)

$(OBJ_DIR)/%.d: %.cc $(OBJ_DIR)
	@echo Building dependency $@
	@echo $(CPP) $(CPPFLAGS) $< -MM -MT $(@:.d=.o) >$@
	@$(CPP) $(CPPFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: PDSA_STUBS
PDSA_STUBS: 				
	$(MAKE) -C stubs/common
	$(MAKE) -C stubs/hals

.PHONY: MGMT_STUBS
MGMT_STUBS: 				
	$(MAKE) -C stubs/mgmt

MS_BUILD_CFG = $(MS_ROOT)/buildcfg/build_config.mk

$(MS_TARGET): $(MS_BUILD_CFG)
	$(MAKE) -C $(MS_ROOT) 

$(MS_BUILD_CFG): 
	cd $(MS_ROOT); ./configure --dynamic --platform linuxmt64 --mode release; cd -

.PHONY: clean
clean:
	$(MAKE) clean -C stubs/common		 
	$(MAKE) clean -C stubs/hals		 
	$(MAKE) clean -C stubs/mgmt
	rm -rf $(PDSA_BIN)
	rm -rf $(PEGASUS_BIN)
	rm -rf $(OBJ_DIR)

.PHONY: cleandep
cleandep:
	$(MAKE) cleandep -C stubs/common
	$(MAKE) cleandep -C stubs/hals
	$(MAKE) cleandep -C stubs/mgmt
	rm -f $(DEP_FILES)
