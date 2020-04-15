package session

import (
	"context"
	"fmt"
	"net/url"
	"strings"
	"sync"
	"time"

	"github.com/vmware/govmomi"
	"github.com/vmware/govmomi/event"
	"github.com/vmware/govmomi/find"
	"github.com/vmware/govmomi/vapi/rest"
	"github.com/vmware/govmomi/vapi/tags"
	"github.com/vmware/govmomi/view"

	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/venice/utils/log"
)

/**
 * This package ensures that consumers are always using
 * a connected client. VC client related objects (client, viewMgr)
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

// ConnectionState contains info about the connection
type ConnectionState struct {
	State string
	Err   error
}

// Session is a struct for maintaining vCenter client objects.
type Session struct {
	ctx        context.Context
	ConnUpdate chan ConnectionState
	vcURL      *url.URL
	logger     log.Logger

	client    *govmomi.Client
	viewMgr   *view.Manager
	tagClient *tags.Manager
	eventMgr  *event.Manager

	WatcherWg    *sync.WaitGroup
	ClientCtx    context.Context
	clientCancel context.CancelFunc

	clientLock sync.RWMutex

	CheckSession    bool
	CheckTagSession bool
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
		ConnUpdate: make(chan ConnectionState, 100),
		LastEvent:  make(map[string]int32),
	}
}

// IsSessionReady returns whether the session is connected and ready for use
func (s *Session) IsSessionReady() bool {
	return s.SessionReady
}

// CreateFinder creates a new finder given a client
func (s *Session) CreateFinder(c *govmomi.Client) *find.Finder {
	return find.NewFinder(c.Client, true)
}

// GetEventManagerWithRLock returns EventManager while holding read lock
func (s *Session) GetEventManagerWithRLock() *event.Manager {
	s.clientLock.RLock()
	return s.eventMgr
}

// GetClientsWithRLock acquires a reader lock and returns the client objects
// Caller must call ReleaseClientsRLock when they are done
func (s *Session) GetClientsWithRLock() (*govmomi.Client, *view.Manager, *tags.Manager) {
	// Get lock
	s.clientLock.RLock()
	return s.client, s.viewMgr, s.tagClient
}

// GetClientWithRLock acquires a reader lock and returns the govmomi client
// Caller must call ReleaseClientsRLock when they are done
func (s *Session) GetClientWithRLock() *govmomi.Client {
	// Get lock
	s.clientLock.RLock()
	return s.client
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
	s.ClientCtx = nil
	if s.clientCancel != nil {
		s.clientCancel()
	}
	s.WatcherWg.Wait()
	s.client = nil
	s.viewMgr = nil
	s.tagClient = nil
	s.eventMgr = nil
	s.clientCancel = nil
}

// IsREST401 returns whether the given error is a 401 auth error
func (s *Session) IsREST401(err error) bool {
	if err == nil {
		return false
	}
	return strings.Contains(err.Error(), "401 Unauthorized")
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
				evt := ConnectionState{
					orchestration.OrchestratorStatus_Success.String(),
					nil,
				}
				s.ConnUpdate <- evt
				s.logger.Infof("Connection success")
				break
			}
			evt := ConnectionState{
				orchestration.OrchestratorStatus_Failure.String(),
				err,
			}
			s.ConnUpdate <- evt

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
		s.viewMgr = view.NewManager(c.Client)
		restCl := rest.NewClient(c.Client)
		s.tagClient = tags.NewManager(restCl)
		s.eventMgr = event.NewManager(c.Client)

		s.CheckSession = false
		s.CheckTagSession = false
		s.SessionReady = true

		err := s.tagClient.Login(s.ClientCtx, s.vcURL.User)
		if err != nil {
			// Unlikely for this to fail with 401, since we just logged in above
			s.logger.Errorf("Tags client failed to login, %s", err)
			s.CheckTagSession = true
		}

		s.clientLock.Unlock()

		s.logger.Infof("Session check starting...")

		// Start tag session check
		s.WatcherWg.Add(1)
		go func() {
			defer s.WatcherWg.Done()

			ctx := s.ClientCtx
			if ctx == nil {
				return
			}

			for {
				select {
				case <-s.ClientCtx.Done():
					return
				case <-time.After(5 * retryDelay):
					if s.CheckTagSession {
						// Re-authenticate tag session
						s.clientLock.Lock()
						err := s.tagClient.Login(s.ClientCtx, s.vcURL.User)
						if err != nil {
							s.logger.Errorf("Tags client failed to login, %s", err)
							evt := ConnectionState{
								orchestration.OrchestratorStatus_Degraded.String(),
								fmt.Errorf("Tags client received authentication error due to an invalid username or password. Tags/Labels functionality may be impacted"),
							}
							s.ConnUpdate <- evt
						} else {
							s.CheckTagSession = false
							evt := ConnectionState{
								orchestration.OrchestratorStatus_Success.String(),
								nil,
							}
							s.ConnUpdate <- evt
						}
						s.clientLock.Unlock()
					}
				}
			}
		}()

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
