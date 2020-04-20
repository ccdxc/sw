package state

import (
	"context"
	"encoding/json"
	"fmt"
	"net"
	"net/http"
	"reflect"
	"sort"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/ipc"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/nic/agent/protos/tpmprotos"
	"github.com/pensando/sw/venice/ctrler/tpm"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/emstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	objstore "github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/syslog"
	"github.com/pensando/sw/venice/utils/tsdb"
)

const maxRetry = 5
const maxCollectorsInVrf = 4

// 300 CPS * 60 (seconds) * 10(minutes)  = 180000
// We should be able to buffer 10 minute logs in case connectivity
// with venice goes down.
// If every entry in the channel is 500 bytes, then to buffer 10 min
// logs it will take 90Mb memory
const logChannelSize = 180000

// PolicyState keeps the policy agent state
type PolicyState struct {
	ctx                context.Context
	cancel             context.CancelFunc
	emstore            emstore.Emstore
	netAgentURL        string
	fwLogCollectors    sync.Map
	fwTable            tsdb.Obj
	objStoreClients    map[string]objstore.Client // map[bucketName]Client
	hostname           string
	appName            string
	shm                *ipc.SharedMem
	ipc                []*ipc.IPC
	wg                 sync.WaitGroup
	logsChannel        chan singleLog
	objStoreFileFormat fileFormat
	zipObjects         bool
}

type psmFwlogCollector struct {
	sync.Mutex
	vrf    uint64
	filter uint32
}

type syslogFwlogCollector struct {
	sync.Mutex
	vrf         uint64
	port        string
	proto       string
	destination string
	facility    syslog.Priority
	filter      uint32
	format      string
	syslogFd    syslog.Writer
	txCount     uint64
	txErr       uint64
}

func (f *syslogFwlogCollector) String() string {
	if f != nil {
		return fmt.Sprintf("vrf:%d format:%v proto:%v destination:%v port:%v fd:%v txCount:%d txErr:%d",
			f.vrf, f.format, f.proto, f.destination, f.port, f.syslogFd != nil, f.txCount, f.txErr)
	}
	return ""
}

func (f *psmFwlogCollector) String() string {
	if f != nil {
		return fmt.Sprintf("vrf:%d filter:%v", f.vrf, f.filter)
	}
	return ""
}

// NewTpAgent creates new telemetry policy agent state
func NewTpAgent(pctx context.Context, agentPort string) (*PolicyState, error) {
	s, err := emstore.NewEmstore(emstore.MemStoreType, "")
	if err != nil {
		log.Errorf("failed to create db, %s", err)
		return nil, err
	}

	ctx, cancel := context.WithCancel(pctx)

	state := &PolicyState{
		ctx:             ctx,
		cancel:          cancel,
		emstore:         s,
		netAgentURL:     "http://127.0.0.1:" + agentPort,
		fwLogCollectors: sync.Map{},
		hostname:        utils.GetHostname(),
		appName:         globals.Tmagent,
		objStoreClients: map[string]objstore.Client{},
		// This channel is used for transmitting logs from the collector to the transmitter routine
		logsChannel:        make(chan singleLog, logChannelSize),
		objStoreFileFormat: csvFileFormat,
		zipObjects:         true,
	}

	state.connectSyslog()
	return state, nil
}

// Close frees all resources
func (s *PolicyState) Close() {
	s.cancel()
	// stop reconnect thread
	s.wg.Wait()
	// close db
	s.emstore.Close()

	// get all vrfs
	vrfList := []uint64{}
	s.fwLogCollectors.Range(func(k interface{}, v interface{}) bool {
		if key, ok := k.(uint64); ok {
			vrfList = append(vrfList, key)
		}
		return true
	})

	// delete all vrfs
	for _, k := range vrfList {
		s.deleteCollectors(k)
	}
}

// reconnect to syslog collectors
func (s *PolicyState) connectSyslog() error {
	s.wg.Add(1)
	go func() {
		defer s.wg.Done()
		for {
			select {
			case <-time.After(time.Second * 2):
				s.fwLogCollectors.Range(func(k interface{}, v interface{}) bool {

					if c, ok := v.(*syslogFwlogCollector); ok {
						c.Lock()
						if c.syslogFd == nil {
							// reconnect to collector that was never connected or had write error
							if err := s.newSyslog(c); err != nil {
								log.Warnf("failed to connect to collector %s:%s:%s, err:%v", c.proto, c.destination, c.port, err)
							}
						}
						c.Unlock()
					}
					return true
				})

			case <-s.ctx.Done():
				log.Infof("stop connectSyslog thread")
				return

			}
		}
	}()

	return nil
}

