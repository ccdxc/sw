/*
Package bookstore is a auto generated package.
Input file: example.proto
*/
package bookstoreApiServer

import (
	"context"

	"github.com/pensando/sw/apiserver"
	"github.com/pensando/sw/apiserver/pkg"

	bookstore "github.com/pensando/sw/utils/apigen/example/generated"
	"github.com/pensando/sw/utils/apigen/example/impl"
	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/runtime"
	"github.com/pkg/errors"
	"google.golang.org/grpc"
)

var apisrv apiserver.Server

type s_bookstoreBackend struct {
	Services map[string]apiserver.Service
	Messages map[string]apiserver.Message

	endpoints_BookstoreV1 *e_BookstoreV1Endpoints

	Hooks []apiserver.ServiceHooks
}

type e_BookstoreV1Endpoints struct {
	Svc s_bookstoreBackend

	fn_AddPublisher    func(ctx context.Context, t interface{}) (interface{}, error)
	fn_UpdatePublisher func(ctx context.Context, t interface{}) (interface{}, error)
	fn_DeletePublisher func(ctx context.Context, t interface{}) (interface{}, error)
	fn_GetPublisher    func(ctx context.Context, t interface{}) (interface{}, error)
	fn_GetBook         func(ctx context.Context, t interface{}) (interface{}, error)
	fn_AddBook         func(ctx context.Context, t interface{}) (interface{}, error)
	fn_UpdateBook      func(ctx context.Context, t interface{}) (interface{}, error)
	fn_DeleteBook      func(ctx context.Context, t interface{}) (interface{}, error)
	fn_OrderOper       func(ctx context.Context, t interface{}) (interface{}, error)
}

