package apigw

import (
	"context"
	"net"
	"net/http"

	"google.golang.org/grpc"

	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// APIGatewayService is the interface satisfied by all all services registering to the API gateway
type APIGatewayService interface {
	// CompleteRegistration is a callback invoked after the API Gateway is done initializing.
	CompleteRegistration(ctx context.Context, logger log.Logger, grpcserver *grpc.Server, mux *http.ServeMux, rslvr resolver.Interface) error
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
	// GetApiServerAddr gets the API server address to connect to
	GetAPIServerAddr(svcaddr string) string
	// WaitRunning blocks till the API gateway is completely initialized
	WaitRunning()
	// GetAddr returns the address at which the API gateway is listening
	//   returns error if the API gateway is not initialized
	GetAddr() (net.Addr, error)
	//GetDevMode returns true if running in dev mode
	GetDevMode() bool
}

// Config is all config used to start the API Gateway
type Config struct {
	// HTTPAddr to start the Service on.
	HTTPAddr string
	// GrpcServerPort on which to start listening to the external GRPC requests.
	APIServerOverride string
	// DebugMode enables verbose logging and stack trace dump support.
	DebugMode bool
	// Logger to be used for logging.
	Logger log.Logger
	// List of resolvers to use
	Resolvers []string
	// DevMode is set if the API Gateway is running in Dev mode
	DevMode bool
}
