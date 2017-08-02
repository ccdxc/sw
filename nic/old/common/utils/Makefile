include ../../build.mk

SRC_DIR = .
OBJ_DIR = ../obj
INCLUDE_DIRS = -I ../../include/ -I ../include \
               -I $(SRC_DIR)/slab \
               -I $(SRC_DIR)/ctrie \

CPP_SRC_FILES :=    $(SRC_DIR)/slab/slab.cc \
                    $(SRC_DIR)/ctrie/ctrie.cc \

CPP_OBJ_FILES := $(patsubst %.cc, %.o, $(CPP_SRC_FILES))

$(CPP_OBJ_FILES): %.o: %.cc | $(OBJ_DIR)
	@echo Compiling $@
	$(CC) $(INCLUDE_DIRS) $(CFLAGS) -c $< -o $(OBJ_DIR)/$(shell basename $@)

all: $(CPP_OBJ_FILES)
	$(CC) -c -fPIC $(INCLUDE_DIRS) $(CFLAGS) $(OBJ_DIR)/*.o -o $(OBJ_DIR)/utils.so

