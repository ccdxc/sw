// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package rpckit

import (
	"net"
	"sync"
)

// rpcListener is a wrapper struct for net.Listener
type rpcListener struct {
	net.Listener
	cond   *sync.Cond
	refCnt int
}

//---------------------- internal methods of rpcListener -----------------------
// incrementRef increments connection ref count
func (s *rpcListener) incrementRef() {
	s.cond.L.Lock()
	s.refCnt++
	s.cond.L.Unlock()
}

// decrementRef decrements connection ref count
func (s *rpcListener) decrementRef() {
	s.cond.L.Lock()
	s.refCnt--
	newRefs := s.refCnt
	s.cond.L.Unlock()
	if newRefs == 0 {
		s.cond.Broadcast()
	}
}

// ------------------------------- net.Listener interface ----------------------

// Accept is a wrapper over regular Accept call
// which also maintains the refCnt
func (s *rpcListener) Accept() (net.Conn, error) {
	s.incrementRef()
	defer s.decrementRef()
	return s.Listener.Accept()
}

// Close closes the contivListener.
func (s *rpcListener) Close() error {
	if err := s.Listener.Close(); err != nil {
		return err
	}

	s.cond.L.Lock()
	for s.refCnt > 0 {
		s.cond.Wait()
	}
	s.cond.L.Unlock()
	return nil
}

// ListenWrapper is a wrapper over net.Listener
func ListenWrapper(l net.Listener) net.Listener {
	return &rpcListener{
		Listener: l,
		cond:     sync.NewCond(&sync.Mutex{})}
}
