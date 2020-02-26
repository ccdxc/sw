package archive

import (
	"google.golang.org/grpc"

	"github.com/pensando/sw/venice/utils/archive/protos"
)

// RegisterService registers archive service
func RegisterService(srv *grpc.Server, service Service) {
	protos.RegisterArchiveServer(srv, service)
}
