GEN_DIR = ${CURDIR}
HAL_PROTO_FILES = $(wildcard ../../../../proto/hal/*.proto)
default:
	$(MAKE) hal-proto
	#FIXME We need a flat structure *.pb.go for imports to work
	mv nic/proto/*.pb.go .
	mv nic/proto/hal/*.pb.go .
	rm -rf nic

	# Generate mock files
	mockgen -source=endpoint.pb.go -destination=endpoint_mock.go -package=halproto
	mockgen -source=interface.pb.go -destination=interface_mock.go -package=halproto
	mockgen -source=l2segment.pb.go -destination=l2segment_mock.go -package=halproto
	mockgen -source=l4lb.pb.go -destination=l4lb_mock.go -package=halproto
	mockgen -source=nwsec.pb.go -destination=nwsec_mock.go -package=halproto
	mockgen -source=session.pb.go -destination=session_mock.go -package=halproto
	mockgen -source=tenant.pb.go -destination=tenant_mock.go -package=halproto

hal-proto:
	# Compile proto definitions
	protoc --proto_path=../../../../../ --gofast_out=Mprotos/github/gogo/protobuf/gogoproto/gogo.proto=github.com/gogo/protobuf/gogoproto,plugins=grpc:$(GEN_DIR) ../../../../../nic/proto/hal/*.proto ../../../../../nic/proto/types.proto


