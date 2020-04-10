package impl

import (
	"context"
	"fmt"
	"net/url"
	"strings"

	"github.com/vmware/govmomi/vim25/soap"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/orchestration"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/globals"
	authzgrpcctx "github.com/pensando/sw/venice/utils/authz/grpc/context"
	"github.com/pensando/sw/venice/utils/ctxutils"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	maxOrchSupported = 64
)

// ErrOrchManaged is the error message returned when a user tries to modify an object created by Orchhub
var ErrOrchManaged = fmt.Errorf("object is managed by Venice and can only be deleted by Admin")

type orchHooks struct {
	logger log.Logger
}

func (o *orchHooks) validateOrchestrator(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string,
	oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	o.logger.DebugLog("method", "validateOrchestrator", "msg", "Validating Orch config")

	orch, ok := i.(orchestration.Orchestrator)
	if !ok {
		o.logger.ErrorLog("method", "validateOrchestrator", "msg", fmt.Sprintf("called for invalid object type [%#v]", i))
		return i, true, fmt.Errorf("Invalid input type")
	}

	var orchs orchestration.OrchestratorList
	kindKey := orchs.MakeKey(string(apiclient.GroupOrchestration))
	err := kv.List(ctx, kindKey, &orchs)
	if err != nil {
		return nil, true, fmt.Errorf("Error retrieving orchestrators: %v", err)
	}

	o.logger.DebugLog("retrieved %d items", len(orchs.Items))
	if oper == apiintf.CreateOper && len(orchs.Items) >= maxOrchSupported {
		return nil, true, fmt.Errorf("Unable to support more than %v orchestrators", maxOrchSupported)
	}

	// Verify URI
	vcURL := &url.URL{
		Scheme: "https",
		Host:   orch.Spec.URI,
		Path:   "/sdk",
	}
	if _, err := soap.ParseURL(vcURL.String()); err != nil {
		o.logger.Infof("Failed to parse URI %s: %s", orch.Spec.URI, err)
		return i, true, fmt.Errorf("%s is not a valid URI", orch.Spec.URI)
	}

	var foundOrch *orchestration.Orchestrator
	for _, otherOrch := range orchs.Items {
		if otherOrch.Name == orch.Name {
			foundOrch = otherOrch
			continue
		}

		if otherOrch.Spec.URI == orch.Spec.URI {
			return i, true, fmt.Errorf("URI is already used by Orchestrator %s", otherOrch.Name)
		}
	}

	if orch.Spec.Credentials == nil {
		return i, true, fmt.Errorf("Orch %v has credentials missing", orch.Name)
	}

	switch orch.Spec.Credentials.AuthType {
	case monitoring.ExportAuthType_AUTHTYPE_USERNAMEPASSWORD.String():
		if len(orch.Spec.Credentials.UserName) == 0 {
			return i, true, fmt.Errorf("Credentials for orchestrator %v missing username", orch.Name)
		}

		if len(orch.Spec.Credentials.Password) == 0 {
			return i, true, fmt.Errorf("Credentials for orchestrator %v missing password", orch.Name)
		}

		if len(orch.Spec.Credentials.BearerToken) > 0 || len(orch.Spec.Credentials.KeyData) > 0 || len(orch.Spec.Credentials.CertData) > 0 || len(orch.Spec.Credentials.CaData) > 0 {
			return i, true, fmt.Errorf("Credentials for orchestrator %v has unnecessary fields passed", orch.Name)
		}
	case monitoring.ExportAuthType_AUTHTYPE_TOKEN.String():
		if len(orch.Spec.Credentials.BearerToken) == 0 {
			return i, true, fmt.Errorf("Credentials for orchestrator %v missing token", orch.Name)
		}

		if len(orch.Spec.Credentials.KeyData) > 0 || len(orch.Spec.Credentials.CertData) > 0 || len(orch.Spec.Credentials.CaData) > 0 || len(orch.Spec.Credentials.UserName) > 0 || len(orch.Spec.Credentials.Password) > 0 {
			return i, true, fmt.Errorf("Credentials for orchestrator %v has unnecessary fields passed", orch.Name)
		}
	case monitoring.ExportAuthType_AUTHTYPE_CERTS.String():
		if len(orch.Spec.Credentials.KeyData) == 0 || len(orch.Spec.Credentials.CertData) == 0 || len(orch.Spec.Credentials.CaData) == 0 {
			return i, true, fmt.Errorf("Credentials for orchestrator %v missing fields", orch.Name)
		}

		if len(orch.Spec.Credentials.UserName) > 0 || len(orch.Spec.Credentials.Password) > 0 || len(orch.Spec.Credentials.BearerToken) > 0 {
			return i, true, fmt.Errorf("Credentials for orchestrator %v has unnecessary fields passed", orch.Name)
		}
	case monitoring.ExportAuthType_AUTHTYPE_NONE.String():
		fallthrough
	default:
		return i, true, fmt.Errorf("Unsupported auth type [%v] passed in orchestrator %v", orch.Spec.Credentials.AuthType, orch.Name)
	}

	isManageAll := false
	if len(orch.Spec.ManageNamespaces) > 0 {
		for ii := range orch.Spec.ManageNamespaces {
			orch.Spec.ManageNamespaces[ii] = strings.TrimSpace(orch.Spec.ManageNamespaces[ii])

			if orch.Spec.ManageNamespaces[ii] == utils.ManageAllDcs {
				isManageAll = true
				break
			}
		}
	}

	if isManageAll && len(orch.Spec.ManageNamespaces) > 1 {
		return i, true, fmt.Errorf("If [%v] is passed, then no other namespace should be passed in the Manage Namespace list", utils.ManageAllDcs)
	}

	// All DCs will be included in managedAll, we don't have to check for Namespaces in use right now
	if !isManageAll && oper == apiintf.UpdateOper {
		if foundOrch == nil {
			return i, true, fmt.Errorf("orchestrator %v not found in the orchestrator list", orch.Name)
		}

		_, deleted, _ := utils.DiffNamespace(foundOrch.Spec.ManageNamespaces, orch.Spec.ManageNamespaces)
		if len(deleted) > 0 {
			// Fetch networks
			// Listing within any hook has potential to return stale information as there is no lock for collections.
			// For cases when the managed namespace is no longer used by network, but the network list returned
			// still has references to the removed namespace, we will disallow update.
			// This can be fixed by a retry of the Orchestrator Update operation.
			var networks network.NetworkList
			listKey := strings.TrimSuffix(networks.MakeKey(string(apiclient.GroupNetwork)), "/")
			err = kv.List(ctx, listKey, &networks)
			if err != nil {
				return i, true, fmt.Errorf("Error retrieving networks: %s", err)
			}

			for _, delNs := range deleted {
				for _, network := range networks.Items {
					for _, o := range network.Spec.Orchestrators {
						if delNs == o.Namespace {
							return i, true, fmt.Errorf("Managed Namespace [%v] still in use by network [%v]", delNs, network.Name)
						}
					}
				}
			}
		}
	}

	return orch, true, nil
}

