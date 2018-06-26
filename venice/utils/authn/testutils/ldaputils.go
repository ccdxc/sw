package testutils

import (
	"crypto/tls"
	"crypto/x509"
	"errors"
	"fmt"
	"os/exec"
	"strings"

	"gopkg.in/ldap.v2"

	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/testutils"
)

const (
	ldapHost    = "localhost"
	registryURL = "registry.test.pensando.io:5000"
	ldapImage   = "osixia/openldap:1.1.10"
	// ServerName is name in TLS certificate of LDAP server
	ServerName = "0a7af420ff67"
	// TrustedCerts is the TLS certificate of LDAP server
	TrustedCerts = `-----BEGIN CERTIFICATE-----
MIIC/TCCAoOgAwIBAgIUF58P7j/wJUrJXKM1LVlrWRaAc8wwCgYIKoZIzj0EAwMw
gZYxCzAJBgNVBAYTAlVTMRUwEwYDVQQKEwxBMUEgQ2FyIFdhc2gxJDAiBgNVBAsT
G0luZm9ybWF0aW9uIFRlY2hub2xvZ3kgRGVwLjEUMBIGA1UEBxMLQWxidXF1ZXJx
dWUxEzARBgNVBAgTCk5ldyBNZXhpY28xHzAdBgNVBAMTFmRvY2tlci1saWdodC1i
YXNlaW1hZ2UwHhcNMTcxMjEzMjIyNDAwWhcNMTgxMjEzMjIyNDAwWjCBjDELMAkG
A1UEBhMCVVMxEzARBgNVBAgTCk5ldyBNZXhpY28xFDASBgNVBAcTC0FsYnVxdWVy
cXVlMRUwEwYDVQQKEwxBMUEgQ2FyIFdhc2gxJDAiBgNVBAsTG0luZm9ybWF0aW9u
IFRlY2hub2xvZ3kgRGVwLjEVMBMGA1UEAxMMMGE3YWY0MjBmZjY3MHYwEAYHKoZI
zj0CAQYFK4EEACIDYgAE0kmi9mFmxknKd5nDSTG/aIzTvJ3Uza7kNJzNP8+F9Fsb
F9A2N0uDcGuEYZfDwfwHcoUIw/+1kNy+endSrAipOYSEZN91bOdGAOzZE+JsrLhW
yS3MrFIUviI1qevigvJwo4GZMIGWMA4GA1UdDwEB/wQEAwIFoDAdBgNVHSUEFjAU
BggrBgEFBQcDAQYIKwYBBQUHAwIwDAYDVR0TAQH/BAIwADAdBgNVHQ4EFgQUOJFK
TNnQJCm5qhClodH6dCz3zkswHwYDVR0jBBgwFoAUT6XpeiVcMBicYtOXhbpQsoeu
bvgwFwYDVR0RBBAwDoIMMGE3YWY0MjBmZjY3MAoGCCqGSM49BAMDA2gAMGUCMQCC
wNQ0bAkWU27WgzOhn0m7wh87W2U9NX0xJLGNDFsjwfn26uagp46V4h2UKVICe98C
MFB3stnk7Lfr/w/14951n5lek97eDTodYfiF4UxeqL386krQ6eduscPIrin1114r
0w==
-----END CERTIFICATE-----`
	// BaseDN is the subtree in LDAP hierarchy to search for user and groups
	BaseDN = "DC=pensando,DC=io"
	// BindDN is the admin user DN
	BindDN = "CN=admin,DC=pensando,DC=io"
	// BindPassword is the admin user password
	BindPassword = "pensando"
	// UserAttribute is an attribute type of user entry where username is stored
	UserAttribute = "cn"
	// UserObjectClassAttribute is the STRUCTURAL object class for user entry in LDAP. It is used as a filter for user search
	UserObjectClassAttribute = "organizationalPerson"
	// GroupAttribute is an attribute type of user entry where group DNs are stored
	GroupAttribute = "ou"
	// GroupObjectClassAttribute is the STRUCTURAL object class for group entry in LDAP. It is used as a filter for group search
	GroupObjectClassAttribute = "groupOfNames"
	// TenantAttribute is an attribute type of user entry where tenant information is stored
	TenantAttribute = "l"
)

