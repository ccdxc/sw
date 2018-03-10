package configs

import (
	"bytes"
	"fmt"
	"os"
	"text/template"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	// Config file
	filebeatCfgFile = globals.FilebeatConfigFile
)

// FilebeatParams has list of parameters needed
// to generate filebeat config yaml
type FilebeatParams struct {

	// LogDir is the directory where Venice logs are written to
	LogDir string

	// ElasticEndpoints is the list of comma separated elastic endpoints of the form "IP/hostname:port"
	ElasticEndpoints string

	// ElasticIndex is the index where venice logs will be stored
	ElasticIndex string
}

const filebeatTemplate = `
###################### Filebeat Configuration Example #########################
# You can find the full configuration reference here:
# https://www.elastic.co/guide/en/beats/filebeat/index.html

#=========================== Filebeat prospectors =============================
filebeat.prospectors:
- input_type: log
  paths:
   - {{.LogDir}}/*.log
  symlinks: true
  json.message_key: event
  json.keys_under_root: true

#================================ General =====================================
fields:
    customer: "infra"
    category: "venice"
    location: "us-west"

#================================ Outputs =====================================

#-------------------------- Elasticsearch output ------------------------------
output.elasticsearch:
  # Array of hosts to connect to.
  hosts: [{{.ElasticEndpoints}}]
  loadbalance: true
  template.name: filebeat
  template.path: filebeat.template.json

  # Optional protocol and basic auth credentials.
  #protocol: "https"
  #username: "elastic"
  #password: "changeme"
  index: "{{.ElasticIndex}}.%{+YYYY.MM.dd}"

setup.template.name: "{{.ElasticIndex}}"
setup.template.pattern: "{{.ElasticIndex}}.*"

#================================ Logging =====================================
#output.console:
#    pretty: true

logging:
    to_files: true
    level: info
    selectors: ["*"]
    files:
        path: /tmp/log/filebeat
        name: beat.log
        keepfiles: 5
        rotateeverybytes: 10485760 # 10 MB
`

// GenerateFilebeatConfig generates configuration file for filebeat service
func GenerateFilebeatConfig(elasticServerAddrs []string) error {

	var buffer bytes.Buffer
	for i, addr := range elasticServerAddrs {
		if i > 0 {
			buffer.WriteString(",")
		}
		buffer.WriteString(fmt.Sprintf("\"%s:%s\"", addr, globals.ElasticsearchRESTPort))
	}
	log.Debugf("@@@ Generating Filebeat config - Elastic Endpoints: %s", buffer.String())
	fbParams := FilebeatParams{
		LogDir:           "/var/log/pensando",
		ElasticEndpoints: buffer.String(),
		ElasticIndex:     fmt.Sprintf("%s.%s.%s", elastic.InternalIndexPrefix, globals.DefaultTenant, "systemlogs"),
	}

	t := template.New("Filebeat config template")

	t, err := t.Parse(filebeatTemplate)
	if err != nil {
		log.Errorf("Failed to parse filebeat template %v: ", err)
		return err
	}
	buf := &bytes.Buffer{}

	err = t.Execute(buf, fbParams)
	if err != nil {
		log.Errorf("Failed to parse filebeat template %v: ", err)
		return err
	}

	f, err := os.Create(filebeatCfgFile)
	if err != nil {
		log.Errorf("Error creating filebeat config file: %v", err)
		return err
	}
	defer f.Close()

	_, err = f.WriteString(buf.String())
	if err != nil {
		log.Errorf("Error writing to filebeat config file: %v", err)
		return err
	}
	f.Sync()

	return nil
}

// RemoveFilebeatConfig removes the config files associated with ApiServer
func RemoveFilebeatConfig() {
	removeFiles([]string{filebeatCfgFile})
}