func (s *s_bookstoreBackend) CompleteRegistration(ctx context.Context, logger log.Logger,
	grpcserver *grpc.Server, scheme *runtime.Scheme) error {
	s.Messages = map[string]apiserver.Message{

		"bookstore.Publisher": apisrvpkg.NewMessage("bookstore.Publisher").WithKeyGenerator(func(i interface{}, prefix string) string {
			r := i.(bookstore.Publisher)
			return r.MakeKey(prefix)
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create bool) (interface{}, error) {
			r := i.(bookstore.Publisher)
			ret := bookstore.Publisher{}
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = kvs.Create(ctx, key, &r, 0, &ret)
				err = errors.Wrap(err, "KV create failed")
			} else {
				err = kvs.Update(ctx, key, &r, 0, &ret)
				err = errors.Wrap(err, "KV update failed")
			}
			return ret, err
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := bookstore.Publisher{}
			err := kvs.Get(ctx, key, &r)
			err = errors.Wrap(err, "KV get failed")
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := bookstore.Publisher{}
			err := kvs.Delete(ctx, key, &r)
			return r, err
		}),
		"bookstore.PublisherSpec": apisrvpkg.NewMessage("bookstore.PublisherSpec"),
		"bookstore.Book": apisrvpkg.NewMessage("bookstore.Book").WithKeyGenerator(func(i interface{}, prefix string) string {
			r := i.(bookstore.Book)
			return r.MakeKey(prefix)
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create bool) (interface{}, error) {
			r := i.(bookstore.Book)
			ret := bookstore.Book{}
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = kvs.Create(ctx, key, &r, 0, &ret)
				err = errors.Wrap(err, "KV create failed")
			} else {
				err = kvs.Update(ctx, key, &r, 0, &ret)
				err = errors.Wrap(err, "KV update failed")
			}
			return ret, err
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := bookstore.Book{}
			err := kvs.Get(ctx, key, &r)
			err = errors.Wrap(err, "KV get failed")
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := bookstore.Book{}
			err := kvs.Delete(ctx, key, &r)
			return r, err
		}),
		"bookstore.BookSpec":   apisrvpkg.NewMessage("bookstore.BookSpec"),
		"bookstore.BookStatus": apisrvpkg.NewMessage("bookstore.BookStatus"),
		"bookstore.Order": apisrvpkg.NewMessage("bookstore.Order").WithKeyGenerator(func(i interface{}, prefix string) string {
			r := i.(bookstore.Order)
			return r.MakeKey(prefix)
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create bool) (interface{}, error) {
			r := i.(bookstore.Order)
			ret := bookstore.Order{}
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = kvs.Create(ctx, key, &r, 0, &ret)
				err = errors.Wrap(err, "KV create failed")
			} else {
				err = kvs.Update(ctx, key, &r, 0, &ret)
				err = errors.Wrap(err, "KV update failed")
			}
			return ret, err
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := bookstore.Order{}
			err := kvs.Get(ctx, key, &r)
			err = errors.Wrap(err, "KV get failed")
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := bookstore.Order{}
			err := kvs.Delete(ctx, key, &r)
			return r, err
		}),
		"bookstore.OrderSpec":   apisrvpkg.NewMessage("bookstore.OrderSpec"),
		"bookstore.OrderItem":   apisrvpkg.NewMessage("bookstore.OrderItem"),
		"bookstore.OrderStatus": apisrvpkg.NewMessage("bookstore.OrderStatus"),
	}

	scheme.AddKnownTypes(
		&bookstore.Publisher{},
		&bookstore.Book{},
		&bookstore.Order{},
	)

	apisrv.RegisterMessages("bookstore", s.Messages)

	{
		srv := apisrvpkg.NewService("BookstoreV1")

		s.endpoints_BookstoreV1.fn_AddPublisher = srv.AddMethod("AddPublisher",
			apisrvpkg.NewMethod(s.Messages["bookstore.Publisher"], s.Messages["bookstore.Publisher"], "bookstore", "AddPublisher")).WithOper("create").WithVersion("v1").HandleInvocation

		s.endpoints_BookstoreV1.fn_UpdatePublisher = srv.AddMethod("UpdatePublisher",
			apisrvpkg.NewMethod(s.Messages["bookstore.Publisher"], s.Messages["bookstore.Publisher"], "bookstore", "UpdatePublisher")).WithOper("update").WithVersion("v1").HandleInvocation

		s.endpoints_BookstoreV1.fn_DeletePublisher = srv.AddMethod("DeletePublisher",
			apisrvpkg.NewMethod(s.Messages["bookstore.Publisher"], s.Messages["bookstore.Publisher"], "bookstore", "DeletePublisher")).WithOper("delete").WithVersion("v1").HandleInvocation

		s.endpoints_BookstoreV1.fn_GetPublisher = srv.AddMethod("GetPublisher",
			apisrvpkg.NewMethod(s.Messages["bookstore.Publisher"], s.Messages["bookstore.Publisher"], "bookstore", "GetPublisher")).WithOper("get").WithVersion("v1").HandleInvocation

		s.endpoints_BookstoreV1.fn_GetBook = srv.AddMethod("GetBook",
			apisrvpkg.NewMethod(s.Messages["bookstore.Book"], s.Messages["bookstore.Book"], "bookstore", "GetBook")).WithVersion("v1").HandleInvocation

		s.endpoints_BookstoreV1.fn_AddBook = srv.AddMethod("AddBook",
			apisrvpkg.NewMethod(s.Messages["bookstore.Book"], s.Messages["bookstore.Book"], "bookstore", "AddBook")).WithOper("create").WithVersion("v1").HandleInvocation

		s.endpoints_BookstoreV1.fn_UpdateBook = srv.AddMethod("UpdateBook",
			apisrvpkg.NewMethod(s.Messages["bookstore.Book"], s.Messages["bookstore.Book"], "bookstore", "UpdateBook")).WithOper("update").WithVersion("v1").HandleInvocation

		s.endpoints_BookstoreV1.fn_DeleteBook = srv.AddMethod("DeleteBook",
			apisrvpkg.NewMethod(s.Messages["bookstore.Book"], s.Messages["bookstore.Book"], "bookstore", "DeleteBook")).WithOper("delete").WithVersion("v1").HandleInvocation

		s.endpoints_BookstoreV1.fn_OrderOper = srv.AddMethod("OrderOper",
			apisrvpkg.NewMethod(s.Messages["bookstore.Order"], s.Messages["bookstore.Order"], "bookstore", "OrderOper")).WithVersion("v1").HandleInvocation

		s.Services = map[string]apiserver.Service{
			"bookstore.BookstoreV1": srv,
		}
		apisrv.RegisterService("bookstore.BookstoreV1", srv)
		endpoints := bookstore.MakeBookstoreV1ServerEndpoints(s.endpoints_BookstoreV1)
		server := bookstore.MakeGRPCServerBookstoreV1(ctx, endpoints, logger)
		bookstore.RegisterBookstoreV1Server(grpcserver, server)
		for _, h := range s.Hooks {
			h.RegisterHooks(logger, srv)
		}
	}
	return nil
}

