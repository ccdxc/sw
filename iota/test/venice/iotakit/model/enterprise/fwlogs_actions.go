package enterprise

import (
	"bytes"
	"compress/gzip"
	"context"
	"crypto/tls"
	"encoding/csv"
	"errors"
	"fmt"
	"io/ioutil"
	"net/http"
	"strconv"
	"strings"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/fwlog"
	"github.com/pensando/sw/api/generated/monitoring"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	searchutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

// GetFwLogObjectCount gets the object count for firewall logs under the bucket with the given name
func (sm *SysModel) GetFwLogObjectCount(
	tenantName string, bucketName string, objectKeyPrefix string) (int, error) {
	opts := api.ListWatchOptions{
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenantName,
			Namespace: bucketName,
		},
	}

	ctx, err := sm.VeniceLoggedInCtx(context.Background())
	if err != nil {
		return 0, err
	}

	restClients, err := sm.VeniceRestClient()
	if err != nil {
		return 0, err
	}

	count := 0
	for _, restClient := range restClients {
		list, err := restClient.ObjstoreV1().Object().List(ctx, &opts)
		if err != nil {
			return 0, err
		}

		if len(list) != 0 {
			if objectKeyPrefix == "" {
				return len(list), nil
			}

			for _, o := range list {
				if strings.Contains(o.Name, objectKeyPrefix) {
					count++
				}
			}

			return count, nil
		}
	}

	return 0, nil
}

// getLatestObjectName gets the last entry from the list
// Minio lists objects in chronological order, hence its safe to return the last
// entry from the list.
func (sm *SysModel) getLatestObjectName(tenantName, bucketName, objectKeyPrefix string) (string, error) {
	temp := []string{}
	timeFormat := "2006-01-02T15:04:05Z"
	// Look for 20 minute time span
	startTs := time.Now().UTC().Add(-10 * time.Minute).Format(timeFormat)
	endTs := time.Now().UTC().Add(10 * time.Minute).Format(timeFormat)
	fs := "start-time=" + startTs + ",end-time=" + endTs + ",dsc-id=" + objectKeyPrefix
	opts := api.ListWatchOptions{
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenantName,
			Namespace: bucketName,
		},
		FieldSelector: fs,
	}

	fmt.Println("Field Selector", fs)
	ctx, err := sm.VeniceLoggedInCtx(context.Background())
	if err != nil {
		return "", err
	}

	restClients, err := sm.VeniceRestClient()
	if err != nil {
		return "", err
	}

	for _, restClient := range restClients {
		list, err := restClient.ObjstoreV1().Object().List(ctx, &opts)
		if err != nil {
			return "", err
		}

		if len(list) != 0 {
			for _, o := range list {
				if objectKeyPrefix == "" || strings.Contains(o.Name, objectKeyPrefix) {
					temp = append(temp, o.Name)
				}
			}
		}
	}

	if len(temp) == 0 {
		return "", fmt.Errorf("no objects found for prefix %s", objectKeyPrefix)
	}

	return temp[len(temp)-1], nil
}

// FindFwlogForWorkloadPairsFromObjStore finds workload ip addresses in firewall log
func (sm *SysModel) FindFwlogForWorkloadPairsFromObjStore(
	tenantName, protocol string, port uint32, fwaction string, wpc *objects.WorkloadPairCollection) error {
	for _, wp := range wpc.Pairs {
		ipA := wp.First.GetIP()
		ipB := wp.Second.GetIP()
		aMac := wp.First.NaplesMAC()
		bMac := wp.Second.NaplesMAC()
		return sm.findFwlogForWorkloadPairsFromObjStore(tenantName,
			ipA, ipB, protocol, port, fwaction, aMac, bMac)
	}
	return nil
}

