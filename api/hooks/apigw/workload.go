package impl

import (
	"context"
	"errors"
	"fmt"

	"github.com/pensando/sw/api/generated/workload"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/log"
)

type workloadHooks struct {
	permissionGetter rbac.PermissionGetter
	logger           log.Logger
}

// userContext is a pre-call hook to set user and permissions in grpc metadata in outgoing context
func (a *workloadHooks) userContext(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	a.logger.DebugLog("msg", "APIGw userContext pre-call hook called")
	switch in.(type) {
	// check read authorization for sg policy included in policy search request
	case *workload.Workload:
	default:
		return ctx, in, true, errors.New("invalid input type")
	}
	nctx, err := newContextWithUserPerms(ctx, a.permissionGetter, a.logger)
	if err != nil {
		return ctx, in, true, err
	}
	return nctx, in, false, nil
}

// validateInterfaces validates the interfaces of the workload
func (a *workloadHooks) validateInterfaces(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	a.logger.DebugLog("msg", "APIGw validateInterfaces pre-call hook called")
	wrkld, ok := in.(*workload.Workload)
	if !ok {
		return ctx, in, true, errors.New("invalid input type")
	}
	for i, inf := range wrkld.Spec.Interfaces {
		if inf.ExternalVlan != 0 && len(inf.Network) != 0 {
			return ctx, in, true, fmt.Errorf("Interface %d: External Vlan and Network cannot both be specified", i)
		}
	}
	return ctx, in, false, nil
}

func (a *workloadHooks) registerWorkloadHooks(svc apigw.APIGatewayService) error {
	prof, err := svc.GetCrudServiceProfile("Workload", apiintf.DeleteOper)
	if err != nil {
		return err
	}
	prof.AddPreCallHook(a.userContext)

	prof, err = svc.GetCrudServiceProfile("Workload", apiintf.CreateOper)
	if err != nil {
		return err
	}
	prof.AddPreCallHook(a.validateInterfaces)
	prof, err = svc.GetCrudServiceProfile("Workload", apiintf.UpdateOper)
	if err != nil {
		return err
	}
	prof.AddPreCallHook(a.validateInterfaces)
	return nil
}

func registerWorkloadHooks(svc apigw.APIGatewayService, l log.Logger) error {
	gw := apigwpkg.MustGetAPIGateway()
	grpcaddr := gw.GetAPIServerAddr(globals.APIServer)
	a := workloadHooks{
		permissionGetter: rbac.GetPermissionGetter(globals.APIGw, grpcaddr, gw.GetResolver()),
		logger:           l,
	}
	return a.registerWorkloadHooks(svc)
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("workload.WorkloadV1", registerWorkloadHooks)
}
