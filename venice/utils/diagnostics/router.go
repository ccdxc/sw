package diagnostics

import (
	"fmt"
	"net"

	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

type router struct {
	rslvr        resolver.Interface
	moduleGetter Getter
	logger       log.Logger
}

func (r *router) GetRoute(diagReq *diagapi.DiagnosticsRequest) (string, string, error) {
	modObj, err := r.moduleGetter.GetModule(diagReq.Name)
	if err != nil {
		return "", "", err
	}
	var svcName string
	svcName = diagReq.ServicePort.Name
	if svcName == "" {
		svcName = modObj.Status.Module
	}
	if !validateService(svcName, modObj) {
		return "", "", fmt.Errorf("invalid service name [%s]", svcName)
	}
	// find service instance URL to route for Venice grpc services
	var svcURL string
	if IsSupported(modObj) {
		switch modObj.Status.Category {
		case diagapi.ModuleStatus_Naples.String():
			if modObj.Status.Node == "" {
				r.logger.ErrorLog("method", "GetRoute", "msg", fmt.Sprintf("IP address not available for the DSC in module object: %#v", modObj.ObjectMeta))
				break // IP address for Naples not available yet
			}
			ip, _, err := net.ParseCIDR(modObj.Status.Node)
			if err != nil {
				return "", "", err
			}
			svcURL = fmt.Sprintf("https://%s:%s/api/diagnostics/", ip.String(), globals.AgentProxyPort)
			svcName = diagapi.ModuleStatus_Naples.String()
		default: // venice modules
			svcInstanceList := r.rslvr.Lookup(svcName)
			if svcInstanceList != nil {
				for _, svcInstance := range svcInstanceList.Items {
					if svcInstance.Node == modObj.Status.Node && svcInstance.Service == svcName {
						svcURL = svcInstance.URL
						break
					}
				}
			}
		}

	} else {
		return "", "", fmt.Errorf("diagnostics not supported for module [%s]", modObj.Name)
	}
	if svcURL == "" {
		return "", "", fmt.Errorf("unable to locate service instance for module [%s]", modObj.Name)
	}
	return svcURL, svcName, nil
}

// NewRouter returns router instance
func NewRouter(rslver resolver.Interface, moduleGetter Getter, l log.Logger) Router {
	return &router{
		rslvr:        rslver,
		moduleGetter: moduleGetter,
		logger:       l,
	}
}

func validateService(svcName string, modObj *diagapi.Module) bool {
	if modObj.Status.Category == diagapi.ModuleStatus_Naples.String() {
		return true // for naples modules, ServicePorts are not set
	}
	for _, port := range modObj.Status.ServicePorts {
		if port.Name == svcName {
			return true
		}
	}
	return false
}
