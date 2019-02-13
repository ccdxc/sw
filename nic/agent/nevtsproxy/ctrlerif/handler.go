package ctrlerif

import (
	"context"
	"fmt"
	"net"
	"strconv"
	"strings"

	"github.com/pensando/sw/nic/agent/nevtsproxy/ctrlerif/types"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/rpcserver/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events/policy"
	"github.com/pensando/sw/venice/utils/log"
)

// EventPolicyHandler represents the REST handler from event policy CRUD operations
type EventPolicyHandler struct {
	policyMgr *policy.Manager // event policy manager
	logger    log.Logger      // logger
}

// NewEventPolicyHandler creates a new event policy handler with the agent store and policy manager
func NewEventPolicyHandler(policyMgr *policy.Manager, logger log.Logger) (types.CtrlerIntf, error) {
	if policyMgr == nil {
		return nil, fmt.Errorf("events policy mgr is required")
	}

	return &EventPolicyHandler{policyMgr: policyMgr, logger: logger}, nil
}

// CreateEventPolicy creates the event policy in the store; creates the associated exporters using policy manager
func (h *EventPolicyHandler) CreateEventPolicy(ctx context.Context, e *evtsmgrprotos.EventPolicy) error {
	if err := h.validate(e); err != nil {
		return err
	}

	return h.policyMgr.Create(e)
}

// GetEventPolicy finds and returns the policy associated with given e.ObjectMeta from the store
func (h *EventPolicyHandler) GetEventPolicy(ctx context.Context, e *evtsmgrprotos.EventPolicy) (*evtsmgrprotos.EventPolicy, error) {
	return h.policyMgr.Get(e)
}

// ListEventPolicy retrieves and returns the list of event policies from store
func (h *EventPolicyHandler) ListEventPolicy(ctx context.Context) ([]*evtsmgrprotos.EventPolicy, error) {
	return h.policyMgr.List()
}

// UpdateEventPolicy updates the policy e; creates/deletes the associated exporters using policy manager
func (h *EventPolicyHandler) UpdateEventPolicy(ctx context.Context, e *evtsmgrprotos.EventPolicy) error {
	if err := h.validate(e); err != nil {
		return err
	}

	return h.policyMgr.Update(e)
}

// DeleteEventPolicy deletes the given policy from store; also deletes the associated exporters using policy manager
func (h *EventPolicyHandler) DeleteEventPolicy(ctx context.Context, e *evtsmgrprotos.EventPolicy) error {
	if utils.IsEmpty(e.GetTenant()) {
		e.Tenant = globals.DefaultTenant
	}

	if utils.IsEmpty(e.GetNamespace()) {
		e.Namespace = globals.DefaultNamespace
	}

	return h.policyMgr.Delete(e)
}

// validate validates the given event policy
func (h *EventPolicyHandler) validate(e *evtsmgrprotos.EventPolicy) error {
	if utils.IsEmpty(e.GetName()) {
		return fmt.Errorf("no name provided in the config")
	}

	if utils.IsEmpty(e.GetTenant()) {
		e.Tenant = globals.DefaultTenant
	}

	if utils.IsEmpty(e.GetNamespace()) {
		e.Namespace = globals.DefaultNamespace
	}

	if len(e.Spec.Targets) == 0 {
		return fmt.Errorf("no targets provided in the config")
	}

	for _, target := range e.Spec.Targets {
		if err := h.validateDestination(target.GetDestination()); err != nil {
			return err
		}

		if err := h.validateTransport(target.GetTransport()); err != nil {
			return err
		}
	}

	return nil
}

func (h *EventPolicyHandler) validateDestination(destination string) error {
	if utils.IsEmpty(destination) {
		return fmt.Errorf("destination cannot be empty")
	}

	netIP := net.ParseIP(destination)
	if netIP == nil { // treat it as hostname and resolve
		if _, err := net.LookupHost(destination); err != nil {
			return fmt.Errorf("failed to resolve destination {%s}, err: %v", destination, err)
		}
	}

	return nil
}

func (h *EventPolicyHandler) validateTransport(transport string) error {
	tr := strings.Split(transport, "/")
	if len(tr) != 2 {
		return fmt.Errorf("transport should be in protocol/port format")
	}

	// check protocol
	if _, ok := map[string]bool{
		"tcp": true,
		"udp": true,
	}[tr[0]]; !ok {
		return fmt.Errorf("invalid protocol in %s", transport)
	}

	// check port
	port, err := strconv.Atoi(tr[1])
	if err != nil {
		return fmt.Errorf("invalid port in %s", transport)
	}
	if uint(port) > uint(^uint16(0)) {
		return fmt.Errorf("invalid port (> %d) in %s", ^uint16(0), transport)
	}

	return nil
}
