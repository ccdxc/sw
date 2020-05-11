package diagnostics

import (
	"fmt"
	"time"

	uuid "github.com/satori/go.uuid"

	"github.com/gogo/protobuf/types"
	"google.golang.org/grpc"

	"github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/diagnostics/protos"
	"github.com/pensando/sw/venice/utils/log"
)

// list of supported modules for diagnostics
var supportedVeniceModules = map[string]bool{
	globals.Spyglass:  true,
	globals.Npm:       true,
	globals.APIGw:     true,
	globals.Citadel:   true,
	globals.EvtsMgr:   true,
	globals.Rollout:   true,
	globals.Tpm:       true,
	globals.APIServer: true,
	globals.OrchHub:   true,
	globals.Perseus:   true,
}

// list of support naples modules for diagnostics
var supportedNaplesModules = map[string]bool{
	globals.Netagent: true,
}

// GetLogFilter maps log level in module object to log filter
func GetLogFilter(level string) log.FilterType {
	logLevel := diagnostics.ModuleSpec_LoggingLevel(diagnostics.ModuleSpec_LoggingLevel_vvalue[level])
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
	return supportedVeniceModules[modObj.Status.Module] || supportedNaplesModules[modObj.Status.Module]
}

// RegisterService registers diagnostics service
func RegisterService(srv *grpc.Server, service Service) {
	protos.RegisterDiagnosticsServer(srv, service)
}

// NewNaplesModules return list of supported modules for Naples
func NewNaplesModules(name, node, apiversion string) []*diagnostics.Module {
	var modObjs []*diagnostics.Module
	// return a list only if naples name is non-empty
	if name != "" {
		for key, ok := range supportedNaplesModules {
			if ok {
				obj := &diagnostics.Module{}
				obj.Defaults("all")
				obj.Name = fmt.Sprintf("%s-%s", name, key)
				obj.Status.Category = diagnostics.ModuleStatus_Naples.String()
				obj.Status.Module = key
				obj.Status.Node = node
				obj.Status.MACAddress = name
				obj.UUID = uuid.NewV4().String()
				ts, _ := types.TimestampProto(time.Now())
				obj.CreationTime.Timestamp = *ts
				obj.ModTime.Timestamp = *ts
				obj.GenerationID = "1"
				obj.APIVersion = apiversion
				obj.SelfLink = obj.MakeURI("configs", obj.APIVersion, "diagnostics")
				modObjs = append(modObjs, obj)
			}
		}
	}
	return modObjs
}

// ListSupportedNaplesModules returns names of Naples process for which diagnostics are supported
func ListSupportedNaplesModules() []string {
	var modules []string
	for name, ok := range supportedNaplesModules {
		if ok {
			modules = append(modules, name)
		}
	}
	return modules
}
