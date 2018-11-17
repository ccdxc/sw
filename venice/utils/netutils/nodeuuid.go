package netutils

import (
	"context"

	"google.golang.org/grpc/metadata"
)

// GetNodeUUIDFromCtx returns the NodeUUID embedde in the GRPC metadata
func GetNodeUUIDFromCtx(ctx context.Context) string {
	// get the remote node uuid from the grpc metadata
	md, ok := metadata.FromIncomingContext(ctx)
	if ok == false {
		return ""
	}
	nodeUUIDs, ok := md["nodeuuid"]
	if ok == false {
		return ""
	}
	if len(nodeUUIDs) < 1 {
		return ""
	}
	return nodeUUIDs[0]
}
