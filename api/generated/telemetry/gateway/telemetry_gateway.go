// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package telemetryGwService is a auto generated package.
Input file: protos/telemetry.proto
*/
package telemetryGwService

import (
	"context"
	"net/http"

	"github.com/pkg/errors"
	oldcontext "golang.org/x/net/context"
	"google.golang.org/grpc"

	"github.com/pensando/grpc-gateway/runtime"

	"github.com/pensando/sw/api"
	telemetry "github.com/pensando/sw/api/generated/telemetry"
	"github.com/pensando/sw/api/generated/telemetry/grpc/client"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// Dummy vars to suppress import errors
var _ api.TypeMeta

type sFlowExportPolicyV1GwService struct {
	logger log.Logger
}

type adapterFlowExportPolicyV1 struct {
	service telemetry.ServiceFlowExportPolicyV1Client
}

func (a adapterFlowExportPolicyV1) AutoAddFlowExportPolicy(oldctx oldcontext.Context, t *telemetry.FlowExportPolicy, options ...grpc.CallOption) (*telemetry.FlowExportPolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	return a.service.AutoAddFlowExportPolicy(ctx, t)
}

func (a adapterFlowExportPolicyV1) AutoDeleteFlowExportPolicy(oldctx oldcontext.Context, t *telemetry.FlowExportPolicy, options ...grpc.CallOption) (*telemetry.FlowExportPolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	return a.service.AutoDeleteFlowExportPolicy(ctx, t)
}

func (a adapterFlowExportPolicyV1) AutoGetFlowExportPolicy(oldctx oldcontext.Context, t *telemetry.FlowExportPolicy, options ...grpc.CallOption) (*telemetry.FlowExportPolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	return a.service.AutoGetFlowExportPolicy(ctx, t)
}

func (a adapterFlowExportPolicyV1) AutoListFlowExportPolicy(oldctx oldcontext.Context, t *api.ListWatchOptions, options ...grpc.CallOption) (*telemetry.FlowExportPolicyList, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	return a.service.AutoListFlowExportPolicy(ctx, t)
}

func (a adapterFlowExportPolicyV1) AutoUpdateFlowExportPolicy(oldctx oldcontext.Context, t *telemetry.FlowExportPolicy, options ...grpc.CallOption) (*telemetry.FlowExportPolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	return a.service.AutoUpdateFlowExportPolicy(ctx, t)
}

func (a adapterFlowExportPolicyV1) AutoWatchFlowExportPolicy(oldctx oldcontext.Context, in *api.ListWatchOptions, options ...grpc.CallOption) (telemetry.FlowExportPolicyV1_AutoWatchFlowExportPolicyClient, error) {
	ctx := context.Context(oldctx)
	return a.service.AutoWatchFlowExportPolicy(ctx, in)
}

func (e *sFlowExportPolicyV1GwService) CompleteRegistration(ctx context.Context,
	logger log.Logger,
	grpcserver *grpc.Server,
	m *http.ServeMux,
	rslvr resolver.Interface) error {
	apigw := apigwpkg.MustGetAPIGateway()
	// IP:port destination or service discovery key.
	grpcaddr := "pen-apiserver"
	grpcaddr = apigw.GetAPIServerAddr(grpcaddr)
	e.logger = logger
	cl, err := e.newClient(ctx, grpcaddr, rslvr, apigw.GetDevMode())
	if cl == nil || err != nil {
		err = errors.Wrap(err, "could not create client")
		return err
	}
	marshaller := runtime.JSONBuiltin{}
	opts := runtime.WithMarshalerOption("*", &marshaller)
	if mux == nil {
		mux = runtime.NewServeMux(opts)
	}
	fileCount++
	err = telemetry.RegisterFlowExportPolicyV1HandlerWithClient(ctx, mux, cl)
	if err != nil {
		err = errors.Wrap(err, "service registration failed")
		return err
	}
	logger.InfoLog("msg", "registered service telemetry.FlowExportPolicyV1")

	m.Handle("/v1/flowExportPolicy/", http.StripPrefix("/v1/flowExportPolicy", mux))
	if fileCount == 1 {
		err = registerSwaggerDef(m, logger)
	}
	return err
}

func (e *sFlowExportPolicyV1GwService) newClient(ctx context.Context, grpcAddr string, rslvr resolver.Interface, devmode bool) (telemetry.FlowExportPolicyV1Client, error) {
	var opts []rpckit.Option
	if rslvr != nil {
		opts = append(opts, rpckit.WithBalancer(balancer.New(rslvr)))
	}

	if !devmode {
		opts = append(opts, rpckit.WithTracerEnabled(false))
		opts = append(opts, rpckit.WithLoggerEnabled(false))
		opts = append(opts, rpckit.WithStatsEnabled(false))
	}

	client, err := rpckit.NewRPCClient("FlowExportPolicyV1GwService", grpcAddr, opts...)
	if err != nil {
		return nil, errors.Wrap(err, "create rpc client")
	}

	e.logger.Infof("Connected to GRPC Server %s", grpcAddr)
	defer func() {
		go func() {
			<-ctx.Done()
			if cerr := client.Close(); cerr != nil {
				e.logger.ErrorLog("msg", "Failed to close conn on Done()", "addr", grpcAddr, "error", cerr)
			}
		}()
	}()

	cl := adapterFlowExportPolicyV1{grpcclient.NewFlowExportPolicyV1Backend(client.ClientConn, e.logger)}
	return cl, nil
}

type sFwlogPolicyV1GwService struct {
	logger log.Logger
}

type adapterFwlogPolicyV1 struct {
	service telemetry.ServiceFwlogPolicyV1Client
}

func (a adapterFwlogPolicyV1) AutoAddFwlogPolicy(oldctx oldcontext.Context, t *telemetry.FwlogPolicy, options ...grpc.CallOption) (*telemetry.FwlogPolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	return a.service.AutoAddFwlogPolicy(ctx, t)
}

func (a adapterFwlogPolicyV1) AutoDeleteFwlogPolicy(oldctx oldcontext.Context, t *telemetry.FwlogPolicy, options ...grpc.CallOption) (*telemetry.FwlogPolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	return a.service.AutoDeleteFwlogPolicy(ctx, t)
}

func (a adapterFwlogPolicyV1) AutoGetFwlogPolicy(oldctx oldcontext.Context, t *telemetry.FwlogPolicy, options ...grpc.CallOption) (*telemetry.FwlogPolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	return a.service.AutoGetFwlogPolicy(ctx, t)
}

func (a adapterFwlogPolicyV1) AutoListFwlogPolicy(oldctx oldcontext.Context, t *api.ListWatchOptions, options ...grpc.CallOption) (*telemetry.FwlogPolicyList, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	return a.service.AutoListFwlogPolicy(ctx, t)
}

func (a adapterFwlogPolicyV1) AutoUpdateFwlogPolicy(oldctx oldcontext.Context, t *telemetry.FwlogPolicy, options ...grpc.CallOption) (*telemetry.FwlogPolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	return a.service.AutoUpdateFwlogPolicy(ctx, t)
}

func (a adapterFwlogPolicyV1) AutoWatchFwlogPolicy(oldctx oldcontext.Context, in *api.ListWatchOptions, options ...grpc.CallOption) (telemetry.FwlogPolicyV1_AutoWatchFwlogPolicyClient, error) {
	ctx := context.Context(oldctx)
	return a.service.AutoWatchFwlogPolicy(ctx, in)
}

func (e *sFwlogPolicyV1GwService) CompleteRegistration(ctx context.Context,
	logger log.Logger,
	grpcserver *grpc.Server,
	m *http.ServeMux,
	rslvr resolver.Interface) error {
	apigw := apigwpkg.MustGetAPIGateway()
	// IP:port destination or service discovery key.
	grpcaddr := "pen-apiserver"
	grpcaddr = apigw.GetAPIServerAddr(grpcaddr)
	e.logger = logger
	cl, err := e.newClient(ctx, grpcaddr, rslvr, apigw.GetDevMode())
	if cl == nil || err != nil {
		err = errors.Wrap(err, "could not create client")
		return err
	}
	marshaller := runtime.JSONBuiltin{}
	opts := runtime.WithMarshalerOption("*", &marshaller)
	if mux == nil {
		mux = runtime.NewServeMux(opts)
	}
	fileCount++
	err = telemetry.RegisterFwlogPolicyV1HandlerWithClient(ctx, mux, cl)
	if err != nil {
		err = errors.Wrap(err, "service registration failed")
		return err
	}
	logger.InfoLog("msg", "registered service telemetry.FwlogPolicyV1")

	m.Handle("/v1/fwlogPolicy/", http.StripPrefix("/v1/fwlogPolicy", mux))
	if fileCount == 1 {

	}
	return err
}

func (e *sFwlogPolicyV1GwService) newClient(ctx context.Context, grpcAddr string, rslvr resolver.Interface, devmode bool) (telemetry.FwlogPolicyV1Client, error) {
	var opts []rpckit.Option
	if rslvr != nil {
		opts = append(opts, rpckit.WithBalancer(balancer.New(rslvr)))
	}

	if !devmode {
		opts = append(opts, rpckit.WithTracerEnabled(false))
		opts = append(opts, rpckit.WithLoggerEnabled(false))
		opts = append(opts, rpckit.WithStatsEnabled(false))
	}

	client, err := rpckit.NewRPCClient("FwlogPolicyV1GwService", grpcAddr, opts...)
	if err != nil {
		return nil, errors.Wrap(err, "create rpc client")
	}

	e.logger.Infof("Connected to GRPC Server %s", grpcAddr)
	defer func() {
		go func() {
			<-ctx.Done()
			if cerr := client.Close(); cerr != nil {
				e.logger.ErrorLog("msg", "Failed to close conn on Done()", "addr", grpcAddr, "error", cerr)
			}
		}()
	}()

	cl := adapterFwlogPolicyV1{grpcclient.NewFwlogPolicyV1Backend(client.ClientConn, e.logger)}
	return cl, nil
}

type sStatsPolicyV1GwService struct {
	logger log.Logger
}

type adapterStatsPolicyV1 struct {
	service telemetry.ServiceStatsPolicyV1Client
}

func (a adapterStatsPolicyV1) AutoAddStatsPolicy(oldctx oldcontext.Context, t *telemetry.StatsPolicy, options ...grpc.CallOption) (*telemetry.StatsPolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	return a.service.AutoAddStatsPolicy(ctx, t)
}

func (a adapterStatsPolicyV1) AutoDeleteStatsPolicy(oldctx oldcontext.Context, t *telemetry.StatsPolicy, options ...grpc.CallOption) (*telemetry.StatsPolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	return a.service.AutoDeleteStatsPolicy(ctx, t)
}

func (a adapterStatsPolicyV1) AutoGetStatsPolicy(oldctx oldcontext.Context, t *telemetry.StatsPolicy, options ...grpc.CallOption) (*telemetry.StatsPolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	return a.service.AutoGetStatsPolicy(ctx, t)
}

func (a adapterStatsPolicyV1) AutoListStatsPolicy(oldctx oldcontext.Context, t *api.ListWatchOptions, options ...grpc.CallOption) (*telemetry.StatsPolicyList, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	return a.service.AutoListStatsPolicy(ctx, t)
}

func (a adapterStatsPolicyV1) AutoUpdateStatsPolicy(oldctx oldcontext.Context, t *telemetry.StatsPolicy, options ...grpc.CallOption) (*telemetry.StatsPolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	return a.service.AutoUpdateStatsPolicy(ctx, t)
}

func (a adapterStatsPolicyV1) AutoWatchStatsPolicy(oldctx oldcontext.Context, in *api.ListWatchOptions, options ...grpc.CallOption) (telemetry.StatsPolicyV1_AutoWatchStatsPolicyClient, error) {
	ctx := context.Context(oldctx)
	return a.service.AutoWatchStatsPolicy(ctx, in)
}

func (e *sStatsPolicyV1GwService) CompleteRegistration(ctx context.Context,
	logger log.Logger,
	grpcserver *grpc.Server,
	m *http.ServeMux,
	rslvr resolver.Interface) error {
	apigw := apigwpkg.MustGetAPIGateway()
	// IP:port destination or service discovery key.
	grpcaddr := "pen-apiserver"
	grpcaddr = apigw.GetAPIServerAddr(grpcaddr)
	e.logger = logger
	cl, err := e.newClient(ctx, grpcaddr, rslvr, apigw.GetDevMode())
	if cl == nil || err != nil {
		err = errors.Wrap(err, "could not create client")
		return err
	}
	marshaller := runtime.JSONBuiltin{}
	opts := runtime.WithMarshalerOption("*", &marshaller)
	if mux == nil {
		mux = runtime.NewServeMux(opts)
	}
	fileCount++
	err = telemetry.RegisterStatsPolicyV1HandlerWithClient(ctx, mux, cl)
	if err != nil {
		err = errors.Wrap(err, "service registration failed")
		return err
	}
	logger.InfoLog("msg", "registered service telemetry.StatsPolicyV1")

	m.Handle("/v1/statsPolicy/", http.StripPrefix("/v1/statsPolicy", mux))
	if fileCount == 1 {

	}
	return err
}

func (e *sStatsPolicyV1GwService) newClient(ctx context.Context, grpcAddr string, rslvr resolver.Interface, devmode bool) (telemetry.StatsPolicyV1Client, error) {
	var opts []rpckit.Option
	if rslvr != nil {
		opts = append(opts, rpckit.WithBalancer(balancer.New(rslvr)))
	}

	if !devmode {
		opts = append(opts, rpckit.WithTracerEnabled(false))
		opts = append(opts, rpckit.WithLoggerEnabled(false))
		opts = append(opts, rpckit.WithStatsEnabled(false))
	}

	client, err := rpckit.NewRPCClient("StatsPolicyV1GwService", grpcAddr, opts...)
	if err != nil {
		return nil, errors.Wrap(err, "create rpc client")
	}

	e.logger.Infof("Connected to GRPC Server %s", grpcAddr)
	defer func() {
		go func() {
			<-ctx.Done()
			if cerr := client.Close(); cerr != nil {
				e.logger.ErrorLog("msg", "Failed to close conn on Done()", "addr", grpcAddr, "error", cerr)
			}
		}()
	}()

	cl := adapterStatsPolicyV1{grpcclient.NewStatsPolicyV1Backend(client.ClientConn, e.logger)}
	return cl, nil
}

func init() {
	apigw := apigwpkg.MustGetAPIGateway()

	svcFlowExportPolicyV1 := sFlowExportPolicyV1GwService{}
	apigw.Register("telemetry.FlowExportPolicyV1", "flowExportPolicy/", &svcFlowExportPolicyV1)
	svcFwlogPolicyV1 := sFwlogPolicyV1GwService{}
	apigw.Register("telemetry.FwlogPolicyV1", "fwlogPolicy/", &svcFwlogPolicyV1)
	svcStatsPolicyV1 := sStatsPolicyV1GwService{}
	apigw.Register("telemetry.StatsPolicyV1", "statsPolicy/", &svcStatsPolicyV1)
}
