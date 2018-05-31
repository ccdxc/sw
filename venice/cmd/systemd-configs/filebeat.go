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
	filebeatCfgFile    = globals.FilebeatConfigFile
	filebeatFieldsFile = globals.FilebeatFieldsFile
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
    category: "venice"
#================================ Processors ===================================
processors:
 - add_locale:
     format: offset
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
setup.template.fields: logging_fields.yml
setup.template.settings:
  index.number_of_shards: 6
  index.number_of_replicas: 2
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

const loggingFields = `
- key: beat
  title: Beat
  description: >
    Contains common beat fields available in all event types.
  fields:

    - name: beat.name
      description: >
        The name of the Beat sending the log messages. If the Beat name is
        set in the configuration file, then that value is used. If it is not
        set, the hostname is used. To set the Beat name, use the "name"
        option in the configuration file.
    - name: beat.hostname
      description: >
        The hostname as returned by the operating system on which the Beat is
        running.
    - name: beat.timezone
      description: >
        The timezone as returned by the operating system on which the Beat is
        running.
    - name: beat.version
      description: >
        The version of the beat that generated this event.
    - name: "@timestamp"
      type: date
      required: true
      format: date
      example: August 26th 2016, 12:35:53.332
      description: >
        The timestamp when the event log record was generated.
    - name: tags
      description: >
        Arbitrary tags that can be set per Beat and per transaction
        type.
    - name: fields
      type: object
      object_type: keyword
      description: >
        Contains user configurable fields.
    - name: error
      type: group
      description: >
        Error fields containing additional info in case of errors.
      fields:
        - name: message
          type: text
          description: >
            Error message.
        - name: code
          type: long
          description: >
            Error code.
        - name: type
          type: keyword
          description: >
            Error type.

- key: log
  title: Log file content
  description: >
    Contains log file lines.
  fields:
    - name: source
      type: keyword
      required: true
      description: >
        The file from which the line was read. This field contains the absolute path to the file.
        For example: "/var/log/system.log".
    - name: offset
      type: long
      required: false
      description: >
        The file offset the reported line starts at.
    - name: message
      type: text
      ignore_above: 0
      required: true
      description: >
        The content of the line read from the log file.
    - name: stream
      type: keyword
      required: false
      description: >
        Log stream when reading container logs, can be 'stdout' or 'stderr'
    - name: prospector.type
      required: true
      description: >
        The prospector type from which the event was generated. This field is set to the value specified for the "type" option in the prospector section of the Filebeat config file.
    - name: read_timestamp
      description: >
        In case the ingest pipeline parses the timestamp from the log contents, it stores
        the original "@timestamp" (representing the time when the log line was read) in this
        field.
    - name: fileset.module
      description: >
        The Filebeat module that generated this event.
    - name: fileset.name
      description: >
        The Filebeat fileset that generated this event.
    - name: caller
      type: text
      ignore_above: 0
      required: true
      description: >
        File name and line number of the caller.
    - name: host
      type: keyword
      ignore_above: 0
      required: true
      description: >
        Host name.
    - name: level
      type: keyword
      ignore_above: 0
      required: true
      description: >
        Log level.
    - name: module
      type: keyword
      ignore_above: 0
      required: true
      description: >
        Name of the module.
    - name: submodule
      type: keyword
      ignore_above: 0
      required: true
      description: >
        Name of the submodule.
    - name: msg
      type: text
      ignore_above: 0
      required: true
      description: >
        Message Info.
    - name: pid
      type: text
      ignore_above: 0
      required: true
      description: >
        Process ID.
    - name: ts
      type: date
      ignore_above: 0
      required: true
      description: >
        Timestamp when the log is generated.
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
	log.Debugf("Generating Filebeat config - Elastic Endpoints: %s", buffer.String())
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

	// Create the logging field mapping yml file
	f, err = os.Create(filebeatFieldsFile)
	if err != nil {
		log.Errorf("Error creating filebeat fields file: %v", err)
		return err
	}
	defer f.Close()

	_, err = f.WriteString(loggingFields)
	if err != nil {
		log.Errorf("Error writing to filebeat fields file: %v", err)
		return err
	}
	f.Sync()
	return nil
}

// RemoveFilebeatConfig removes the config files associated with ApiServer
func RemoveFilebeatConfig() {
	removeFiles([]string{filebeatCfgFile, filebeatFieldsFile})
}
