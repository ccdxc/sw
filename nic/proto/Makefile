
GEN_DIR = ../gen/protobuf
TOPDIR = ..
CUSTOM = $(GEN_DIR)/proto
CUSTOM_CLEAN =

LCXXFLAGS = \
			-I/usr/local/include \
			-pthread \
			-g \
			$(NULL)

include $(TOPDIR)/make/Makefile.inc

# Adding more clean files
LCLEANFILES = $(OBJ_DIR)/*.pb.o $(OBJ_DIR)/*.pb.d $(GEN_DIR)/*.pb.* $(GEN_DIR)/*.pb-c.* $(GEN_DIR)/*.py $(GEN_DIR)/*_proto $(GEN_DIR) 

PROTOC = protoc
C_PROTOC = protoc-c
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`
GRPC_PYTHON_PLUGIN_PATH ?= `which grpc_python_plugin`
COMMON_PROTO_DIR = ${CURDIR}
AGENTS_PROTO_DIR = ./agents
HAL_PROTO_DIR = ./hal
OBJ_DIR = ../obj
CPPFLAGS += -I/usr/local/include -std=c++11 -pthread -fpic -O3

#$(GEN_DIR)/proto: $(GEN_DIR)/.dir $(GEN_DIR)/types_proto $(GEN_DIR)/agents_proto $(GEN_DIR)/hal_proto $(GEN_DIR)/cp_mfile_proto cxx_build
$(GEN_DIR)/proto: $(GEN_DIR)/.dir $(GEN_DIR)/types_proto $(GEN_DIR)/agents_proto $(GEN_DIR)/hal_proto
	cp Makefile.protobuf $(GEN_DIR)/Makefile
	touch $@

tproto_src = $(wildcard ./*.proto)
aproto_src = $(wildcard agents/*.proto)
hproto_src = $(wildcard hal/*.proto)

# .PHONY: types_proto agents_proto hal_proto
$(GEN_DIR)/types_proto: $(tproto_src)
	@echo tproto_src = $(tproto_src)
	$(PROTOC) -I. --cpp_out=$(GEN_DIR) --grpc_out=$(GEN_DIR) --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $(tproto_src)
	$(PROTOC) -I. --python_out=$(GEN_DIR) --grpc_out=$(GEN_DIR) --plugin=protoc-gen-grpc=$(GRPC_PYTHON_PLUGIN_PATH) $(tproto_src)
	$(C_PROTOC) -I. -I$(COMMON_PROTO_DIR) --c_out=$(GEN_DIR) $(tproto_src)
	@( cd $(COMMON_PROTO_DIR); \
	   for file in $(patsubst $(COMMON_PROTO_DIR)/%.proto,%.proto,$(wildcard $(COMMON_PROTO_DIR)/*.proto));  do \
		  $(PROTOC) -I. --go_out=plugins=grpc:$(GEN_DIR) $${file}; \
	   done )
	touch $@

$(GEN_DIR)/agents_proto: $(aproto_src)
	@echo aproto_src = $(aproto_src)
	$(PROTOC) -I$(COMMON_PROTO_DIR) -I$(AGENTS_PROTO_DIR) --cpp_out=$(GEN_DIR) --grpc_out=$(GEN_DIR) --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $(aproto_src)
	$(C_PROTOC) -I$(COMMON_PROTO_DIR) -I$(AGENTS_PROTO_DIR) --c_out=$(GEN_DIR) $(aproto_src)
	@( cd $(AGENTS_PROTO_DIR); \
	   for file in $(patsubst $(AGENTS_PROTO_DIR)/%.proto,%.proto,$(wildcard $(AGENTS_PROTO_DIR)/*.proto));  do \
		  $(PROTOC) -I$(COMMON_PROTO_DIR) -I. --go_out=plugins=grpc:../$(GEN_DIR) $${file}; \
	   done )
	touch $@

$(GEN_DIR)/hal_proto: $(hproto_src)
	$(PROTOC) -I$(COMMON_PROTO_DIR) -I$(HAL_PROTO_DIR) --cpp_out=$(GEN_DIR) --grpc_out=$(GEN_DIR) --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $(hproto_src)
	$(PROTOC) -I$(COMMON_PROTO_DIR) -I$(HAL_PROTO_DIR) --python_out=$(GEN_DIR) --grpc_out=$(GEN_DIR) --plugin=protoc-gen-grpc=$(GRPC_PYTHON_PLUGIN_PATH) $(hproto_src)
	$(C_PROTOC) -I$(COMMON_PROTO_DIR) -I$(HAL_PROTO_DIR) --c_out=$(GEN_DIR) hal/*.proto
	@( cd $(HAL_PROTO_DIR); \
	   for file in $(patsubst $(HAL_PROTO_DIR)/%.proto,%.proto,$(wildcard $(HAL_PROTO_DIR)/*.proto));  do \
		  $(PROTOC) -I$(COMMON_PROTO_DIR) -I. --go_out=plugins=grpc:../$(GEN_DIR) $${file}; \
	   done )
	touch $@

$(GEN_DIR)/cp_mfile_proto:
	cp Makefile.protobuf $(GEN_DIR)/Makefile
	touch $@


cxx_build:
	$(MAKE) -O3 -C $(GEN_DIR)

# clean:
# 	rm -f $(GEN_DIR)/*.pb.* $(GEN_DIR)/*.pb-c.* $(GEN_DIR)/*.py $((GEN_DIR)/*.pb.go $(GEN_DIR)/*proto 
# 	$(MAKE) -C $(GEN_DIR) clean