// create a hash key to flatten collector config
func (s *PolicyState) getCollectorKey(vrf uint64, policy *tpmprotos.FwlogPolicy, m monitoring.ExportConfig) string {
	// keys that requires new connection to collector
	key := []string{fmt.Sprintf("%d", vrf),
		policy.ObjectMeta.Name,
		policy.Spec.Format,
	}

	key = append(key, policy.Spec.Filter...)
	if policy.Spec.Config != nil {
		key = append(key, policy.Spec.Config.FacilityOverride)
	}
	key = append(key, m.Destination, m.Transport)

	return strings.Join(key, ":")
}

func (s *PolicyState) getPSMCollectorKey(vrf uint64, policy *tpmprotos.FwlogPolicy) string {
	// keys that requires new connection to collector
	key := []string{fmt.Sprintf("%d", vrf),
		policy.ObjectMeta.Name,
	}
	key = append(key, policy.Spec.Filter...)
	return strings.Join(key, ":")
}

// get vrf from netagent
func (s *PolicyState) getvrf(tenant, namespace, vrfName string) (uint64, error) {
	var err error
	reqURL := fmt.Sprintf("%s/api/vrfs/", s.netAgentURL)

	// find vrf from netagent
	var vrfList []netproto.Vrf
	// TODO to use native netagent state's ValidateVrf method instead of repeated curls.
	if len(vrfName) == 0 {
		// Set the name to default. If we move to using ValidateVrf from netagent, it is automatically handled there.
		tenant = globals.DefaultTenant
		namespace = globals.DefaultVrf
		vrfName = globals.DefaultVrf

	}
	for i := 0; i < maxRetry; i++ {
		err = netutils.HTTPGet(reqURL, &vrfList)
		if err == nil {
			for _, vrf := range vrfList {
				if vrfName == vrfName && vrf.Tenant == tenant && vrf.Namespace == namespace {
					return vrf.Status.VrfID, nil
				}
			}
		}
		log.Warnf("failed to GET from %s, %v", reqURL, err)
		time.Sleep(time.Millisecond * 100)
	}

	return 0, fmt.Errorf("GET [%s], %s", reqURL, err)
}

// connect to syslog server
func (s *PolicyState) newSyslog(c *syslogFwlogCollector) error {
	facility := syslog.LogUser // default facility
	if c.facility != 0 {
		facility = c.facility
	}

	priority := syslog.LogInfo // default priority

	switch c.format {
	case monitoring.MonitoringExportFormat_SYSLOG_BSD.String():
		fd, err := syslog.NewBsd(strings.ToLower(c.proto), fmt.Sprintf("%s:%s", c.destination, c.port), facility|priority, s.hostname, s.hostname)
		if err != nil {
			return err
		}
		c.syslogFd = fd
		log.Infof("connected to syslog %v %v://%v:%v", c.format, c.proto, c.destination, c.port)

	case monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String():
		fd, err := syslog.NewRfc5424(strings.ToLower(c.proto), fmt.Sprintf("%s:%s", c.destination, c.port), facility, s.hostname, s.appName)
		if err != nil {
			return err
		}
		c.syslogFd = fd
		log.Infof("connected to syslog %v %v://%v:%v", c.format, c.proto, c.destination, c.port)

	default:
		return fmt.Errorf("invalid syslog format %v", c.format)
	}
	return nil
}

func (s *PolicyState) closeSyslog(c *syslogFwlogCollector) {
	if c.syslogFd != nil {
		c.syslogFd.Close()
		c.syslogFd = nil
	}
}

