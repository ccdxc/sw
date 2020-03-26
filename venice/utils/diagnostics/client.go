package diagnostics

import (
	"context"
	"crypto/tls"
	"net/http"
	"sync"

	"github.com/pensando/sw/venice/utils/tokenauth/readutils"

	"github.com/gogo/protobuf/types"

	"google.golang.org/grpc"

	"github.com/pensando/sw/api"
	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	tokenapi "github.com/pensando/sw/api/generated/tokenauth"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/diagnostics/protos"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

type client struct {
	rpcClient         *rpckit.RPCClient
	diagnosticsClient protos.DiagnosticsClient
	router            Router
}

func (c *client) Debug(ctx context.Context, in *diagapi.DiagnosticsRequest, opts ...grpc.CallOption) (*diagapi.DiagnosticsResponse, error) {
	return c.diagnosticsClient.Debug(ctx, in, opts...)
}

func (c *client) Close() {
	c.rpcClient.Close()
}

// apigwClient to handle API Gateway Debug requests
type apigwClient struct {
	service Service
}

func (ac *apigwClient) Debug(ctx context.Context, in *diagapi.DiagnosticsRequest, opts ...grpc.CallOption) (*diagapi.DiagnosticsResponse, error) {
	return ac.service.Debug(ctx, in)
}

func (ac *apigwClient) Close() {}

type naplesClient struct {
	svcURL     string
	httpClient *naplesHTTPClient
}

func (nc naplesClient) Debug(ctx context.Context, in *diagapi.DiagnosticsRequest, opts ...grpc.CallOption) (*diagapi.DiagnosticsResponse, error) {
	return nc.httpClient.Debug(ctx, nc.svcURL, in, opts...)
}

func (nc *naplesClient) Close() {}

func newNaplesClient(svcURL, name string, rslvr resolver.Interface) (Client, error) {
	client := &naplesClient{
		svcURL: svcURL,
	}
	client.httpClient = getNaplesHTTPClient(name, rslvr)
	return client, nil
}

var naplesOnce sync.Once
var gNaplesHTTPClient *naplesHTTPClient

type naplesHTTPClient struct {
	sync.RWMutex
	name       string
	rslvr      resolver.Interface
	httpClient *netutils.HTTPClient
}

func (nc *naplesHTTPClient) Debug(ctx context.Context, svcURL string, in *diagapi.DiagnosticsRequest, opts ...grpc.CallOption) (*diagapi.DiagnosticsResponse, error) {
	if nc.httpClient == nil {
		httpClient, err := getHTTPClient(nc.name, nc.rslvr)
		if err != nil {
			return nil, err
		}
		nc.Lock()
		nc.httpClient = httpClient
		nc.Unlock()
	}
	out := &diagapi.DiagnosticsResponse{}
	defer nc.RUnlock()
	nc.RLock()
	b, code, err := nc.httpClient.ReqRaw("POST", svcURL, in)
	if err != nil {
		return nil, err
	}
	if code != http.StatusOK {
		return nil, &api.Status{
			TypeMeta: api.TypeMeta{Kind: "Status"},
			Code:     int32(code),
			Message:  []string{string(b)},
		}
	}
	anyObj, err := types.MarshalAny(&api.Interface{Value: &api.Interface_Str{Str: string(b)}})
	out.Object = &api.Any{Any: *anyObj}
	return out, nil
}

func getNaplesHTTPClient(name string, rslvr resolver.Interface) *naplesHTTPClient {
	naplesOnce.Do(func() {
		gNaplesHTTPClient = &naplesHTTPClient{
			name:  name,
			rslvr: rslvr,
		}
		gNaplesHTTPClient.httpClient, _ = getHTTPClient(name, rslvr)
	})
	return gNaplesHTTPClient
}

func getHTTPClient(name string, rslvr resolver.Interface) (*netutils.HTTPClient, error) {
	var opts []rpckit.Option
	opts = append(opts, rpckit.WithTLSClientIdentity(name))
	if rslvr != nil {
		opts = append(opts, rpckit.WithBalancer(balancer.New(rslvr)))
	} else {
		opts = append(opts, rpckit.WithRemoteServerName(globals.Cmd))
	}

	rpcClient, err := rpckit.NewRPCClient(name, globals.Cmd, opts...)
	if err != nil {
		return nil, err
	}
	defer rpcClient.Close()
	tokenClient := tokenapi.NewTokenAuthV1Client(rpcClient.ClientConn)
	resp, err := tokenClient.GenerateNodeToken(context.Background(), &tokenapi.NodeTokenRequest{Audience: []string{"*"}})
	if err != nil {
		return nil, err
	}
	tlsCert, err := readutils.ParseNodeToken(resp.Token)
	if err != nil {
		return nil, err
	}
	httpClient := netutils.NewHTTPClient()
	httpClient.WithTLSConfig(&tls.Config{
		InsecureSkipVerify: true, // do not check agent's certificate
		Certificates:       []tls.Certificate{tlsCert},
	})
	return httpClient, nil
}

// NewClientGetter returns an implementation of ClientGetter
func NewClientGetter(name string, diagRequest *diagapi.DiagnosticsRequest, router Router, diagSvc Service, rslvr resolver.Interface) (ClientGetter, error) {
	var clgetter ClientGetterFunc
	svcURL, remoteServer, err := router.GetRoute(diagRequest)
	if err != nil {
		return nil, err
	}
	switch remoteServer {
	case globals.APIGw:
		clgetter = func() (Client, error) {
			return &apigwClient{service: diagSvc}, nil
		}
	case diagapi.ModuleStatus_Naples.String():
		clgetter = func() (Client, error) {
			return newNaplesClient(svcURL, name, rslvr)
		}
	default:
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
	}
	return clgetter, nil
}
