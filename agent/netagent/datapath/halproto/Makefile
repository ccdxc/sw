default:
	# Compile proto definitions
	protoc --go_out=Mtypes/types.proto=github.com/pensando/sw/agent/netagent/datapath/halproto/types,plugins=grpc:. *.proto
	protoc --go_out=plugins=grpc:. ./types/types.proto
	# Install gomock tool
	go get github.com/golang/mock/gomock
	go get github.com/golang/mock/mockgen
	# Generate mock files
	mockgen -source=endpoint.pb.go -destination=endpoint_mock.go -package=halproto
	mockgen -source=interface.pb.go -destination=interface_mock.go -package=halproto
	mockgen -source=l2segment.pb.go -destination=l2segment_mock.go -package=halproto
	mockgen -source=l4lb.pb.go -destination=l4lb_mock.go -package=halproto
	mockgen -source=nwsec.pb.go -destination=nwsec_mock.go -package=halproto
	mockgen -source=session.pb.go -destination=session_mock.go -package=halproto
	mockgen -source=tenant.pb.go -destination=tenant_mock.go -package=halproto
