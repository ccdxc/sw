SRC := $(shell ls *.proto)
DEST := $(SRC:%.proto=%.pb.go)

default: ${DEST}

%.pb.go: %.proto Makefile
	protoc -I/usr/local/include -I. -I../../../../../vendor -I${GOPATH}/src \
	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party \
	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
        --gofast_out=plugins=grpc:. $<
	#sed -i.bak_ 's/api\/protos/api\/generated\/rollout/g' $@
	#rm -f *.bak_

	goimports -local "github.com/pensando/sw" -l -w *.go

clean:
	rm -f *.pb.go
