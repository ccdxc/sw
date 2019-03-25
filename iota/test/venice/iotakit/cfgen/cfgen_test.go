package cfgen

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"testing"

	"github.com/pensando/sw/api/generated/cluster"
)

func TestCfgenPolicyGen(t *testing.T) {
	cfg := DefaultCfgenParams
	cfg.SGPolicyParams.NumPolicies = 1
	cfg.SGPolicyParams.NumRulesPerPolicy = 50000
	cfg.WorkloadParams.WorkloadsPerHost = 200
	cfg.AppParams.NumApps = 10000
	cfg.NumDnsAlgs = 10

	// create smartnic macs from a template
	smartnics := []*cluster.SmartNIC{}
	snicsFile := os.Getenv("SNICS_FILE")
	if snicsFile != "" {
		smnicList := &cluster.SmartNICList{}
		jdata, err := ioutil.ReadFile(snicsFile)
		if err != nil {
			panic(fmt.Sprintf("file %s, error %s", snicsFile, err))
		}
		err = json.Unmarshal(jdata, smnicList)
		if err != nil {
			panic(fmt.Sprintf("error unmarshing json data %s", string(jdata)))
		}
		for _, smnic := range smnicList.Items {
			smartnics = append(smartnics, smnic)
		}
	} else {
		smnic := smartNicTemplate
		smnicCtx := NewIterContext()
		for ii := 0; ii < 128; ii++ {
			tSmartNIC := smnicCtx.transform(smnic).(*cluster.SmartNIC)
			smartnics = append(smartnics, tSmartNIC)
		}
	}
	cfg.Smartnics = smartnics

	// generate the configuration now
	cfg.Do()

	// verify and keep the data in some file
	ofile, err := os.OpenFile("/tmp/cfg.json", os.O_RDWR|os.O_CREATE, 0755)
	if err != nil {
		panic(err)
	}

	for _, o := range cfg.Networks {
		if j, err := json.MarshalIndent(o, "", "  "); err == nil {
			ofile.Write(j)
			ofile.WriteString("\n")
		}
	}
	for _, o := range cfg.Hosts {
		if j, err := json.MarshalIndent(o, "", "  "); err == nil {
			ofile.Write(j)
			ofile.WriteString("\n")
		}
	}
	for _, o := range cfg.Workloads {
		if j, err := json.MarshalIndent(o, "", "  "); err == nil {
			ofile.Write(j)
			ofile.WriteString("\n")
		}
	}
	for _, o := range cfg.Apps {
		if j, err := json.MarshalIndent(o, "", "  "); err == nil {
			ofile.Write(j)
			ofile.WriteString("\n")
		}
	}

	for _, o := range cfg.SGPolicies {
		if j, err := json.MarshalIndent(o, "", "  "); err == nil {
			ofile.Write(j)
			ofile.WriteString("\n")
		}
	}

	if j, err := json.MarshalIndent(cfg.Fwprofile, "", "  "); err == nil {
		ofile.Write(j)
		ofile.WriteString("\n")
	}
}
