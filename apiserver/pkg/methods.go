package apisrvpkg

import (
	"context"
	"sync"

	opentracing "github.com/opentracing/opentracing-go"
	"github.com/opentracing/opentracing-go/log"
	"github.com/pkg/errors"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/metadata"

	"fmt"

	"github.com/pensando/sw/apiserver"
	"github.com/pensando/sw/utils/kvstore"
)

// TODO(sanjayt): Add method level stats.

// MethodHdlr is a representation of a gRPC method for the API server.
type MethodHdlr struct {
	sync.Mutex
	// enabled is true if the method is enabled.
	enabled bool
	// requestType is the Message type
	requestType apiserver.Message
	// responseType is the response defined for the method. Both response and request
	// types can point to the same Message type
	responseType apiserver.Message
	// precommitFunc is the registered precommitFunc. See HandleInvocation() for details
	precommitFunc []apiserver.PreCommitFunc
	// postcommmitFunc is the registered function called after KV store operation. see HandleInvocation for details.
	postcommitFunc []apiserver.PostCommitFunc
	// registered resposeWriter for custom responses.
	responseWriter apiserver.ResponseWriterFunc
	// svcPrefix is the prefix configured for the parent service.
	svcPrefix string
	// name is name of the method.
	name string
	// oper is the CRUD opertion this method serves.
	oper string
	// version is the version of the API this method serves.
	version string
}

var (
	errApiDisabled        = grpc.Errorf(codes.ResourceExhausted, "API is disabled")
	errRequestInfo        = grpc.Errorf(codes.InvalidArgument, "internal error (request information)")
	errVersionTransform   = grpc.Errorf(codes.Unimplemented, "internal error (version transformation)")
	errRequestValidation  = grpc.Errorf(codes.InvalidArgument, "request validation failed")
	errKVStoreOperation   = grpc.Errorf(codes.AlreadyExists, "internal error (persisting)")
	errKVStoreNotFound    = grpc.Errorf(codes.NotFound, "Not Found")
	errResponseWriter     = grpc.Errorf(codes.Unimplemented, "internal error(response writer)")
	errUnknownOperation   = grpc.Errorf(codes.Unimplemented, "unknown operation")
	errPreOpChecksFailed  = grpc.Errorf(codes.FailedPrecondition, "failed pre conditions")
	errTransactionFailed  = grpc.Errorf(codes.FailedPrecondition, "cannot execute operation")
	errTransactionErrored = grpc.Errorf(codes.Internal, "transaction execution error")
	errInternalError      = grpc.Errorf(codes.Internal, "internal error")
)

// NewMethod initializes and returns a new Method object.
func NewMethod(req, resp apiserver.Message, prefix, name string) apiserver.Method {
	return &MethodHdlr{enabled: true, requestType: req, responseType: resp, svcPrefix: prefix, name: name}
}

// Enable enables a method.
func (m *MethodHdlr) Enable() {
	m.Lock()
	defer m.Unlock()
	m.enabled = true
}

// Disable disables a method and all further method invocations will be forbidden.
func (m *MethodHdlr) Disable() {
	m.Lock()
	defer m.Unlock()
	m.enabled = false
}

// WithPreCommitHook registers a precommit function.
func (m *MethodHdlr) WithPreCommitHook(fn apiserver.PreCommitFunc) apiserver.Method {
	m.precommitFunc = append(m.precommitFunc, fn)
	return m
}

// WithPostCommitHook registers a post commit function.
func (m *MethodHdlr) WithPostCommitHook(fn apiserver.PostCommitFunc) apiserver.Method {
	m.postcommitFunc = append(m.postcommitFunc, fn)
	return m
}

// WithResponseWriter registers a response generation function when custom responses are desired.
func (m *MethodHdlr) WithResponseWriter(fn apiserver.ResponseWriterFunc) apiserver.Method {
	m.responseWriter = fn
	return m
}

// WithOper sets the oper for the method. Usually set by the generated code.
func (m *MethodHdlr) WithOper(oper string) apiserver.Method {
	m.oper = oper
	return m
}

// WithVersion sets the version that the method serves. Usually set by the generated code.
func (m *MethodHdlr) WithVersion(ver string) apiserver.Method {
	m.version = ver
	return m
}

// GetRequestType returns the message corresponding to the request for the method.
func (m *MethodHdlr) GetRequestType() apiserver.Message {
	return m.requestType
}