// ValidateFwLogPolicy validates policy, called from api-server for pre-commit
func ValidateFwLogPolicy(s *monitoring.FwlogPolicySpec) error {
	syslogTargetsPresent, psmTargetPresent := false, false
	if len(s.Targets) != 0 {
		syslogTargetsPresent = true
	}

	if s.PSMTarget != nil {
		psmTargetPresent = true
	}

	if !syslogTargetsPresent && !psmTargetPresent {
		return fmt.Errorf("no collectors configured")
	}

	// The following validations are only needed if syslogtarget is configured
	if syslogTargetsPresent {
		if _, ok := monitoring.MonitoringExportFormat_vvalue[s.Format]; !ok {
			return fmt.Errorf("invalid format %v", s.Format)
		}

		for _, f := range s.Filter {
			if _, ok := monitoring.FwlogFilter_vvalue[f]; !ok {
				return fmt.Errorf("invalid filter %v", f)
			}
		}

		if s.Config != nil {
			if _, ok := monitoring.SyslogFacility_vvalue[s.Config.FacilityOverride]; !ok {
				return fmt.Errorf("invalid facility override %v", s.Config.FacilityOverride)
			}

			if s.Config.Prefix != "" {
				return fmt.Errorf("prefix is not allowed in firewall log")
			}
		}

		if len(s.Targets) > tpm.MaxNumCollectorsPerPolicy {
			return fmt.Errorf("cannot configure more than %v collectors", tpm.MaxNumCollectorsPerPolicy)
		}

		collectors := map[string]bool{}
		for _, c := range s.Targets {
			if key, err := json.Marshal(c); err == nil {
				ks := string(key)
				if _, ok := collectors[ks]; ok {
					return fmt.Errorf("found duplicate collector %v %v", c.Destination, c.Transport)
				}
				collectors[ks] = true

			}

			if c.Destination == "" {
				return fmt.Errorf("cannot configure empty collector")
			}

			netIP, _, err := net.ParseCIDR(c.Destination)
			if err != nil {
				netIP = net.ParseIP(c.Destination)
			}

			if netIP == nil {
				// treat it as hostname and resolve
				if _, err := net.LookupHost(c.Destination); err != nil {
					return fmt.Errorf("failed to resolve name %s, error: %v", c.Destination, err)
				}
			}

			tr := strings.Split(c.Transport, "/")
			if len(tr) != 2 {
				return fmt.Errorf("transport should be in protocol/port format")
			}

			if _, ok := map[string]bool{
				"tcp": true,
				"udp": true,
			}[strings.ToLower(tr[0])]; !ok {
				return fmt.Errorf("invalid protocol %v\n Accepted protocols: TCP, UDP", tr[0])
			}

			port, err := strconv.Atoi(tr[1])
			if err != nil {
				return fmt.Errorf("invalid port %v", tr[1])
			}

			if uint(port) > uint(^uint16(0)) {
				return fmt.Errorf("invalid port %v (> %d)", port, ^uint16(0))
			}
		}
	}

	return nil
}

// validate policy, can be received from REST/venice
func (s *PolicyState) validateFwLogPolicy(p *tpmprotos.FwlogPolicy) error {
	// set default
	if p.Tenant == "" {
		p.Tenant = globals.DefaultTenant
	}

	if p.Namespace == "" {
		p.Namespace = globals.DefaultNamespace
	}

	if p.Spec.VrfName == "" {
		p.Spec.VrfName = globals.DefaultVrf
	}

	return ValidateFwLogPolicy(&p.Spec)
}

// get bitmap for the firewall action
func (s *PolicyState) getFilter(actions []string) uint32 {
	filter := uint32(0)

	for _, f := range actions {
		if f == monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String() {
			// set all bits
			filter |= 0xFFFF

		} else {
			// set bit
			filter |= 1 << uint32(monitoring.FwlogFilter_vvalue[f])
		}
	}

	return filter
}

// get collectors with matching vrf
func (s *PolicyState) getCollector(vrf uint64) (map[string]interface{}, bool) {
	c := map[string]interface{}{}

	s.fwLogCollectors.Range(func(k interface{}, v interface{}) bool {
		if key, ok := k.(string); ok {
			if val, ok := v.(*syslogFwlogCollector); ok {
				val.Lock()
				if val.vrf == vrf {
					c[key] = val
				}
				val.Unlock()
			} else if val, ok := v.(*psmFwlogCollector); ok {
				val.Lock()
				if val.vrf == vrf {
					c[key] = val
				}
				val.Unlock()
			}
		}
		return true
	})

	if len(c) == 0 {
		return nil, false
	}

	return c, true
}

// delete collectors in a vrf
func (s *PolicyState) deleteCollectors(vrf uint64) error {
	delList := map[string]bool{}

	s.fwLogCollectors.Range(func(k interface{}, v interface{}) bool {
		if key, ok := k.(string); ok {
			if c, ok := v.(*syslogFwlogCollector); ok {
				c.Lock()
				if c.vrf == vrf {
					s.closeSyslog(c)
					delList[key] = true
				}
				c.Unlock()
			} else if c, ok := v.(*psmFwlogCollector); ok {
				c.Lock()
				if c.vrf == vrf {
					delList[key] = true
				}
				c.Unlock()
			} else {
				log.Errorf("invalid collector data")
			}
		} else {
			log.Errorf("invalid collector key")
		}
		return true
	})

	// remove keys
	for k := range delList {
		s.fwLogCollectors.Delete(k)
	}

	return nil
}

