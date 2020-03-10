package apigw

import (
	"github.com/pensando/grpc-gateway/runtime"

	"github.com/pensando/sw/venice/utils/audit"
)

// Grpc Metadata Options used by the API GW to annotate requests sent
//  to the API Server.
const (
	GrpcMDRequestVersion     = "Grpc-Metadata-Req-Version"
	GrpcMDRequestURI         = "Grpc-Metadata-Req-Uri"
	GrpcMDRequestMethod      = "Grpc-Metadata-Req-Method"
	GrpcMDRequestTenant      = "Grpc-Metadata-Req-Tenant"
	GrpcMDReplaceStatusField = "Grpc-Metadata-Replace-Status-Field"
	GrpcMDStagingBufferID    = "Grpc-Metadata-Staging-Buffer-Id"
	GrpcMDAuditRequestURI    = "Grpc-Metadata-Audit-Req-Uri"
	GrpcMDExtRequestID       = audit.GrpcMDExtRequestID
)

// Authentication related constants used by the API GW and Login service
const (
	// CookieHeader is Cookie header and contains cookie 'sid=<token>' with JWT as the token for browser requests.
	CookieHeader = "Cookie"

	// XForwardedHostHeader is X-Forwarded-Host header. It is compared against Origin header if request is same origin if Host header doesn't match with Origin header.
	// This will be the case when API Gateway sits behind proxy.
	XForwardedHostHeader = "X-Forwarded-Host"

	// XForwardedFor is X-Forwarded-For header. It is used to get client IPs
	XForwardedFor = "X-Forwarded-For"

	// XRealIP is X-Real-IP header user to get client IP
	XRealIP = "X-Real-IP"

	// Forwarded is Forwarded header used to get client IPs (rfc7239)
	Forwarded = "Forwarded"

	// GrpcMDCsrfHeader is CSRF Token header name set in http response upon login. Set CSRF token header with grpc metadata header prefix so that grpc gateway
	// puts it in context in runtime.AnnotateContext()
	GrpcMDCsrfHeader = runtime.MetadataHeaderPrefix + "Csrf-Token"

	// GrpcMDAuthorizationHeader is authorization header with value 'Bearer <token>'. It contains the JWT for non-browser requests.
	GrpcMDAuthorizationHeader = runtime.MetadataPrefix + "Authorization"
)

const (
	// ExtRequestIDHeader contains an external ID attached to a request by caller of the API. It is saved in audit logs.
	ExtRequestIDHeader = audit.ExtRequestIDHeader
)
