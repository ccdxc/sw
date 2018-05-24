package apigw

import "github.com/pensando/grpc-gateway/runtime"

// Grpc Metadata Options used by the API GW to annotate requests sent
//  to the API Server.
const (
	GrpcMDRequestVersion     = "Grpc-Metadata-Req-Version"
	GrpcMDRequestURI         = "Grpc-Metadata-Req-Uri"
	GrpcMDRequestMethod      = "Grpc-Metadata-Req-Method"
	GrpcMDReplaceStatusField = "Grpc-Metadata-Replace-Status-Field"
)

// Authentication related constants used by the API GW and Login service
const (
	// CookieHeader is Cookie header and contains cookie 'sid=<token>' with JWT as the token for browser requests.
	CookieHeader = "Cookie"

	// XForwardedHostHeader is X-Forwarded-Host header. It is compared against Origin header if request is same origin if Host header doesn't match with Origin header.
	// This will be the case when API Gateway sits behind proxy.
	XForwardedHostHeader = "X-Forwarded-Host"

	// GrpcMDCsrfHeader is CSRF Token header name set in http response upon login. Set CSRF token header with grpc metadata header prefix so that grpc gateway
	// puts it in context in runtime.AnnotateContext()
	GrpcMDCsrfHeader = runtime.MetadataHeaderPrefix + "Csrf-Token"

	// GrpcMDAuthorizationHeader is authorization header with value 'Bearer <token>'. It contains the JWT for non-browser requests.
	GrpcMDAuthorizationHeader = runtime.MetadataPrefix + "Authorization"
)
