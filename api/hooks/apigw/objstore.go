package impl

import (
	"context"

	"github.com/pensando/sw/api/generated/audit"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"

	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/log"
)

type objstoreHooks struct {
	permissionGetter rbac.PermissionGetter
	logger           log.Logger
}

func (b *objstoreHooks) addObjUploadImageOps(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	user, ok := apigwpkg.UserFromContext(ctx)
	if !ok || user == nil {
		b.logger.Errorf("no user present in context passed to addObjUploadOps authz hook")
		return ctx, in, apigwpkg.ErrNoUserInContext
	}

	// Uploads are allowed only for the default tenat now.
	resource := authz.NewResource(globals.DefaultTenant, string(apiclient.GroupObjstore), string(objstore.KindObject), string(objstore.Buckets_images), "")
	// get existing operations from context
	operations, _ := apigwpkg.OperationsFromContext(ctx)
	// append requested operation
	operations = append(operations, authz.NewOperation(resource, auth.Permission_Create.String()))

	nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
	return nctx, in, nil
}

func (b *objstoreHooks) addObjUploadSnapshotsOps(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	user, ok := apigwpkg.UserFromContext(ctx)
	if !ok || user == nil {
		b.logger.Errorf("no user present in context passed to addObjUploadOps authz hook")
		return ctx, in, apigwpkg.ErrNoUserInContext
	}

	// Uploads are allowed only for the default tenat now.
	resource := authz.NewResource(globals.DefaultTenant, string(apiclient.GroupObjstore), string(objstore.KindObject), string(objstore.Buckets_snapshots), "")
	// get existing operations from context
	operations, _ := apigwpkg.OperationsFromContext(ctx)
	// append requested operation
	operations = append(operations, authz.NewOperation(resource, auth.Permission_Create.String()))

	nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
	return nctx, in, nil
}

func (b *objstoreHooks) addObjDownloadOps(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	user, ok := apigwpkg.UserFromContext(ctx)
	if !ok || user == nil {
		b.logger.Errorf("no user present in context passed to addObjDownloadOps authz hook")
		return ctx, in, apigwpkg.ErrNoUserInContext
	}
	obj, ok := in.(*objstore.Object)
	if !ok {
		return ctx, in, errors.New("invalid input type")
	}
	resource := authz.NewResource(obj.Tenant, string(apiclient.GroupObjstore), string(objstore.KindObject), obj.Namespace, obj.Name)
	// get existing operations from context
	operations, _ := apigwpkg.OperationsFromContext(ctx)
	// append requested operation
	operations = append(operations, authz.NewOperation(resource, auth.Permission_Read.String()))

	nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
	return nctx, in, nil
}

func (b *objstoreHooks) addObjDownloadFileByPrefixOps(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	user, ok := apigwpkg.UserFromContext(ctx)
	if !ok || user == nil {
		b.logger.Errorf("no user present in context passed to addObjDownloadFileByPrefixOps authz hook")
		return ctx, in, apigwpkg.ErrNoUserInContext
	}

	// Downloads are allowed only for the default tenat now.
	resource := authz.NewResource(globals.DefaultTenant, string(apiclient.GroupObjstore), "", string(objstore.Buckets_images), "")
	// get existing operations from context
	operations, _ := apigwpkg.OperationsFromContext(ctx)
	// append requested operation
	operations = append(operations, authz.NewOperation(resource, auth.Permission_Read.String()))

	nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
	return nctx, in, nil
}

// userContext is a pre-call hook to set user and permissions in grpc metadata in outgoing context
func (b *objstoreHooks) userContext(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	b.logger.DebugLog("msg", "APIGw userContext pre-call hook called for Objstore userContext")
	nctx, err := newContextWithUserPerms(ctx, b.permissionGetter, b.logger)
	if err != nil {
		return ctx, in, true, err
	}
	return nctx, in, false, nil
}

func registerObjstoreHooks(svc apigw.APIGatewayService, l log.Logger) error {
	gw := apigwpkg.MustGetAPIGateway()
	grpcaddr := globals.APIServer
	grpcaddr = gw.GetAPIServerAddr(grpcaddr)
	r := objstoreHooks{logger: l, permissionGetter: rbac.GetPermissionGetter(globals.APIGw, grpcaddr, gw.GetResolver())}

	prof, err := svc.GetProxyServiceProfile("/uploads/images")
	if err != nil {
		return err
	}
	prof.AddPreAuthZHook(r.addObjUploadImageOps)
	prof.AddPreCallHook(r.userContext)
	prof.SetAuditLevel(audit.Level_Basic.String())

	prof, err = svc.GetProxyServiceProfile("/uploads/snapshots")
	if err != nil {
		return err
	}
	prof.AddPreAuthZHook(r.addObjUploadSnapshotsOps)
	prof.AddPreCallHook(r.userContext)
	prof.SetAuditLevel(audit.Level_Basic.String())

	prof, err = svc.GetServiceProfile("DownloadFile")
	if err != nil {
		return err
	}
	prof.AddPreAuthZHook(r.addObjDownloadOps)
	prof.SetAuditLevel(audit.Level_Basic.String())

	prof, err = svc.GetServiceProfile("DownloadFileByPrefix")
	if err != nil {
		return err
	}
	prof.AddPreAuthZHook(r.addObjDownloadFileByPrefixOps)
	prof, err = svc.GetCrudServiceProfile("Object", apiintf.ListOper)
	if err != nil {
		return err
	}
	prof.AddPreCallHook(r.userContext)

	prof, err = svc.GetCrudServiceProfile("Object", apiintf.WatchOper)
	if err != nil {
		return err
	}
	prof.AddPreCallHook(r.userContext)
	return nil
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("objstore.ObjstoreV1", registerObjstoreHooks)
}
