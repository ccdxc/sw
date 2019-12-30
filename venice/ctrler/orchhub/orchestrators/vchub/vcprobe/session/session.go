package session

import (
	"context"
	"net/url"
	"sync"
	"time"

	"github.com/vmware/govmomi"
	"github.com/vmware/govmomi/find"
	"github.com/vmware/govmomi/view"

	"github.com/pensando/sw/venice/utils/log"
)

/**
 * This package ensures that consumers are always using
 * a connected client. VC client related objects (finder, client, viewMgr)
 * are only available through methods that first acquire a lock. The consumer
 * is expected to call the release lock function once they are done with the client.
 * This lock will only be free when SessionCheck is not re-establishing
 * the connection.
 *
 * Similarly, the  watcher related objects are locked so that when
 * session is re-initialzing the watcher ctx, no watchers are restarting their connection.
 */

const (
	retryDelay = time.Second
	retryCount = 3
)

// Session is a struct for maintaining vCenter client objects.
type Session struct {
	ctx        context.Context
	ConnUpdate chan bool // Boolean of the status of the session
	vcURL      *url.URL
	logger     log.Logger

	finder  *find.Finder
	client  *govmomi.Client
	viewMgr *view.Manager

	WatcherWg     *sync.WaitGroup
	ClientCtx     context.Context
	watcherCancel context.CancelFunc

	clientLock sync.RWMutex

	CheckSession bool
	// SessionReady indicates whether watchers should join the wg or not.
	// When we cancel the watcherWg, we don't want watchers adding themselves back on before
	// all of them have finished cancelling.
	SessionReady bool
}

// NewSession returns a new session object
func NewSession(ctx context.Context, VcURL *url.URL, logger log.Logger) *Session {
	return &Session{
		ctx:        ctx,
		vcURL:      VcURL,
		logger:     logger,
		WatcherWg:  &sync.WaitGroup{},
		ConnUpdate: make(chan bool, 100),
	}
}

// ClearClientCtx sets the watcher ctx to be nil
func (s *Session) ClearClientCtx() {
	s.logger.Info("Clearing watcher ctx")
	s.ClientCtx = nil
}

// GetClientWithRLock acquires a reader lock and returns the client objects
// Caller must call ReleaseClientRLock when they are done
func (s *Session) GetClientWithRLock() (*govmomi.Client, *find.Finder, *view.Manager) {
	// Get lock
	s.clientLock.RLock()
	return s.client, s.finder, s.viewMgr
}

// ReleaseClientRLock releases the client lock
func (s *Session) ReleaseClientRLock() {
	s.clientLock.RUnlock()
}

// ClearSession sets internal state to be nil
func (s *Session) ClearSession() {
	s.logger.Debug("Clearing session")
	s.clientLock.Lock()
	defer s.clientLock.Unlock()
	if s.client != nil {
		// Using background context since it's likely that
		// VCHub's context has been cancelled
		err := s.client.Logout(context.Background())
		if err != nil {
			s.logger.Errorf("Received err while logging out %s", err)
		}
	}
	s.client = nil
	s.finder = nil
	s.viewMgr = nil
}

// PeriodicSessionCheck starts a goroutine that re-establishes the session
// if it goes down at any point
func (s *Session) PeriodicSessionCheck(wg *sync.WaitGroup) {
	defer wg.Done()
	var c *govmomi.Client
	var err error

	s.logger.Infof("Starting session check")

	s.clientLock.Lock()
	// Connect and login to vcenter
	for {
		if s.ctx.Err() != nil {
			break
		}
		// Set ClientCtx
		s.ClientCtx, s.watcherCancel = context.WithCancel(s.ctx)

		// Forever try to login until it succeeds
		for {
			c, err = govmomi.NewClient(s.ClientCtx, s.vcURL, true)
			if err == nil {
				s.ConnUpdate <- true
				s.logger.Infof("Connection success")
				break
			}
			s.ConnUpdate <- false

			s.logger.Errorf("login failed: %v", err)
			select {
			case <-s.ctx.Done():
				s.clientLock.Unlock()
				s.logger.Infof("Session check exiting")
				return
			case <-time.After(5 * retryDelay):
			}
		}

		s.client = c
		s.finder = find.NewFinder(c.Client, true)
		s.viewMgr = view.NewManager(c.Client)

		s.CheckSession = false
		s.SessionReady = true

		s.clientLock.Unlock()

		s.logger.Infof("Session check starting...")
		count := retryCount
		for count > 0 {
			select {
			case <-s.ctx.Done():
				s.logger.Infof("Session check exiting")
				return
			case <-time.After(retryDelay):
				// if any of the watchers is experiencing the problem, check the session state
				if s.CheckSession {
					client, _, _ := s.GetClientWithRLock()
					active, err := client.SessionManager.SessionIsActive(s.ClientCtx)
					if err != nil {
						s.logger.Errorf("Received err %v while testing session", err)
						count--
					} else if active {
						count = retryCount
						s.CheckSession = false
					} else {
						s.logger.Infof("Session is not active .. retry")
						count--
					}
					s.ReleaseClientRLock()
				}
			}
		}
		// Logout, stop watchers and retry
		// Lock will not be released until
		// we re-establish the client, or the ctx is cancelled
		s.clientLock.Lock()
		s.SessionReady = false
		if s.watcherCancel != nil {
			s.watcherCancel()
		}
		s.WatcherWg.Wait()
		if s.client != nil {
			s.client.Logout(s.ClientCtx)
		}
		s.client = nil
		s.finder = nil
		s.viewMgr = nil
		s.watcherCancel = nil
		// Hold onto lock
	}
	// Exiting run release lock
	s.clientLock.Unlock()
	s.logger.Infof("Session check exiting")
}
