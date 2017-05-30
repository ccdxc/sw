/*
Package bookstore is a auto generated package.
Input file: example.proto
*/
package bookstore

import (
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"io/ioutil"
	"net/http"

	oldcontext "golang.org/x/net/context"
	"google.golang.org/grpc/metadata"

	grpctransport "github.com/go-kit/kit/transport/grpc"
	"github.com/pensando/sw/utils/log"
)

var (
	ErrInconsistentIDs = errors.New("inconsistent IDs")
	ErrAlreadyExists   = errors.New("already exists")
	ErrNotFound        = errors.New("not found")
)

// FIXME: add HTTP handler here.
func recoverVersion(ctx context.Context, md metadata.MD) context.Context {
	var pairs []string
	xmd := md
	v, ok := xmd["req-version"]
	if ok {
		pairs = append(pairs, "req-version", v[0])
	}
	if v, ok = xmd["req-uri"]; ok {
		pairs = append(pairs, "req-uri", v[0])
	}
	if v, ok = xmd["req-method"]; ok {
		pairs = append(pairs, "req-method", v[0])
	}
	nmd := metadata.Pairs(pairs...)
	ctx = metadata.NewContext(ctx, nmd)
	return ctx
}

type grpcServerBookstoreV1 struct {
	AddPublisherHdlr    grpctransport.Handler
	UpdatePublisherHdlr grpctransport.Handler
	DeletePublisherHdlr grpctransport.Handler
	GetPublisherHdlr    grpctransport.Handler
	GetBookHdlr         grpctransport.Handler
	AddBookHdlr         grpctransport.Handler
	UpdateBookHdlr      grpctransport.Handler
	DeleteBookHdlr      grpctransport.Handler
	OrderOperHdlr       grpctransport.Handler
}

func MakeGRPCServerBookstoreV1(ctx context.Context, endpoints Endpoints_BookstoreV1, logger log.Logger) BookstoreV1Server {
	options := []grpctransport.ServerOption{
		grpctransport.ServerErrorLogger(logger),
		grpctransport.ServerBefore(recoverVersion),
	}
	return &grpcServerBookstoreV1{

		AddPublisherHdlr: grpctransport.NewServer(
			endpoints.AddPublisherEndpoint,
			DecodeGrpcReqPublisher,
			EncodeGrpcRespPublisher,
			options...,
		),

		UpdatePublisherHdlr: grpctransport.NewServer(
			endpoints.UpdatePublisherEndpoint,
			DecodeGrpcReqPublisher,
			EncodeGrpcRespPublisher,
			options...,
		),

		DeletePublisherHdlr: grpctransport.NewServer(
			endpoints.DeletePublisherEndpoint,
			DecodeGrpcReqPublisher,
			EncodeGrpcRespPublisher,
			options...,
		),

		GetPublisherHdlr: grpctransport.NewServer(
			endpoints.GetPublisherEndpoint,
			DecodeGrpcReqPublisher,
			EncodeGrpcRespPublisher,
			options...,
		),

		GetBookHdlr: grpctransport.NewServer(
			endpoints.GetBookEndpoint,
			DecodeGrpcReqBook,
			EncodeGrpcRespBook,
			options...,
		),

		AddBookHdlr: grpctransport.NewServer(
			endpoints.AddBookEndpoint,
			DecodeGrpcReqBook,
			EncodeGrpcRespBook,
			options...,
		),

		UpdateBookHdlr: grpctransport.NewServer(
			endpoints.UpdateBookEndpoint,
			DecodeGrpcReqBook,
			EncodeGrpcRespBook,
			options...,
		),

		DeleteBookHdlr: grpctransport.NewServer(
			endpoints.DeleteBookEndpoint,
			DecodeGrpcReqBook,
			EncodeGrpcRespBook,
			options...,
		),

		OrderOperHdlr: grpctransport.NewServer(
			endpoints.OrderOperEndpoint,
			DecodeGrpcReqOrder,
			EncodeGrpcRespOrder,
			options...,
		),
	}
}

