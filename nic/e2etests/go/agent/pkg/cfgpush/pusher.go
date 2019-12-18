package cfgpush

import (
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"path"

	"github.com/pensando/sw/nic/e2etests/go/agent/pkg/libs"

	log "github.com/sirupsen/logrus"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/protos/generated/restclient"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/nic/agent/protos/tsproto"
	"github.com/pensando/sw/nic/e2etests/go/agent/pkg"
)

type CfgPush struct {
	GenDir string

	NetAgentRESTClients []*restclient.AgentClient

	Namespaces         []*netproto.Namespace
	Networks           []*netproto.Network
	Endpoints          []*netproto.Endpoint
	Apps               []*netproto.App
	SGPolicies         []*netproto.NetworkSecurityPolicy
	SecurityProfiles   []*netproto.SecurityProfile
	FlowExportPolicies []*monitoring.FlowExportPolicy
	Tunnels            []*netproto.Tunnel
	MirrorSessions     []*tsproto.MirrorSession
}

func NewPusher(genDir string, netagentRESTEndpoints []string) (*CfgPush, error) {
	p := CfgPush{
		GenDir: genDir,
	}

	for _, url := range netagentRESTEndpoints {
		r := restclient.NewAgentClient(url)
		p.NetAgentRESTClients = append(p.NetAgentRESTClients, r)
	}
	if err := p.ReadJSON(); err != nil {
		return nil, fmt.Errorf("failed to read JSONs. Err: %v", err)
	}

	return &p, nil
}

func (p *CfgPush) PushConfigs() error {
	for _, restClient := range p.NetAgentRESTClients {
		restClient := restClient

		// push Networks
		for _, net := range p.Networks {
			if err := restClient.NetworkCreate(*net); err != nil {
				log.Errorf("Failed to push network %v. Err: %v", net.Name, err)
				return fmt.Errorf("failed to push network %v. Err: %v", net.Name, err)
			}
		}

		// push EPs
		for _, ep := range p.Endpoints {
			if err := restClient.EndpointCreate(*ep); err != nil {
				log.Errorf("Failed to push endpoint %v. Err: %v", ep.Name, err)
				return fmt.Errorf("failed to push endpoint %v. Err: %v", ep.Name, err)
			}
		}

		// push Apps
		for _, app := range p.Apps {
			if err := restClient.AppCreate(*app); err != nil {
				log.Errorf("Failed to push app %v. Err: %v", app.Name, err)
				return fmt.Errorf("failed to push app %v. Err: %v", app.Name, err)
			}
		}

		// push SG Policy
		for _, sgp := range p.SGPolicies {
			if err := restClient.NetworkSecurityPolicyCreate(*sgp); err != nil {
				log.Errorf("Failed to push sg policy %v. Err: %v", sgp.Name, err)
				return fmt.Errorf("failed to push sg policy %v. Err: %v", sgp.Name, err)
			}
		}

		// push tunnels
		for _, tun := range p.Tunnels {
			if err := restClient.TunnelCreate(*tun); err != nil {
				log.Errorf("Failed to push tunnel %v. Err: %v", tun.Name, err)
				return fmt.Errorf("failed to push tunnel %v. Err: %v", tun.Name, err)
			}
		}
		// TODO Enable pushes for FlowExportPolicy and Mirror Sessions
		fmt.Println(restClient)
	}
	return nil
}

