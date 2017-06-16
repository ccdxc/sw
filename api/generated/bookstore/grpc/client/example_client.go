package grpcclient

import (
	"context"

	"github.com/go-kit/kit/endpoint"
	"github.com/go-kit/kit/tracing/opentracing"
	grpctransport "github.com/go-kit/kit/transport/grpc"
	stdopentracing "github.com/opentracing/opentracing-go"
	bookstore "github.com/pensando/sw/api/generated/bookstore"
	"github.com/pensando/sw/utils/log"
	"google.golang.org/grpc"
	"google.golang.org/grpc/metadata"
)

func dummyBefore(ctx context.Context, md *metadata.MD) context.Context {
	xmd, ok := metadata.FromContext(ctx)
	if ok {
		cmd := metadata.Join(*md, xmd)
		*md = cmd
	}
	return ctx
}

func NewBookstoreV1(conn *grpc.ClientConn, logger log.Logger) bookstore.ServiceBookstoreV1 {

	var l_AddPublisherEndpoint endpoint.Endpoint
	{
		l_AddPublisherEndpoint = grpctransport.NewClient(
			conn,
			"bookstore.BookstoreV1",
			"AddPublisher",
			bookstore.EndcodeGrpcReqPublisher,
			bookstore.DecodeGrpcRespPublisher,
			bookstore.Publisher{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_AddPublisherEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "BookstoreV1:AddPublisher")(l_AddPublisherEndpoint)
	}
	var l_UpdatePublisherEndpoint endpoint.Endpoint
	{
		l_UpdatePublisherEndpoint = grpctransport.NewClient(
			conn,
			"bookstore.BookstoreV1",
			"UpdatePublisher",
			bookstore.EndcodeGrpcReqPublisher,
			bookstore.DecodeGrpcRespPublisher,
			bookstore.Publisher{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_UpdatePublisherEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "BookstoreV1:UpdatePublisher")(l_UpdatePublisherEndpoint)
	}
	var l_DeletePublisherEndpoint endpoint.Endpoint
	{
		l_DeletePublisherEndpoint = grpctransport.NewClient(
			conn,
			"bookstore.BookstoreV1",
			"DeletePublisher",
			bookstore.EndcodeGrpcReqPublisher,
			bookstore.DecodeGrpcRespPublisher,
			bookstore.Publisher{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_DeletePublisherEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "BookstoreV1:DeletePublisher")(l_DeletePublisherEndpoint)
	}
	var l_GetPublisherEndpoint endpoint.Endpoint
	{
		l_GetPublisherEndpoint = grpctransport.NewClient(
			conn,
			"bookstore.BookstoreV1",
			"GetPublisher",
			bookstore.EndcodeGrpcReqPublisher,
			bookstore.DecodeGrpcRespPublisher,
			bookstore.Publisher{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_GetPublisherEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "BookstoreV1:GetPublisher")(l_GetPublisherEndpoint)
	}
	var l_GetBookEndpoint endpoint.Endpoint
	{
		l_GetBookEndpoint = grpctransport.NewClient(
			conn,
			"bookstore.BookstoreV1",
			"GetBook",
			bookstore.EndcodeGrpcReqBook,
			bookstore.DecodeGrpcRespBook,
			bookstore.Book{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_GetBookEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "BookstoreV1:GetBook")(l_GetBookEndpoint)
	}
	var l_AddBookEndpoint endpoint.Endpoint
	{
		l_AddBookEndpoint = grpctransport.NewClient(
			conn,
			"bookstore.BookstoreV1",
			"AddBook",
			bookstore.EndcodeGrpcReqBook,
			bookstore.DecodeGrpcRespBook,
			bookstore.Book{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_AddBookEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "BookstoreV1:AddBook")(l_AddBookEndpoint)
	}
	var l_UpdateBookEndpoint endpoint.Endpoint
	{
		l_UpdateBookEndpoint = grpctransport.NewClient(
			conn,
			"bookstore.BookstoreV1",
			"UpdateBook",
			bookstore.EndcodeGrpcReqBook,
			bookstore.DecodeGrpcRespBook,
			bookstore.Book{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_UpdateBookEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "BookstoreV1:UpdateBook")(l_UpdateBookEndpoint)
	}
	var l_DeleteBookEndpoint endpoint.Endpoint
	{
		l_DeleteBookEndpoint = grpctransport.NewClient(
			conn,
			"bookstore.BookstoreV1",
			"DeleteBook",
			bookstore.EndcodeGrpcReqBook,
			bookstore.DecodeGrpcRespBook,
			bookstore.Book{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_DeleteBookEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "BookstoreV1:DeleteBook")(l_DeleteBookEndpoint)
	}
	var l_OrderOperEndpoint endpoint.Endpoint
	{
		l_OrderOperEndpoint = grpctransport.NewClient(
			conn,
			"bookstore.BookstoreV1",
			"OrderOper",
			bookstore.EndcodeGrpcReqOrder,
			bookstore.DecodeGrpcRespOrder,
			bookstore.Order{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_OrderOperEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "BookstoreV1:OrderOper")(l_OrderOperEndpoint)
	}
	return bookstore.Endpoints_BookstoreV1{

		AddPublisherEndpoint:    l_AddPublisherEndpoint,
		UpdatePublisherEndpoint: l_UpdatePublisherEndpoint,
		DeletePublisherEndpoint: l_DeletePublisherEndpoint,
		GetPublisherEndpoint:    l_GetPublisherEndpoint,
		GetBookEndpoint:         l_GetBookEndpoint,
		AddBookEndpoint:         l_AddBookEndpoint,
		UpdateBookEndpoint:      l_UpdateBookEndpoint,
		DeleteBookEndpoint:      l_DeleteBookEndpoint,
		OrderOperEndpoint:       l_OrderOperEndpoint,
	}
}

func NewBookstoreV1Backend(conn *grpc.ClientConn, logger log.Logger) bookstore.ServiceBookstoreV1 {
	cl := NewBookstoreV1(conn, logger)
	cl = bookstore.LoggingBookstoreV1Middleware(logger)(cl)
	return cl
}
