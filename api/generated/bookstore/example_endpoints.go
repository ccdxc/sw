/*
Package bookstore is a auto generated package.
Input file: protos/example.proto
*/

package bookstore

import (
	"net/url"
	"strings"

	"context"
	"time"

	"github.com/go-kit/kit/endpoint"
	"github.com/go-kit/kit/tracing/opentracing"
	httptransport "github.com/go-kit/kit/transport/http"
	stdopentracing "github.com/opentracing/opentracing-go"
	"github.com/pensando/sw/utils/log"
)

type MiddlewareBookstoreV1 func(ServiceBookstoreV1) ServiceBookstoreV1
type Endpoints_BookstoreV1 struct {
	AddPublisherEndpoint    endpoint.Endpoint
	UpdatePublisherEndpoint endpoint.Endpoint
	DeletePublisherEndpoint endpoint.Endpoint
	GetPublisherEndpoint    endpoint.Endpoint
	GetBookEndpoint         endpoint.Endpoint
	AddBookEndpoint         endpoint.Endpoint
	UpdateBookEndpoint      endpoint.Endpoint
	DeleteBookEndpoint      endpoint.Endpoint
	OrderOperEndpoint       endpoint.Endpoint
}

func (e Endpoints_BookstoreV1) AddPublisher(ctx context.Context, in Publisher) (Publisher, error) {
	resp, err := e.AddPublisherEndpoint(ctx, in)
	if err != nil {
		return Publisher{}, err
	}
	return *resp.(*Publisher), nil
}

type respBookstoreV1AddPublisher struct {
	V   Publisher
	Err error
}

func MakeBookstoreV1AddPublisherEndpoint(s ServiceBookstoreV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*Publisher)
		v, err := s.AddPublisher(ctx, *req)
		return respBookstoreV1AddPublisher{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "BookstoreV1:AddPublisher")(f)
}
func (e Endpoints_BookstoreV1) UpdatePublisher(ctx context.Context, in Publisher) (Publisher, error) {
	resp, err := e.UpdatePublisherEndpoint(ctx, in)
	if err != nil {
		return Publisher{}, err
	}
	return *resp.(*Publisher), nil
}

type respBookstoreV1UpdatePublisher struct {
	V   Publisher
	Err error
}

func MakeBookstoreV1UpdatePublisherEndpoint(s ServiceBookstoreV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*Publisher)
		v, err := s.UpdatePublisher(ctx, *req)
		return respBookstoreV1UpdatePublisher{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "BookstoreV1:UpdatePublisher")(f)
}
func (e Endpoints_BookstoreV1) DeletePublisher(ctx context.Context, in Publisher) (Publisher, error) {
	resp, err := e.DeletePublisherEndpoint(ctx, in)
	if err != nil {
		return Publisher{}, err
	}
	return *resp.(*Publisher), nil
}

type respBookstoreV1DeletePublisher struct {
	V   Publisher
	Err error
}

func MakeBookstoreV1DeletePublisherEndpoint(s ServiceBookstoreV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*Publisher)
		v, err := s.DeletePublisher(ctx, *req)
		return respBookstoreV1DeletePublisher{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "BookstoreV1:DeletePublisher")(f)
}
func (e Endpoints_BookstoreV1) GetPublisher(ctx context.Context, in Publisher) (Publisher, error) {
	resp, err := e.GetPublisherEndpoint(ctx, in)
	if err != nil {
		return Publisher{}, err
	}
	return *resp.(*Publisher), nil
}

type respBookstoreV1GetPublisher struct {
	V   Publisher
	Err error
}

func MakeBookstoreV1GetPublisherEndpoint(s ServiceBookstoreV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*Publisher)
		v, err := s.GetPublisher(ctx, *req)
		return respBookstoreV1GetPublisher{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "BookstoreV1:GetPublisher")(f)
}
func (e Endpoints_BookstoreV1) GetBook(ctx context.Context, in Book) (Book, error) {
	resp, err := e.GetBookEndpoint(ctx, in)
	if err != nil {
		return Book{}, err
	}
	return *resp.(*Book), nil
}

