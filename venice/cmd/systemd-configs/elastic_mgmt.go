// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package configs

import (
	"bytes"
	"os"
	"text/template"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	// Config file
	elasticMgmtCfgFile = globals.ElasticMgmtConfigFile
)

// ElasticMgmtParams has the Mgmt-Addr
type ElasticMgmtParams struct {

	// MgmtAddr is the IP/hostname for binding and publishing
	// This is used to set the network.host parameter in elastic.
	MgmtAddr string

	// Min master node count for Elastic cluster
	ElasticMinMasters int
}

const elasticMgmtTemplate = `
export PENS_MGMT_IP={{.MgmtAddr}}
export ELASTIC_MIN_MASTERS={{.ElasticMinMasters}}
`

// GenerateElasticMgmtConfig generates mgmt configuration file for elastic service
func GenerateElasticMgmtConfig(mgmtAddr string, quorumSize int) error {

	var err error
	var f *os.File

	log.Debugf("Generating elastic-mgmt config, mgmt-addr: %s min-masters: %d",
		mgmtAddr,
		quorumSize/2+1)

	fbParams := ElasticMgmtParams{
		MgmtAddr:          mgmtAddr,
		ElasticMinMasters: (quorumSize/2 + 1),
	}

	t := template.New("elastic-mgmt config template")

	if t, err = t.Parse(elasticMgmtTemplate); err != nil {
		log.Errorf("Failed to parse elastic-mgmt template %v: ", err)
		return err
	}
	buf := &bytes.Buffer{}

	if err = t.Execute(buf, fbParams); err != nil {
		log.Errorf("Failed to parse elastic-mgmt template %v: ", err)
		return err
	}

	if f, err = os.Create(elasticMgmtCfgFile); err != nil {
		log.Errorf("Error creating elastic-mgmt config file: %v", err)
		return err
	}
	defer f.Close()

	if _, err = f.WriteString(buf.String()); err != nil {
		log.Errorf("Error writing to elastic-mgmt config file: %v", err)
		return err
	}

	f.Sync()
	return nil
}

// RemoveElasticMgmtConfig removes elastic-mgmt config file
func RemoveElasticMgmtConfig() {
	removeFiles([]string{elasticMgmtCfgFile})
}