func (sm *SysModel) findFwlogForWorkloadPairsFromObjStore(
	tenantName, srcIP, destIP, protocol string, port uint32, fwaction, naplesA, naplesB string) error {
	latestObjectNameA, err := sm.getLatestObjectName(tenantName, "fwlogs", naplesA)
	if err != nil {
		return fmt.Errorf("could not find latest object for naples %s", naplesA)
	}

	latestObjectNameB, err := sm.getLatestObjectName(tenantName, "fwlogs", naplesB)
	if err != nil {
		return fmt.Errorf("could not find latest object for naples %s", naplesB)
	}

	url := sm.GetVeniceURL()[0]

	dataNaplesA, err := sm.downloadCsvFileViaPSMRESTAPI("fwlogs", latestObjectNameA, url)
	if err != nil {
		return err
	}

	fmt.Println("Data naplesA", dataNaplesA)

	dataNaplesB, err := sm.downloadCsvFileViaPSMRESTAPI("fwlogs", latestObjectNameB, url)
	if err != nil {
		return err
	}

	fmt.Println("Data naplesB", dataNaplesB)

	// reject or deny logs will appear only on one DSC.
	shouldVerifyBothNaples := true
	if fwaction == "reject" || fwaction == "deny" {
		shouldVerifyBothNaples = false
	}

	sm.ForEachNaples(func(nc *objects.NaplesCollection) error {
		if out, err := sm.RunNaplesCommand(nc,
			"/nic/bin/shmdump -file=/dev/shm/fwlog_ipc_shm -type=fwlog"); err == nil {
			fmt.Println("shmDump, naples", nc.Nodes[0].UUID, nc.Nodes[0].IPAddress, strings.Join(out, ","))
		} else {
			fmt.Println("failed to run shmdump", nc.Nodes[0].UUID, nc.Nodes[0].IPAddress, err)
		}
		return nil
	})

	// enhance it to match the given log info
	errA := sm.isLogPresent(dataNaplesA, srcIP, destIP, protocol, port, fwaction)
	errB := sm.isLogPresent(dataNaplesB, srcIP, destIP, protocol, port, fwaction)

	if shouldVerifyBothNaples {
		if errA != nil {
			return fmt.Errorf("error for naplesA %s, err %s", naplesA, errA.Error())
		}
		if errB != nil {
			return fmt.Errorf("error for naplesB %s, err %s", naplesB, errB.Error())
		}
	} else if errA != nil && errB != nil {
		return fmt.Errorf("error on one of the naples, err %s", errA.Error())
	}

	return nil
}

func (sm *SysModel) isLogPresent(data [][]string,
	srcIP, destIP, protocol string, port uint32, fwaction string) error {
	for _, line := range data {
		if (line[2] == srcIP || line[2] == destIP) &&
			(line[3] == destIP || line[3] == srcIP) &&
			line[6] == strconv.Itoa(int(port)) &&
			line[7] == protocol &&
			line[8] == fwaction {
			return nil
		}
	}
	return fmt.Errorf("log not found for srcIP %s, destIP %s, protocol %s, port %d, fwAction %s",
		srcIP, destIP, protocol, port, fwaction)
}

func (sm *SysModel) downloadCsvFileViaPSMRESTAPI(bucketName, objectName string, url string) ([][]string, error) {
	ctx, err := sm.VeniceLoggedInCtx(context.Background())
	if err != nil {
		return nil, err
	}

	// replace first 5 "/" with "_"
	name := strings.Replace(objectName, "/", "_", 5)
	uri := fmt.Sprintf("https://%s/objstore/v1/downloads/%s/%s", url, bucketName, name)
	req, err := http.NewRequest("GET", uri, nil)
	if err != nil {
		return nil, err
	}

	authzHeader, ok := loginctx.AuthzHeaderFromContext(ctx)
	if !ok {
		return nil, fmt.Errorf("error in getting authorization header")
	}

	req.Header.Set("Authorization", authzHeader)
	transport := &http.Transport{TLSClientConfig: &tls.Config{InsecureSkipVerify: true}}
	client := &http.Client{Transport: transport}
	resp, err := client.Do(req)
	if err != nil {
		return nil, err
	}

	defer resp.Body.Close()
	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return nil, err
	}

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("returned error code from PSM %d", resp.StatusCode)
	}

	// body is a zipped file
	reader := bytes.NewReader(body)
	zipReader, err := gzip.NewReader(reader)
	if err != nil {
		return nil, err
	}

	rd := csv.NewReader(zipReader)
	lines, err := rd.ReadAll()
	if err != nil {
		return nil, err
	}

	return lines, nil
}

