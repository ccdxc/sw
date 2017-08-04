package apigw

import (
	"context"
	"net/http"

	"github.com/pensando/sw/utils/log"
	"google.golang.org/grpc"
)

// APIGatewayService is the interface satisfied by all all services registering to the API gateway
type APIGatewayService interface {
	// CompleteRegistration is a callback invoked after the API Gateway is done initializing.
	CompleteRegistration(ctx context.Context, logger log.Logger, grpcserver *grpc.Server, mux *http.ServeMux) error
}

// APIGateway is satisfied by the API Gateway implementation.
type APIGateway interface {
	// Register registers a service to the API Gateway.
	//   Duplicate registrations with the same name cause a panic
	Register(name, path string, svc APIGatewayService) APIGatewayService
	// GetService returns a APIGatewayService that was registered with name.
	GetService(name string) APIGatewayService
	// Run starts the API gateway eventloop
	Run(config Config)
	// Stop sends stop signal to API gateway
	Stop()
}

// Config is all config used to start the API Gateway
type Config struct {
	// HTTPAddr to start the Service on.
	HTTPAddr string
	// GrpcServerPort on which to start listening to the external GRPC requests.
	GrpcServerPort string
	// DebugMode enables verbose logging and stack trace dump support.
	DebugMode bool
	// Logger to be used for logging.
	Logger log.Logger
}
