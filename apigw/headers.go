package apigw

// Grpc Metadata Options used by the API GW to annotate requests sent
//  to the API Server.
const (
	GrpcMDRequestVersion = "Grpc-Metadata-Req-Version"
	GrpcMDRequestURI     = "Grpc-Metadata-Req-Uri"
	GrpcMDRequestMethod  = "Grpc-Metadata-Req-Method"
)