// FindFwlogForWorkloadPairsFromElastic finds workload ip addresses in firewall log
func (sm *SysModel) FindFwlogForWorkloadPairsFromElastic(
	tenantName, protocol string, port uint32, fwaction string, wpc *objects.WorkloadPairCollection) error {
	for _, wp := range wpc.Pairs {
		ipA := wp.First.GetIP()
		ipB := wp.Second.GetIP()
		aMac := wp.First.NaplesMAC()
		bMac := wp.Second.NaplesMAC()
		return sm.findFwlogForWorkloadPairsFromObjStore(tenantName,
			ipA, ipB, protocol, port, fwaction, aMac, bMac)
	}
	return nil
}

func (sm *SysModel) findFwlogForWorkloadPairsFromElastic(
	tenantName, srcIP, destIP, protocol string, port uint32, fwaction, naplesA, naplesB string) error {
	url := sm.GetVeniceURL()[0]
	ctx, err := sm.VeniceLoggedInCtx(context.Background())
	if err != nil {
		return err
	}

	query := &fwlog.FwLogQuery{
		DestIPs:    []string{destIP},
		SourceIPs:  []string{srcIP},
		DestPorts:  []uint32{port},
		MaxResults: 50,
		Tenants:    []string{globals.DefaultTenant},
	}

	resp := fwlog.FwLogList{}
	err = searchutils.FwLogQuery(ctx, url, query, &resp)
	if err != nil {
		return err
	}

	for _, log := range resp.Items {
		if log.SrcIP == srcIP &&
			log.DestIP == destIP &&
			log.DestPort == port &&
			log.Protocol == protocol &&
			log.Action == fwaction {
			return nil
		}
	}

	return fmt.Errorf("log not found in Elastic for srcIP %s, destIP %s, protocol %s, port %d, fwAction %s",
		srcIP, destIP, protocol, port, fwaction)
}

func (sm *SysModel) VerifyFwlogFromAllNaples(tenantName string, bucketName string, failOnZero bool) error {
	var failedCount = 0
	for _, sim := range sm.FakeNaples {
		mac := sim.Instances[0].Dsc.Status.PrimaryMAC
		cnt, err := sm.GetFwLogObjectCount(tenantName, bucketName, mac)
		if err != nil {
			return err
		}
		if cnt == 0 {
			if failOnZero {
				return errors.New(fmt.Sprintf("zero fwlogs for sim %s", mac))
			}
			failedCount += 1
		}
	}
	log.Infof("%d sims had zero fwlogs", failedCount)
	return nil
}

// NewFwlogPolicy creates a new policy
func (sm *SysModel) NewFwlogPolicy(name string) *objects.FwlogPolicyCollection {
	policy := &objects.FwlogPolicy{
		VenicePolicy: &monitoring.FwlogPolicy{
			TypeMeta: api.TypeMeta{
				Kind: "fwLogPolicy",
			},
			ObjectMeta: api.ObjectMeta{
				Name:      name,
				Tenant:    globals.DefaultTenant,
				Namespace: globals.DefaultNamespace,
			},
			Spec: monitoring.FwlogPolicySpec{
				VrfName: globals.DefaultVrf,
				Targets: []monitoring.ExportConfig{
					{
						Destination: "192.168.99.1",
						Transport:   "tcp/11001",
					},
				},
				Format: monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String(),
				Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALLOW.String()},
				Config: &monitoring.SyslogExportConfig{
					FacilityOverride: monitoring.SyslogFacility_LOG_LOCAL0.String(),
				},
				PSMTarget: &monitoring.PSMExportTarget{
					Enable: true,
				},
			},
		},
	}
	sm.fwlogpolicies[name] = policy
	return objects.NewFwlogPolicyCollection(policy, sm.ObjClient(), sm.Tb)
}

// FwlogPolicy finds an FwlogPolicy by name
func (sm *SysModel) FwlogPolicy(name string) *objects.FwlogPolicyCollection {
	pol, ok := sm.fwlogpolicies[name]
	if !ok {
		pol := objects.NewFwlogPolicyCollection(nil, sm.ObjClient(), sm.Tb)
		pol.SetError(fmt.Errorf("Policy %v not found", name))
		log.Infof("Error %v", pol.Error())
		return pol
	}

	policyCollection := objects.NewFwlogPolicyCollection(pol, sm.ObjClient(), sm.Tb)
	policyCollection.Policies = []*objects.FwlogPolicy{pol}
	return policyCollection
}
