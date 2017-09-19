package pkg1

import (
	"github.com/pensando/sw/venice/utils/log"
)

// Create logContext, if there are package specific
// annotations needed for logging. This step can be skipped
// if there is no need to add context annotations at package
// level.
var logCtx log.Context

func init() {
	logCtx = log.Context{"pkg", "pkg1", "svc", "svc1"}
}

// Run starts the service
func Run() error {

	// For logging with pkg specific context, use WithContext()
	// method as shown below.
	log.WithContext(logCtx...).Info("started svc1")
	log.WithContext(logCtx...).Debug("svc1 data bar")
	return nil
}

// Close terminates the service
func Close() error {
	log.WithContext(logCtx...).Info("terminated service1")
	return nil
}
