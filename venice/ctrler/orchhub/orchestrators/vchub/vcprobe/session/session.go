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
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
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
	retryCount           = 3
	sessionCheckDelay    = time.Second
	tagCheckDelay        = 5 * time.Second
	connectionCheckDelay = time.Second
	versionCheckDelay    = 300 * time.Second // when connected to unsupported version, try every 5 min
)

var (
	supportedVersionPrefixes = []string{
		"6.5.",
		"6.7.",
		"7.0.",
	}
)

// ConnectionState contains info about the connection
type ConnectionState struct {
	State string
	Err   error
}

// Session is a struct for maintaining vCenter client objects.
type Session struct {
	ctx        context.Context
	ConnUpdate chan<- defs.Probe2StoreMsg
	vcURL      *url.URL
	logger     log.Logger
	OrchConfig *orchestration.Orchestrator

	client    *govmomi.Client
	viewMgr   *view.Manager
	tagClient *tags.Manager
	eventMgr  *event.Manager

	WatcherWg    *sync.WaitGroup
	ClientCtx    context.Context
	clientCancel context.CancelFunc

	CheckSession    bool
	CheckTagSession bool
	// SessionReady indicates whether watchers should join the wg or not.
	// When we cancel the watcherWg, we don't want watchers adding themselves back on before
	// all of them have finished cancelling.
	sessionLock      sync.Mutex
	SessionReady     bool
	LastEvent        map[string]int32 // last event processed for a given vc object (datacenter)
	EventTrackerLock sync.Mutex
}

