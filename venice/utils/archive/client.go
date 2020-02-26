package archive

import (
	"context"
	"fmt"

	"google.golang.org/grpc"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/archive/protos"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

type client struct {
	rpcClients    []*rpckit.RPCClient
	archiveClient protos.ArchiveClient
	l             log.Logger
}

func (c *client) CancelRequest(ctx context.Context, in *monitoring.ArchiveRequest, opts ...grpc.CallOption) (*monitoring.ArchiveRequest, error) {
	var err error
	for _, rpccl := range c.rpcClients {
		_, err = protos.NewArchiveClient(rpccl.ClientConn).CancelRequest(ctx, in, opts...)
		if err == nil {
			c.l.DebugLog("method", "CancelRequest", "msg", fmt.Sprintf("no error in rpc call CancelRequest for archive request [%s|%s]", in.Tenant, in.Name))
			break
		}
	}
	return in, err
}

func (c *client) Close() {
	for _, rpccl := range c.rpcClients {
		rpccl.Close()
	}
}

// NewClientGetter returns an implementation of ClientGetter
func NewClientGetter(name string, logtype string, rslvr resolver.Interface, l log.Logger) (ClientGetter, error) {
	var clgetter ClientGetterFunc
	switch logtype {
	case monitoring.ArchiveRequestSpec_Event.String(), monitoring.ArchiveRequestSpec_AuditEvent.String():
		clgetter = func() (Client, error) {
			b := balancer.New(rslvr)
			svccl, err := rpckit.NewRPCClient(name, globals.Spyglass, rpckit.WithBalancer(b))
			if err != nil {
				b.Close()
				return nil, err
			}
			return &client{
				rpcClients:    []*rpckit.RPCClient{svccl},
				archiveClient: protos.NewArchiveClient(svccl.ClientConn),
				l:             l,
			}, nil
		}
	case monitoring.ArchiveRequestSpec_FwLog.String():
	default:
		return nil, fmt.Errorf("unknown archive request type: %s", logtype)
	}
	return clgetter, nil
}
