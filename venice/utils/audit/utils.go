package audit

import (
	"encoding/json"
	"net/http"
	"net/http/httputil"

	"github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/globals"
)

const (
	// APIErrorKey in audit event data map to save API error
	APIErrorKey = "error"
)

// NewRequestObjectPopulator populates audit event with request object. This will body from http request if API Gateway acts as a reverse proxy for instance for object store
func NewRequestObjectPopulator(reqObj interface{}, body bool) EventPopulator {
	return func(event *audit.Event) error {
		if reqObj == nil {
			return nil
		}
		switch obj := reqObj.(type) {
		case *http.Request: // for reverse proxy use cases
			// TODO: clear cookie header
			b, err := httputil.DumpRequest(obj, body)
			if err != nil {
				return err
			}
			event.RequestObject = string(b)
		default: // for API server, search, metrics, events
			b, err := json.Marshal(reqObj)
			if err != nil {
				return err
			}
			event.RequestObject = string(b)
		}
		return nil
	}
}

// NewResponseObjectPopulator populates audit event with response object. This will body from http response if API Gateway acts as a reverse proxy for instance for object store
func NewResponseObjectPopulator(resObj interface{}, body bool) EventPopulator {
	return func(event *audit.Event) error {
		if resObj == nil {
			return nil
		}
		switch obj := resObj.(type) {
		case *http.Response: // for reverse proxy use cases
			b, err := httputil.DumpResponse(obj, body)
			if err != nil {
				return err
			}
			event.ResponseObject = string(b)
		default: // for API server, search, metrics, events
			b, err := json.Marshal(resObj)
			if err != nil {
				return err
			}
			event.ResponseObject = string(b)
		}
		return nil
	}
}

// NewErrorPopulator adds error information to Data map in audit event
func NewErrorPopulator(apierr error) EventPopulator {
	return func(event *audit.Event) error {
		if apierr == nil {
			return nil
		}
		status := apierrors.FromError(apierr)
		b, err := json.Marshal(status)
		if err != nil {
			return err
		}
		event.Data[APIErrorKey] = string(b)
		return nil
	}
}

type policyChecker struct{}

func (p *policyChecker) PopulateEvent(event *audit.Event, populators ...EventPopulator) (bool, error) {
	switch event.Resource.Kind {
	// do not log search, events, audit, metrics queries
	case auth.Permission_Search.String(), auth.Permission_Event.String(), auth.Permission_MetricsQuery.String(), auth.Permission_AuditEvent.String():
		return false, nil
	default:
		switch event.Action {
		// do not log reads
		case auth.Permission_Read.String(), "get":
			return false, nil
		}
		event.ServiceName = globals.APIServer
	}
	for _, populator := range populators {
		err := populator(event)
		if err != nil {
			return false, err
		}
	}
	return true, nil
}

// NewPolicyChecker creates PolicyChecker to populate information in audit event based on audit policy
func NewPolicyChecker() PolicyChecker {
	return &policyChecker{}
}
