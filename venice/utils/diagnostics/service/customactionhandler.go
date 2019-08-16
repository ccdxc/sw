package service

import (
	"context"
	"fmt"
	"strings"

	"encoding/json"
	"sync"

	"github.com/gogo/protobuf/proto"
	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics/protos"
)

type customActionHandler struct {
	sync.Mutex
	actions      map[string]diagnostics.CustomHandler
	knownActions []string
}

// HandleRequest processes diagnostic query specified in the request
func (d *customActionHandler) HandleRequest(ctx context.Context, req *diagapi.DiagnosticsRequest) (*api.Any, error) {
	action, ok := req.Parameters[diagapi.DiagnosticsRequest_Action.String()]
	if !ok {
		return nil, fmt.Errorf("Action not specified")
	}
	params := req.Parameters

	defer d.Unlock()
	d.Lock()
	cb, ok := d.actions[action]
	if !ok {
		return nil, fmt.Errorf("[%v] action is unknown. Valid actions are %v", action, d.knownActions)
	}
	ret, err := cb(action, params)
	if err != nil {
		return nil, err
	}
	if pb, ok := ret.(proto.Message); ok {
		anyObj, err := types.MarshalAny(pb)
		if err != nil {
			return nil, fmt.Errorf("error marshalling response (%s)", err)
		}
		return &api.Any{Any: *anyObj}, nil
	}

	bytes, err := json.Marshal(ret)
	if err != nil {
		return nil, fmt.Errorf("error marshalling response (%s)", err)
	}
	pobj := protos.String{Content: strings.Replace(string(bytes), "\\\"", "\"", -1)}

	anyObj, err := types.MarshalAny(&pobj)
	if err != nil {
		return nil, fmt.Errorf("error marshalling response (%s)", err)
	}
	return &api.Any{Any: *anyObj}, nil
}

// Start initializes the handler. If the handler is already started it should be a no-op
func (d *customActionHandler) Start() error {
	return nil
}

// Stop stops the handler. If the handler is already stopped it should be a no-op
func (d *customActionHandler) Stop() {}
