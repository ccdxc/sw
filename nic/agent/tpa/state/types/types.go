package types

import (
	"context"

	"github.com/pensando/sw/venice/ctrler/tpm/rpcserver/protos"
)

// CtrlerIntf provides all CRUD operations on telemetry policy objects
type CtrlerIntf interface {
	CreateFlowExportPolicy(ctx context.Context, p *tpmprotos.FlowExportPolicy) error
	GetFlowExportPolicy(tx context.Context, p *tpmprotos.FlowExportPolicy) (*tpmprotos.FlowExportPolicy, error)
	ListFlowExportPolicy(tx context.Context) ([]*tpmprotos.FlowExportPolicy, error)
	UpdateFlowExportPolicy(ctx context.Context, p *tpmprotos.FlowExportPolicy) error
	DeleteFlowExportPolicy(ctx context.Context, p *tpmprotos.FlowExportPolicy) error

	CreateFwLogPolicy(ctx context.Context, p *tpmprotos.FwlogPolicy) error
	GetFwLogPolicy(tx context.Context, p *tpmprotos.FwlogPolicy) (*tpmprotos.FwlogPolicy, error)
	ListFwLogPolicy(tx context.Context) ([]*tpmprotos.FwlogPolicy, error)
	UpdateFwLogPolicy(ctx context.Context, p *tpmprotos.FwlogPolicy) error
	DeleteFwLogPolicy(ctx context.Context, p *tpmprotos.FwlogPolicy) error
}
