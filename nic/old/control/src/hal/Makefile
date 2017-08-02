include ../../build.mk

SRC_DIR = ./
OBJ_DIR = ../../obj
CONTROL_INCLUDE = ../../include
COMMON_INCLUDE = ../../../common/include
UTILS_INCLUDE = ../utils
INCLUDE_DIRS = -I ../../include/ -I ../../../common/include \
               -I $(UTILS_INCLUDE)/slab \
               -I $(UTILS_INCLUDE)/timer \
               -I $(UTILS_INCLUDE)/ctrie \

CPP_SRC_FILES :=    $(SRC_DIR)/utils/slab/slab.cc \
                    $(SRC_DIR)/utils/ctrie/ctrie.cc \
                    $(SRC_DIR)/utils/timer/timestamp.cc \
                    $(SRC_DIR)/utils/timer/twheel.cc \
                    $(SRC_DIR)/hal/l4.cc \
                    $(SRC_DIR)/main.cc \

GTEST_SRC_FILES := $(SRC_DIR)/utils/slab/slab_test.cc \
                    $(SRC_DIR)/utils/timer/twheel_test.cc \

C_SRC_FILES :=

CPP_OBJ_FILES := $(patsubst %.cc, %.o, $(CPP_SRC_FILES))
GTEST_OBJ_FILES := $(patsubst %.cc, %, $(GTEST_SRC_FILES))
C_OBJ_FILES := $(patsubst %.c, %.o, $(C_SRC_FILES))