// GetResponseType returns the message corresponding to the response for the method.
func (m *MethodHdlr) GetResponseType() apiserver.Message {
	return m.responseType
}

// updateKvStore handles updating the KV store either via a transaction or without as needed.
func (m *MethodHdlr) updateKvStore(ctx context.Context, i interface{}, oper string, txn kvstore.Txn) (interface{}, error) {
	l := singletonApiSrv.Logger
	key, err := m.requestType.GetKVKey(i, m.svcPrefix)
	if err != nil {
		l.ErrorLog("msg", "could not get key", "error", err)
		return nil, errInternalError
	}
	nonTxn := txn.IsEmpty()
	kvOp := kvstore.OperUnknown
	// Update the KV if desired.
	var (
		resp interface{}
	)
	switch oper {
	case "POST":
		if nonTxn {
			resp, err = m.requestType.WriteToKv(ctx, i, m.svcPrefix, true)
			err = errors.Wrap(err, "oper: POST")
		} else {
			err = m.requestType.WriteToKvTxn(ctx, txn, i, m.svcPrefix, true)
			err = errors.Wrap(err, "oper: Txn POST")
			resp = i
		}
		kvOp = kvstore.OperUpdate
	case "PUT":
		if nonTxn {
			resp, err = m.requestType.WriteToKv(ctx, i, m.svcPrefix, false)
			err = errors.Wrap(err, "oper: PUT")
		} else {
			err = m.requestType.WriteToKvTxn(ctx, txn, i, m.svcPrefix, false)
			err = errors.Wrap(err, "oper: Txn PUT")
			resp = i
		}
		kvOp = kvstore.OperUpdate
	case "DELETE":
		err = errors.Wrap(err, "oper: DELETE")
		if err == nil {
			if nonTxn {
				resp, err = m.requestType.DelFromKv(ctx, key)
				err = errors.Wrap(err, "oper: DELETE")
			} else {
				err = m.requestType.DelFromKvTxn(ctx, txn, key)
				err = errors.Wrap(err, "oper: Txn DELETE")
			}
		}
		kvOp = kvstore.OperDelete
	case "GET":
		// TODO(sanjayt): Add support for listing all objects of a type.
		//  and with selectors.
		// Transactions are not supported for a GET operation.
		err = errors.Wrap(err, "oper: GET")
		if err == nil {
			resp, err = m.requestType.GetFromKv(ctx, key)
			err = errors.Wrap(err, "oper: GET")
		}
		kvOp = kvstore.OperGet
	default:
		err = errors.Wrap(errUnknownOperation, fmt.Sprintf("oper: [%s]", oper))
	}
	if err != nil {
		l.ErrorLog("msg", "failed Kv store operation", "error", err, "resp", resp)
		return nil, errKVStoreOperation
	}
	txnResp := kvstore.TxnResponse{}
	if !nonTxn && (oper == "POST" || oper == "PUT" || oper == "DELETE") {
		txnResp, err = txn.Commit(ctx)
		if err != nil {
			err = errors.Wrap(err, "transaction failed")
		} else {
			if !txnResp.Succeeded {
				l.ErrorLog("msg", "transaction failed")
				return nil, errTransactionFailed
			}
			for _, t := range txnResp.Responses {
				if t.Oper == kvOp && t.Key == key {
					resp = t.Obj
				}
			}
		}
	}
	if err != nil {
		l.ErrorLog("msg", "failed Kv store transaction operation", "error", err, "resp", resp)
		return nil, errTransactionErrored
	}
	return resp, nil
}

