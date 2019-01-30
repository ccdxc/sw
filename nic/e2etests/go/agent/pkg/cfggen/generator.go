package cfggen

import (
	"encoding/json"
	"io/ioutil"
	"os"
	"path"

	"gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/e2etests/go/agent/pkg"
	"github.com/pensando/sw/venice/utils/log"
)

const defaultRemoteUUIDName = "GWUUID"

type CfgGen struct {
	NodeUUIDs                                                                               []string
	ManifestFile                                                                            string
	Config                                                                                  pkg.Config
	Template                                                                                pkg.ConfigTemplate
	Namespaces, Networks, Endpoints, Apps, SGPolicies, SecurityProfiles, FlowExportPolicies IOTAConfig
	EpCache                                                                                 map[string][]string
	NodeEPLUT                                                                               map[string]NodeEPPairs
	SubnetIPLUT                                                                             map[string][]string
}

type IOTAConfig struct {
	Type         string      `json:"type,omitempty"`
	RestEndpoint string      `json:"rest-endpoint,omitempty"`
	ObjectKey    string      `json:"object-key,omitempty"`
	Objects      interface{} `json:"objects,omitempty"`
}

type NodeEPPairs struct {
	LocalEPPairs, RemoteEPPairs []EPPair
}

type EPPair struct {
	SrcEP, DstEP string
}

func NewGenerator(manifestFile, templateFile string, nodeUUIDs []string) (*CfgGen, error) {
	c := CfgGen{
		ManifestFile: manifestFile,
		NodeUUIDs:    nodeUUIDs,
		SubnetIPLUT:  make(map[string][]string),
		NodeEPLUT:    make(map[string]NodeEPPairs),
		EpCache:      make(map[string][]string),
	}

	dat, err := ioutil.ReadFile(manifestFile)
	if err != nil {
		return nil, err
	}

	if err := yaml.Unmarshal(dat, &c.Config); err != nil {
		return nil, err
	}

	// Template file not specified. Use default
	if len(templateFile) == 0 {
		templateFile = pkg.DEFAULT_TEMPLATE_FILE
	}

	dat, err = ioutil.ReadFile(templateFile)
	if err != nil {
		return nil, err
	}

	if err := json.Unmarshal(dat, &c.Template); err != nil {
		return nil, err
	}

	return &c, nil
}

func (c *CfgGen) WriteJSON(outDir string) error {
	if err := os.MkdirAll(outDir, 0755); err != nil {
		log.Errorf("Failed to create output directory. Err: %v", err)
	}

	if err := writeJSON(path.Join(outDir, "namespaces.json"), c.Namespaces); err != nil {
		log.Errorf("Failed to write JSON. Err: %v", err)
		return err
	}

	if err := writeJSON(path.Join(outDir, "networks.json"), c.Networks); err != nil {
		log.Errorf("Failed to write JSON. Err: %v", err)
		return err
	}

	if err := writeJSON(path.Join(outDir, "endpoints.json"), c.Endpoints); err != nil {
		log.Errorf("Failed to write JSON. Err: %v", err)
		return err
	}

	if err := writeJSON(path.Join(outDir, "apps.json"), c.Apps); err != nil {
		log.Errorf("Failed to write JSON. Err: %v", err)
		return err
	}

	if err := writeJSON(path.Join(outDir, "sgpolicies.json"), c.SGPolicies); err != nil {
		log.Errorf("Failed to write JSON. Err: %v", err)
		return err
	}

	if err := writeJSON(path.Join(outDir, "secprofiles.json"), c.SecurityProfiles); err != nil {
		log.Errorf("Failed to write JSON. Err: %v", err)
		return err
	}

	if err := writeJSON(path.Join(outDir, "flowmon.json"), c.FlowExportPolicies); err != nil {
		log.Errorf("Failed to write JSON. Err: %v", err)
		return err
	}

	return nil
}

func writeJSON(filePath string, obj interface{}) error {
	dat, err := json.MarshalIndent(obj, "", "  ")
	if err != nil {
		return err
	}
	return ioutil.WriteFile(filePath, dat, 0644)
}
