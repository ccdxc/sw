
sysmgr_events.pb.go: sysmgr_events.proto
	protoc ./sysmgr_events.proto \
           --go_out=Mgogo.proto=github.com/gogo/protobuf/gogoproto:. \
           -I . \
           -I ../ \
           -I ../../../ \
           -I ../../../vendor/github.com/gogo/protobuf/gogoproto
	goimports -local “github.com/pensando/sw” -l -w sysmgr_events.pb.go
