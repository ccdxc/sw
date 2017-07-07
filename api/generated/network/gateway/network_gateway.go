/*
Package network is a auto generated package.
Input file: protos/network.proto
*/
package networkGwService

import (
	"context"
	"net/http"
	"time"

	"github.com/GeertJohan/go.rice"
	gogocodec "github.com/gogo/protobuf/codec"
	"github.com/pensando/grpc-gateway/runtime"
	network "github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/network/grpc/client"
	"github.com/pensando/sw/apigw/pkg"
	"github.com/pensando/sw/utils/log"
	"github.com/pkg/errors"
	oldcontext "golang.org/x/net/context"
	"google.golang.org/grpc"
)

const codecSize = 1024 * 1024

type s_TenantV1GwService struct {
	logger log.Logger
}

type adapterTenantV1 struct {
	service network.ServiceTenantV1
}

func (a adapterTenantV1) GetTenantList(oldctx oldcontext.Context, t *network.TenantList, options ...grpc.CallOption) (*network.TenantList, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.GetTenantList(ctx, *t)
	return &r, e
}

func (a adapterTenantV1) TenantOper(oldctx oldcontext.Context, t *network.Tenant, options ...grpc.CallOption) (*network.Tenant, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.TenantOper(ctx, *t)
	return &r, e
}

func (e *s_TenantV1GwService) CompleteRegistration(ctx context.Context,
	logger log.Logger,
	grpcserver *grpc.Server,
	m *http.ServeMux) error {
	// IP:port destination or service discovery key.

	grpcaddr := "localhost:8082"
	e.logger = logger
	codec := gogocodec.New(codecSize)
	cl, err := e.newClient(ctx, grpcaddr, grpc.WithInsecure(), grpc.WithTimeout(time.Second), grpc.WithCodec(codec))
	if cl == nil || err != nil {
		err = errors.Wrap(err, "could not create client")
		return err
	}
	marshaller := runtime.JSONBuiltin{}
	opts := runtime.WithMarshalerOption("*", &marshaller)
	mux := runtime.NewServeMux(opts)
	err = network.RegisterTenantV1HandlerWithClient(ctx, mux, cl)
	if err != nil {
		err = errors.Wrap(err, "service registration failed")
		return err
	}
	logger.InfoLog("msg", "registered service network.TenantV1")
	m.Handle("/v1/tenant/", http.StripPrefix("/v1/tenant", mux))
	err = registerSwaggerDef(m, logger)
	return err
}

func (e *s_TenantV1GwService) newClient(ctx context.Context, grpcAddr string, opts ...grpc.DialOption) (network.TenantV1Client, error) {
	conn, err := grpc.Dial(grpcAddr, opts...)
	if err != nil {
		err = errors.Wrap(err, "dial failed")
		if cerr := conn.Close(); cerr != nil {
			e.logger.ErrorLog("msg", "Failed to close conn", "addr", grpcAddr, "error", cerr)
		}
		return nil, err
	}
	e.logger.Infof("Connected to GRPC Server", grpcAddr)
	defer func() {
		go func() {
			<-ctx.Done()
			if cerr := conn.Close(); cerr != nil {
				e.logger.ErrorLog("msg", "Failed to close conn on Done()", "addr", grpcAddr, "error", cerr)
			}
		}()
	}()

	cl := adapterTenantV1{grpcclient.NewTenantV1Backend(conn, e.logger)}
	return cl, nil
}

type s_NetworkV1GwService struct {
	logger log.Logger
}

type adapterNetworkV1 struct {
	service network.ServiceNetworkV1
}

func (a adapterNetworkV1) GetNetworkList(oldctx oldcontext.Context, t *network.NetworkList, options ...grpc.CallOption) (*network.NetworkList, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.GetNetworkList(ctx, *t)
	return &r, e
}

func (a adapterNetworkV1) NetworkOper(oldctx oldcontext.Context, t *network.Network, options ...grpc.CallOption) (*network.Network, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.NetworkOper(ctx, *t)
	return &r, e
}

