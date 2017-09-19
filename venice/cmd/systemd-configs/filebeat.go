package configs

import (
	"bytes"
	"fmt"
	"os"
	"text/template"

	"github.com/pensando/sw/venice/globals"
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

	// ElasticEndpoint is the elastic client endpoint of the form "IP/hostname:port"
	ElasticEndpoint string

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

#================================ General =====================================
fields:
    customer: "infra"
    category: "venice"
    location: "us-west"

#================================ Outputs =====================================

#-------------------------- Elasticsearch output ------------------------------
output.elasticsearch:
  # Array of hosts to connect to.
  hosts: ["{{.ElasticEndpoint}}"]

  # Optional protocol and basic auth credentials.
  #protocol: "https"
  #username: "elastic"
  #password: "changeme"
  index: "{{.ElasticIndex}}.%{+YYYY.MM.dd}"

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
func GenerateFilebeatConfig(virtualIP string) error {

	fbParams := FilebeatParams{
		"/var/log/pensando",
		virtualIP + ":9200",
		"venice.logs",
	}

	t := template.New("Filebeat config template")

	t, err := t.Parse(filebeatTemplate)
	if err != nil {
		fmt.Printf("Failed to parse filebeat template %v: ", err)
		return err
	}
	buf := &bytes.Buffer{}

	err = t.Execute(buf, fbParams)
	if err != nil {
		fmt.Printf("Failed to parse filebeat template %v: ", err)
		return err
	}

	f, err := os.Create(filebeatCfgFile)
	if err != nil {
		fmt.Printf("Error creating filebeat config file: %v", err)
		return err
	}
	defer f.Close()

	_, err = f.WriteString(buf.String())
	if err != nil {
		fmt.Printf("Error writing to filebeat config file: %v", err)
		return err
	}
	f.Sync()

	return nil
}

// RemoveFilebeatConfig removes the config files associated with ApiServer
func RemoveFilebeatConfig() {
	removeFiles([]string{filebeatCfgFile})
}
