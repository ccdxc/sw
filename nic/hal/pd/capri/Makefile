TOPDIR = ../../..
SHAREDLIBRARY = libcapri.so

CAPTOP = $(TOPDIR)/asic/capri/model
CAPMODS = $(wildcard $(CAPTOP)/cap_*)
CAPMODS += $(CAPTOP)/utils
CAPMODS += $(TOPDIR)/asic/ip/verif/pcpp
CAPINC = $(patsubst %, -I%, $(CAPMODS))

LCXXFLAGS = \
			-I. \
			-I./csr \
			-I../.. \
			-I../../src \
			-I../../../include \
			-I../../../third-party/spdlog/include \
			-I/usr/local/include \
			-I../../../capsim/lib/libmpuobj/include \
			-I../../../capsim/lib/libcapisa/include \
			-I../../../model_sim/include \
			-I../../../gen/iris/include \
			-I../../../p4/nw/include \
			-I../../../hal/pd \
			-I../../../hal/src \
			$(CAPINC) \
			-g \
			-Wno-maybe-uninitialized \
			$(NULL)

SLFLAGS = \
		  -Wl,--whole-archive $(TOPDIR)/capsim/gen/lib/libmpuobj.a $(TOPDIR)/capsim/gen/lib/libcapisa.a $(TOPDIR)/capsim/gen/lib/libisa.a \
		  -Wl,--no-whole-archive -L$(TOPDIR)/model_sim/build -L/usr/local/lib -L$(TOPDIR)/obj -lmodelclient -lzmq -lcapricsr
SOURCES = \
		  capri_init.cc \
		  capri_asic_rw.cc \
		  capri_config.cc \
		  capri_loader.cc \
		  capri_tbl_rw.cc \
		  capri_hbm.cc \
		  capri_tm_rw.cc \
		  capri_qstate.cc \
		  $(NULL)

include $(TOPDIR)/make/Makefile.inc

# CXX = g++
# CPPFLAGS += -I/usr/local/include -std=c++11 -g -DHAL_MRL_PORT
# OBJ_DIR = ../../../obj
# INC_DIRS = -I. -I./mrl -I.. -I../.. -I../../../include -I../../../third-party/spdlog/include
# INC_DIRS += -I../../../capsim/lib/libmpuobj/include/
# INC_DIRS += -I../../../capsim/lib/libcapisa/include/
# INC_DIRS += -I../../../model_sim/include
# INC_DIRS += -I../../../gen/iris/include/
# LIB_DIR = ../../../model_sim/build
# LDFLAGS = -L/usr/local/lib -L../../../model_sim/build -L../../../obj
# 
# LIBS = ../../../capsim/gen/lib/libmpuobj.a
# LIBS += ../../../capsim/gen/lib/libcapisa.a
# LIBS += ../../../capsim/gen/lib/libisa.a
# 
# SRCS := \
#     capri_init.cc \
#     capri_asic_rw.cc \
#     capri_loader.cc \
#     capri_tbl_rw.cc \
# 	capri_hbm.cc \
#     $(NULL)
# 
# OBJS := $(basename $(SRCS))
# OBJS := $(OBJS:%=$(OBJ_DIR)/%.o)
# 
# all: $(OBJS)
# 	$(CXX) -shared -o $(OBJ_DIR)/libcapri.so $(OBJS) \
#         -Wl,--whole-archive $(LIBS) -Wl,--no-whole-archive -L$(LIB_DIR) \
# 		-lmodelclient -lzmq $(LDFLAGS) -lcaprimrl
# 
# $(OBJ_DIR)/%.o : %.cc
# 	$(CXX) $(CPPFLAGS) $(INC_DIRS) -c -Wall -Werror -fpic -o $@ $<
# 
# clean:
# 	rm -rf $(OBJ_DIR)/*capri*.o $(OBJ_DIR)/libcapri.so