// NewSession returns a new session object
func NewSession(ctx context.Context, outbox chan<- defs.Probe2StoreMsg, VcURL *url.URL, logger log.Logger, orchConfig *orchestration.Orchestrator) *Session {
	return &Session{
		ctx:        ctx,
		vcURL:      VcURL,
		logger:     logger,
		OrchConfig: orchConfig,
		WatcherWg:  &sync.WaitGroup{},
		ConnUpdate: outbox,
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

// ReserveClient increments the wg. Caller must call ReleaseClient when done
func (s *Session) ReserveClient() error {
	s.sessionLock.Lock()
	if !s.SessionReady {
		s.sessionLock.Unlock()
		return fmt.Errorf("session is not ready")
	}
	s.WatcherWg.Add(1)
	s.sessionLock.Unlock()
	return nil
}

// ReleaseClient releases a reserved client
func (s *Session) ReleaseClient() {
	s.WatcherWg.Done()
}

// GetEventManager returns EventManager. Caller should have already called ReserveClient
func (s *Session) GetEventManager() *event.Manager {
	return s.eventMgr
}

// GetClients returns all clients. Caller should have already called ReserveClient
func (s *Session) GetClients() (*govmomi.Client, *view.Manager, *tags.Manager) {
	return s.client, s.viewMgr, s.tagClient
}

// GetClient returns govmomi client. Caller should have already called ReserveClient
func (s *Session) GetClient() *govmomi.Client {
	return s.client
}

// GetViewManager returns view manager. Caller should have already called ReserveClient
func (s *Session) GetViewManager() *view.Manager {
	return s.viewMgr
}

// GetTagClient returns tag client. Caller should have already called ReserveClient
func (s *Session) GetTagClient() *tags.Manager {
	return s.tagClient
}

// ClearSession tears down the session
func (s *Session) ClearSession() {
	s.logger.Infof("Clearing session")
	s.sessionLock.Lock()
	s.SessionReady = false
	s.sessionLock.Unlock()
	if s.client != nil {
		// Using background context since it's likely that
		// VCHub's context has been cancelled
		ctx, cancel := context.WithTimeout(context.Background(), time.Second)
		defer cancel()
		err := s.client.Logout(ctx)
		if err != nil {
			s.logger.Errorf("Received err while logging out %s", err)
		}
	}
	if s.tagClient != nil {
		// Using background context since it's likely that
		// VCHub's context has been cancelled
		ctx, cancel := context.WithTimeout(context.Background(), time.Second)
		defer cancel()
		err := s.tagClient.Logout(ctx)
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

// IsREST403 returns whether the given error is a 403 forbidden error
func (s *Session) IsREST403(err error) bool {
	if err == nil {
		return false
	}
	return strings.Contains(err.Error(), "403 Forbidden")
}

// PeriodicSessionCheck starts a goroutine that re-establishes the session
// if it goes down at any point
func (s *Session) PeriodicSessionCheck(wg *sync.WaitGroup) {
	defer wg.Done()
	var c *govmomi.Client
	var err error

	s.logger.Infof("Starting session check")

	s.SessionReady = false
	// Connect and login to vcenter
	for {
		if s.ctx.Err() != nil {
			break
		}
		// Set ClientCtx
		s.ClientCtx, s.clientCancel = context.WithCancel(s.ctx)

		// Forever try to login until it succeeds
		for {
			if s.OrchConfig.Spec.Credentials.DisableServerAuthentication {
				s.logger.Infof("Client using insecure mode")
				c, err = govmomi.NewClientWithCA(s.ClientCtx, s.vcURL, true, []byte{})
			} else {
				s.logger.Infof("Client using secure mode")
				c, err = govmomi.NewClientWithCA(s.ClientCtx, s.vcURL, false,
					[]byte(s.OrchConfig.Spec.Credentials.CaData))
			}

			var versionErr error

			if err == nil {
				versionErr = s.checkSupportedVersion(c)
			}
			if err == nil && versionErr == nil {
				evt := ConnectionState{
					orchestration.OrchestratorStatus_Success.String(),
					nil,
				}
				s.sendConnEvent(evt)
				s.logger.Infof("Connection success")
				break
			}
			evt := ConnectionState{
				orchestration.OrchestratorStatus_Failure.String(),
				nil,
			}
			retryDelay := connectionCheckDelay
			if versionErr != nil {
				c.Logout(s.ClientCtx)
				evt.Err = versionErr
				retryDelay = versionCheckDelay
			} else {
				evt.Err = err
			}
			s.sendConnEvent(evt)

			s.logger.Errorf("login failed: %v", err)
			select {
			case <-s.ctx.Done():
				s.logger.Infof("Session check exiting")
				return
			case <-time.After(retryDelay):
			}
		}

		s.client = c
		s.viewMgr = view.NewManager(c.Client)
		restCl := rest.NewClient(c.Client)
		s.tagClient = tags.NewManager(restCl)
		s.eventMgr = event.NewManager(c.Client)

		s.CheckSession = false
		s.CheckTagSession = false

		err := s.tagClient.Login(s.ClientCtx, s.vcURL.User)
		if err != nil {
			// Unlikely for this to fail with 401, since we just logged in above
			s.logger.Errorf("Tags client failed to login, %s", err)
			s.CheckTagSession = true
		}

		// Watchers will now be able to use the session
		s.SessionReady = true

		s.logger.Infof("Tags session check starting...")

		// Start tag session check
		err = s.ReserveClient()
		if err != nil {
			// This shouldn't happen. Only possibility is ClearSession was called by another
			// goroutine.
			s.logger.Errorf("ReserveClient failed unexpectedly %s", err)
			return
		}
		go func() {
			defer s.ReleaseClient()

			ctx := s.ClientCtx
			if ctx == nil {
				return
			}

			for {
				select {
				case <-ctx.Done():
					return
				case <-time.After(tagCheckDelay):
					if s.CheckTagSession {
						// Re-authenticate tag session
						// TODO: prevent multiple tag users?
						err := s.tagClient.Login(ctx, s.vcURL.User)
						if err != nil {
							s.logger.Errorf("Tags client failed to login, %s", err)
							evt := ConnectionState{
								orchestration.OrchestratorStatus_Degraded.String(),
								fmt.Errorf("Tags client received authentication error due to an invalid username or password. Tags/Labels functionality may be impacted"),
							}
							s.sendConnEvent(evt)
						} else {
							s.CheckTagSession = false
							evt := ConnectionState{
								orchestration.OrchestratorStatus_Success.String(),
								nil,
							}
							s.sendConnEvent(evt)
						}
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
			case <-time.After(sessionCheckDelay):
				// if any of the watchers is experiencing the problem, check the session state
				if s.CheckSession {
					err := s.ReserveClient()
					if err != nil {
						// This should never happen as this goroutine is the only one
						// managing the connection
						count--
						s.logger.Errorf("get client error while testing session: %s", err)
						break
					}
					client := s.GetClient()
					active, err := client.SessionManager.SessionIsActive(s.ClientCtx)
					if err != nil {
						count--
						s.logger.Errorf("Received err %v while testing session, attempts left %d", err, count)
					} else if active {
						count = retryCount
						s.CheckSession = false
						s.logger.Infof("Connection status is active, setting check session back to false")
					} else {
						count--
						s.logger.Infof("Session is not active.. retrying, attempts left %d", count)
					}
					s.ReleaseClient()
				}
			}
		}
		// Set status to unknown
		// Status will be updated to either success or failure after login attempt
		evt := ConnectionState{
			orchestration.OrchestratorStatus_Unknown.String(),
			nil,
		}
		s.sendConnEvent(evt)

		// Logout, stop watchers and retry
		// Lock will not be released until
		// we re-establish the client, or the ctx is cancelled
		s.ClearSession()
		s.logger.Infof("Attempting to rebuild session")
		// Hold onto lock
	}
	// Exiting run release lock
	s.logger.Infof("Session check exiting")
}

func (s *Session) checkSupportedVersion(c *govmomi.Client) error {
	aboutInfo := c.ServiceContent.About
	for _, sver := range supportedVersionPrefixes {
		// check only prefix to avoid minor# and build# comparison
		// hopefully these minor versions are api-comatible
		if strings.HasPrefix(aboutInfo.Version, sver) {
			s.logger.Infof("Orchestrator version is %s", aboutInfo.Version)
			return nil
		}
	}
	return fmt.Errorf("%s - %s", utils.UnsupportedVersionMsg, aboutInfo.Version)
}

func (s *Session) sendConnEvent(evt ConnectionState) {
	if s.ConnUpdate != nil {
		m := defs.Probe2StoreMsg{
			MsgType: defs.VCConnectionStatus,
			Val:     evt,
		}
		select {
		case <-s.ctx.Done():
			return
		case s.ConnUpdate <- m:
		}
	}
}
