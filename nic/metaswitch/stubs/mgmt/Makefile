TOPDIR = ../../..
MS_ROOT = $(TOPDIR)/third-party/metaswitch

include ${MS_ROOT}/buildcfg/build_config.mk
MS_LIB_DIR = $(MS_ROOT)/output/$(GMK_TARGETOS)/$(MODE)

ifeq (,$(wildcard $(MS_ROOT)/buildcfg))
    $(error $(MS_ROOT) build incomplete, Missing directory $(MS_ROOT)/buildcfg)
endif
        
ifeq (,$(wildcard $(MS_ROOT)/buildcfg/build_config.mk))
    $(error $(MS_ROOT) build incomplete, Missing build_config.mk)
endif

ifeq (,$(wildcard $(MS_LIB_DIR)))
    $(error $(MS_ROOT) build incomplete, Missing directory $(MS_LIB_DIR))
endif
        
MS_COMPILATION_SWITCH=LINUX_MT_NBASE \
                      NBB_64BIT_TYPES_NATIVE \
                      NBB_64_BIT_POINTERS

MS_INCLPATH = code/comn/intface        \
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
			  code/stubs/hals/          \
			  code/stubs/sms/          \
			  code/comn/l2         \
			  code/comn/worker         \
			  output/$(GMK_TARGETOS)

OBJ_DIR = obj
LIB_DIR = lib
INC_DIRS = -I../common/ -I. $(addprefix -I$(MS_ROOT)/,$(MS_INCLPATH))
LD_LIBS = -lpthread -ldl -lzmq

SRC_FILES = $(wildcard *.cc)
OBJ_FILES = $(SRC_FILES:%.cc=$(OBJ_DIR)/%.o)
DEP_FILES = $(OBJ_FILES:.o=.d)

MS_LD_LIBS = -ldcgeolocn -ldchm -ldci3dl -ldcadll -ldctpl -ldchals -ldcjoinxcpi \
			 -ldcftm -ldcjoinropi -ldclim -ldcjoinbdii -ldcjoinlapi -ldcjointci \
			 -ldcjointpi -ldcjoinsmi -ldci3filter -ldcysscam -ldcyssdef -ldcpcepsl \
			 -ldcsck -ldcipl -ldccpfl -ldcnrm -ldcjoinnbpi -ldcl2radv -ldcnar \
			 -ldcjoinnari -ldcevpn -ldcbdpl -ldcraid -ldcbgp -ldcqaql -ldcamh \
			 -ldcqcft -ldcla -ldcl2 -ldcli -ldcjoinfri -ldcfrl -ldcjoinlipi \
			 -ldcqcrt -ldccipr -ldcqrml -ldcl2rdisc -ldcpsm -ldcjoinbfdi \
			 -ldcjoinnhpi -ldcjoini3 -ldcjoinnri -ldcjoinpri -ldcjoinari \
			 -ldcjointpqi -ldcl2f -ldcjoinbdpi -ldcjoinmai -ldcjoinmmi \
			 -ldcmal -ldcbasewo -ldcfte -ldchasl -ldcai3 -ldccss -ldccssu \
			 -ldcamb -ldcsm -ldchaf -ldcamkl -ldcnbase -ldcregex   \
			 -lpthread -lrt -lstdc++ -lcrypto

SHARED_LIB = $(LIB_DIR)/libpegasusmgmt.so

CXX = g++
CPPFLAGS = $(addprefix -D,$(MS_COMPILATION_SWITCH)) -I/usr/local/include -std=c++11 \
        	$(INC_DIRS) -fpic -pthread -g -Wall -Werror -O0
LD_FLAGS = $(MS_LD_LIBS) $(LD_LIBS) -L$(MS_LIB_DIR)
CPP_FLAGS += $(INC_DIRS)

install: $(OBJ_DIR) $(LIB_DIR) $(SHARED_LIB)
$(OBJ_DIR):	
	mkdir -p $(OBJ_DIR)
$(LIB_DIR):	
	mkdir -p $(LIB_DIR)

$(SHARED_LIB): $(OBJ_FILES)
	@echo Linking $@
	$(CXX) -g -shared -o $@ $^ $(LD_FLAGS)

$(OBJ_DIR)/%.o: %.cc
	@echo Compiling $@
	$(CXX) $(CPPFLAGS) -c $< -o $@

include $(DEP_FILES)

$(OBJ_DIR)/%.d: %.cc $(OBJ_DIR)
	@$(CPP) $(CPPFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: install clean
clean:
	rm -rf $(LIB_DIR)
	rm -rf $(OBJ_DIR)

.PHONY: cleandep
cleandep:
	rm -f $(DEP_FILES)