type respBookstoreV1GetBook struct {
	V   Book
	Err error
}

func MakeBookstoreV1GetBookEndpoint(s ServiceBookstoreV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*Book)
		v, err := s.GetBook(ctx, *req)
		return respBookstoreV1GetBook{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "BookstoreV1:GetBook")(f)
}
func (e Endpoints_BookstoreV1) AddBook(ctx context.Context, in Book) (Book, error) {
	resp, err := e.AddBookEndpoint(ctx, in)
	if err != nil {
		return Book{}, err
	}
	return *resp.(*Book), nil
}

type respBookstoreV1AddBook struct {
	V   Book
	Err error
}

func MakeBookstoreV1AddBookEndpoint(s ServiceBookstoreV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*Book)
		v, err := s.AddBook(ctx, *req)
		return respBookstoreV1AddBook{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "BookstoreV1:AddBook")(f)
}
func (e Endpoints_BookstoreV1) UpdateBook(ctx context.Context, in Book) (Book, error) {
	resp, err := e.UpdateBookEndpoint(ctx, in)
	if err != nil {
		return Book{}, err
	}
	return *resp.(*Book), nil
}

type respBookstoreV1UpdateBook struct {
	V   Book
	Err error
}

func MakeBookstoreV1UpdateBookEndpoint(s ServiceBookstoreV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*Book)
		v, err := s.UpdateBook(ctx, *req)
		return respBookstoreV1UpdateBook{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "BookstoreV1:UpdateBook")(f)
}
func (e Endpoints_BookstoreV1) DeleteBook(ctx context.Context, in Book) (Book, error) {
	resp, err := e.DeleteBookEndpoint(ctx, in)
	if err != nil {
		return Book{}, err
	}
	return *resp.(*Book), nil
}

type respBookstoreV1DeleteBook struct {
	V   Book
	Err error
}

func MakeBookstoreV1DeleteBookEndpoint(s ServiceBookstoreV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*Book)
		v, err := s.DeleteBook(ctx, *req)
		return respBookstoreV1DeleteBook{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "BookstoreV1:DeleteBook")(f)
}
func (e Endpoints_BookstoreV1) OrderOper(ctx context.Context, in Order) (Order, error) {
	resp, err := e.OrderOperEndpoint(ctx, in)
	if err != nil {
		return Order{}, err
	}
	return *resp.(*Order), nil
}

type respBookstoreV1OrderOper struct {
	V   Order
	Err error
}

func MakeBookstoreV1OrderOperEndpoint(s ServiceBookstoreV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*Order)
		v, err := s.OrderOper(ctx, *req)
		return respBookstoreV1OrderOper{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "BookstoreV1:OrderOper")(f)
}

func MakeBookstoreV1ServerEndpoints(s ServiceBookstoreV1, logger log.Logger) Endpoints_BookstoreV1 {
	return Endpoints_BookstoreV1{
		AddPublisherEndpoint:    MakeBookstoreV1AddPublisherEndpoint(s, logger),
		UpdatePublisherEndpoint: MakeBookstoreV1UpdatePublisherEndpoint(s, logger),
		DeletePublisherEndpoint: MakeBookstoreV1DeletePublisherEndpoint(s, logger),
		GetPublisherEndpoint:    MakeBookstoreV1GetPublisherEndpoint(s, logger),
		GetBookEndpoint:         MakeBookstoreV1GetBookEndpoint(s, logger),
		AddBookEndpoint:         MakeBookstoreV1AddBookEndpoint(s, logger),
		UpdateBookEndpoint:      MakeBookstoreV1UpdateBookEndpoint(s, logger),
		DeleteBookEndpoint:      MakeBookstoreV1DeleteBookEndpoint(s, logger),
		OrderOperEndpoint:       MakeBookstoreV1OrderOperEndpoint(s, logger),
	}
}

func LoggingBookstoreV1Middleware(logger log.Logger) MiddlewareBookstoreV1 {
	return func(next ServiceBookstoreV1) ServiceBookstoreV1 {
		return loggingBookstoreV1Middleware{
			logger: logger,
			next:   next,
		}
	}
}

