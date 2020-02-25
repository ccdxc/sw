package testutils

import (
	"context"
	"crypto/tls"
	"crypto/x509"
	"errors"
	"fmt"
	"os/exec"
	"sort"
	"strings"
	"time"

	"gopkg.in/ldap.v2"

	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/testutils"
)

const (
	ldapHost    = "localhost"
	registryURL = "registry.test.pensando.io:5000"
	ldapImage   = "pensando/openldap:0.1"
	// ServerName is name in TLS certificate of LDAP server
	ServerName = "testServer"
	// TrustedCerts is the TLS certificate of LDAP server
	TrustedCerts = `-----BEGIN CERTIFICATE-----
MIIFvzCCA6egAwIBAgIJAJr5JYzAXwoUMA0GCSqGSIb3DQEBCwUAMHYxCzAJBgNV
BAYTAlVTMQswCQYDVQQIDAJDQTERMA8GA1UEBwwIU2FuIEpvc2UxDjAMBgNVBAoM
BU15QXBwMRMwEQYDVQQDDApncnBjLmxvY2FsMSIwIAYJKoZIhvcNAQkBFhNndWVz
dEBudW92YW1lbnRlLmlvMB4XDTE2MTEyMzE3NTAwNVoXDTI2MTEyMTE3NTAwNVow
djELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAkNBMREwDwYDVQQHDAhTYW4gSm9zZTEO
MAwGA1UECgwFTXlBcHAxEzARBgNVBAMMCmdycGMubG9jYWwxIjAgBgkqhkiG9w0B
CQEWE2d1ZXN0QG51b3ZhbWVudGUuaW8wggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAw
ggIKAoICAQDKJ0DvjH+bteiz9R84XwAY+EYt6r6zmZmNT9n5PeEgGdHOJQzsarAv
K95kHNJY99TkLAYM3wMU2dv3khBOODHldgGr7ThJUg3JPTTRJbBRyfhVDIYNUgjh
iqz0t/C86fZN7OYeLs6j1UU7fFsqdn+TwFCu/9RG6fDfnZmOnmfqxp1+hPg3WB1j
DOxZxGjUVmg0PUPALO558yio+S7lV56Q+FD5MHV/NOUVv/pKNolQYj/6T3a60Zz2
ujvK2br6zD1TRmRq5XHptK2y71BHeEtgOA7v4BKBDn0aw2JmUM+/kyiR5thi6b7s
x00BFO0piwjuQ50v02vqoCnMdtyVIHymcHlfi197xO9tZUlckFulsiSuCIqYxvAj
p3WIbIAzymSNq52joHCm5s8An63RIAAPrD/gXgwm3tX/6M9n21z0yfOEQgyedfe+
UBk/O6XNYoBVZ0Qc2z3Zkwn9v8HH7wtmkRolBjBWwXJcniomRXcHzJGoQjA9Kjm3
l9fFWlajvIh3CDZ9mq1hh+PU+DtKFEcwF3rP/OHUOlzMEvpyqf9whlkx0/UmgPRj
YQl4keN6K783WtrXWQ8ia5VH/NjyjJ294lile9lbaS3BI/K9pHT7O6+sL9GaHyA5
Ou213V+/s2bB/mjE8ypzlFz5nOFfFIG0cqSfu2t4gMB3Me0BeF6yhQIDAQABo1Aw
TjAdBgNVHQ4EFgQUcaG/Cs+TnqYPgb24J5nahEh/E6wwHwYDVR0jBBgwFoAUcaG/
Cs+TnqYPgb24J5nahEh/E6wwDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQsFAAOC
AgEANqL9nvCtK6WreKvnsHFCWd6mKz6H2ArMMv/MR6t/uAzDMvcMlYPr7G4SseCD
61d2sx/67c6vvSdQmLy2f4LWEy/1TrN/rLeMkQLq5+Cg4tWerc9onkiMOkJQCXmF
HpPd21KRUoP4rgmTJTk9o7mMpxU+K0P1l5JkPbau3YR/CSDaG74cuYUIB4Xbx8GV
7wiPmKxfLUa0NAjPlrbH8lL6zNDl6vxcZD2UpJCmNbQmdbmTN+I8oPD9uxrh8d23
KZpwRckQN5zlpaOZDTr2atPVgKsowM9uFncJ34Cb5XV9BIGYWQOzZMgDVft1A381
eG6LRnxyWiSSpwaehTPmvIvPHnf5SKf8oC7PY3tkGmLaTcJQduAItyOHXwsfKQ/2
3ALmAq15Jxwd62Yax9tQjF8C1+xuJK6/wFFib24P0I9n3zKNZTH+5Wx23Rb/E49B
4YK79YItlj4YNLRQpgBd8sv2y7xTTZDSm6EflIO47w4ZaQ/bSQtaPNgNl+2U5u71
x1791/HWcJQa6u/q9a6XVA9Hez9vtC2N7lKuhVpSfiH9AAF4M72ymE1eKdrUD0ll
2CBUky0yhpbfsJee5BeR2ymjtfsnTvNb2t/G///dDOLTeSPYnw96jhAaqNrRjQOm
L+TWM+qWaQYbnJN8n5n5/GCMuJfKHPQ/QBxQmzIx3Im4BYk=
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

// LdapConfig to define config for AD or OpenLdap
type LdapConfig struct {
	// ServerName is name in TLS certificate of LDAP server
	ServerName string
	// TrustedCerts is the TLS certificate of LDAP server
	TrustedCerts string
	// URL is Ldap URL to connect to
	URL string
	// BaseDN is the subtree in LDAP hierarchy to search for user and groups
	BaseDN string
	// BindDN is the admin user DN
	BindDN string
	// BindPassword is the admin user password
	BindPassword string
	// UserAttribute is an attribute type of user entry where username is stored
	UserAttribute string
	// UserObjectClassAttribute is the STRUCTURAL object class for user entry in LDAP. It is used as a filter for user search
	UserObjectClassAttribute string
	// GroupAttribute is an attribute type of user entry where group DNs are stored
	GroupAttribute string
	// GroupObjectClassAttribute is the STRUCTURAL object class for group entry in LDAP. It is used as a filter for group search
	GroupObjectClassAttribute string
	// TenantAttribute is an attribute type of user entry where tenant information is stored
	TenantAttribute string
	// LdapServer is name of the LDAP server process
	LdapServer string
	// LdapUser is ldap username
	LdapUser string
	// LdapUserGroupsDN are groups DN to which the user belongs to
	LdapUserGroupsDN []string
	// LdapUserPassword is ldap user password
	LdapUserPassword string
	// ReferralServer is name of the LDAP referral server process
	ReferralServer string
	// ReferralUser is ldap referral username
	ReferralUser string
	// ReferralUserDN is ldap referral user DN
	ReferralUserDN string
	// ReferralUserGroupDN is group DN to which ldap referral user belongs to
	ReferralUserGroupDN string
	// ReferralUserPassword is ldap referral user password
	ReferralUserPassword string
}

// StartOpenLdapServer starts OpenLDAP container with the specified name and returns host:port string
func StartOpenLdapServer(name string, skipPorts ...int) (string, int, error) {
	log.Infof("starting openldap container [%s]", name)
	sort.Ints(skipPorts)
	for port := 49152; port < 65535; port++ {
		i := sort.Search(len(skipPorts), func(i int) bool {
			return skipPorts[i] >= port
		})
		if i < len(skipPorts) && skipPorts[i] == port {
			continue // skip port if present in skipPorts
		}
		cmd := []string{
			"run", "--rm", "-d", "-p", fmt.Sprintf("%d:%d", port, 389),
			fmt.Sprintf("--name=%s", name),
			"-e", "LDAP_ORGANISATION=pensando",
			"-e", "LDAP_DOMAIN=pensando.io",
			"-e", "LDAP_ADMIN_PASSWORD=pensando",
			"-e", "LDAP_TLS_VERIFY_CLIENT=try",
			"-e", "LDAP_TLS_CRT_FILENAME=testServer.crt",
			"-e", "LDAP_TLS_KEY_FILENAME=testServer.key",
			"-e", "LDAP_TLS_CA_CRT_FILENAME=testCA.crt",
			fmt.Sprintf("%s/%s", registryURL, ldapImage)}

		// run the command
		out, err := exec.Command("docker", cmd...).CombinedOutput()

		// stop and retry if a container with the same name exists already
		if strings.Contains(string(out), "Conflict") {
			log.Errorf("conflicting names, retrying")
			StopOpenLdapServer(name)
			continue
		}

		// retry with a different port
		if strings.Contains(string(out), "port is already allocated") {
			log.Errorf("port already allocated, retrying")
			continue
		}

		if strings.Contains(string(out), "address already in use") {
			log.Errorf("address already in use, retrying")
			continue
		}
		if err != nil {
			return "", 0, fmt.Errorf("%s, err: %v", out, err)
		}

		ldapAddr := fmt.Sprintf("%s:%d", ldapHost, port)
		log.Infof("started openldap: %s", ldapAddr)
		if !testutils.CheckEventually(func() (bool, interface{}) {
			getConnectionFn := func(ctx context.Context) (interface{}, error) {
				if _, err := getConnection(ldapAddr, true); err != nil {
					return false, err
				}
				return true, nil
			}
			ctx, cancel := context.WithTimeout(context.Background(), time.Second)
			defer cancel()
			_, err := utils.ExecuteWithContext(ctx, getConnectionFn)
			if err != nil {
				return false, nil
			}
			return true, nil
		}, "2s", "10s") {
			log.Errorf("failed to get connection to openldap [%s], err: %v", ldapAddr, err)
			StopOpenLdapServer(ldapAddr)
			return "", 0, err
		}
		return ldapAddr, port, nil
	}

	return "", 0, fmt.Errorf("exhausted all the ports from 49152-65534, failed to start openldap server")
}

// StopOpenLdapServer stops OpenLDAP container with the specified name
func StopOpenLdapServer(name string) error {
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
	if len(parents) > 0 {
		req.Attribute(GroupAttribute, parents)
	}
	if len(members) > 0 {
		req.Attribute("member", members)
	}
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
