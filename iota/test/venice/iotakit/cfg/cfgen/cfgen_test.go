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
	cfg.NetworkSecurityPolicyParams.NumPolicies = 1
	cfg.NetworkSecurityPolicyParams.NumRulesPerPolicy = 50000
	cfg.WorkloadParams.WorkloadsPerHost = 200
	cfg.AppParams.NumApps = 10000
	cfg.NumDNSAlgs = 10
	cfg.NumRoutingConfigs = 2
	cfg.NumUnderlayRoutingConfigs = 2
	cfg.NumNeighbors = 2
	cfg.NumUnderlayNeighbors = 2
	cfg.NumOfTenants = 2
	cfg.NumOfVRFsPerTenant = 2
	cfg.NumOfSubnetsPerVpc = 2
	cfg.NumOfIPAMPsPerTenant = 1

	// create smartnic macs from a template
	smartnics := []*cluster.DistributedServiceCard{}
	snicsFile := os.Getenv("SNICS_FILE")
	if snicsFile != "" {
		smnicList := &cluster.DistributedServiceCardList{}
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
		smnicCtx := newIterContext()
		for ii := 0; ii < 128; ii++ {
			tSmartNIC := smnicCtx.transform(smnic).(*cluster.DistributedServiceCard)
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

	for _, o := range cfg.ConfigItems.Networks {
		if j, err := json.MarshalIndent(o, "", "  "); err == nil {
			ofile.Write(j)
			ofile.WriteString("\n")
		}
	}
	for _, o := range cfg.ConfigItems.Hosts {
		if j, err := json.MarshalIndent(o, "", "  "); err == nil {
			ofile.Write(j)
			ofile.WriteString("\n")
		}
	}
	for _, o := range cfg.ConfigItems.Workloads {
		if j, err := json.MarshalIndent(o, "", "  "); err == nil {
			ofile.Write(j)
			ofile.WriteString("\n")
		}
	}
	for _, o := range cfg.ConfigItems.TenantWorkloads {
		if j, err := json.MarshalIndent(o, "", "  "); err == nil {
			ofile.Write(j)
			ofile.WriteString("\n")
		}
	}

	for _, o := range cfg.ConfigItems.Apps {
		if j, err := json.MarshalIndent(o, "", "  "); err == nil {
			ofile.Write(j)
			ofile.WriteString("\n")
		}
	}

	for _, o := range cfg.ConfigItems.SGPolicies {
		if j, err := json.MarshalIndent(o, "", "  "); err == nil {
			ofile.Write(j)
			ofile.WriteString("\n")
		}
	}

	if j, err := json.MarshalIndent(cfg.Fwprofile, "", "  "); err == nil {
		ofile.Write(j)
		ofile.WriteString("\n")
	}

	for _, o := range cfg.ConfigItems.RouteConfig {
		if j, err := json.MarshalIndent(o, "", "  "); err == nil {
			ofile.Write(j)
			ofile.WriteString("\n")
		}
	}

	for _, o := range cfg.ConfigItems.UnderlayRouteConfig {
		if j, err := json.MarshalIndent(o, "", "  "); err == nil {
			ofile.Write(j)
			ofile.WriteString("\n")
		}
	}

	for _, o := range cfg.ConfigItems.Tenants {
		if j, err := json.MarshalIndent(o, "", "  "); err == nil {
			ofile.Write(j)
			ofile.WriteString("\n")
		}
	}

	for _, o := range cfg.ConfigItems.VRFs {
		if j, err := json.MarshalIndent(o, "", "  "); err == nil {
			ofile.Write(j)
			ofile.WriteString("\n")
		}
	}

	for _, o := range cfg.ConfigItems.IPAMPs {
		if j, err := json.MarshalIndent(o, "", "  "); err == nil {
			ofile.Write(j)
			ofile.WriteString("\n")
		}
	}

	for _, o := range cfg.ConfigItems.Subnets {
		if j, err := json.MarshalIndent(o, "", "  "); err == nil {
			ofile.Write(j)
			ofile.WriteString("\n")
		}
	}

}
