package impl

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"net/http/httptest"
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/objstore"
	apiintf "github.com/pensando/sw/api/interfaces"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apigw/pkg/mocks"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
	. "github.com/pensando/sw/venice/utils/authz/testutils"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestUploadPostCallHook(t *testing.T) {
	tests := []struct {
		name               string
		in                 interface{}
		operations         []authz.Operation
		expectedOperations []authz.Operation
		err                bool
	}{
		{
			name: "upload image",
			in: &objstore.Object{
				TypeMeta: api.TypeMeta{Kind: string(objstore.KindObject)},
				ObjectMeta: api.ObjectMeta{
					Tenant:    globals.DefaultTenant,
					Name:      "bundle.tar",
					Namespace: globals.DefaultNamespace,
				},
			},
			operations: []authz.Operation{
				authz.NewAuditOperation(authz.NewResource(globals.DefaultTenant,
					string(apiclient.GroupObjstore), string(objstore.KindObject),
					string(objstore.Buckets_images), ""),
					auth.Permission_Create.String(), "Create"),
			},
			expectedOperations: []authz.Operation{
				authz.NewAuditOperation(authz.NewResource(globals.DefaultTenant,
					string(apiclient.GroupObjstore), string(objstore.KindObject),
					string(objstore.Buckets_images), "bundle.tar"),
					auth.Permission_Create.String(), "Create"),
			},
			err: false,
		},
		{
			name: "invalid object",
			in:   &struct{ name string }{name: "invalid object type"},
			operations: []authz.Operation{
				authz.NewAuditOperation(authz.NewResource(globals.DefaultTenant,
					string(apiclient.GroupObjstore), string(objstore.KindObject),
					string(objstore.Buckets_images), ""),
					auth.Permission_Create.String(), "Create"),
			},
			expectedOperations: []authz.Operation{
				authz.NewAuditOperation(authz.NewResource(globals.DefaultTenant,
					string(apiclient.GroupObjstore), string(objstore.KindObject),
					string(objstore.Buckets_images), ""),
					auth.Permission_Create.String(), "Create"),
			},
			err: false,
		},
	}
	r := &objstoreHooks{}
	logConfig := log.GetDefaultConfig("TestAPIGwObjstoreHooks")
	r.logger = log.GetNewLogger(logConfig)
	for _, test := range tests {
		nctx := apigwpkg.NewContextWithOperations(context.TODO(), test.operations...)
		objw := &objectWriter{
			ResponseWriter: httptest.NewRecorder(),
			body:           new(bytes.Buffer),
		}
		b, err := json.Marshal(test.in)
		AssertOk(t, err, fmt.Sprintf("[%s] test failed, failed to marshal object into json", test.name))
		objw.WriteHeader(200)
		objw.Write(b)
		nctx, out, err := r.uploadPostCallHook(nctx, objw)
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
		operations, _ := apigwpkg.OperationsFromContext(nctx)
		Assert(t, AreOperationsEqual(test.expectedOperations, operations),
			fmt.Sprintf("unexpected operations, [%s] test failed, expected: %s, got: %s", test.name, authz.PrintOperations(test.expectedOperations), authz.PrintOperations(operations)))
		Assert(t, reflect.DeepEqual(objw, out),
			fmt.Sprintf("expected returned object [%v], got [%v], [%s] test failed", objw, out, test.name))
	}
}

func TestObjstoreHooksRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwObjstoreHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &objstoreHooks{}
	r.logger = l
	err := r.registerObjstoreHooks(svc)
	AssertOk(t, err, "apigw objstore hook registration failed")
	prof, err := svc.GetProxyServiceProfile("/uploads/images")
	AssertOk(t, err, "error getting service profile for /uploads/images")
	Assert(t, len(prof.PreAuthZHooks()) == 1, fmt.Sprintf("unexpected number of pre authz hooks [%d] for /uploads/images profile", len(prof.PreAuthZHooks())))
	Assert(t, len(prof.PreCallHooks()) == 2, fmt.Sprintf("unexpected number of pre call hooks [%d] for /uploads/images profile", len(prof.PreCallHooks())))
	Assert(t, len(prof.PostCallHooks()) == 1, fmt.Sprintf("unexpected number of post call hooks [%d] for /uploads/images profile", len(prof.PostCallHooks())))

	prof, err = svc.GetProxyServiceProfile("/uploads/snapshots")
	AssertOk(t, err, "error getting service profile for /uploads/snapshots")
	Assert(t, len(prof.PreAuthZHooks()) == 1, fmt.Sprintf("unexpected number of pre authz hooks [%d] for /uploads/snapshots profile", len(prof.PreAuthZHooks())))
	Assert(t, len(prof.PreCallHooks()) == 2, fmt.Sprintf("unexpected number of pre call hooks [%d] for /uploads/snapshots profile", len(prof.PreCallHooks())))
	Assert(t, len(prof.PostCallHooks()) == 1, fmt.Sprintf("unexpected number of post call hooks [%d] for /uploads/snapshots profile", len(prof.PostCallHooks())))

	prof, err = svc.GetServiceProfile("DownloadFile")
	AssertOk(t, err, "error getting service profile for DownloadFile")
	Assert(t, len(prof.PreAuthZHooks()) == 1, fmt.Sprintf("unexpected number of pre authz hooks [%d] for DownloadFile profile", len(prof.PreAuthZHooks())))

	prof, err = svc.GetServiceProfile("DownloadFileByPrefix")
	AssertOk(t, err, "error getting service profile for DownloadFileByPrefix")
	Assert(t, len(prof.PreAuthZHooks()) == 1, fmt.Sprintf("unexpected number of pre authz hooks [%d] for DownloadFile profile", len(prof.PreAuthZHooks())))

	prof, err = svc.GetCrudServiceProfile("Object", apiintf.ListOper)
	AssertOk(t, err, "error getting service profile for Object List")
	Assert(t, len(prof.PreCallHooks()) == 1, fmt.Sprintf("unexpected number of pre call hooks [%d] for Object list profile", len(prof.PreCallHooks())))

	prof, err = svc.GetCrudServiceProfile("Object", apiintf.WatchOper)
	AssertOk(t, err, "error getting service profile for Object Watch")
	Assert(t, len(prof.PreCallHooks()) == 1, fmt.Sprintf("unexpected number of pre call hooks [%d] for Object watch profile", len(prof.PreCallHooks())))

	// test error
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerObjstoreHooks(svc)
	Assert(t, err != nil, "expected error in objstore hook registration")
}