// Reset deletes all existing fwlog policies
func (s *PolicyState) Reset() error {
	if fwlogPolicies, err := s.ListFwlogPolicy(s.ctx); err == nil {
		for _, policy := range fwlogPolicies {
			if err := s.DeleteFwlogPolicy(s.ctx, policy); err != nil {
				log.Errorf("failed to delete fwlog policy, err: %v", err)
			}
		}
	} else {
		log.Errorf("failed to get fwlog policy, %v", err)
	}

	return nil
}

// CreateFwlogPolicy is the POST() entry point
func (s *PolicyState) CreateFwlogPolicy(ctx context.Context, p *tpmprotos.FwlogPolicy) (err error) {
	log.Infof("POST: fwlog policy %+v", p)

	if err = s.validateFwLogPolicy(p); err != nil {
		log.Errorf("fwlog policy validation failed, %v", err)
		return err
	}

	if _, err := s.emstore.Read(p); err == nil {
		return fmt.Errorf("policy %s already exists", p.Name)
	}

	if objList, err := s.emstore.List(&tpmprotos.FwlogPolicy{
		TypeMeta: api.TypeMeta{
			Kind: "FwlogPolicy",
		},
	}); err == nil {
		if len(objList) >= tpm.MaxNumExportPolicy {
			return fmt.Errorf("can't configure more than %v FwlogPolicy", tpm.MaxNumExportPolicy)
		}
	}

	vrf, err := s.getvrf(p.Tenant, p.Namespace, p.Spec.VrfName)
	if err != nil {
		msg := fmt.Errorf("failed to get vrf for %s/%s/%s, %s", p.Tenant, p.Namespace, p.Spec.VrfName, err)
		log.Errorf("%v", msg)
		return msg
	}

	filter := s.getFilter(p.Spec.Filter)
	for _, target := range p.Spec.Targets {
		key := s.getCollectorKey(vrf, p, target)
		transport := strings.Split(target.Transport, "/")
		fwcollector := &syslogFwlogCollector{
			vrf:         vrf,
			filter:      filter,
			format:      p.Spec.Format,
			destination: target.Destination,
			proto:       transport[0],
			port:        transport[1],
		}

		if p.Spec.Config != nil {
			fwcollector.facility = syslog.Priority(monitoring.SyslogFacility_vvalue[p.Spec.Config.FacilityOverride])
		}

		// we 'll keep the config & connect from the goroutine to avoid blocking
		s.fwLogCollectors.Store(key, fwcollector)
	}

	if p.Spec.PSMTarget != nil && p.Spec.PSMTarget.Enable {
		key := s.getPSMCollectorKey(vrf, p)
		fwcollector := &psmFwlogCollector{
			vrf:    vrf,
			filter: filter,
		}
		s.fwLogCollectors.Store(key, fwcollector)
	}

	if err := s.emstore.Write(p); err != nil {
		s.DeleteFwlogPolicy(ctx, p)
		msg := fmt.Errorf("failed to save policy, %s", err)
		log.Errorf("%v", msg)
		return msg
	}

	log.Infof("created %+v", p)
	return nil
}

// UpdateHostName updates hostname; all the syslog message will carry this updated hostname in the syslog message.
func (s *PolicyState) UpdateHostName(hostname string) {
	if s.hostname != hostname {
		s.hostname = hostname
	}
}

