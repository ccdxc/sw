/*
Package bookstore is a auto generated package.
Input file: protos/example.proto
*/
package bookstoreGwService

import (
	"context"
	"net/http"
	"time"

	"github.com/GeertJohan/go.rice"
	gogocodec "github.com/gogo/protobuf/codec"
	"github.com/pensando/grpc-gateway/runtime"
	bookstore "github.com/pensando/sw/api/generated/bookstore"
	"github.com/pensando/sw/api/generated/bookstore/grpc/client"
	"github.com/pensando/sw/apigw/pkg"
	"github.com/pensando/sw/utils/log"
	"github.com/pkg/errors"
	oldcontext "golang.org/x/net/context"
	"google.golang.org/grpc"
)

const codecSize = 1024 * 1024

type s_BookstoreV1GwService struct {
	logger log.Logger
}

type adapterBookstoreV1 struct {
	service bookstore.ServiceBookstoreV1
}

func (a adapterBookstoreV1) AddPublisher(oldctx oldcontext.Context, t *bookstore.Publisher, options ...grpc.CallOption) (*bookstore.Publisher, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.AddPublisher(ctx, *t)
	return &r, e
}

func (a adapterBookstoreV1) UpdatePublisher(oldctx oldcontext.Context, t *bookstore.Publisher, options ...grpc.CallOption) (*bookstore.Publisher, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.UpdatePublisher(ctx, *t)
	return &r, e
}

func (a adapterBookstoreV1) DeletePublisher(oldctx oldcontext.Context, t *bookstore.Publisher, options ...grpc.CallOption) (*bookstore.Publisher, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.DeletePublisher(ctx, *t)
	return &r, e
}

func (a adapterBookstoreV1) GetPublisher(oldctx oldcontext.Context, t *bookstore.Publisher, options ...grpc.CallOption) (*bookstore.Publisher, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.GetPublisher(ctx, *t)
	return &r, e
}

func (a adapterBookstoreV1) GetBook(oldctx oldcontext.Context, t *bookstore.Book, options ...grpc.CallOption) (*bookstore.Book, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.GetBook(ctx, *t)
	return &r, e
}

func (a adapterBookstoreV1) AddBook(oldctx oldcontext.Context, t *bookstore.Book, options ...grpc.CallOption) (*bookstore.Book, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.AddBook(ctx, *t)
	return &r, e
}

func (a adapterBookstoreV1) UpdateBook(oldctx oldcontext.Context, t *bookstore.Book, options ...grpc.CallOption) (*bookstore.Book, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.UpdateBook(ctx, *t)
	return &r, e
}

func (a adapterBookstoreV1) DeleteBook(oldctx oldcontext.Context, t *bookstore.Book, options ...grpc.CallOption) (*bookstore.Book, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.DeleteBook(ctx, *t)
	return &r, e
}

func (a adapterBookstoreV1) OrderOper(oldctx oldcontext.Context, t *bookstore.Order, options ...grpc.CallOption) (*bookstore.Order, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	r, e := a.service.OrderOper(ctx, *t)
	return &r, e
}

func (e *s_BookstoreV1GwService) CompleteRegistration(ctx context.Context,
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
	err = bookstore.RegisterBookstoreV1HandlerWithClient(ctx, mux, cl)
	if err != nil {
		err = errors.Wrap(err, "service registration failed")
		return err
	}
	logger.InfoLog("msg", "registered service bookstore.BookstoreV1")
	m.Handle("/v1/bookstore/", http.StripPrefix("/v1/bookstore", mux))
	err = registerSwaggerDef(m, logger)
	return err
}

func (e *s_BookstoreV1GwService) newClient(ctx context.Context, grpcAddr string, opts ...grpc.DialOption) (bookstore.BookstoreV1Client, error) {
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

	cl := adapterBookstoreV1{grpcclient.NewBookstoreV1Backend(conn, e.logger)}
	return cl, nil
}

func registerSwaggerDef(m *http.ServeMux, logger log.Logger) error {
	box, err := rice.FindBox("../../../../../sw/api/generated/bookstore/swagger")
	if err != nil {
		err = errors.Wrap(err, "error opening rice.Box")
		return err
	}
	content, err := box.Bytes("example.swagger.json")
	if err != nil {
		err = errors.Wrap(err, "error opening rice.File")
		return err
	}
	m.HandleFunc("/swagger/bookstore/", func(w http.ResponseWriter, r *http.Request) {
		w.Write(content)
	})
	return nil
}

func init() {
	apigw := apigwpkg.MustGetAPIGateway()

	svcBookstoreV1 := s_BookstoreV1GwService{}
	apigw.Register("bookstore.BookstoreV1", "bookstore/", &svcBookstoreV1)
}
