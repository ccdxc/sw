// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

package grpcclient

import (
	"context"
	"errors"
	oldlog "log"

	"github.com/go-kit/kit/endpoint"
	grpctransport "github.com/go-kit/kit/transport/grpc"
	"google.golang.org/grpc"

	api "github.com/pensando/sw/api"
	x509 "github.com/pensando/sw/api/generated/x509"
	listerwatcher "github.com/pensando/sw/api/listerwatcher"
	apiserver "github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/trace"
)

// Dummy vars to suppress import errors
var _ api.TypeMeta
var _ listerwatcher.WatcherClient
var _ kvstore.Interface

// NewCertificateV1 sets up a new client for CertificateV1
func NewCertificateV1(conn *grpc.ClientConn, logger log.Logger) x509.ServiceCertificateV1Client {

	var lAutoAddCertificateEndpoint endpoint.Endpoint
	{
		lAutoAddCertificateEndpoint = grpctransport.NewClient(
			conn,
			"x509.CertificateV1",
			"AutoAddCertificate",
			x509.EncodeGrpcReqCertificate,
			x509.DecodeGrpcRespCertificate,
			&x509.Certificate{},
			grpctransport.ClientBefore(trace.ToGRPCRequest(logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		lAutoAddCertificateEndpoint = trace.ClientEndPoint("CertificateV1:AutoAddCertificate")(lAutoAddCertificateEndpoint)
	}
	var lAutoDeleteCertificateEndpoint endpoint.Endpoint
	{
		lAutoDeleteCertificateEndpoint = grpctransport.NewClient(
			conn,
			"x509.CertificateV1",
			"AutoDeleteCertificate",
			x509.EncodeGrpcReqCertificate,
			x509.DecodeGrpcRespCertificate,
			&x509.Certificate{},
			grpctransport.ClientBefore(trace.ToGRPCRequest(logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		lAutoDeleteCertificateEndpoint = trace.ClientEndPoint("CertificateV1:AutoDeleteCertificate")(lAutoDeleteCertificateEndpoint)
	}
	var lAutoGetCertificateEndpoint endpoint.Endpoint
	{
		lAutoGetCertificateEndpoint = grpctransport.NewClient(
			conn,
			"x509.CertificateV1",
			"AutoGetCertificate",
			x509.EncodeGrpcReqCertificate,
			x509.DecodeGrpcRespCertificate,
			&x509.Certificate{},
			grpctransport.ClientBefore(trace.ToGRPCRequest(logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		lAutoGetCertificateEndpoint = trace.ClientEndPoint("CertificateV1:AutoGetCertificate")(lAutoGetCertificateEndpoint)
	}
	var lAutoListCertificateEndpoint endpoint.Endpoint
	{
		lAutoListCertificateEndpoint = grpctransport.NewClient(
			conn,
			"x509.CertificateV1",
			"AutoListCertificate",
			x509.EncodeGrpcReqListWatchOptions,
			x509.DecodeGrpcRespCertificateList,
			&x509.CertificateList{},
			grpctransport.ClientBefore(trace.ToGRPCRequest(logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		lAutoListCertificateEndpoint = trace.ClientEndPoint("CertificateV1:AutoListCertificate")(lAutoListCertificateEndpoint)
	}
	var lAutoUpdateCertificateEndpoint endpoint.Endpoint
	{
		lAutoUpdateCertificateEndpoint = grpctransport.NewClient(
			conn,
			"x509.CertificateV1",
			"AutoUpdateCertificate",
			x509.EncodeGrpcReqCertificate,
			x509.DecodeGrpcRespCertificate,
			&x509.Certificate{},
			grpctransport.ClientBefore(trace.ToGRPCRequest(logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		lAutoUpdateCertificateEndpoint = trace.ClientEndPoint("CertificateV1:AutoUpdateCertificate")(lAutoUpdateCertificateEndpoint)
	}
	return x509.EndpointsCertificateV1Client{
		Client: x509.NewCertificateV1Client(conn),

		AutoAddCertificateEndpoint:    lAutoAddCertificateEndpoint,
		AutoDeleteCertificateEndpoint: lAutoDeleteCertificateEndpoint,
		AutoGetCertificateEndpoint:    lAutoGetCertificateEndpoint,
		AutoListCertificateEndpoint:   lAutoListCertificateEndpoint,
		AutoUpdateCertificateEndpoint: lAutoUpdateCertificateEndpoint,
	}
}

// NewCertificateV1Backend creates an instrumented client with middleware
func NewCertificateV1Backend(conn *grpc.ClientConn, logger log.Logger) x509.ServiceCertificateV1Client {
	cl := NewCertificateV1(conn, logger)
	cl = x509.LoggingCertificateV1MiddlewareClient(logger)(cl)
	return cl
}

type grpcObjCertificateV1Certificate struct {
	logger log.Logger
	client x509.ServiceCertificateV1Client
}

func (a *grpcObjCertificateV1Certificate) Create(ctx context.Context, in *x509.Certificate) (*x509.Certificate, error) {
	a.logger.DebugLog("msg", "received call", "object", "Certificate", "oper", "create")
	if in == nil {
		return nil, errors.New("invalid input")
	}
	nctx := addVersion(ctx, "v1")
	return a.client.AutoAddCertificate(nctx, in)
}

func (a *grpcObjCertificateV1Certificate) Update(ctx context.Context, in *x509.Certificate) (*x509.Certificate, error) {
	a.logger.DebugLog("msg", "received call", "object", "Certificate", "oper", "update")
	if in == nil {
		return nil, errors.New("invalid input")
	}
	nctx := addVersion(ctx, "v1")
	return a.client.AutoUpdateCertificate(nctx, in)
}

func (a *grpcObjCertificateV1Certificate) Get(ctx context.Context, objMeta *api.ObjectMeta) (*x509.Certificate, error) {
	a.logger.DebugLog("msg", "received call", "object", "Certificate", "oper", "get")
	if objMeta == nil {
		return nil, errors.New("invalid input")
	}
	in := x509.Certificate{}
	in.ObjectMeta = *objMeta
	nctx := addVersion(ctx, "v1")
	return a.client.AutoGetCertificate(nctx, &in)
}

func (a *grpcObjCertificateV1Certificate) Delete(ctx context.Context, objMeta *api.ObjectMeta) (*x509.Certificate, error) {
	a.logger.DebugLog("msg", "received call", "object", "Certificate", "oper", "delete")
	if objMeta == nil {
		return nil, errors.New("invalid input")
	}
	in := x509.Certificate{}
	in.ObjectMeta = *objMeta
	nctx := addVersion(ctx, "v1")
	return a.client.AutoDeleteCertificate(nctx, &in)
}

func (a *grpcObjCertificateV1Certificate) List(ctx context.Context, options *api.ListWatchOptions) ([]*x509.Certificate, error) {
	a.logger.DebugLog("msg", "received call", "object", "Certificate", "oper", "list")
	if options == nil {
		return nil, errors.New("invalid input")
	}
	nctx := addVersion(ctx, "v1")
	r, err := a.client.AutoListCertificate(nctx, options)
	if err == nil {
		return r.Items, nil
	}
	return nil, err
}

func (a *grpcObjCertificateV1Certificate) Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error) {
	a.logger.DebugLog("msg", "received call", "object", "Certificate", "oper", "WatchOper")
	nctx := addVersion(ctx, "v1")
	if options == nil {
		return nil, errors.New("invalid input")
	}
	stream, err := a.client.AutoWatchCertificate(nctx, options)
	if err != nil {
		return nil, err
	}
	wstream := stream.(x509.CertificateV1_AutoWatchCertificateClient)
	bridgefn := func(lw *listerwatcher.WatcherClient) {
		for {
			r, err := wstream.Recv()
			if err != nil {
				a.logger.ErrorLog("msg", "error on receive", "error", err)
				close(lw.OutCh)
				return
			}
			ev := kvstore.WatchEvent{
				Type:   kvstore.WatchEventType(r.Type),
				Object: r.Object,
			}
			select {
			case lw.OutCh <- &ev:
			case <-wstream.Context().Done():
				close(lw.OutCh)
				return
			}
		}
	}
	lw := listerwatcher.NewWatcherClient(wstream, bridgefn)
	lw.Run()
	return lw, nil
}

func (a *grpcObjCertificateV1Certificate) Allowed(oper apiserver.APIOperType) bool {
	return true
}

type restObjCertificateV1Certificate struct {
	endpoints x509.EndpointsCertificateV1RestClient
	instance  string
}

func (a *restObjCertificateV1Certificate) Create(ctx context.Context, in *x509.Certificate) (*x509.Certificate, error) {
	if in == nil {
		return nil, errors.New("invalid input")
	}
	return a.endpoints.AutoAddCertificate(ctx, in)
}

func (a *restObjCertificateV1Certificate) Update(ctx context.Context, in *x509.Certificate) (*x509.Certificate, error) {
	if in == nil {
		return nil, errors.New("invalid input")
	}
	return a.endpoints.AutoUpdateCertificate(ctx, in)
}

func (a *restObjCertificateV1Certificate) Get(ctx context.Context, objMeta *api.ObjectMeta) (*x509.Certificate, error) {
	if objMeta == nil {
		return nil, errors.New("invalid input")
	}
	in := x509.Certificate{}
	in.ObjectMeta = *objMeta
	return a.endpoints.AutoGetCertificate(ctx, &in)
}

func (a *restObjCertificateV1Certificate) Delete(ctx context.Context, objMeta *api.ObjectMeta) (*x509.Certificate, error) {
	if objMeta == nil {
		return nil, errors.New("invalid input")
	}
	in := x509.Certificate{}
	in.ObjectMeta = *objMeta
	return a.endpoints.AutoDeleteCertificate(ctx, &in)
}

func (a *restObjCertificateV1Certificate) List(ctx context.Context, options *api.ListWatchOptions) ([]*x509.Certificate, error) {
	if options == nil {
		return nil, errors.New("invalid input")
	}
	r, err := a.endpoints.AutoListCertificate(ctx, options)
	if err == nil {
		return r.Items, nil
	}
	return nil, err
}

func (a *restObjCertificateV1Certificate) Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error) {
	return nil, errors.New("not allowed")
}

func (a *restObjCertificateV1Certificate) Allowed(oper apiserver.APIOperType) bool {
	switch oper {
	case apiserver.CreateOper:
		return true
	case apiserver.UpdateOper:
		return true
	case apiserver.GetOper:
		return true
	case apiserver.DeleteOper:
		return true
	case apiserver.ListOper:
		return true
	case apiserver.WatchOper:
		return false
	default:
		return false
	}
}

type crudClientCertificateV1 struct {
	grpcCertificate x509.CertificateV1CertificateInterface
}

// NewGrpcCrudClientCertificateV1 creates a GRPC client for the service
func NewGrpcCrudClientCertificateV1(conn *grpc.ClientConn, logger log.Logger) x509.CertificateV1Interface {
	client := NewCertificateV1Backend(conn, logger)
	return &crudClientCertificateV1{

		grpcCertificate: &grpcObjCertificateV1Certificate{client: client, logger: logger},
	}
}

func (a *crudClientCertificateV1) Certificate() x509.CertificateV1CertificateInterface {
	return a.grpcCertificate
}

type crudRestClientCertificateV1 struct {
	restCertificate x509.CertificateV1CertificateInterface
}

// NewRestCrudClientCertificateV1 creates a REST client for the service.
func NewRestCrudClientCertificateV1(url string) x509.CertificateV1Interface {
	endpoints, err := x509.MakeCertificateV1RestClientEndpoints(url)
	if err != nil {
		oldlog.Fatal("failed to create client")
	}
	return &crudRestClientCertificateV1{

		restCertificate: &restObjCertificateV1Certificate{endpoints: endpoints, instance: url},
	}
}

func (a *crudRestClientCertificateV1) Certificate() x509.CertificateV1CertificateInterface {
	return a.restCertificate
}
