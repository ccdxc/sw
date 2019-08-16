// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

// gRPC wrapper library that provides additional functionality for Pensando
// cluster clients and server (TLS, service-discovery, load-balancing, etc.)

package rpckit

// To be used for testing only

// SetDefaultListenerConnectionBurst sets the burst size for default rpckit rate limiter
func SetDefaultListenerConnectionBurst(b int) {
	defaultListenerConnectionBurst = b
}

// SetDefaultListenerConnectionRateLimit sets the rate (connections/s) for default rpckit rate limiter
func SetDefaultListenerConnectionRateLimit(r float64) {
	defaultListenerConnectionRateLimit = r
}