type loggingBookstoreV1Middleware struct {
	logger log.Logger
	next   ServiceBookstoreV1
}

func (m loggingBookstoreV1Middleware) AddPublisher(ctx context.Context, in Publisher) (resp Publisher, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "BookstoreV1", "method", "AddPublisher", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.AddPublisher(ctx, in)
	return
}

func (m loggingBookstoreV1Middleware) UpdatePublisher(ctx context.Context, in Publisher) (resp Publisher, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "BookstoreV1", "method", "UpdatePublisher", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.UpdatePublisher(ctx, in)
	return
}

func (m loggingBookstoreV1Middleware) DeletePublisher(ctx context.Context, in Publisher) (resp Publisher, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "BookstoreV1", "method", "DeletePublisher", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.DeletePublisher(ctx, in)
	return
}

func (m loggingBookstoreV1Middleware) GetPublisher(ctx context.Context, in Publisher) (resp Publisher, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "BookstoreV1", "method", "GetPublisher", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.GetPublisher(ctx, in)
	return
}

func (m loggingBookstoreV1Middleware) GetBook(ctx context.Context, in Book) (resp Book, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "BookstoreV1", "method", "GetBook", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.GetBook(ctx, in)
	return
}

func (m loggingBookstoreV1Middleware) AddBook(ctx context.Context, in Book) (resp Book, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "BookstoreV1", "method", "AddBook", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.AddBook(ctx, in)
	return
}

func (m loggingBookstoreV1Middleware) UpdateBook(ctx context.Context, in Book) (resp Book, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "BookstoreV1", "method", "UpdateBook", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.UpdateBook(ctx, in)
	return
}

func (m loggingBookstoreV1Middleware) DeleteBook(ctx context.Context, in Book) (resp Book, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "BookstoreV1", "method", "DeleteBook", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.DeleteBook(ctx, in)
	return
}

func (m loggingBookstoreV1Middleware) OrderOper(ctx context.Context, in Order) (resp Order, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "BookstoreV1", "method", "OrderOper", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.OrderOper(ctx, in)
	return
}

func MakeBookstoreV1RestClientEndpoints(instance string) (Endpoints_BookstoreV1, error) {
	if !strings.HasPrefix(instance, "http") {
		instance = "http://" + instance
	}
	tgt, err := url.Parse(instance)
	if err != nil {
		return Endpoints_BookstoreV1{}, err
	}
	tgt.Path = ""
	options := []httptransport.ClientOption{}

	return Endpoints_BookstoreV1{
		AddPublisherEndpoint:    httptransport.NewClient("POST", tgt, encodeHttpPublisher, decodeHttprespBookstoreV1AddPublisher, options...).Endpoint(),
		UpdatePublisherEndpoint: httptransport.NewClient("POST", tgt, encodeHttpPublisher, decodeHttprespBookstoreV1UpdatePublisher, options...).Endpoint(),
		DeletePublisherEndpoint: httptransport.NewClient("POST", tgt, encodeHttpPublisher, decodeHttprespBookstoreV1DeletePublisher, options...).Endpoint(),
		GetPublisherEndpoint:    httptransport.NewClient("POST", tgt, encodeHttpPublisher, decodeHttprespBookstoreV1GetPublisher, options...).Endpoint(),
		GetBookEndpoint:         httptransport.NewClient("POST", tgt, encodeHttpBook, decodeHttprespBookstoreV1GetBook, options...).Endpoint(),
		AddBookEndpoint:         httptransport.NewClient("POST", tgt, encodeHttpBook, decodeHttprespBookstoreV1AddBook, options...).Endpoint(),
		UpdateBookEndpoint:      httptransport.NewClient("POST", tgt, encodeHttpBook, decodeHttprespBookstoreV1UpdateBook, options...).Endpoint(),
		DeleteBookEndpoint:      httptransport.NewClient("POST", tgt, encodeHttpBook, decodeHttprespBookstoreV1DeleteBook, options...).Endpoint(),
		OrderOperEndpoint:       httptransport.NewClient("POST", tgt, encodeHttpOrder, decodeHttprespBookstoreV1OrderOper, options...).Endpoint(),
	}, nil
}
