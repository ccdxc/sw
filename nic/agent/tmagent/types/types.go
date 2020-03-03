package types

import (
	"context"
	"net/http"

	"github.com/pensando/sw/nic/agent/protos/tpmprotos"
)

// CtrlerIntf provides all CRUD operations on telemetry policy objects
type CtrlerIntf interface {
	CreateFwlogPolicy(ctx context.Context, p *tpmprotos.FwlogPolicy) error
	GetFwlogPolicy(tx context.Context, p *tpmprotos.FwlogPolicy) (*tpmprotos.FwlogPolicy, error)
	ListFwlogPolicy(tx context.Context) ([]*tpmprotos.FwlogPolicy, error)
	UpdateFwlogPolicy(ctx context.Context, p *tpmprotos.FwlogPolicy) error
	DeleteFwlogPolicy(ctx context.Context, p *tpmprotos.FwlogPolicy) error
	Debug(r *http.Request) (interface{}, error)
}
