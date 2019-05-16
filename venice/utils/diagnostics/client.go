package diagnostics

import (
	"context"

	"google.golang.org/grpc"

	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics/protos"
	"github.com/pensando/sw/venice/utils/rpckit"
)

type client struct {
	rpcClient         *rpckit.RPCClient
	diagnosticsClient protos.DiagnosticsClient
}

func (c *client) Debug(ctx context.Context, in *diagapi.DiagnosticsRequest, opts ...grpc.CallOption) (*diagapi.DiagnosticsResponse, error) {
	return c.diagnosticsClient.Debug(ctx, in, opts...)
}

func (c *client) Close() {
	c.rpcClient.Close()
}

// GetClientGetter returns an implementation of ClientGetter
func GetClientGetter(name, svcURL, remoteServer string) ClientGetter {
	var clgetter ClientGetterFunc
	clgetter = func() (Client, error) {
		svccl, err := rpckit.NewRPCClient(name, svcURL, rpckit.WithRemoteServerName(remoteServer))
		if err != nil {
			return nil, err
		}
		return &client{
			rpcClient:         svccl,
			diagnosticsClient: protos.NewDiagnosticsClient(svccl.ClientConn),
		}, nil
	}
	return clgetter
}
