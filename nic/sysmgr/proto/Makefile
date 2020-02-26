
sysmgr.pb.go: sysmgr.proto
	protoc --delphigo_out=Mdelphi.proto=github.com/pensando/sw/nic/delphi/proto/delphi:./sysmgr \
        	./sysmgr.proto -I . -I ../../.. -I ../../../nic/sysmgr -I ../../../nic/delphi/proto/delphi

gen-clean: ;