func (e *s_NetworkV1GwService) CompleteRegistration(ctx context.Context,
	logger log.Logger,
	grpcserver *grpc.Server,
	m *http.ServeMux) error {
	// IP:port destination or service discovery key.

	grpcaddr := "localhost:8082"
	e.logger = logger
	codec := gogocodec.New(codecSize)
	cl, err := e.newClient(ctx, grpcaddr, grpc.WithInsecure(), grpc.WithTimeout(time.Second), grpc.WithCodec(codec))
	if cl == nil || err != nil {
		err = errors.Wrap(err, "could not create client")
		return err
	}
	marshaller := runtime.JSONBuiltin{}
	opts := runtime.WithMarshalerOption("*", &marshaller)
	mux := runtime.NewServeMux(opts)
	err = network.RegisterNetworkV1HandlerWithClient(ctx, mux, cl)
	if err != nil {
		err = errors.Wrap(err, "service registration failed")
		return err
	}
	logger.InfoLog("msg", "registered service network.NetworkV1")
	m.Handle("/v1/network/", http.StripPrefix("/v1/network", mux))

	return err
}

func (e *s_NetworkV1GwService) newClient(ctx context.Context, grpcAddr string, opts ...grpc.DialOption) (network.NetworkV1Client, error) {
	conn, err := grpc.Dial(grpcAddr, opts...)
	if err != nil {
		err = errors.Wrap(err, "dial failed")
		if cerr := conn.Close(); cerr != nil {
			e.logger.ErrorLog("msg", "Failed to close conn", "addr", grpcAddr, "error", cerr)
		}
		return nil, err
	}
	e.logger.Infof("Connected to GRPC Server", grpcAddr)
	defer func() {
		go func() {
			<-ctx.Done()
			if cerr := conn.Close(); cerr != nil {
				e.logger.ErrorLog("msg", "Failed to close conn on Done()", "addr", grpcAddr, "error", cerr)
			}
		}()
	}()

	cl := adapterNetworkV1{grpcclient.NewNetworkV1Backend(conn, e.logger)}
	return cl, nil
}

type s_SecurityGroupV1GwService struct {
	logger log.Logger
}

type adapterSecurityGroupV1 struct {
	service network.ServiceSecurityGroupV1
}

func (a adapterSecurityGroupV1) GetSecurityGroupList(oldctx oldcontext.Context, t *network.SecurityGroupList, options ...grpc.CallOption) (*network.SecurityGroupList, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.GetSecurityGroupList(ctx, *t)
	return &r, e
}

func (a adapterSecurityGroupV1) SecurityGroupOper(oldctx oldcontext.Context, t *network.SecurityGroup, options ...grpc.CallOption) (*network.SecurityGroup, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.SecurityGroupOper(ctx, *t)
	return &r, e
}

func (e *s_SecurityGroupV1GwService) CompleteRegistration(ctx context.Context,
	logger log.Logger,
	grpcserver *grpc.Server,
	m *http.ServeMux) error {
	// IP:port destination or service discovery key.

	grpcaddr := "localhost:8082"
	e.logger = logger
	codec := gogocodec.New(codecSize)
	cl, err := e.newClient(ctx, grpcaddr, grpc.WithInsecure(), grpc.WithTimeout(time.Second), grpc.WithCodec(codec))
	if cl == nil || err != nil {
		err = errors.Wrap(err, "could not create client")
		return err
	}
	marshaller := runtime.JSONBuiltin{}
	opts := runtime.WithMarshalerOption("*", &marshaller)
	mux := runtime.NewServeMux(opts)
	err = network.RegisterSecurityGroupV1HandlerWithClient(ctx, mux, cl)
	if err != nil {
		err = errors.Wrap(err, "service registration failed")
		return err
	}
	logger.InfoLog("msg", "registered service network.SecurityGroupV1")
	m.Handle("/v1/security-group/", http.StripPrefix("/v1/security-group", mux))

	return err
}

func (e *s_SecurityGroupV1GwService) newClient(ctx context.Context, grpcAddr string, opts ...grpc.DialOption) (network.SecurityGroupV1Client, error) {
	conn, err := grpc.Dial(grpcAddr, opts...)
	if err != nil {
		err = errors.Wrap(err, "dial failed")
		if cerr := conn.Close(); cerr != nil {
			e.logger.ErrorLog("msg", "Failed to close conn", "addr", grpcAddr, "error", cerr)
		}
		return nil, err
	}
	e.logger.Infof("Connected to GRPC Server", grpcAddr)
	defer func() {
		go func() {
			<-ctx.Done()
			if cerr := conn.Close(); cerr != nil {
				e.logger.ErrorLog("msg", "Failed to close conn on Done()", "addr", grpcAddr, "error", cerr)
			}
		}()
	}()

	cl := adapterSecurityGroupV1{grpcclient.NewSecurityGroupV1Backend(conn, e.logger)}
	return cl, nil
}

