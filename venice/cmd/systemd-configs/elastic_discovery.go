// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package configs

import (
	"bytes"
	"fmt"
	"os"
	"text/template"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	// Config file
	elasticDiscoveryCfgFile = globals.ElasticDiscoveryConfigFile
)

// ElasticDiscoveryParams has list of parameters needed
// to generate elastic unicast hosts file
type ElasticDiscoveryParams struct {

	// ElasticEndpoints is the list of newline separated elastic endpoints of the form "IP/hostname:port"
	ElasticEndpoints string
}

const elasticDiscoveryTemplate = `
# The unicast_hosts.txt file contains the list of unicast hosts to connect to
# for pinging during the discovery process, when using the file-based discovery
# mechanism.  This file should contain one entry per line, where an entry is a
# host/port combination.  The host and port should be separated by a colon. If
# the port is left off, a default port of 9300 is assumed.  For example, if the
# cluster has three nodes that participate in the discovery process:
# (1) 66.77.88.99 running on port 9300 (2) 66.77.88.100 running on port 9305
# and (3) 66.77.88.101 running on port 10005, then this file should contain the
# following text:
#
#66.77.88.99
#66.77.88.100:9305
#66.77.88.191:10005

{{.ElasticEndpoints}}

# For IPv6 addresses, make sure to put a bracket around the host part of the address,
# for example: [2001:cdba:0000:0000:0000:0000:3257:9652]:9301 (where 9301 is the port).
#
# NOTE all lines starting with a # are comments, and comments must exist
# on lines of their own (i.e. comments cannot begin in the middle of a line)
`

// GenerateElasticDiscoveryConfig generates configuration file for elastic service
func GenerateElasticDiscoveryConfig(elasticServerAddrs []string) error {

	var buffer bytes.Buffer
	var err error
	var f *os.File

	for i, addr := range elasticServerAddrs {
		if i > 0 {
			buffer.WriteString("\n")
		}
		buffer.WriteString(fmt.Sprintf("%s:%s", addr, globals.ElasticsearchRPCPort))
	}

	log.Debugf("Generating elastic-discovery config - Elastic Endpoints: %s", buffer.String())
	fbParams := ElasticDiscoveryParams{
		ElasticEndpoints: buffer.String(),
	}

	t := template.New("elastic-discovery config template")

	if t, err = t.Parse(elasticDiscoveryTemplate); err != nil {
		log.Errorf("Failed to parse elastic-discovery template %v: ", err)
		return err
	}
	buf := &bytes.Buffer{}

	if err = t.Execute(buf, fbParams); err != nil {
		log.Errorf("Failed to parse elastic-discovery template %v: ", err)
		return err
	}

	if f, err = os.Create(elasticDiscoveryCfgFile); err != nil {
		log.Errorf("Error creating elastic-discovery config file: %v", err)
		return err
	}
	defer f.Close()

	if _, err = f.WriteString(buf.String()); err != nil {
		log.Errorf("Error writing to elastic-discovery config file: %v", err)
		return err
	}

	f.Sync()
	return nil
}

// RemoveElasticDiscoveryConfig removes elastic-discovery config file
func RemoveElasticDiscoveryConfig() {
	removeFiles([]string{elasticDiscoveryCfgFile})
}
