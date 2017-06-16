/*
Package cmd is a auto generated package.
Input file: protos/cmd.proto
*/
package cmd

import (
	"context"
)

type ServiceCmdV1 interface {
	GetNodeList(ctx context.Context, t NodeList) (NodeList, error)
	NodeOper(ctx context.Context, t Node) (Node, error)
	ClusterOper(ctx context.Context, t Cluster) (Cluster, error)
}
