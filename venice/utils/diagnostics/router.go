package diagnostics

import (
	"fmt"

	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics/module"
	"github.com/pensando/sw/venice/utils/resolver"
)

type router struct {
	rslvr        resolver.Interface
	moduleGetter module.Getter
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
		svcInstanceList := r.rslvr.Lookup(svcName)
		if svcInstanceList != nil {
			for _, svcInstance := range svcInstanceList.Items {
				if svcInstance.Node == modObj.Status.Node && svcInstance.Service == svcName {
					svcURL = svcInstance.URL
					break
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
func NewRouter(rslver resolver.Interface, moduleGetter module.Getter) Router {
	return &router{
		rslvr:        rslver,
		moduleGetter: moduleGetter,
	}
}

func validateService(svcName string, modObj *diagapi.Module) bool {
	for _, port := range modObj.Status.ServicePorts {
		if port.Name == svcName {
			return true
		}
	}
	return false
}
