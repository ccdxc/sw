TOPDIR = ../../..

OBJ_DIR = obj
LIB_DIR = lib
INC_DIRS = -I../common/ -I. -I../../.. -I../.. -I../../sdk -I../../build/aarch64/apollo -I../../hal/third-party/spdlog/include -I../../build/aarch64/apollo/gen/proto
LD_LIBS = -lpthread -ldl -lzmq

SRC_FILES = $(wildcard *.cc)
OBJ_FILES = $(SRC_FILES:%.cc=$(OBJ_DIR)/%.o)
DEP_FILES = $(OBJ_FILES:.o=.d)

SHARED_LIB = $(LIB_DIR)/libpdsasvc.so

CXX = g++
CPPFLAGS = $(addprefix -D,$(MS_COMPILATION_SWITCH)) -I/usr/local/include -std=c++11 \
        	$(INC_DIRS) -fpic -pthread -g -Wall -Werror -O0
LD_FLAGS = $(MS_LD_LIBS) $(LD_LIBS)  `pkg-config --libs grpc++ grpc` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -lprotobuf -lpthread -ldl -L../build/x86_64/apollo/lib -L/usr/local/lib `pkg-config --libs grpc++ grpc` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed  -lprotobuf -lpthread
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
