// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package dscagent

import (
	"sync"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/controller"
	"github.com/pensando/sw/nic/agent/dscagent/infra"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/venice/utils/log"
)

// DSCAgent implements unified config agents
type DSCAgent struct {
	sync.Mutex
	ControllerAPI types.ControllerAPI
	PipelineAPI   types.PipelineAPI
	InfraAPI      types.InfraAPI
	Logger        log.Logger
}

// NewDSCAgent returns a new instance of DSCAgent
func NewDSCAgent(logger log.Logger, npmURL, tpmURL, tsmURL, restURL string) (*DSCAgent, error) {
	infraAPI, err := infra.NewInfraAPI(types.NetagentPrimaryDBPath, types.NetagentBackupDBPath)
	if err != nil {
		return nil, err
	}

	pipelineAPI, err := pipeline.NewPipelineAPI(infraAPI)
	if err != nil {
		return nil, err
	}

	d := DSCAgent{
		PipelineAPI:   pipelineAPI,
		ControllerAPI: controller.NewControllerAPI(pipelineAPI, infraAPI, npmURL, tpmURL, tsmURL, restURL),
		InfraAPI:      infraAPI,
		Logger:        logger,
	}

	return &d, nil
}

// Stop stops DSCAgent
func (ag *DSCAgent) Stop() {
	if ag.Logger != nil {
		ag.Logger.Close()
	}

	if err := ag.ControllerAPI.Stop(); err != nil {
		log.Error(errors.Wrapf(types.ErrControllerWatcherStop, "Controller API: %s", err))
	}

	ag.InfraAPI.Close()
}
