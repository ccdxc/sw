.PHONY: all build clean

PROTO_SRC = accel_rgroup.proto fwlog.proto rawrcb.proto acl.proto gft.proto rdma.proto \
            barco_rings.proto multicast.proto session.proto \
            interface.proto nat.proto system.proto cpucb.proto internal.proto nic.proto \
            table.proto  crypto_apis.proto nw.proto tcp_proxy_cb.proto \
            crypto_keys.proto ipsec.proto nwsec.proto tcp_proxy.proto debug.proto \
            kh.proto port.proto telemetry.proto descriptor_aol.proto l2segment.proto \
            proxyccb.proto tls_proxy_cb2.proto dos.proto l4lb.proto proxy.proto \
            tls_proxy_cb.proto endpoint.proto proxyrcb.proto types.proto \
            eplearn.proto qos.proto vrf.proto event.proto rawccb.proto wring.proto

all: build

build: 
	protoc --delphigo_out=Mdelphi.proto=github.com/pensando/sw/nic/delphi/proto/delphi,Mgogo.proto=github.com/gogo/protobuf/gogoproto:./halproto ${PROTO_SRC} \
                -I ../../../../proto/hal/ -I /usr/src/github.com/pensando/sw -I . \
                -I ../../../../../vendor/github.com/gogo/protobuf/gogoproto/ \
                -I ../../../../delphi/proto/delphi/ 
	goimports -w ./halproto/*.go
	go install ./halproto/

clean:
	rm -f ./halproto/*.pb.go
	rm -f ./halproto/*.delphi.go
