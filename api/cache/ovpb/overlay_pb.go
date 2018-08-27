package overlaypb

import golangproto "github.com/gogo/protobuf/proto"

func init() {
	// Register this with regular golang proto so it is accessible for grpc code
	golangproto.RegisterType((*BufferItem)(nil), "overlaypb.BufferItem")
}
