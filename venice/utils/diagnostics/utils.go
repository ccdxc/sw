package diagnostics

import (
	"google.golang.org/grpc"

	"github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/diagnostics/protos"
	"github.com/pensando/sw/venice/utils/log"
)

// list of supported modules for diagnostics
var supportedModules = map[string]bool{
	globals.Spyglass: true,
	globals.Npm:      true,
	globals.APIGw:    true,
	globals.Citadel:  true,
	globals.EvtsMgr:  true,
}

// GetLogFilter maps log level in module object to log filter
func GetLogFilter(level string) log.FilterType {
	logLevel := diagnostics.ModuleSpec_LoggingLevel(diagnostics.ModuleSpec_LoggingLevel_value[level])
	switch logLevel {
	case diagnostics.ModuleSpec_Error:
		return log.AllowErrorFilter
	case diagnostics.ModuleSpec_Warn:
		return log.AllowWarnFilter
	case diagnostics.ModuleSpec_Debug:
		return log.AllowDebugFilter
	default:
		return log.AllowInfoFilter
	}
}

// IsSupported returns true if it supports diagnostics
func IsSupported(modObj *diagnostics.Module) bool {
	return supportedModules[modObj.Status.Module]
}

// RegisterService registers diagnostics service
func RegisterService(srv *grpc.Server, service Service) {
	protos.RegisterDiagnosticsServer(srv, service)
}