func (s *grpcServerBookstoreV1) AddPublisher(ctx oldcontext.Context, req *Publisher) (*Publisher, error) {
	_, resp, err := s.AddPublisherHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respBookstoreV1AddPublisher).V
	return &r, resp.(respBookstoreV1AddPublisher).Err
}

func decodeHttprespBookstoreV1AddPublisher(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respBookstoreV1AddPublisher
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

func (s *grpcServerBookstoreV1) UpdatePublisher(ctx oldcontext.Context, req *Publisher) (*Publisher, error) {
	_, resp, err := s.UpdatePublisherHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respBookstoreV1UpdatePublisher).V
	return &r, resp.(respBookstoreV1UpdatePublisher).Err
}

func decodeHttprespBookstoreV1UpdatePublisher(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respBookstoreV1UpdatePublisher
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

func (s *grpcServerBookstoreV1) DeletePublisher(ctx oldcontext.Context, req *Publisher) (*Publisher, error) {
	_, resp, err := s.DeletePublisherHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respBookstoreV1DeletePublisher).V
	return &r, resp.(respBookstoreV1DeletePublisher).Err
}

func decodeHttprespBookstoreV1DeletePublisher(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respBookstoreV1DeletePublisher
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

func (s *grpcServerBookstoreV1) GetPublisher(ctx oldcontext.Context, req *Publisher) (*Publisher, error) {
	_, resp, err := s.GetPublisherHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respBookstoreV1GetPublisher).V
	return &r, resp.(respBookstoreV1GetPublisher).Err
}

func decodeHttprespBookstoreV1GetPublisher(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respBookstoreV1GetPublisher
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

func (s *grpcServerBookstoreV1) GetBook(ctx oldcontext.Context, req *Book) (*Book, error) {
	_, resp, err := s.GetBookHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respBookstoreV1GetBook).V
	return &r, resp.(respBookstoreV1GetBook).Err
}

func decodeHttprespBookstoreV1GetBook(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respBookstoreV1GetBook
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

func (s *grpcServerBookstoreV1) AddBook(ctx oldcontext.Context, req *Book) (*Book, error) {
	_, resp, err := s.AddBookHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respBookstoreV1AddBook).V
	return &r, resp.(respBookstoreV1AddBook).Err
}

func decodeHttprespBookstoreV1AddBook(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respBookstoreV1AddBook
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

func (s *grpcServerBookstoreV1) UpdateBook(ctx oldcontext.Context, req *Book) (*Book, error) {
	_, resp, err := s.UpdateBookHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respBookstoreV1UpdateBook).V
	return &r, resp.(respBookstoreV1UpdateBook).Err
}

func decodeHttprespBookstoreV1UpdateBook(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respBookstoreV1UpdateBook
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

func (s *grpcServerBookstoreV1) DeleteBook(ctx oldcontext.Context, req *Book) (*Book, error) {
	_, resp, err := s.DeleteBookHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respBookstoreV1DeleteBook).V
	return &r, resp.(respBookstoreV1DeleteBook).Err
}

func decodeHttprespBookstoreV1DeleteBook(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respBookstoreV1DeleteBook
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

func (s *grpcServerBookstoreV1) OrderOper(ctx oldcontext.Context, req *Order) (*Order, error) {
	_, resp, err := s.OrderOperHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respBookstoreV1OrderOper).V
	return &r, resp.(respBookstoreV1OrderOper).Err
}

