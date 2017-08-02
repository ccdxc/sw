include ./build.mk

GTEST_ROOT = ./test/gtest/googletest-release-1.8.0/googletest
SRC_DIR = ./src
OBJ_DIR = ./obj
GTEST_OBJ_DIR = ./obj/gtest/
INCLUDE_DIRS = -I ./include/ -I ../common/include \
			   -I $(SRC_DIR)/utils/slab \
			   -I $(SRC_DIR)/utils/timer \
			   -I $(SRC_DIR)/utils/ctrie \
			   -I $(GTEST_ROOT)/include \

CPP_SRC_FILES :=	$(SRC_DIR)/utils/slab/slab.cc \
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

MKDIR = mkdir -p

$(OBJ_DIRS):
	$(MKDIR) obj
	$(MKDIR) obj/gtest

$(DIRS): $(OBJ_DIRS)

$(OBJ_DIR):
	mkdir -p $@

$(GTEST_OBJ_DIR):
	mkdir -p $@

$(CPP_OBJ_FILES): %.o: %.cc | $(OBJ_DIR)
	@echo Compiling $@
	$(CC) $(INCLUDE_DIRS) $(CFLAGS) -c $< -o obj/$(shell basename $@)

#$(GTEST_OBJ_FILES): %: %.cc | $(GTEST_OBJ_DIR)
#	@echo Compiling $<
#	@echo $(shell echo $(shell basename $< | cut -d'_' -f1)).o
#	@echo $(shell cut -d'_' -f1 << basename $<)
#	$(CC) $(INCLUDE_DIRS) $(CFLAGS) $(GTEST_ROOT)/make/gtest.a obj/$(shell echo $(shell basename $< | cut -d'_' -f1)).o -lpthread $< -o obj/gtest/$(shell basename $@)

$(C_OBJ_FILES): %.o: %.c | $(OBJ_DIR)
	@echo Compiling $@
	$(CC) $(INCLUDE_DIRS) $(CFLAGS) -c $< -o obj/$(shell basename $@)

clean :
	@rm -fR obj/*

# Below this is all googletest unit tests, ideally all the libs should have been
# compiled as .so files which are linked here
twheel_test:
	$(CC) $(INCLUDE_DIRS) $(CFLAGS) $(SRC_DIR)/utils/timer/twheel_test.cc $(GTEST_ROOT)/make/gtest.a obj/twheel.o obj/slab.o obj/timestamp.o -lpthread $< -o obj/gtest/$(shell basename $@)
slab_test:
	$(CC) $(INCLUDE_DIRS) $(CFLAGS) $(SRC_DIR)/utils/slab/slab_test.cc $(GTEST_ROOT)/make/gtest.a obj/slab.o -lpthread $< -o obj/gtest/$(shell basename $@)

gtest: $(GTEST_OBJ_DIR) twheel_test slab_test

all: $(CPP_OBJ_FILES) $(C_OBJ_FILES)
	$(CC) $(INCLUDE_DIRS) $(CFLAGS) obj/*.o -o obj/hal
	@mkdir obj/gtest
	$(CC) $(INCLUDE_DIRS) $(CFLAGS) -Wno-sign-compare $(SRC_DIR)/utils/slab/slab_test.cc $(GTEST_ROOT)/make/gtest.a $(OBJ_DIR)/slab.o -lpthread -o obj/gtest/slab_test
	$(CC) $(INCLUDE_DIRS) $(CFLAGS) -Wno-sign-compare $(SRC_DIR)/utils/timer/twheel_test.cc $(GTEST_ROOT)/make/gtest.a $(OBJ_DIR)/twheel.o $(OBJ_DIR)/slab.o $(OBJ_DIR)/timestamp.o -lpthread -o obj/gtest/twheel_test

