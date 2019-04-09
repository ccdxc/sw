package state

import (
	tsconfig "github.com/pensando/sw/venice/ctrler/tsm/config"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	action "github.com/pensando/sw/venice/utils/techsupport/actionengine"
	work "github.com/pensando/sw/venice/utils/techsupport/workq"
)

// State State of techsupport
type State struct {
	ResolverConfig *resolver.Config
	Cfg            *tsconfig.TechSupportConfig
	RQ             *work.WorkQ
}

// NewState has all the state information which is shared across various components of TechSupport
func NewState(configPath string) *State {
	log.Infof("Creating new support state.")

	res, err := action.ReadConfig(configPath)
	if err != nil {
		log.Errorf("Failed to read config file :%v. Err: %v", configPath, err)
		return nil
	}

	return &State{
		Cfg: res,
		RQ:  work.NewWorkQ(5),
	}
}