func decodeHttprespBookstoreV1OrderOper(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respBookstoreV1OrderOper
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

func encodeHttpPublisher(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpPublisher(_ context.Context, r *http.Request) (interface{}, error) {
	var req Publisher
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqPublisher(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(Publisher)
	return &req, nil
}

func DecodeGrpcReqPublisher(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*Publisher)
	return req, nil
}

func EncodeGrpcRespPublisher(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespPublisher(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpPublisherSpec(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpPublisherSpec(_ context.Context, r *http.Request) (interface{}, error) {
	var req PublisherSpec
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqPublisherSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(PublisherSpec)
	return &req, nil
}

func DecodeGrpcReqPublisherSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*PublisherSpec)
	return req, nil
}

func EncodeGrpcRespPublisherSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespPublisherSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpBook(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpBook(_ context.Context, r *http.Request) (interface{}, error) {
	var req Book
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqBook(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(Book)
	return &req, nil
}

func DecodeGrpcReqBook(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*Book)
	return req, nil
}

func EncodeGrpcRespBook(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespBook(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpBookSpec(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpBookSpec(_ context.Context, r *http.Request) (interface{}, error) {
	var req BookSpec
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqBookSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(BookSpec)
	return &req, nil
}

func DecodeGrpcReqBookSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*BookSpec)
	return req, nil
}

func EncodeGrpcRespBookSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespBookSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpBookStatus(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpBookStatus(_ context.Context, r *http.Request) (interface{}, error) {
	var req BookStatus
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqBookStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(BookStatus)
	return &req, nil
}

func DecodeGrpcReqBookStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*BookStatus)
	return req, nil
}

func EncodeGrpcRespBookStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespBookStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpOrder(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpOrder(_ context.Context, r *http.Request) (interface{}, error) {
	var req Order
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqOrder(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(Order)
	return &req, nil
}

func DecodeGrpcReqOrder(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*Order)
	return req, nil
}

func EncodeGrpcRespOrder(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespOrder(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpOrderSpec(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpOrderSpec(_ context.Context, r *http.Request) (interface{}, error) {
	var req OrderSpec
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqOrderSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(OrderSpec)
	return &req, nil
}

func DecodeGrpcReqOrderSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*OrderSpec)
	return req, nil
}

func EncodeGrpcRespOrderSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespOrderSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpOrderItem(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpOrderItem(_ context.Context, r *http.Request) (interface{}, error) {
	var req OrderItem
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqOrderItem(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(OrderItem)
	return &req, nil
}

func DecodeGrpcReqOrderItem(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*OrderItem)
	return req, nil
}

func EncodeGrpcRespOrderItem(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespOrderItem(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpOrderStatus(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpOrderStatus(_ context.Context, r *http.Request) (interface{}, error) {
	var req OrderStatus
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqOrderStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(OrderStatus)
	return &req, nil
}

func DecodeGrpcReqOrderStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*OrderStatus)
	return req, nil
}

func EncodeGrpcRespOrderStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespOrderStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpResponse(ctx context.Context, w http.ResponseWriter, response interface{}) error {
	if e, ok := response.(errorer); ok && e.error() != nil {
		// Not a Go kit transport error, but a business-logic error.
		// Provide those as HTTP errors.
		encodeError(ctx, e.error(), w)
		return nil
	}
	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	return json.NewEncoder(w).Encode(response)
}

func encodeHttpRequest(_ context.Context, req *http.Request, request interface{}) error {
	var buf bytes.Buffer
	err := json.NewEncoder(&buf).Encode(request)
	if err != nil {
		return err
	}
	req.Body = ioutil.NopCloser(&buf)
	return nil
}

type errorer interface {
	error() error
}

func encodeError(_ context.Context, err error, w http.ResponseWriter) {
	if err == nil {
		panic("encodeError with nil error")
	}
	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	w.WriteHeader(codeFrom(err))
	json.NewEncoder(w).Encode(map[string]interface{}{
		"error": err.Error(),
	})
}

func errorDecoder(r *http.Response) error {
	var w errorWrapper
	if err := json.NewDecoder(r.Body).Decode(&w); err != nil {
		return err
	}
	return errors.New(w.Error)
}

type errorWrapper struct {
	Error string `json:"error"`
}

func codeFrom(err error) int {
	switch err {
	case ErrNotFound:
		return http.StatusNotFound
	case ErrAlreadyExists, ErrInconsistentIDs:
		return http.StatusBadRequest
	default:
		return http.StatusInternalServerError
	}
}