type s_SgpolicyV1GwService struct {
	logger log.Logger
}

type adapterSgpolicyV1 struct {
	service network.ServiceSgpolicyV1
}

func (a adapterSgpolicyV1) GetSgpolicyList(oldctx oldcontext.Context, t *network.SgpolicyList, options ...grpc.CallOption) (*network.SgpolicyList, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.GetSgpolicyList(ctx, *t)
	return &r, e
}

func (a adapterSgpolicyV1) SgpolicyOper(oldctx oldcontext.Context, t *network.Sgpolicy, options ...grpc.CallOption) (*network.Sgpolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.SgpolicyOper(ctx, *t)
	return &r, e
}

func (e *s_SgpolicyV1GwService) CompleteRegistration(ctx context.Context,
	logger log.Logger,
	grpcserver *grpc.Server,
	m *http.ServeMux) error {
	// IP:port destination or service discovery key.

	grpcaddr := "localhost:8082"
	e.logger = logger
	codec := gogocodec.New(codecSize)
	cl, err := e.newClient(ctx, grpcaddr, grpc.WithInsecure(), grpc.WithTimeout(time.Second), grpc.WithCodec(codec))
	if cl == nil || err != nil {
		err = errors.Wrap(err, "could not create client")
		return err
	}
	marshaller := runtime.JSONBuiltin{}
	opts := runtime.WithMarshalerOption("*", &marshaller)
	mux := runtime.NewServeMux(opts)
	err = network.RegisterSgpolicyV1HandlerWithClient(ctx, mux, cl)
	if err != nil {
		err = errors.Wrap(err, "service registration failed")
		return err
	}
	logger.InfoLog("msg", "registered service network.SgpolicyV1")
	m.Handle("/v1/sgpolicy/", http.StripPrefix("/v1/sgpolicy", mux))

	return err
}

func (e *s_SgpolicyV1GwService) newClient(ctx context.Context, grpcAddr string, opts ...grpc.DialOption) (network.SgpolicyV1Client, error) {
	conn, err := grpc.Dial(grpcAddr, opts...)
	if err != nil {
		err = errors.Wrap(err, "dial failed")
		if cerr := conn.Close(); cerr != nil {
			e.logger.ErrorLog("msg", "Failed to close conn", "addr", grpcAddr, "error", cerr)
		}
		return nil, err
	}
	e.logger.Infof("Connected to GRPC Server", grpcAddr)
	defer func() {
		go func() {
			<-ctx.Done()
			if cerr := conn.Close(); cerr != nil {
				e.logger.ErrorLog("msg", "Failed to close conn on Done()", "addr", grpcAddr, "error", cerr)
			}
		}()
	}()

	cl := adapterSgpolicyV1{grpcclient.NewSgpolicyV1Backend(conn, e.logger)}
	return cl, nil
}

type s_ServiceV1GwService struct {
	logger log.Logger
}

type adapterServiceV1 struct {
	service network.ServiceServiceV1
}

func (a adapterServiceV1) GetServiceList(oldctx oldcontext.Context, t *network.ServiceList, options ...grpc.CallOption) (*network.ServiceList, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.GetServiceList(ctx, *t)
	return &r, e
}

func (a adapterServiceV1) ServiceOper(oldctx oldcontext.Context, t *network.Service, options ...grpc.CallOption) (*network.Service, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.ServiceOper(ctx, *t)
	return &r, e
}

func (e *s_ServiceV1GwService) CompleteRegistration(ctx context.Context,
	logger log.Logger,
	grpcserver *grpc.Server,
	m *http.ServeMux) error {
	// IP:port destination or service discovery key.

	grpcaddr := "localhost:8082"
	e.logger = logger
	codec := gogocodec.New(codecSize)
	cl, err := e.newClient(ctx, grpcaddr, grpc.WithInsecure(), grpc.WithTimeout(time.Second), grpc.WithCodec(codec))
	if cl == nil || err != nil {
		err = errors.Wrap(err, "could not create client")
		return err
	}
	marshaller := runtime.JSONBuiltin{}
	opts := runtime.WithMarshalerOption("*", &marshaller)
	mux := runtime.NewServeMux(opts)
	err = network.RegisterServiceV1HandlerWithClient(ctx, mux, cl)
	if err != nil {
		err = errors.Wrap(err, "service registration failed")
		return err
	}
	logger.InfoLog("msg", "registered service network.ServiceV1")
	m.Handle("/v1/service/", http.StripPrefix("/v1/service", mux))

	return err
}

