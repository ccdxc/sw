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
CXX = g++
CPPFLAGS += $(addprefix -D,$(MS_COMPILATION_SWITCH)) -I/usr/local/include -std=c++11 \
			-pthread -g -c -Wall -Werror -O0
OBJ_DIR = obj
LIB_DIR = lib

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
			  code/comn/l2         \
			  code/comn/worker         \
			  output/$(GMK_TARGETOS)

INC_DIRS = -I../common/ -I. $(addprefix -I$(MS_ROOT)/,$(MS_INCLPATH))
LD_FLAGS = -L$(MS_LIB_DIR)
DLD_FLAGS = -L$(LIB_DIR) -Wl,-rpath-link=$(MS_LIB_DIR)
LD_LIBS = -lpthread -ldl -lzmq

SRC_FILES :=	pdsa_stub_li.cc \
				pdsa_stub_l2f.cc \
				$(NULL)


#OBJECTS := $(basename $(SRC_FILES))
#OBJECTS := $(OBJECTS:%=$(OBJ_DIR)/%.o)
#DEPENDS = $(OBJECTS:.o=.d)

#MS_LD_LIBS = -ldcsi

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
			 -ldcsm -ldcamb -ldchaf -ldcamkl -ldcnbase -ldcregex   \
			 -lpthread -lrt -lstdc++ -lcrypto -lxml2

OBJ_FILES := $(patsubst %.cc, $(OBJ_DIR)/%.o, $(SRC_FILES))
SHARED_LIB = $(LIB_DIR)/libpdsahal.so

$(OBJ_DIR)/%.o: %.cc
	@echo Compiling $@
	$(shell [ ! -d $(@D) ] && mkdir -p $(@D))
	$(CXX) -m64 $(CPPFLAGS) $(INC_DIRS) -fpic $< -o $@

$(SHARED_LIB): $(OBJ_FILES)
	@echo Linking $@
	$(shell [ ! -d $(@D) ] && mkdir -p $(@D))
	$(CXX) -g -shared -o $@ $^ $(LD_FLAGS) $(DLD_FLAGS) $(MS_LD_LIBS)

.PHONY: clean all
clean:
	rm -f $(LIB_DIR)/*.* $(OBJ_DIR)/*.o