func createOrchCheckHook(kind string) apiserver.PreCommitFunc {
	return func(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {

		existingObj, err := runtime.GetDefaultScheme().New(kind)
		err = kvs.Get(ctx, key, existingObj)
		if err != nil {
			// Update/delete call will return 404
			return i, true, nil
		}

		existingObjMeta, err := runtime.GetObjectMeta(existingObj)
		if err != nil {
			return i, true, fmt.Errorf("Failed to extract object meta: %s", err)
		}

		if ctxutils.GetPeerID(ctx) != globals.APIGw {
			// No checks required if coming from inside venice
			return i, true, nil
		}

		// If the request is a delete, and the user is super admin, no need to check
		if oper == apiintf.DeleteOper {
			isAdmin, ok := authzgrpcctx.UserIsAdminFromIncomingContext(ctx)
			if !ok {
				return i, true, fmt.Errorf("Failed to get user role from context")
			}
			if isAdmin {
				// Admin is allowed to delete all objects
				return i, true, nil
			}
		}
		for k := range existingObjMeta.Labels {
			// if it has orch-name label then the object cannot be deleted unless the user is admin
			if k == utils.OrchNameKey {
				return i, true, ErrOrchManaged
			}
		}
		return i, true, nil
	}
}

func registerOrchestrationHooks(svc apiserver.Service, l log.Logger) {
	l.Log("msg", "registering Hooks")
	oh := orchHooks{logger: l.WithContext("Service", "Orchestrator")}
	svc.GetCrudService("Orchestrator", apiintf.CreateOper).WithPreCommitHook(oh.validateOrchestrator)
	svc.GetCrudService("Orchestrator", apiintf.UpdateOper).WithPreCommitHook(oh.validateOrchestrator)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("orchestration.OrchestratorV1", registerOrchestrationHooks)
}