func (e *s_ServiceV1GwService) newClient(ctx context.Context, grpcAddr string, opts ...grpc.DialOption) (network.ServiceV1Client, error) {
	conn, err := grpc.Dial(grpcAddr, opts...)
	if err != nil {
		err = errors.Wrap(err, "dial failed")
		if cerr := conn.Close(); cerr != nil {
			e.logger.ErrorLog("msg", "Failed to close conn", "addr", grpcAddr, "error", cerr)
		}
		return nil, err
	}
	e.logger.Infof("Connected to GRPC Server", grpcAddr)
	defer func() {
		go func() {
			<-ctx.Done()
			if cerr := conn.Close(); cerr != nil {
				e.logger.ErrorLog("msg", "Failed to close conn on Done()", "addr", grpcAddr, "error", cerr)
			}
		}()
	}()

	cl := adapterServiceV1{grpcclient.NewServiceV1Backend(conn, e.logger)}
	return cl, nil
}

type s_LbPolicyV1GwService struct {
	logger log.Logger
}

type adapterLbPolicyV1 struct {
	service network.ServiceLbPolicyV1
}

func (a adapterLbPolicyV1) GetLbPolicyList(oldctx oldcontext.Context, t *network.LbPolicyList, options ...grpc.CallOption) (*network.LbPolicyList, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.GetLbPolicyList(ctx, *t)
	return &r, e
}

func (a adapterLbPolicyV1) LbPolicyOper(oldctx oldcontext.Context, t *network.LbPolicy, options ...grpc.CallOption) (*network.LbPolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.LbPolicyOper(ctx, *t)
	return &r, e
}

func (e *s_LbPolicyV1GwService) CompleteRegistration(ctx context.Context,
	logger log.Logger,
	grpcserver *grpc.Server,
	m *http.ServeMux) error {
	// IP:port destination or service discovery key.

	grpcaddr := "localhost:8082"
	e.logger = logger
	codec := gogocodec.New(codecSize)
	cl, err := e.newClient(ctx, grpcaddr, grpc.WithInsecure(), grpc.WithTimeout(time.Second), grpc.WithCodec(codec))
	if cl == nil || err != nil {
		err = errors.Wrap(err, "could not create client")
		return err
	}
	marshaller := runtime.JSONBuiltin{}
	opts := runtime.WithMarshalerOption("*", &marshaller)
	mux := runtime.NewServeMux(opts)
	err = network.RegisterLbPolicyV1HandlerWithClient(ctx, mux, cl)
	if err != nil {
		err = errors.Wrap(err, "service registration failed")
		return err
	}
	logger.InfoLog("msg", "registered service network.LbPolicyV1")
	m.Handle("/v1/lb-policy/", http.StripPrefix("/v1/lb-policy", mux))

	return err
}

func (e *s_LbPolicyV1GwService) newClient(ctx context.Context, grpcAddr string, opts ...grpc.DialOption) (network.LbPolicyV1Client, error) {
	conn, err := grpc.Dial(grpcAddr, opts...)
	if err != nil {
		err = errors.Wrap(err, "dial failed")
		if cerr := conn.Close(); cerr != nil {
			e.logger.ErrorLog("msg", "Failed to close conn", "addr", grpcAddr, "error", cerr)
		}
		return nil, err
	}
	e.logger.Infof("Connected to GRPC Server", grpcAddr)
	defer func() {
		go func() {
			<-ctx.Done()
			if cerr := conn.Close(); cerr != nil {
				e.logger.ErrorLog("msg", "Failed to close conn on Done()", "addr", grpcAddr, "error", cerr)
			}
		}()
	}()

	cl := adapterLbPolicyV1{grpcclient.NewLbPolicyV1Backend(conn, e.logger)}
	return cl, nil
}

type s_EndpointV1GwService struct {
	logger log.Logger
}

type adapterEndpointV1 struct {
	service network.ServiceEndpointV1
}

