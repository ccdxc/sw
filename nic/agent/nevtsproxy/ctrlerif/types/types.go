package types

import (
	"context"

	"github.com/pensando/sw/venice/ctrler/evtsmgr/rpcserver/protos"
)

// CtrlerIntf provides all CRUD operations on event policy object
// Implemented in nevtsproxy/ctrlerif/
type CtrlerIntf interface {
	CreateEventPolicy(ctx context.Context, e *evtsmgrprotos.EventPolicy) error
	GetEventPolicy(tx context.Context, e *evtsmgrprotos.EventPolicy) (*evtsmgrprotos.EventPolicy, error)
	ListEventPolicy(tx context.Context) ([]*evtsmgrprotos.EventPolicy, error)
	UpdateEventPolicy(ctx context.Context, e *evtsmgrprotos.EventPolicy) error
	DeleteEventPolicy(ctx context.Context, e *evtsmgrprotos.EventPolicy) error
}
