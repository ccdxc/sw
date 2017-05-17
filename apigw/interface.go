package apigw

import (
	"context"
	"net/http"

	"github.com/pensando/sw/utils/log"
	"google.golang.org/grpc"
)

// ApiGwService is the interface satisfied by all all services registering to the API gateway
type ApiGwService interface {
	// CompleteRegistration is a callback invoked after the API Gateway is done initializing.
	CompleteRegistration(ctx context.Context, logger log.Logger, grpcserver *grpc.Server, mux *http.ServeMux) error
}

// ApiGateway is satisfied by the API Gateway implementation.
type ApiGateway interface {
	// Register registers a service to the API Gateway.
	//   Duplicate registrations with the same name cause a panic
	Register(name, path string, svc ApiGwService) ApiGwService
	// GetService returns a ApiGwService that was registered with name.
	GetService(name string) ApiGwService
	// Run starts the API gateway eventloop
	Run(config Config)
}

// Config is all config used to start the API Gateway
type Config struct {
	// HttpAddr to start the Service on.
	HttpAddr string
	// GrpcServerPort on which to start listening to the external GRPC requests.
	GrpcServerPort string
	// DebugMode enables verbose logging and stack trace dump support.
	DebugMode bool
	// Logger to be used for logging.
	Logger log.Logger
}