func (p *CfgPush) ReadJSON() error {
	var ok bool
	// Read Namespace
	objs, err := readJSON("Namespace", path.Join(p.GenDir, "namespaces.json"))
	if err != nil && err != libs.ErrHeimdallSkip {
		log.Errorf("Failed to read JSON. Err: %v", err)
		return err
	}
	p.Namespaces, ok = objs.([]*netproto.Namespace)
	if err != libs.ErrHeimdallSkip && !ok {
		log.Errorf("Failed to cast JSON data onto []*netproto.Namespace")
		return errors.New("failed to cast JSON data onto []*netproto.Namespace")
	}

	// Read Network
	objs, err = readJSON("Network", path.Join(p.GenDir, "networks.json"))
	if err != nil && err != libs.ErrHeimdallSkip {
		log.Errorf("Failed to read JSON. Err: %v", err)
		return err
	}
	p.Networks, ok = objs.([]*netproto.Network)
	if err != libs.ErrHeimdallSkip && !ok {
		log.Errorf("Failed to cast JSON data onto []*netproto.Network")
		return errors.New("failed to cast JSON data onto []*netproto.Network")
	}

	// Read Endpoint
	objs, err = readJSON("Endpoint", path.Join(p.GenDir, "endpoints.json"))
	if err != nil && err != libs.ErrHeimdallSkip {
		log.Errorf("Failed to read JSON. Err: %v", err)
		return err
	}
	p.Endpoints, ok = objs.([]*netproto.Endpoint)
	if err != libs.ErrHeimdallSkip && !ok {
		log.Errorf("Failed to cast JSON data onto []*netproto.Endpoint")
		return errors.New("failed to cast JSON data onto []*netproto.Endpoint")
	}

	// Read App
	objs, err = readJSON("App", path.Join(p.GenDir, "apps.json"))
	if err != nil && err != libs.ErrHeimdallSkip {
		log.Errorf("Failed to read JSON. Err: %v", err)
		return err
	}
	p.Apps, ok = objs.([]*netproto.App)
	if err != libs.ErrHeimdallSkip && !ok {
		log.Errorf("Failed to cast JSON data onto []*netproto.App")
		return errors.New("failed to cast JSON data onto []*netproto.App")
	}

	// Read NetworkSecurityPolicy
	objs, err = readJSON("NetworkSecurityPolicy", path.Join(p.GenDir, "sgpolicies.json"))
	if err != nil && err != libs.ErrHeimdallSkip {
		log.Errorf("Failed to read JSON. Err: %v", err)
		return err
	}
	p.SGPolicies, ok = objs.([]*netproto.NetworkSecurityPolicy)
	if err != libs.ErrHeimdallSkip && !ok {
		log.Errorf("Failed to cast JSON data onto []*netproto.NetworkSecurityPolicy")
		return errors.New("failed to cast JSON data onto []*netproto.NetworkSecurityPolicy")
	}

	// Read SecurityProfile
	objs, err = readJSON("SecurityProfile", path.Join(p.GenDir, "secprofiles.json"))
	if err != nil && err != libs.ErrHeimdallSkip {
		log.Errorf("Failed to read JSON. Err: %v", err)
		return err
	}
	p.SecurityProfiles, ok = objs.([]*netproto.SecurityProfile)
	if err != libs.ErrHeimdallSkip && !ok {
		log.Errorf("Failed to cast JSON data onto []*netproto.SecurityProfile")
		return errors.New("failed to cast JSON data onto []*netproto.SecurityProfile")
	}

	// Read FlowExportPolicy
	objs, err = readJSON("FlowExportPolicy", path.Join(p.GenDir, "fepolicies.json"))
	if err != nil && err != libs.ErrHeimdallSkip {
		log.Errorf("Failed to read JSON. Err: %v", err)
		return err
	}
	p.FlowExportPolicies, ok = objs.([]*monitoring.FlowExportPolicy)
	if err != libs.ErrHeimdallSkip && !ok {
		log.Errorf("Failed to cast JSON data onto []*netproto.FlowExportPolicy")
		return errors.New("failed to cast JSON data onto []*netproto.FlowExportPolicy")
	}

	// Read Tunnel
	objs, err = readJSON("Tunnel", path.Join(p.GenDir, "tunnels.json"))
	if err != nil && err != libs.ErrHeimdallSkip {
		log.Errorf("Failed to read JSON. Err: %v", err)
		return err
	}
	p.Tunnels, ok = objs.([]*netproto.Tunnel)
	if err != libs.ErrHeimdallSkip && !ok {
		log.Errorf("Failed to cast JSON data onto []*netproto.Tunnel")
		return errors.New("failed to cast JSON data onto []*netproto.Tunnel")
	}

	// Read MirrorSession
	objs, err = readJSON("MirrorSession", path.Join(p.GenDir, "mirrors.json"))
	if err != nil && err != libs.ErrHeimdallSkip {
		log.Errorf("Failed to read JSON. Err: %v", err)
		return err
	}
	p.MirrorSessions, ok = objs.([]*tsproto.MirrorSession)
	if err != libs.ErrHeimdallSkip && !ok {
		log.Errorf("Failed to cast JSON data onto []*netproto.MirrorSession")
		return errors.New("failed to cast JSON data onto []*netproto.MirrorSession")
	}

	return nil
}

func readJSON(kind, filePath string) (interface{}, error) {
	var dat []byte
	var cfg pkg.IOTAConfig
	_, statErr := os.Stat(filePath)
	if statErr != nil {
		log.Infof("File doesn't exist. Ignoring..")
		return cfg, libs.ErrHeimdallSkip
	}

	//
	// Read the JSON
	dat, err := ioutil.ReadFile(filePath)
	if err != nil {
		return cfg, err
	}

	var objMap map[string]*json.RawMessage
	err = json.Unmarshal(dat, &objMap)
	objBytes := objMap["objects"]
	//

	switch kind {
	case "Namespace":
		var ns []*netproto.Namespace
		err := json.Unmarshal(*objBytes, &ns)
		return ns, err
	case "Network":
		var nets []*netproto.Network
		err := json.Unmarshal(*objBytes, &nets)
		return nets, err
	case "Endpoint":
		var eps []*netproto.Endpoint
		err := json.Unmarshal(*objBytes, &eps)
		return eps, err
	case "App":
		var apps []*netproto.App
		err := json.Unmarshal(*objBytes, &apps)
		return apps, err
	case "NetworkSecurityPolicy":
		var sgp []*netproto.NetworkSecurityPolicy
		err := json.Unmarshal(*objBytes, &sgp)
		return sgp, err
	case "SecurityProfile":
		var secProfiles []*netproto.SecurityProfile
		err := json.Unmarshal(*objBytes, &secProfiles)
		return secProfiles, err
	case "FlowExportPolicy":
		var fe []*monitoring.FlowExportPolicy
		err := json.Unmarshal(*objBytes, &fe)
		return fe, err
	case "Tunnel":
		var tun []*netproto.Tunnel
		err := json.Unmarshal(*objBytes, &tun)
		return tun, err
	case "MirrorSession":
		var ms []*tsproto.MirrorSession
		err := json.Unmarshal(*objBytes, &ms)
		return ms, err
	default:
		return nil, fmt.Errorf("unknown Object Kind: %v", kind)
	}
}
