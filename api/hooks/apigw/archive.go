package impl

import (
	"context"
	"errors"
	"fmt"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/objstore"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/archive"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

type archiveHooks struct {
	logger        log.Logger
	apiServer     string
	rslvr         resolver.Interface
	archiveGetter archive.Getter
	clientGetter  archive.ClientGetter // to support unit testing
}

// operations is a pre authz hook to determine authz.Operation
func (a *archiveHooks) operationsPreAuthzHook(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	user, ok := apigwpkg.UserFromContext(ctx)
	if !ok || user == nil {
		a.logger.ErrorLog("method", "operationsPreAuthzHook", "msg", "no user present in context passed to ArchiveRequest operations authz hook")
		return ctx, in, apigwpkg.ErrNoUserInContext
	}
	var resource authz.Resource
	var resourceType, bucketName string
	switch obj := in.(type) {
	case *monitoring.ArchiveRequest:
		if obj.Spec.Query == nil {
			obj.Spec.Query = &monitoring.ArchiveQuery{}
		}
		if len(obj.Spec.Query.Tenants) == 0 || user.Tenant != globals.DefaultTenant {
			obj.Spec.Query.Tenants = []string{user.Tenant}
		}
		// get existing operations from context
		operations, _ := apigwpkg.OperationsFromContext(ctx)
		switch obj.Spec.Type {
		case monitoring.ArchiveRequestSpec_AuditEvent.String():
			resourceType = auth.Permission_AuditEvent.String()
			bucketName = objstore.Buckets_auditevents.String()
		case monitoring.ArchiveRequestSpec_Event.String():
			resourceType = auth.Permission_Event.String()
			bucketName = objstore.Buckets_events.String()
		}
		for _, tenant := range obj.Spec.Query.Tenants {
			resource = authz.NewResource(
				tenant,
				"",
				resourceType,
				globals.DefaultNamespace,
				"",
			)
			operations = append(operations, authz.NewOperation(resource, auth.Permission_Read.String()))
		}
		// add permission check for creating object in objstore
		resource = authz.NewResource(obj.Tenant, string(apiclient.GroupObjstore), string(objstore.KindObject), bucketName, "")
		operations = append(operations, authz.NewOperation(resource, auth.Permission_Create.String()))

		nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
		return nctx, in, nil
	default:
		return ctx, in, errors.New("invalid input type")
	}
}

// cancelPreCallHook implements CancelArchiveRequest action
func (a *archiveHooks) cancelPreCallHook(ctx context.Context, in interface{}, out interface{}) (context.Context, interface{}, interface{}, bool, error) {
	obj, ok := in.(*monitoring.CancelArchiveRequest)
	if !ok {
		return ctx, nil, nil, true, errors.New("invalid input type")
	}
	archiveGetter := a.archiveGetter
	if archiveGetter == nil {
		archiveGetter = archive.NewGetter(a.apiServer, a.rslvr, a.logger)
	}
	req, err := archiveGetter.GetArchiveRequest(&obj.ObjectMeta)
	if err != nil {
		return ctx, nil, nil, true, err
	}
	a.logger.DebugLog("method", "cancelPreCallHook", "msg", fmt.Sprintf("request UUID %s", req.UUID))
	clGetter := a.clientGetter
	if clGetter == nil { // will be not nil for unit testing, will set a mock
		clGetter, err = archive.NewClientGetter(globals.APIGw, req.Spec.Type, a.rslvr, a.logger)
		if err != nil {
			a.logger.ErrorLog("method", "cancelPreCallHook", "msg", fmt.Sprintf("unable to instantiate ClientGetter to cancel archiverequest [%#v]", *obj), "error", err)
			return ctx, nil, nil, true, err
		}
	}
	archiveCl, err := clGetter.GetClient()
	if err != nil {
		a.logger.ErrorLog("method", "cancelPreCallHook", "msg", "failed to get archive client", "error", err)
		return ctx, nil, nil, true, err
	}
	defer archiveCl.Close()
	resp, err := archiveCl.CancelRequest(ctx, req)
	if err != nil {
		a.logger.ErrorLog("method", "cancelPreCallHook", "msg", fmt.Sprintf("rpc call CancelRequest failed for archive request [%#v] with name [%s]", *req, req.Name), "error", err)
		return ctx, nil, nil, true, err
	}
	return ctx, resp, out, true, nil
}

func (a *archiveHooks) registerCancelPreCallHook(svc apigw.APIGatewayService) error {
	prof, err := svc.GetServiceProfile("Cancel")
	if err != nil {
		return err
	}
	prof.AddPreCallHook(a.cancelPreCallHook)
	return nil
}

func (a *archiveHooks) registerOperationsPreAuthzHook(svc apigw.APIGatewayService) error {
	prof, err := svc.GetCrudServiceProfile("ArchiveRequest", apiintf.CreateOper)
	if err != nil {
		return err
	}
	prof.AddPreAuthZHook(a.operationsPreAuthzHook)
	return nil
}

func registerArchiveHooks(svc apigw.APIGatewayService, l log.Logger) error {
	gw := apigwpkg.MustGetAPIGateway()
	d := &archiveHooks{
		logger:    l,
		apiServer: gw.GetAPIServerAddr(globals.APIServer),
		rslvr:     gw.GetResolver(),
	}
	d.archiveGetter = archive.NewGetter(globals.APIServer, d.rslvr, d.logger)
	if err := d.registerCancelPreCallHook(svc); err != nil {
		return err
	}
	return d.registerOperationsPreAuthzHook(svc)

}