func (a adapterEndpointV1) GetEndpointList(oldctx oldcontext.Context, t *network.EndpointList, options ...grpc.CallOption) (*network.EndpointList, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.GetEndpointList(ctx, *t)
	return &r, e
}

func (a adapterEndpointV1) EndpointOper(oldctx oldcontext.Context, t *network.Endpoint, options ...grpc.CallOption) (*network.Endpoint, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.EndpointOper(ctx, *t)
	return &r, e
}

func (e *s_EndpointV1GwService) CompleteRegistration(ctx context.Context,
	logger log.Logger,
	grpcserver *grpc.Server,
	m *http.ServeMux) error {
	// IP:port destination or service discovery key.

	grpcaddr := "localhost:8082"
	e.logger = logger
	codec := gogocodec.New(codecSize)
	cl, err := e.newClient(ctx, grpcaddr, grpc.WithInsecure(), grpc.WithTimeout(time.Second), grpc.WithCodec(codec))
	if cl == nil || err != nil {
		err = errors.Wrap(err, "could not create client")
		return err
	}
	marshaller := runtime.JSONBuiltin{}
	opts := runtime.WithMarshalerOption("*", &marshaller)
	mux := runtime.NewServeMux(opts)
	err = network.RegisterEndpointV1HandlerWithClient(ctx, mux, cl)
	if err != nil {
		err = errors.Wrap(err, "service registration failed")
		return err
	}
	logger.InfoLog("msg", "registered service network.EndpointV1")
	m.Handle("/v1/endpoint/", http.StripPrefix("/v1/endpoint", mux))

	return err
}

func (e *s_EndpointV1GwService) newClient(ctx context.Context, grpcAddr string, opts ...grpc.DialOption) (network.EndpointV1Client, error) {
	conn, err := grpc.Dial(grpcAddr, opts...)
	if err != nil {
		err = errors.Wrap(err, "dial failed")
		if cerr := conn.Close(); cerr != nil {
			e.logger.ErrorLog("msg", "Failed to close conn", "addr", grpcAddr, "error", cerr)
		}
		return nil, err
	}
	e.logger.Infof("Connected to GRPC Server", grpcAddr)
	defer func() {
		go func() {
			<-ctx.Done()
			if cerr := conn.Close(); cerr != nil {
				e.logger.ErrorLog("msg", "Failed to close conn on Done()", "addr", grpcAddr, "error", cerr)
			}
		}()
	}()

	cl := adapterEndpointV1{grpcclient.NewEndpointV1Backend(conn, e.logger)}
	return cl, nil
}

func registerSwaggerDef(m *http.ServeMux, logger log.Logger) error {
	box, err := rice.FindBox("../../../../../sw/api/generated/network/swagger")
	if err != nil {
		err = errors.Wrap(err, "error opening rice.Box")
		return err
	}
	content, err := box.Bytes("network.swagger.json")
	if err != nil {
		err = errors.Wrap(err, "error opening rice.File")
		return err
	}
	m.HandleFunc("/swagger/network/", func(w http.ResponseWriter, r *http.Request) {
		w.Write(content)
	})
	return nil
}

func init() {
	apigw := apigwpkg.MustGetAPIGateway()

	svcTenantV1 := s_TenantV1GwService{}
	apigw.Register("network.TenantV1", "tenant/", &svcTenantV1)
	svcNetworkV1 := s_NetworkV1GwService{}
	apigw.Register("network.NetworkV1", "network/", &svcNetworkV1)
	svcSecurityGroupV1 := s_SecurityGroupV1GwService{}
	apigw.Register("network.SecurityGroupV1", "security-group/", &svcSecurityGroupV1)
	svcSgpolicyV1 := s_SgpolicyV1GwService{}
	apigw.Register("network.SgpolicyV1", "sgpolicy/", &svcSgpolicyV1)
	svcServiceV1 := s_ServiceV1GwService{}
	apigw.Register("network.ServiceV1", "service/", &svcServiceV1)
	svcLbPolicyV1 := s_LbPolicyV1GwService{}
	apigw.Register("network.LbPolicyV1", "lb-policy/", &svcLbPolicyV1)
	svcEndpointV1 := s_EndpointV1GwService{}
	apigw.Register("network.EndpointV1", "endpoint/", &svcEndpointV1)
}