// StartLdapServer starts OpenLDAP container with the specified name and returns host:port string
func StartLdapServer(name string) (string, error) {
	log.Infof("starting openldap container [%s]", name)
	for port := 49152; port < 65535; port++ {
		cmd := []string{
			"run", "--rm", "-d", "-p", fmt.Sprintf("%d:%d", port, 389),
			fmt.Sprintf("--name=%s", name),
			"-e", "LDAP_ORGANISATION=pensando",
			"-e", "LDAP_DOMAIN=pensando.io",
			"-e", "LDAP_ADMIN_PASSWORD=pensando",
			"-e", "LDAP_TLS_VERIFY_CLIENT=try",
			fmt.Sprintf("%s/%s", registryURL, ldapImage)}

		// run the command
		out, err := exec.Command("docker", cmd...).CombinedOutput()

		// stop and retry if a container with the same name exists already
		if strings.Contains(string(out), "Conflict") {
			log.Errorf("conflicting names, retrying")
			StopLdapServer(name)
			continue
		}

		// retry with a different port
		if strings.Contains(string(out), "port is already allocated") {
			log.Errorf("port already allocated, retrying")
			continue
		}

		if err != nil {
			return "", fmt.Errorf("%s, err: %v", out, err)
		}

		ldapAddr := fmt.Sprintf("%s:%d", ldapHost, port)
		log.Infof("started openldap: %s", ldapAddr)
		if !testutils.CheckEventually(func() (bool, interface{}) {
			_, err = getConnection(ldapAddr, true)
			if err != nil {
				return false, nil
			}
			return true, nil
		}) {
			log.Errorf("failed to get connection to openldap [%s], err: %v", ldapAddr, err)
			StopLdapServer(ldapAddr)
			return "", err
		}
		return ldapAddr, nil
	}

	return "", fmt.Errorf("exhausted all the ports from 49512-65534, failed to start openldap server")
}

// StopLdapServer stops OpenLDAP container with the specified name
func StopLdapServer(name string) error {
	if len(strings.TrimSpace(name)) == 0 {
		return nil
	}

	log.Infof("stopping openldap container [%s]", name)

	cmd := []string{"rm", "-f", name}

	// run the command
	out, err := exec.Command("docker", cmd...).CombinedOutput()

	if err != nil && !strings.Contains(string(out), "No such container") {
		log.Infof("docker run cmd failed, err: %+v", err)
		return fmt.Errorf("%s, err: %v", out, err)
	}

	return err
}

// CreateLdapUser creates user entry in LDAP
func CreateLdapUser(addr, username, password, tenant string, groups []string) error {
	userDN := fmt.Sprintf("%s=%s,%s", UserAttribute, username, BaseDN)
	req := ldap.NewAddRequest(userDN)
	req.Attribute("objectClass", []string{UserObjectClassAttribute, "person", "top", "simpleSecurityObject"})
	req.Attribute("userPassword", []string{password})
	req.Attribute(GroupAttribute, groups)
	req.Attribute("sn", []string{username})
	req.Attribute(TenantAttribute, []string{tenant})
	conn, err := getConnection(addr, true)
	if err != nil {
		return err
	}
	if err := conn.Bind(BindDN, BindPassword); err != nil {
		return err
	}
	return conn.Add(req)
}

// CreateGroup creates group entry in LDAP
func CreateGroup(addr, dn string, parents, members []string) error {
	req := ldap.NewAddRequest(dn)
	req.Attribute("objectClass", []string{GroupObjectClassAttribute, "top"})
	req.Attribute(GroupAttribute, parents)
	req.Attribute("member", members)
	conn, err := getConnection(addr, true)
	if err != nil {
		return err
	}
	if err := conn.Bind(BindDN, BindPassword); err != nil {
		return err
	}
	return conn.Add(req)
}

// CreateReferral creates referral entry in LDAP
func CreateReferral(addr, name, referral string) error {
	dn := fmt.Sprintf("%s=%s,%s", UserAttribute, name, BaseDN)
	req := ldap.NewAddRequest(dn)
	req.Attribute("objectClass", []string{"extensibleObject", "referral", "top"})
	req.Attribute("ref", []string{referral})
	conn, err := getConnection(addr, true)
	if err != nil {
		return err
	}
	if err := conn.Bind(BindDN, BindPassword); err != nil {
		return err
	}
	return conn.Add(req)
}

func getConnection(addr string, tlsEnabled bool) (*ldap.Conn, error) {
	conn, err := ldap.Dial("tcp", addr)
	if err != nil {
		return nil, err
	}
	if tlsEnabled {
		var certpool *x509.CertPool
		if TrustedCerts != "" {
			certpool = x509.NewCertPool()
			if !certpool.AppendCertsFromPEM([]byte(TrustedCerts)) {
				conn.Close()
				return nil, errors.New("error creating cert pool from trusted certs")
			}
		}
		if err := conn.StartTLS(
			&tls.Config{
				InsecureSkipVerify: false,
				ServerName:         ServerName,
				RootCAs:            certpool, // if certpool is nil TLS uses the host's root CA set.
			}); err != nil {
			conn.Close()
			return nil, err
		}
	}
	return conn, nil
}