// HandleInvocation handles the invocation of an API.
// THe invocation goes through
// 1. Version tranformation of the request if the request version is different
//    than the API server version
// 2. Defaulting - Custom defaulting if registered
// 3. Validation - Custom validation registerd by service.
// 4. Pre-Commit hooks - invokes all pre-commit hooks registered for the Method
//    Any of the called hooks can prevent the next stage of KV operation by returning
//    false
// 5. KV operation - CRUD operation on the object. Key for the object is derived from
//    the protobuf specification of the service.
// 6. Post-commit hooks - Invoke all post commits hooks registered.
// 7. Form response - Response is formed by one of the actions below in priority order
//    a. If there is a registered response override function. That registered function
//       forms the response
//    b. The KV store object operated on by the CRU operation is returned back as the
//       response
// 8. Version transform - The response is version transformed from the API Server verion
//    to the request version if needed.
func (m *MethodHdlr) HandleInvocation(ctx context.Context, i interface{}) (interface{}, error) {
	var (
		old, resp interface{}
		err       error
		ver       string
		key       string
	)
	l := singletonApiSrv.Logger

	l.DebugLog("service", m.svcPrefix, "method", m.name, "version", m.version)
	if m.enabled == false {
		l.Infof("Api is disabled ignoring invocation")
		return nil, errApiDisabled
	}

	md, ok := metadata.FromContext(ctx)
	if !ok {
		l.Errorf("unable to get metadata from context")
		return nil, errRequestInfo
	}

	if v, ok := md["req-version"]; ok {
		ver = v[0]
	} else {
		ver = m.version
	}

	// mapOper handles HTTP and gRPC oper types.
	oper := m.mapOper(md)
	l.DebugLog("version", ver, "operation", oper)

	// Version transform if needed.
	if singletonApiSrv.version != ver {
		l.DebugLog("msg", "version mismatch", "version-from", singletonApiSrv.version, "version-to", ver)
		i, err = m.requestType.PrepareMsg(ver, singletonApiSrv.version, i)
		if err != nil {
			l.ErrorLog("msg", "Version transformation failed", "version-from", ver, "version-to", singletonApiSrv.version, "error", err)
			return nil, errVersionTransform
		}
	}
	var span opentracing.Span
	span = opentracing.SpanFromContext(ctx)
	if span != nil {
		span.SetTag("version", ver)
		span.SetTag("operation", oper)
		if v, ok := md["req-method"]; ok {
			span.SetTag("req-method", v[0])
		}
	}

	// Apply any defaults to the request message
	i = m.requestType.Default(i)

	// Validate the request.
	err = m.requestType.Validate(i)
	if err != nil {
		l.ErrorLog("msg", "request validation failed", "error", err)
		return nil, errRequestValidation
	}
	if span != nil {
		span.LogFields(log.String("event", "calling precommit hooks"))
	}

	txn := singletonApiSrv.kv.NewTxn()
	key, err = m.requestType.GetKVKey(i, m.svcPrefix)
	if err != nil {
		l.ErrorLog("msg", "could not get key", "error", err)
		return nil, errInternalError
	}

	// Invoke registered precommit hooks
	kvwrite := true
	for _, v := range m.precommitFunc {
		kvold := kvwrite
		i, kvwrite, err = v(ctx, singletonApiSrv.kv, txn, key, oper, i)
		if err != nil {
			l.ErrorLog("msg", "precommit hook failed", "error", err)
			return nil, errPreOpChecksFailed
		}
		kvwrite = kvwrite && kvold
	}
	if span != nil {
		span.LogFields(log.String("event", "precommit hooks done"))
	}

	if kvwrite {
		resp, err = m.updateKvStore(ctx, i, oper, txn)
		if err != nil {
			return nil, err
		}
	} else {
		l.DebugLog("msg", "KV operation over-ridden")
	}

	if span != nil {
		span.LogFields(log.String("event", "calling postcommit hooks"))
	}

	// Invoke registered postcommit hooks
	for _, v := range m.postcommitFunc {
		v(ctx, oper, i)
	}

	if span != nil {
		span.LogFields(log.String("event", "postcommit hooks done"))
	}
	//Generate response
	if m.responseWriter != nil {
		l.DebugLog("msg", "response overide is enabled")
		resp, err = m.responseWriter(ctx, singletonApiSrv.kv, m.svcPrefix, i, old, oper)
		if err != nil {
			l.ErrorLog("msg", "response writer returned", "error", err)
			return nil, errResponseWriter
		}
	}

	// transform to request Version.
	if singletonApiSrv.version != ver {
		resp, err = m.requestType.PrepareMsg(singletonApiSrv.version, ver, resp)
		if err != nil {
			l.ErrorLog("msg", "response version transformation failed", "ver-from", singletonApiSrv.version, "ver-to", ver)
			return nil, errVersionTransform
		}
	}
	return resp, nil
}

func (m *MethodHdlr) mapOper(md metadata.MD) string {
	if v, ok := md["req-method"]; ok {
		return v[0]
	}

	switch m.oper {
	case "create":
		return "POST"
	case "get":
		return "GET"
	case "update":
		return "PUT"
	case "delete":
		return "DELETE"
	}
	return ""
}
