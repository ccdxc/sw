all: clean delphi-build

delphi-build:
	protoc --delphigo_out=Mdelphi.proto=github.com/pensando/sw/nic/delphi/proto/delphi,Mgithub.com/pensando/sw/api/protos/smartnic.proto=github.com/pensando/sw/api/generated/cluster:. naples_status.proto \
               -I ./ \
               -I ../../../../../../../../ \
               -I ../../../../../vendor \
               -I ../../../../../api/protos \
               -I ../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis/ \
               -I ../../../../delphi/proto/delphi/
	goimports -w *.go

gen-clean: ;
clean:
	 rm -rf nmd.delphi.go
