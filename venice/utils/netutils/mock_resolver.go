// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package netutils

import (
	"context"
	"fmt"
)

// MockResolver is a mock partial implementation of go net.Resolver
type MockResolver struct {
	hostMap map[string][]string
}

// AddHost adds a host-to-addresses mapping
func (r *MockResolver) AddHost(host string, addrs []string) {
	r.hostMap[host] = addrs
}

// DeleteHost removes a host-to-addresses mapping
func (r *MockResolver) DeleteHost(host string) bool {
	_, ok := r.hostMap[host]
	if !ok {
		return false
	}
	delete(r.hostMap, host)
	return true
}

// LookupHost returns the addresses associated with the specified host
func (r *MockResolver) LookupHost(ctx context.Context, host string) (addrs []string, err error) {
	addrs, ok := r.hostMap[host]
	if !ok {
		return nil, fmt.Errorf("Host not found")
	}
	return addrs, nil
}

// NewMockResolver returns a new instance of the mock resolver
func NewMockResolver() *MockResolver {
	return &MockResolver{
		hostMap: make(map[string][]string),
	}
}
