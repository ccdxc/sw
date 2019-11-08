package audit

import (
	"encoding/json"
	"fmt"
	"net/http"
	"net/http/httputil"
	"reflect"
	"strings"

	"github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/monitoring"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/ref"
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
		dumpBody := body
		if event.Level == audit.Level_Basic.String() || event.Level == audit.Level_Response.String() {
			dumpBody = false
		}
		switch obj := reqObj.(type) {
		case *http.Request: // for reverse proxy use cases
			// do not log sensitive headers
			cookie := obj.Header.Get("Cookie")
			if cookie != "" {
				obj.Header.Del("Cookie")
				defer obj.Header.Set("Cookie", cookie)
			}
			authzHdr := obj.Header.Get("Authorization")
			if authzHdr != "" {
				obj.Header.Del("Authorization")
				defer obj.Header.Set("Authorization", authzHdr)
			}
			b, err := httputil.DumpRequest(obj, dumpBody)
			if err != nil {
				return err
			}
			event.RequestObject = string(b)
		default: // for API server, search, metrics, events
			if dumpBody {
				objCopy := ref.DeepCopy(obj)
				txfrm := reflect.ValueOf(objCopy).MethodByName("EraseSecrets")
				if txfrm.IsValid() {
					txfrm.Call([]reflect.Value{})
				}
				b, err := json.Marshal(objCopy)
				if err != nil {
					return err
				}
				event.RequestObject = string(b)
			}
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
		dumpBody := body
		if event.Level == audit.Level_Basic.String() || event.Level == audit.Level_Request.String() {
			dumpBody = false
		}
		switch obj := resObj.(type) {
		case *http.Response: // for reverse proxy use cases
			b, err := httputil.DumpResponse(obj, dumpBody)
			if err != nil {
				return err
			}
			event.ResponseObject = string(b)
		default: // for API server, search, metrics, events
			if dumpBody {
				objCopy := ref.DeepCopy(obj)
				txfrm := reflect.ValueOf(objCopy).MethodByName("EraseSecrets")
				if txfrm.IsValid() {
					txfrm.Call([]reflect.Value{})
				}
				b, err := json.Marshal(objCopy)
				if err != nil {
					return err
				}
				event.ResponseObject = string(b)
			}
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

func (p *policyChecker) PopulateEvent(event *audit.Event, populators ...EventPopulator) (bool, bool, error) {
	failOp := true
	switch event.Resource.Kind {
	// do not log search, events, audit, metrics queries, fwlogs queries
	case auth.Permission_Search.String(), auth.Permission_Event.String(), auth.Permission_MetricsQuery.String(), auth.Permission_FwlogsQuery.String(), auth.Permission_AuditEvent.String():
		return false, false, nil
	case auth.Permission_TokenAuth.String():
		event.ServiceName = globals.Cmd
	case string(monitoring.KindTechSupportRequest):
		failOp = false
		fallthrough
	default:
		switch event.Action {
		// do not log reads
		case strings.Title(string(apiintf.GetOper)), strings.Title(string(apiintf.ListOper)), strings.Title(string(apiintf.WatchOper)), auth.Permission_Read.String(), "get":
			return false, false, nil
		}
		event.ServiceName = globals.APIServer
	}
	for _, populator := range populators {
		err := populator(event)
		if err != nil {
			return false, failOp, err
		}
	}
	return true, failOp, nil
}

// NewPolicyChecker creates PolicyChecker to populate information in audit event based on audit policy
func NewPolicyChecker() PolicyChecker {
	return &policyChecker{}
}

// GetAuditLevelFromString gets the audit level given the string.
func GetAuditLevelFromString(level string) (audit.Level, error) {
	switch level {
	case audit.Level_Basic.String():
		return audit.Level_Basic, nil
	case audit.Level_Request.String():
		return audit.Level_Request, nil
	case audit.Level_Response.String():
		return audit.Level_Response, nil
	case audit.Level_RequestResponse.String():
		return audit.Level_RequestResponse, nil
	default:
		return audit.Level_Basic, fmt.Errorf("unknown level")
	}
}
