package impl

import (
	"context"

	"github.com/gogo/protobuf/proto"
	"github.com/gogo/protobuf/types"
	"github.com/pkg/errors"
	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/cache"
	"github.com/pensando/sw/api/generated/staging"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

type stagingHooks struct {
	l   log.Logger
	svc apiserver.Service
}

// SetStagingBufferInGrpcMD sets the GRPC metadata with buffer ID
func SetStagingBufferInGrpcMD(ctx context.Context, id string) context.Context {
	pairs := metadata.Pairs(apiserver.RequestParamStagingBufferID, id)
	inMd, ok := metadata.FromIncomingContext(ctx)
	if ok {
		nMd := metadata.Join(inMd, pairs)
		return metadata.NewIncomingContext(ctx, nMd)
	}
	return metadata.NewIncomingContext(ctx, pairs)
}

func (h *stagingHooks) updateStatus(ctx context.Context, buf *staging.Buffer) (*staging.Buffer, error) {
	ov, err := cache.GetOverlay(buf.Tenant, buf.Name)
	if err != nil {
		h.l.Errorf("could not find overlay (%s/%s) in get post commit hook", buf.Tenant, buf.Name)
		return buf, errors.New("could not find overlay")
	}
	nctx := SetStagingBufferInGrpcMD(ctx, buf.Name)
	status, err := ov.Verify(nctx)
	if err != nil {
		h.l.InfoLog("msg", "error verifying overlay", "tenant", buf.Tenant, "buffer", buf.Name, "error", err)
	} else {
		if len(status.Failed) == 0 {
			buf.Status.ValidationResult = staging.BufferStatus_SUCCESS.String()
		} else {
			buf.Status.ValidationResult = staging.BufferStatus_FAILED.String()
		}
		for it := range status.Items {
			p, err := types.MarshalAny(status.Items[it].Object.(proto.Message))
			if err != nil {
				h.l.ErrorLog("msg", "failed to marshalAny", "tenant", buf.Tenant, "buffer", buf.Name, "error", err)
			}
			item := &staging.Item{
				ItemId: staging.ItemId{
					URI:    status.Items[it].Key.URI,
					Method: status.Items[it].Key.Oper,
				},
				Object: &api.Any{Any: *p},
			}
			buf.Status.Items = append(buf.Status.Items, item)
		}
		h.l.Infof("added %d items to status", len(buf.Status.Items))
		for it := range status.Failed {

			item := &staging.ValidationError{
				ItemId: staging.ItemId{
					URI: status.Failed[it].Key.URI,
				},
			}
			for j := range status.Failed[it].Errors {
				item.Errors = append(item.Errors, status.Failed[it].Errors[j].Error())
			}
			buf.Status.Errors = append(buf.Status.Errors, item)
		}
	}
	h.l.Infof("returning %d items and %d errors", len(buf.Status.Items), len(buf.Status.Errors))
	return buf, nil
}

// getBuffer modifies the response to a get request on the buffer. The plain KVStore config object is updated with the status including
//  verification state of the staging buffer
func (h *stagingHooks) getBuffer(ctx context.Context, kvs kvstore.Interface, prefix string, i, old, resp interface{}, oper apiserver.APIOperType) (interface{}, error) {
	h.l.Infof("received response writer hook for get Buffer")
	buf, ok := resp.(staging.Buffer)
	if !ok {
		h.l.ErrorLog("invalid object in post commit hook for staging GetBuffer")
	}
	h.l.Infof("in get buffer item count is %d", len(buf.Status.Items))
	_, err := h.updateStatus(ctx, &buf)
	return buf, err
}

func (h *stagingHooks) listBuffer(ctx context.Context, kvs kvstore.Interface, prefix string, i, old, resp interface{}, oper apiserver.APIOperType) (interface{}, error) {
	h.l.InfoLog("msg", "received listBuffer response writer hook")
	list, ok := resp.(staging.BufferList)
	if !ok {
		h.l.Errorf("invalid object in post commit hook for staging GetBuffer [%+v]", i)
	}
	h.l.Infof("received buffer list has length %d", len(list.Items))
	for k, v := range list.Items {
		list.Items[k], _ = h.updateStatus(ctx, v)
	}
	return list, nil
}

func (h *stagingHooks) createBuffer(ctx context.Context, oper apiserver.APIOperType, i interface{}, dryrun bool) {
	// Create an overlay on the cache.
	h.l.InfoLog("msg", "received createBuffer postCommit Hook")
	if buf, ok := i.(staging.Buffer); ok {
		apisrv := apisrvpkg.MustGetAPIServer()
		_, err := apisrv.CreateOverlay(buf.Tenant, buf.Name, globals.StagingBasePath)
		h.l.Infof("Create call for staging buffer ID %s/%s (%s)", buf.Tenant, buf.Name, err)
	}
}

func (h *stagingHooks) deleteBuffer(ctx context.Context, oper apiserver.APIOperType, i interface{}, dryrun bool) {
	h.l.InfoLog("msg", "received deleteBuffer postCommit Hook")
	if buf, ok := i.(staging.Buffer); ok {
		cache.DelOverlay(buf.Tenant, buf.Name)
	}
}

func (h *stagingHooks) commitAction(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiserver.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {
	h.l.InfoLog("msg", "received commitAction preCommit Hook")
	buf, ok := i.(staging.CommitAction)
	if !ok {
		h.l.ErrorLog("invalid object in post commit hook for staging Commit")
		return nil, false, errors.New("invalid object")
	}
	ov, err := cache.GetOverlay(buf.Tenant, buf.Name)
	if err != nil {
		h.l.Errorf("could not find overlay (%s/%s) in Commit action hook (%s)", buf.Tenant, buf.Name, err)
		return nil, false, err
	}
	nctx := SetStagingBufferInGrpcMD(ctx, buf.Name)
	err = ov.Commit(nctx, nil)
	if err != nil {
		buf.Status.Status = staging.CommitActionStatus_FAILED.String()
		buf.Status.Reason = err.Error()
	} else {
		buf.Status.Status = staging.CommitActionStatus_SUCCESS.String()
	}
	return buf, false, nil
}

func (h *stagingHooks) clearAction(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiserver.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {
	h.l.InfoLog("msg", "received clearAction preCommit Hook")
	buf, ok := i.(staging.ClearAction)
	if !ok {
		h.l.ErrorLog("invalid object in post commit hook for staging Clear")
		return nil, false, errors.New("invalid object")
	}
	ov, err := cache.GetOverlay(buf.Tenant, buf.Name)
	if err != nil {
		h.l.Errorf("could not find overlay (%s/%s) in Clear action hook (%s)", buf.Tenant, buf.Name, err)
		return nil, false, err
	}
	var items []apiintf.OverlayKey
	for _, v := range buf.Spec.Items {
		item := apiintf.OverlayKey{
			URI:  v.URI,
			Oper: v.Method,
		}
		items = append(items, item)
	}
	nctx := SetStagingBufferInGrpcMD(ctx, buf.Name)
	err = ov.ClearBuffer(nctx, items)
	if err != nil {
		buf.Status.Status = staging.ClearActionStatus_FAILED.String()
		buf.Status.Reason = err.Error()
	} else {
		buf.Status.Status = staging.ClearActionStatus_SUCCESS.String()
	}
	return buf, false, nil
}

func registerStagingHooks(svc apiserver.Service, logger log.Logger) {
	h := stagingHooks{}
	h.svc = svc
	h.l = logger
	logger.InfoLog("Service", "StagingV1", "msg", "registering hooks")
	svc.GetCrudService("Buffer", apiserver.CreateOper).WithPostCommitHook(h.createBuffer)
	svc.GetCrudService("Buffer", apiserver.DeleteOper).WithPostCommitHook(h.deleteBuffer)
	svc.GetCrudService("Buffer", apiserver.GetOper).WithResponseWriter(h.getBuffer)
	svc.GetCrudService("Buffer", apiserver.ListOper).WithResponseWriter(h.listBuffer)
	svc.GetMethod("Commit").WithPreCommitHook(h.commitAction)
	svc.GetMethod("Clear").WithPreCommitHook(h.clearAction)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("staging.StagingV1", registerStagingHooks)
}
