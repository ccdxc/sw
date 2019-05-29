package service

import (
	"context"
	"expvar"
	"fmt"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	debugStats "github.com/pensando/sw/venice/utils/debug/stats"
	"github.com/pensando/sw/venice/utils/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics/protos"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	// ExpVarKey contains the name of expvar variable to be retrieved
	ExpVarKey = "expvar"
	// ActionKey contains actions like Clear to clear stats
	ActionKey = "action"
	// ClearAction to clear stats
	ClearAction = "clear"
	// GetAction to get stats
	GetAction = "get"
)

type expvarRetriever struct {
	module     string
	node       string
	category   diagapi.ModuleStatus_CategoryType
	debugStats *debugStats.Stats
	logger     log.Logger
}

func (e *expvarRetriever) HandleRequest(ctx context.Context, req *diagapi.DiagnosticsRequest) (*api.Any, error) {
	name := req.Parameters[ExpVarKey]
	action := req.Parameters[ActionKey]
	switch action {
	case ClearAction:
		debugStats.Clear()
	case GetAction:
		fallthrough
	default:
		m := make(map[string]string)
		if name == "" {
			// list all expvars
			expvar.Do(func(kv expvar.KeyValue) {
				m[kv.Key] = kv.Value.String()
			})
		} else {
			v := expvar.Get(name)
			if v != nil {
				m[name] = v.String()
			}
		}

		stats := &protos.Stats{
			Stats: m,
		}
		anyObj, err := types.MarshalAny(stats)
		if err != nil {
			e.logger.ErrorLog("method", "HandleRequest", "msg", "unable to marshal Stats {%+v} to Any object", m, "error", err)
			return nil, err
		}
		return &api.Any{Any: *anyObj}, nil
	}
	return nil, nil
}

func (e *expvarRetriever) Start() error {
	e.debugStats = debugStats.New(fmt.Sprintf("%s-%s", e.node, e.module)).Build()
	return nil
}

func (e *expvarRetriever) Stop() {
	e.debugStats.Close()
}

// NewExpVarHandler returns handler to export stats from expvars
func NewExpVarHandler(module, node string, category diagapi.ModuleStatus_CategoryType, logger log.Logger) diagnostics.Handler {
	return &expvarRetriever{
		module:   module,
		node:     node,
		category: category,
		logger:   logger,
	}

}
