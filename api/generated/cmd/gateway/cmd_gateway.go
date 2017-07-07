/*
Package cmd is a auto generated package.
Input file: protos/cmd.proto
*/
package cmdGwService

import (
	"context"
	"net/http"
	"time"

	"github.com/GeertJohan/go.rice"
	gogocodec "github.com/gogo/protobuf/codec"
	"github.com/pensando/grpc-gateway/runtime"
	cmd "github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/api/generated/cmd/grpc/client"
	"github.com/pensando/sw/apigw/pkg"
	"github.com/pensando/sw/utils/log"
	"github.com/pkg/errors"
	oldcontext "golang.org/x/net/context"
	"google.golang.org/grpc"
)

const codecSize = 1024 * 1024

type s_CmdV1GwService struct {
	logger log.Logger
}

type adapterCmdV1 struct {
	service cmd.ServiceCmdV1
}

func (a adapterCmdV1) GetNodeList(oldctx oldcontext.Context, t *cmd.NodeList, options ...grpc.CallOption) (*cmd.NodeList, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.GetNodeList(ctx, *t)
	return &r, e
}

func (a adapterCmdV1) NodeOper(oldctx oldcontext.Context, t *cmd.Node, options ...grpc.CallOption) (*cmd.Node, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.NodeOper(ctx, *t)
	return &r, e
}

func (a adapterCmdV1) ClusterOper(oldctx oldcontext.Context, t *cmd.Cluster, options ...grpc.CallOption) (*cmd.Cluster, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.ClusterOper(ctx, *t)
	return &r, e
}

func (e *s_CmdV1GwService) CompleteRegistration(ctx context.Context,
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
	err = cmd.RegisterCmdV1HandlerWithClient(ctx, mux, cl)
	if err != nil {
		err = errors.Wrap(err, "service registration failed")
		return err
	}
	logger.InfoLog("msg", "registered service cmd.CmdV1")
	m.Handle("/v1/cmd/", http.StripPrefix("/v1/cmd", mux))
	err = registerSwaggerDef(m, logger)
	return err
}

func (e *s_CmdV1GwService) newClient(ctx context.Context, grpcAddr string, opts ...grpc.DialOption) (cmd.CmdV1Client, error) {
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

	cl := adapterCmdV1{grpcclient.NewCmdV1Backend(conn, e.logger)}
	return cl, nil
}

func registerSwaggerDef(m *http.ServeMux, logger log.Logger) error {
	box, err := rice.FindBox("../../../../../sw/api/generated/cmd/swagger")
	if err != nil {
		err = errors.Wrap(err, "error opening rice.Box")
		return err
	}
	content, err := box.Bytes("cmd.swagger.json")
	if err != nil {
		err = errors.Wrap(err, "error opening rice.File")
		return err
	}
	m.HandleFunc("/swagger/cmd/", func(w http.ResponseWriter, r *http.Request) {
		w.Write(content)
	})
	return nil
}

func init() {
	apigw := apigwpkg.MustGetAPIGateway()

	svcCmdV1 := s_CmdV1GwService{}
	apigw.Register("cmd.CmdV1", "cmd/", &svcCmdV1)
}