// UpdateFwlogPolicy is the PUT entry point
func (s *PolicyState) UpdateFwlogPolicy(ctx context.Context, p *tpmprotos.FwlogPolicy) error {
	log.Infof("PUT: fwlog policy %+v", p)

	if err := s.validateFwLogPolicy(p); err != nil {
		return err
	}

	obj, err := s.emstore.Read(p)
	if err != nil {
		return fmt.Errorf("policy %s doesn't exist", p.Name)
	}

	sp, ok := obj.(*tpmprotos.FwlogPolicy)
	if !ok {
		return fmt.Errorf("found invalid policy type %T", obj)
	}

	if reflect.DeepEqual(sp.Spec, p.Spec) {
		log.Infof("no change in policy %v", p.Name)
		return nil
	}

	vrf, err := s.getvrf(p.Tenant, p.Namespace, p.Spec.VrfName)
	if err != nil {
		return fmt.Errorf("failed to get tenant for %s/%s", p.Tenant, p.Namespace)
	}

	oldCollector := make(map[string]interface{})
	oldFilter := s.getFilter(sp.Spec.Filter)
	for _, target := range sp.Spec.Targets {
		key := s.getCollectorKey(vrf, sp, target)
		transport := strings.Split(target.Transport, "/")
		fwcollector := &syslogFwlogCollector{
			vrf:         vrf,
			filter:      oldFilter,
			format:      sp.Spec.Format,
			destination: target.Destination,
			proto:       transport[0],
			port:        transport[1],
		}

		if sp.Spec.Config != nil {
			fwcollector.facility = syslog.Priority(monitoring.SyslogFacility_vvalue[p.Spec.Config.FacilityOverride])
		}
		oldCollector[key] = fwcollector
	}
	if sp.Spec.PSMTarget != nil && sp.Spec.PSMTarget.Enable {
		key := s.getPSMCollectorKey(vrf, sp)
		oldCollector[key] = &psmFwlogCollector{
			vrf:    vrf,
			filter: oldFilter,
		}
	}

	newCollector := make(map[string]interface{})
	filter := s.getFilter(p.Spec.Filter)
	for _, target := range p.Spec.Targets {
		key := s.getCollectorKey(vrf, p, target)

		if _, ok := oldCollector[key]; ok {
			delete(oldCollector, key)
			continue
		}
		transport := strings.Split(target.Transport, "/")
		fwcollector := &syslogFwlogCollector{
			vrf:         vrf,
			filter:      filter,
			format:      p.Spec.Format,
			destination: target.Destination,
			proto:       transport[0],
			port:        transport[1],
		}

		if p.Spec.Config != nil {
			fwcollector.facility = syslog.Priority(monitoring.SyslogFacility_vvalue[p.Spec.Config.FacilityOverride])
		}
		newCollector[key] = fwcollector
	}
	if p.Spec.PSMTarget != nil && p.Spec.PSMTarget.Enable {
		key := s.getPSMCollectorKey(vrf, p)
		if _, ok := oldCollector[key]; ok {
			delete(oldCollector, key)
		} else {
			newCollector[key] = &psmFwlogCollector{
				vrf:    vrf,
				filter: filter,
			}
		}
	}

	// delete
	for k := range oldCollector {
		log.Infof("delete collector %v", k)
		if v, ok := s.fwLogCollectors.Load(k); ok {
			if col, ok := v.(*syslogFwlogCollector); ok {
				col.Lock()
				s.closeSyslog(col)
				col.Unlock()
			}
		}
		s.fwLogCollectors.Delete(k)
	}

	// add new collectors
	for key := range newCollector {
		log.Infof("add collector %v", key)
		// connect() is done by connectSyslog() to avoid blocking
		s.fwLogCollectors.Store(key, newCollector[key])
	}

	if err := s.emstore.Write(p); err != nil {
		s.DeleteFwlogPolicy(ctx, p)
		return fmt.Errorf("failed to save policy, %s", err)
	}

	return nil
}

// DeleteFwlogPolicy is the DELETE entry point
func (s *PolicyState) DeleteFwlogPolicy(ctx context.Context, p *tpmprotos.FwlogPolicy) error {
	log.Infof("DEL: fwlog policy %+v", p)
	// set default
	if p.Tenant == "" {
		p.Tenant = globals.DefaultTenant
	}

	if p.Namespace == "" {
		p.Namespace = globals.DefaultNamespace
	}

	obj, err := s.emstore.Read(p)
	if err != nil {
		return fmt.Errorf("policy %s doesn't exist", p.Name)
	}

	sp, ok := obj.(*tpmprotos.FwlogPolicy)
	if !ok {
		return fmt.Errorf("invalid fwlog policy type %T", obj)
	}

	vrf, err := s.getvrf(sp.Tenant, sp.Namespace, sp.Spec.VrfName)
	if err != nil {
		return fmt.Errorf("failed to get vrf for %s/%s", sp.Tenant, sp.Namespace)
	}

	delList := map[string]bool{}
	for _, target := range sp.Spec.Targets {
		key := s.getCollectorKey(vrf, sp, target)
		delList[key] = true
	}
	if sp.Spec.PSMTarget != nil && sp.Spec.PSMTarget.Enable {
		key := s.getPSMCollectorKey(vrf, sp)
		delList[key] = true
	}

	// Close syslogs that will be deleted
	s.fwLogCollectors.Range(func(k interface{}, v interface{}) bool {
		key := k.(string)
		if col, ok := v.(*syslogFwlogCollector); ok {
			col.Lock()
			if _, ok := delList[key]; ok {
				// remove it
				s.closeSyslog(col)
			}
			col.Unlock()
		}
		return true
	})

	// delete
	for k := range delList {
		s.fwLogCollectors.Delete(k)
	}

	s.emstore.Delete(sp)
	return nil
}

