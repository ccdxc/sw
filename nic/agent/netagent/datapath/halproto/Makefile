GEN_DIR = ${CURDIR}
TOP_DIR = ../../../..
LD_LIBRARY_PATH=$(TOP_DIR)/hal/third-party/google/x86_64/lib/
HAL_PROTO_FILES = $(shell ls ${CURDIR}/../../../../proto/hal/ | grep .proto | tr " " "\n" | LC_ALL=C sort | tr "\n" " ")
default:
	$(MAKE) hal-proto
	# Generate mock files
	mockgen -source=endpoint.pb.go -destination=endpoint_mock.go -package=halproto
	mockgen -source=interface.pb.go -destination=interface_mock.go -package=halproto
	mockgen -source=l2segment.pb.go -destination=l2segment_mock.go -package=halproto
	mockgen -source=l4lb.pb.go -destination=l4lb_mock.go -package=halproto
	mockgen -source=nwsec.pb.go -destination=nwsec_mock.go -package=halproto
	mockgen -source=session.pb.go -destination=session_mock.go -package=halproto
	mockgen -source=vrf.pb.go -destination=tenant_mock.go -package=halproto
	mockgen -source=nw.pb.go -destination=network_mock.go -package=halproto
	mockgen -source=nat.pb.go -destination=nat_mock.go -package=halproto
	# Run go imports on mock files
	goimports -w -l *mock.go

hal-proto: clean-proto
	# Compile proto definitions
	 LC_ALL=C LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) $(TOP_DIR)/hal/third-party/google/x86_64/bin/protoc --proto_path=../../../../../ -I../../../../../nic/proto \
	-I../../../../../nic/proto/hal \
	-I$(TOP_DIR)/hal/third-party/google/include \
	-I../../../../../vendor/github.com/gogo/protobuf/gogoproto \
	--gofast_out=Mgogo.proto=github.com/gogo/protobuf/gogoproto,plugins=grpc:. types.proto ${HAL_PROTO_FILES}

clean-proto:
	# Regenerate all .pb.go. This avoids conflicts due to file renames in nic/proto/hal
	rm -f ${GEN_DIR}/*.pb.go

proto-sync: hal-proto
	# Will verify if the generated pb.go are committed
	@$(eval DIFFS=`git ls-files --exclude-standard --modified --directory ${CURDIR}`)
	@echo "Found the following uncommitted pb.go files, if any"
	@echo $(DIFFS)
	@test -z "$(DIFFS)"
