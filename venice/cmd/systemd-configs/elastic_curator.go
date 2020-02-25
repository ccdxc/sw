package configs

import (
	"bytes"
	"fmt"
	"os"
	"text/template"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/log"
)

// ElasticCuratorParams has list of parameters needed to generate pen-elastic-curator config yaml
type ElasticCuratorParams struct {
	// LogDir is the directory where Venice logs are written to
	LogDir string

	// ElasticEndpoints is the list of comma separated elastic endpoints of the form "IP/hostname:port"
	ElasticEndpoints string

	// TLSCertPath is the location of the file containing the certificate to authenticate to ES cluster
	TLSCertPath string

	// TLSCertPath is the location of the file containing the key to authenticate to ES cluster
	TLSKeyPath string

	// TLSCertPath is the location of the file containing the trust roots used to validate
	// the certificate presented by the ES cluster
	TLSCABundlePath string
}

const elasticCuratorConfigTemplate = `
# Remember, leave a key empty if there is no value.  None will be a string,
# not a Python "NoneType"
client:
  hosts: [{{.ElasticEndpoints}}]
  port: 9200
  url_prefix:
  use_ssl: True
  certificate: {{.TLSCABundlePath}}
  client_cert: {{.TLSCertPath}}
  client_key: {{.TLSKeyPath}}
  ssl_no_validate: False
  http_auth:
  timeout: 30
  master_only: False

logging:
  loglevel: DEBUG # move it to INFO once this module stabilizes
  logfile: {{.LogDir}}/pen-elastic-curator.log # no log rotation in place today, we can remove logging to the file once this module stabilizes
  logformat: default
  blacklist: ['elasticsearch', 'urllib3'] # filter log certain log traffic. e.g. 'elasticsearch', 'urllib3'
`

// GenerateElasticCuratorConfig generates configuration file for pen-elastic-curator service.
func GenerateElasticCuratorConfig(elasticServerAddrs []string) error {

	var buffer bytes.Buffer
	for i, addr := range elasticServerAddrs {
		if i > 0 {
			buffer.WriteString(",")
		}
		buffer.WriteString(fmt.Sprintf("%s", addr))
	}
	log.Debugf("generating %s config - elastic endpoints: %s", globals.ElasticSearchCurator, buffer.String())
	tlsCertPath, tlsKeyPath, tlsCABundlePath := certs.GetTLSCredentialsPaths(globals.ElasticClientAuthDir)

	fbParams := FilebeatParams{
		LogDir:           globals.LogDir,
		ElasticEndpoints: buffer.String(),
		TLSCertPath:      tlsCertPath,
		TLSKeyPath:       tlsKeyPath,
		TLSCABundlePath:  tlsCABundlePath,
	}

	t := template.New(fmt.Sprintf("%s config template", globals.ElasticSearchCurator))

	t, err := t.Parse(elasticCuratorConfigTemplate)
	if err != nil {
		log.Errorf("Failed to parse %s template %v: ", globals.ElasticSearchCurator, err)
		return err
	}
	buf := &bytes.Buffer{}

	err = t.Execute(buf, fbParams)
	if err != nil {
		log.Errorf("Failed to parse %s template %v: ", globals.ElasticSearchCurator, err)
		return err
	}

	f, err := os.Create(globals.ElasticCuratorConfigFile)
	if err != nil {
		log.Errorf("Error creating %s config file: %v", globals.ElasticCuratorConfigFile, err)
		return err
	}
	defer f.Close()

	_, err = f.WriteString(buf.String())
	if err != nil {
		log.Errorf("Error writing to %s config file: %v", globals.ElasticCuratorConfigFile, err)
		return err
	}
	f.Sync()

	return nil
}

// RemoveElasticCuratorConfig removes the elastic curator config file
func RemoveElasticCuratorConfig() {
	removeFiles([]string{globals.ElasticCuratorConfigFile})
}
