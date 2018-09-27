INCLUDES := -I. \
			-I../../vendor \
			-I${GOPATH}/src \
			-I../../vendor/github.com/pensando/grpc-gateway/third_party \
			-I../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \

SRCS := $(wildcard *.proto)

default: pythongen
	protoc ${INCLUDES} --gofast_out=plugins=grpc:./gogen ${SRCS}

pythongen:
	protoc ${INCLUDES} --python_out=plugins=grpc:./pygen ${SRCS}
