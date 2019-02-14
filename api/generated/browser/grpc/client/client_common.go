// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

package grpcclient

import (
	"context"

	"google.golang.org/grpc/metadata"

	apiserver "github.com/pensando/sw/venice/apiserver"
)

func dummyBefore(ctx context.Context, md *metadata.MD) context.Context {
	xmd, ok := metadata.FromOutgoingContext(ctx)
	if ok {
		cmd := metadata.Join(*md, xmd)
		*md = cmd
	}
	return ctx
}

func addVersion(ctx context.Context, version string) context.Context {
	pairs := []string{apiserver.RequestParamVersion, version}
	inmd, ok := metadata.FromOutgoingContext(ctx)
	var outmd metadata.MD
	if ok {
		outmd = metadata.Join(inmd, metadata.Pairs(pairs...))
	} else {
		outmd = metadata.Pairs(pairs...)
	}
	return metadata.NewOutgoingContext(ctx, outmd)
}