func (e *e_BookstoreV1Endpoints) AddPublisher(ctx context.Context, t bookstore.Publisher) (bookstore.Publisher, error) {
	r, err := e.fn_AddPublisher(ctx, t)
	if err == nil {
		return r.(bookstore.Publisher), err
	} else {
		return bookstore.Publisher{}, err
	}
}
func (e *e_BookstoreV1Endpoints) UpdatePublisher(ctx context.Context, t bookstore.Publisher) (bookstore.Publisher, error) {
	r, err := e.fn_UpdatePublisher(ctx, t)
	if err == nil {
		return r.(bookstore.Publisher), err
	} else {
		return bookstore.Publisher{}, err
	}
}
func (e *e_BookstoreV1Endpoints) DeletePublisher(ctx context.Context, t bookstore.Publisher) (bookstore.Publisher, error) {
	r, err := e.fn_DeletePublisher(ctx, t)
	if err == nil {
		return r.(bookstore.Publisher), err
	} else {
		return bookstore.Publisher{}, err
	}
}
func (e *e_BookstoreV1Endpoints) GetPublisher(ctx context.Context, t bookstore.Publisher) (bookstore.Publisher, error) {
	r, err := e.fn_GetPublisher(ctx, t)
	if err == nil {
		return r.(bookstore.Publisher), err
	} else {
		return bookstore.Publisher{}, err
	}
}
func (e *e_BookstoreV1Endpoints) GetBook(ctx context.Context, t bookstore.Book) (bookstore.Book, error) {
	r, err := e.fn_GetBook(ctx, t)
	if err == nil {
		return r.(bookstore.Book), err
	} else {
		return bookstore.Book{}, err
	}
}
func (e *e_BookstoreV1Endpoints) AddBook(ctx context.Context, t bookstore.Book) (bookstore.Book, error) {
	r, err := e.fn_AddBook(ctx, t)
	if err == nil {
		return r.(bookstore.Book), err
	} else {
		return bookstore.Book{}, err
	}
}
func (e *e_BookstoreV1Endpoints) UpdateBook(ctx context.Context, t bookstore.Book) (bookstore.Book, error) {
	r, err := e.fn_UpdateBook(ctx, t)
	if err == nil {
		return r.(bookstore.Book), err
	} else {
		return bookstore.Book{}, err
	}
}
func (e *e_BookstoreV1Endpoints) DeleteBook(ctx context.Context, t bookstore.Book) (bookstore.Book, error) {
	r, err := e.fn_DeleteBook(ctx, t)
	if err == nil {
		return r.(bookstore.Book), err
	} else {
		return bookstore.Book{}, err
	}
}
func (e *e_BookstoreV1Endpoints) OrderOper(ctx context.Context, t bookstore.Order) (bookstore.Order, error) {
	r, err := e.fn_OrderOper(ctx, t)
	if err == nil {
		return r.(bookstore.Order), err
	} else {
		return bookstore.Order{}, err
	}
}

func init() {
	apisrv = apisrvpkg.MustGetApiServer()

	svc := s_bookstoreBackend{}

	{
		e := e_BookstoreV1Endpoints{Svc: svc}
		svc.endpoints_BookstoreV1 = &e
		svc.Hooks = append(svc.Hooks, impl.NewBookstoreV1Hooks())
	}
	apisrv.Register("bookstore.example.proto", &svc)
}
