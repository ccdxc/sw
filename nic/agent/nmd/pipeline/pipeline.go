package pipeline

import (
	"errors"

	"github.com/pensando/sw/nic/agent/nmd/pipeline/apollo"
	"github.com/pensando/sw/nic/agent/nmd/pipeline/iris"
	"github.com/pensando/sw/nic/agent/nmd/state"
	"github.com/pensando/sw/venice/globals"
)

// NewPipeline to get the interface implementation on a per-pipeline basis
func NewPipeline(kind state.Kind) (state.Pipeline, error) {
	if kind.String() == globals.NaplesPipelineIris {
		ret, _ := iris.NewPipeline()
		return ret, nil
	} else if kind.String() == globals.NaplesPipelineApollo {
		ret, _ := apollo.NewPipeline()
		return ret, nil
	}
	return nil, errors.New("Unknown pipeline " + kind.String())

}