// GetFwlogPolicy is the GET entry point
func (s *PolicyState) GetFwlogPolicy(tx context.Context, p *tpmprotos.FwlogPolicy) (*tpmprotos.FwlogPolicy, error) {

	obj, err := s.emstore.Read(p)
	if err != nil {
		return nil, err
	}
	if pol, ok := obj.(*tpmprotos.FwlogPolicy); ok {
		return pol, nil
	}
	return nil, fmt.Errorf("failed to find policy")
}

// ListFwlogPolicy is the LIST all entry point
func (s *PolicyState) ListFwlogPolicy(tx context.Context) ([]*tpmprotos.FwlogPolicy, error) {
	fwlogPol := []*tpmprotos.FwlogPolicy{}

	objList, err := s.emstore.List(&tpmprotos.FwlogPolicy{
		TypeMeta: api.TypeMeta{
			Kind: "FwlogPolicy",
		},
	})

	if err != nil && err != emstore.ErrTableNotFound {
		log.Errorf("failed to list FwlogPolicy, %v", err)
		return fwlogPol, nil
	}

	for _, obj := range objList {
		pol, ok := obj.(*tpmprotos.FwlogPolicy)
		if !ok {
			log.Errorf("invalid fwlog policy type %+v", obj)
			return fwlogPol, nil
		}
		fwlogPol = append(fwlogPol, pol)
	}
	return fwlogPol, nil
}

// send fwlog to collector
func (s *PolicyState) sendFwLog(c *syslogFwlogCollector, fwev *fwevent) {
	if c.format == monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String() ||
		c.format == monitoring.MonitoringExportFormat_SYSLOG_BSD.String() {

		if err := c.syslogFd.Info(&syslog.Message{
			Msg: fwev.String(),
		}); err != nil {
			c.txErr++
			log.Debugf("failed to send to %v://%v:%v, %v", c.proto, c.destination, c.port, err)
			s.closeSyslog(c)
		}
		c.txCount++
	}
}

// Debug is the debug entry point from REST
func (s *PolicyState) Debug(r *http.Request) (interface{}, error) {
	ipcInfo := map[string]string{}
	for i, ipc := range s.ipc {
		ipcInfo[fmt.Sprintf("ipc-%d", i)] = ipc.String()

	}

	dbgState := map[string]interface{}{
		"tsdb": tsdb.Debug(),
		"ipc":  ipcInfo,
	}

	if s.shm != nil {
		dbgState["shm"] = s.shm.String()
	}

	if fwpol, err := s.emstore.List(
		&tpmprotos.FwlogPolicy{
			TypeMeta: api.TypeMeta{
				Kind: "FwlogPolicy",
			},
		}); err == nil {
		dbgState["fwLogPolicy"] = fwpol
	}

	if flowExp, err := s.emstore.List(
		&netproto.FlowExportPolicy{
			TypeMeta: api.TypeMeta{
				Kind: "flowExportPolicy",
			},
		}); err == nil {
		dbgState["flowExportPolicy"] = flowExp
	}

	var collectors []string
	s.fwLogCollectors.Range(func(k interface{}, v interface{}) bool {
		var temp string
		if col, ok := v.(*syslogFwlogCollector); ok {
			col.Lock()
			temp = col.String()
			col.Unlock()

		} else if col, ok := v.(*psmFwlogCollector); ok {
			col.Lock()
			temp = col.String()
			col.Unlock()
		}
		collectors = append(collectors, temp)
		return true
	})
	sort.Slice(collectors, func(i, j int) bool { return strings.Compare(collectors[i], collectors[j]) < 0 })
	dbgState["fwlog-collectors"] = collectors
	return dbgState, nil
}
