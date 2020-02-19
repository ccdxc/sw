package session

import (
	"context"
	"net/url"
	"sync"
	"time"

	"github.com/vmware/govmomi"
	"github.com/vmware/govmomi/event"
	"github.com/vmware/govmomi/find"
	"github.com/vmware/govmomi/vapi/rest"
	"github.com/vmware/govmomi/vapi/tags"
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
	ConnUpdate chan error // Boolean of the status of the session
	vcURL      *url.URL
	logger     log.Logger

	finder    *find.Finder
	client    *govmomi.Client
	viewMgr   *view.Manager
	tagClient *tags.Manager
	eventMgr  *event.Manager

	WatcherWg    *sync.WaitGroup
	ClientCtx    context.Context
	clientCancel context.CancelFunc

	clientLock sync.RWMutex

	CheckSession bool
	// SessionReady indicates whether watchers should join the wg or not.
	// When we cancel the watcherWg, we don't want watchers adding themselves back on before
	// all of them have finished cancelling.
	SessionReady     bool
	LastEvent        map[string]int32 // last event processed for a given vc object (datacenter)
	EventTrackerLock sync.Mutex
}

// NewSession returns a new session object
func NewSession(ctx context.Context, VcURL *url.URL, logger log.Logger) *Session {
	return &Session{
		ctx:        ctx,
		vcURL:      VcURL,
		logger:     logger,
		WatcherWg:  &sync.WaitGroup{},
		ConnUpdate: make(chan error, 100),
		LastEvent:  make(map[string]int32),
	}
}

// IsSessionReady returns whether the session is connected and ready for use
func (s *Session) IsSessionReady() bool {
	return s.SessionReady
}

// GetEventManagerWithRLock returns EventManager while holding read lock
func (s *Session) GetEventManagerWithRLock() *event.Manager {
	s.clientLock.RLock()
	return s.eventMgr
}

// GetClientsWithRLock acquires a reader lock and returns the client objects
// Caller must call ReleaseClientsRLock when they are done
func (s *Session) GetClientsWithRLock() (*govmomi.Client, *find.Finder, *view.Manager, *tags.Manager) {
	// Get lock
	s.clientLock.RLock()
	return s.client, s.finder, s.viewMgr, s.tagClient
}

// GetClientWithRLock acquires a reader lock and returns the govmomi client
// Caller must call ReleaseClientsRLock when they are done
func (s *Session) GetClientWithRLock() *govmomi.Client {
	// Get lock
	s.clientLock.RLock()
	return s.client
}

// GetFinderWithRLock acquires a reader lock and returns the finder object
// Caller must call ReleaseClientsRLock when they are done
func (s *Session) GetFinderWithRLock() *find.Finder {
	// Get lock
	s.clientLock.RLock()
	return s.finder
}

// GetViewManagerWithRLock acquires a reader lock and returns the view manager object
// Caller must call ReleaseClientsRLock when they are done
func (s *Session) GetViewManagerWithRLock() *view.Manager {
	// Get lock
	s.clientLock.RLock()
	return s.viewMgr
}

// GetTagClientWithRLock acquires a reader lock and returns the tag client object
// Caller must call ReleaseClientsRLock when they are done
func (s *Session) GetTagClientWithRLock() *tags.Manager {
	// Get lock
	s.clientLock.RLock()
	return s.tagClient
}

// ReleaseClientsRLock releases the client lock
func (s *Session) ReleaseClientsRLock() {
	s.clientLock.RUnlock()
}

// ClearSessionWithLock Acquires clientLock and sets internal state to be nil
func (s *Session) ClearSessionWithLock() {
	s.clientLock.Lock()
	defer s.clientLock.Unlock()
	s.clearSession()
}

func (s *Session) clearSession() {
	s.logger.Debug("Clearing session")
	s.SessionReady = false
	if s.client != nil {
		// Using background context since it's likely that
		// VCHub's context has been cancelled
		err := s.client.Logout(context.Background())
		if err != nil {
			s.logger.Errorf("Received err while logging out %s", err)
		}
	}
	if s.tagClient != nil {
		// Using background context since it's likely that
		// VCHub's context has been cancelled
		err := s.tagClient.Logout(context.Background())
		if err != nil {
			s.logger.Errorf("Received err while logging tag client out %s", err)
		}
	}
	// Stop watchers and Event receivers
	if s.clientCancel != nil {
		s.clientCancel()
	}
	s.WatcherWg.Wait()
	s.client = nil
	s.finder = nil
	s.viewMgr = nil
	s.tagClient = nil
	s.eventMgr = nil
	s.clientCancel = nil
	s.ClientCtx = nil
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
		s.ClientCtx, s.clientCancel = context.WithCancel(s.ctx)

		// Forever try to login until it succeeds
		for {
			c, err = govmomi.NewClient(s.ClientCtx, s.vcURL, true)
			if err == nil {
				s.ConnUpdate <- nil
				s.logger.Infof("Connection success")
				break
			}
			s.ConnUpdate <- err

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
		restCl := rest.NewClient(c.Client)
		s.tagClient = tags.NewManager(restCl)
		s.eventMgr = event.NewManager(c.Client)

		s.CheckSession = false
		s.SessionReady = true

		err := s.tagClient.Login(s.ClientCtx, s.vcURL.User)
		if err != nil {
			s.logger.Errorf("Tags client failed to login, %s", err)
			s.CheckSession = true
		}

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
					client := s.GetClientWithRLock()
					active, err := client.SessionManager.SessionIsActive(s.ClientCtx)
					if err != nil {
						count--
						s.logger.Errorf("Received err %v while testing session, attempts left %d", err, count)
					} else if active {
						count = retryCount
						s.CheckSession = false
						s.logger.Debugf("Connection status is active, setting check session back to false")
					} else {
						count--
						s.logger.Infof("Session is not active.. retrying, attempts left %d", count)
					}
					s.ReleaseClientsRLock()
				}
			}
		}
		// Logout, stop watchers and retry
		// Lock will not be released until
		// we re-establish the client, or the ctx is cancelled
		s.clientLock.Lock()
		s.clearSession()
		s.logger.Infof("Attempting to rebuild session")
		// Hold onto lock
	}
	// Exiting run release lock
	s.clientLock.Unlock()
	s.logger.Infof("Session check exiting")
}
