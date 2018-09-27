INCLUDES := -I. \
			-I../../vendor \
			-I${GOPATH}/src \
			-I../../vendor/github.com/pensando/grpc-gateway/third_party \
			-I../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \

SRCS := $(wildcard *.proto)

default: pythongen
	@mkdir -p ./gogen
	protoc ${INCLUDES} --gofast_out=plugins=grpc:./gogen ${SRCS}

pythongen:
	@mkdir -p ./pygen
	protoc ${INCLUDES} --python_out=plugins=grpc:./pygen ${SRCS}
