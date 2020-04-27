package impl

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"net/http"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/objstore"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
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
func (b *objstoreHooks) userContext(ctx context.Context, in, out interface{}) (context.Context, interface{}, interface{}, bool, error) {
	b.logger.DebugLog("msg", "APIGw userContext pre-call hook called for Objstore userContext")
	nctx, err := newContextWithUserPerms(ctx, b.permissionGetter, b.logger)
	if err != nil {
		return ctx, in, out, true, err
	}
	return nctx, in, out, false, nil
}

// uploadPreCallHook wraps http.ResponseWriter in objectWriter to capture the response and marshal it into objstore.Object
func (b *objstoreHooks) uploadPreCallHook(ctx context.Context, in, out interface{}) (context.Context, interface{}, interface{}, bool, error) {
	nctx := ctx
	switch obj := out.(type) {
	case http.ResponseWriter:
		nout := &objectWriter{
			ResponseWriter: obj,
			body:           new(bytes.Buffer),
		}
		return nctx, in, nout, false, nil
	default:
		b.logger.ErrorLog("method", "uploadPreCallHook", "msg", fmt.Sprintf("invalid input type: %#v", out))
		return ctx, in, out, true, errors.New("invalid input type")
	}
}

// uploadPostCallHook modifies operation to include uploaded filename captured from the response so that filename is captured in audit logs
func (b *objstoreHooks) uploadPostCallHook(ctx context.Context, out interface{}) (context.Context, interface{}, error) {
	nctx := ctx
	switch objw := out.(type) {
	case *objectWriter:
		obj, err := objw.GetObject()
		if err != nil {
			b.logger.InfoLog("method", "uploadPostCallHook", "msg", "unable to extract object from response", "error", err)
			return ctx, out, nil
		}
		var nOps []authz.Operation
		operations, _ := apigwpkg.OperationsFromContext(ctx)
		for _, op := range operations {
			if op.GetResource().GetKind() == string(objstore.KindObject) {
				resource := op.GetResource()
				nOp := authz.NewOperationWithID(authz.NewResource(resource.GetTenant(), resource.GetGroup(), resource.GetKind(), resource.GetNamespace(), obj.Name),
					op.GetAction(), op.GetAuditAction(), op.GetID())
				nOps = append(nOps, nOp)
			} else {
				nOps = append(nOps, op)
			}
		}
		nctx = apigwpkg.NewContextWithOperations(ctx, nOps...)
	default:
		b.logger.ErrorLog("method", "uploadPostCallHook", "msg", fmt.Sprintf("invalid input type: %#v", out))
		return ctx, out, errors.New("invalid input type")
	}
	return nctx, out, nil
}

func (b *objstoreHooks) registerObjstoreHooks(svc apigw.APIGatewayService) error {
	prof, err := svc.GetProxyServiceProfile("/uploads/images")
	if err != nil {
		return err
	}
	prof.AddPreAuthZHook(b.addObjUploadImageOps)
	prof.AddPreCallHook(b.userContext)
	prof.AddPreCallHook(b.uploadPreCallHook)
	prof.AddPostCallHook(b.uploadPostCallHook)
	prof.SetAuditLevel(audit.Level_Basic.String())

	prof, err = svc.GetProxyServiceProfile("/uploads/snapshots")
	if err != nil {
		return err
	}
	prof.AddPreAuthZHook(b.addObjUploadSnapshotsOps)
	prof.AddPreCallHook(b.userContext)
	prof.AddPreCallHook(b.uploadPreCallHook)
	prof.AddPostCallHook(b.uploadPostCallHook)
	prof.SetAuditLevel(audit.Level_Basic.String())

	prof, err = svc.GetServiceProfile("DownloadFile")
	if err != nil {
		return err
	}
	prof.AddPreAuthZHook(b.addObjDownloadOps)
	prof.SetAuditLevel(audit.Level_Basic.String())

	prof, err = svc.GetServiceProfile("DownloadFileByPrefix")
	if err != nil {
		return err
	}
	prof.AddPreAuthZHook(b.addObjDownloadFileByPrefixOps)
	prof, err = svc.GetCrudServiceProfile("Object", apiintf.ListOper)
	if err != nil {
		return err
	}
	prof.AddPreCallHook(b.userContext)

	prof, err = svc.GetCrudServiceProfile("Object", apiintf.WatchOper)
	if err != nil {
		return err
	}
	prof.AddPreCallHook(b.userContext)
	return nil
}

func registerObjstoreHooks(svc apigw.APIGatewayService, l log.Logger) error {
	gw := apigwpkg.MustGetAPIGateway()
	grpcaddr := gw.GetAPIServerAddr(globals.APIServer)
	r := objstoreHooks{logger: l, permissionGetter: rbac.GetPermissionGetter(globals.APIGw, grpcaddr, gw.GetResolver())}
	return r.registerObjstoreHooks(svc)
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("objstore.ObjstoreV1", registerObjstoreHooks)
}

// objectWriter is a wrapper around http.ResponseWriter to capture the response and marshal it into objstore.Object to determine name of the uploaded file
type objectWriter struct {
	http.ResponseWriter
	body        *bytes.Buffer
	wroteHeader bool
	statusCode  int
}

func (ow *objectWriter) WriteHeader(code int) {
	ow.ResponseWriter.WriteHeader(code)
	if ow.wroteHeader {
		return
	}
	ow.statusCode = code
	ow.wroteHeader = true
}

func (ow *objectWriter) Write(buf []byte) (int, error) {
	if ow.wroteHeader && ow.body != nil {
		ow.body.Write(buf)
	}
	return ow.ResponseWriter.Write(buf)
}

func (ow *objectWriter) GetObject() (*objstore.Object, error) {
	if ow.statusCode == 200 && ow.body != nil {
		obj := &objstore.Object{}
		if err := json.Unmarshal(ow.body.Bytes(), obj); err != nil {
			return nil, err
		}
		return obj, nil
	}
	return nil, errors.New("unable to get object from response")
}
