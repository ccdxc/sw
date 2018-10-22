TOPDIR := $(abspath ../../)
INCLUDES := -I. \
            -I${TOPDIR}/vendor \
            -I${GOPATH}/src \
            -I${TOPDIR}/vendor/github.com/pensando/grpc-gateway/third_party \
            -I${TOPDIR}/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \

SRCS := $(wildcard *.proto)
GRPC_PYTHON_PLUGIN_PATH ?= $(TOPDIR)/nic/hal/third-party/grpc/x86_64/bin/grpc_python_plugin

PYTHONGEN_OPTS := --python_out=./pygen \
                  --grpc_out=./pygen \
                  --plugin=protoc-gen-grpc=${GRPC_PYTHON_PLUGIN_PATH}

default: .pygen .gogen

.gogen: ${SRCS}
	@mkdir -p ./gogen
	protoc ${INCLUDES} --gofast_out=plugins=grpc:./gogen ${SRCS}
	touch $@

.pygen: ${SRCS}
	@mkdir -p ./pygen
	protoc ${INCLUDES} ${PYTHONGEN_OPTS} ${SRCS}
	touch $@

clean:
	rm -rf ./pygen
	rm -rf ./gogen
	@rm -f .gogen
	@rm -f .pygen
