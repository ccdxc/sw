package auth

import (
	"testing"

	. "github.com/pensando/sw/venice/utils/authn/testutils"
)

func getADConfig() *LdapConfig {
	tinfo.ldapAddr = "10.11.100.100:389"
	return &LdapConfig{
		ServerName: "WIN-HQEM46VDNCA.pensando.io",
		TrustedCerts: `-----BEGIN CERTIFICATE-----
MIIGJDCCBQygAwIBAgITFwAAAAMOnQv5vrz9cAAAAAAAAzANBgkqhkiG9w0BAQsF
ADBUMRIwEAYKCZImiZPyLGQBGRYCaW8xGDAWBgoJkiaJk/IsZAEZFghwZW5zYW5k
bzEkMCIGA1UEAxMbcGVuc2FuZG8tV0lOLUhRRU00NlZETkNBLUNBMB4XDTE5MDgx
NjA5MDE1OVoXDTIwMDgxNTA5MDE1OVowJjEkMCIGA1UEAxMbV0lOLUhRRU00NlZE
TkNBLnBlbnNhbmRvLmlvMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA
5U+VNv1QqWhBlpgnmGG8OJaRDj3lOhuRL8nELbMcAe4woxLJCHB/UlDvR84xC3/Y
R20ZnNHV/Q5zYxggc1PsgLnjNQSxGpH5tx1pgJajZMkokXcukbNxx9nT0HGt8aIi
4pkXWHYCp4Vb73Sk6iiSjpoC4/jzDAFXri4hr2yy1Pl4BoFzUAbkOHBrwGVC5Akb
eo2ehmQx7NLkdQv9QJUsL7XNlSiqkivIESiieq/77Jb1uBOjWQKqJry6Nty2cr8+
sQ/H097uf50xYgODzA99twdJqgANFm3FI9UZ8cHJqG8Lrk0Sm+njUl/H2WBzntFR
2OOXC1Ui89aaMH20CAUw7wIDAQABo4IDGzCCAxcwLwYJKwYBBAGCNxQCBCIeIABE
AG8AbQBhAGkAbgBDAG8AbgB0AHIAbwBsAGwAZQByMB0GA1UdJQQWMBQGCCsGAQUF
BwMCBggrBgEFBQcDATAOBgNVHQ8BAf8EBAMCBaAweAYJKoZIhvcNAQkPBGswaTAO
BggqhkiG9w0DAgICAIAwDgYIKoZIhvcNAwQCAgCAMAsGCWCGSAFlAwQBKjALBglg
hkgBZQMEAS0wCwYJYIZIAWUDBAECMAsGCWCGSAFlAwQBBTAHBgUrDgMCBzAKBggq
hkiG9w0DBzBHBgNVHREEQDA+oB8GCSsGAQQBgjcZAaASBBCSYP6Edb9iTIiVCoxT
pSRightXSU4tSFFFTTQ2VkROQ0EucGVuc2FuZG8uaW8wHQYDVR0OBBYEFGJYgGI+
UEA1Y7JPY+HpEau2mlajMB8GA1UdIwQYMBaAFJknRiTiCBIXctRmQO4Ms5LuB3Q7
MIHhBgNVHR8EgdkwgdYwgdOggdCggc2GgcpsZGFwOi8vL0NOPXBlbnNhbmRvLVdJ
Ti1IUUVNNDZWRE5DQS1DQSxDTj1XSU4tSFFFTTQ2VkROQ0EsQ049Q0RQLENOPVB1
YmxpYyUyMEtleSUyMFNlcnZpY2VzLENOPVNlcnZpY2VzLENOPUNvbmZpZ3VyYXRp
b24sREM9cGVuc2FuZG8sREM9aW8/Y2VydGlmaWNhdGVSZXZvY2F0aW9uTGlzdD9i
YXNlP29iamVjdENsYXNzPWNSTERpc3RyaWJ1dGlvblBvaW50MIHNBggrBgEFBQcB
AQSBwDCBvTCBugYIKwYBBQUHMAKGga1sZGFwOi8vL0NOPXBlbnNhbmRvLVdJTi1I
UUVNNDZWRE5DQS1DQSxDTj1BSUEsQ049UHVibGljJTIwS2V5JTIwU2VydmljZXMs
Q049U2VydmljZXMsQ049Q29uZmlndXJhdGlvbixEQz1wZW5zYW5kbyxEQz1pbz9j
QUNlcnRpZmljYXRlP2Jhc2U/b2JqZWN0Q2xhc3M9Y2VydGlmaWNhdGlvbkF1dGhv
cml0eTANBgkqhkiG9w0BAQsFAAOCAQEAWBcuznj+rnlLvPxiFEozkaATD7i1uIGl
R3UsrbL8ecYzkrtu5/+7A+EKtm7mqv8rq9VhCDUp022gkOWXvk3T3iKSoCumA8wq
LC+I5IyUEMJ+VFUDwro1qzucIPQVP0GLodhrAUz7wxqEwJLbG33SS7IJUK/flSzu
lzNgQ1sARgLFKtJMSSeh6E1tsH+IQGa+Ft237LTocxp1976YPT6ewTEzIK3bg1UU
hU4GQ9nTWYcyk3aPLHxWo4C5JMPywKyNPmc6AyuAbRjapj7uK/qfWicqVXt3Qbka
l5J+aSPYHBx8IcFbmpmJZ/paZc+bVth6E5CcoBHDjB7ctfaL24E9HQ==
-----END CERTIFICATE-----`,
		URL:                       tinfo.ldapAddr,
		BaseDN:                    "DC=pensando,DC=io",
		BindDN:                    "pensando\\Administrator",
		BindPassword:              "G0pensando",
		UserAttribute:             "sAMAccountName",
		UserObjectClassAttribute:  "organizationalPerson",
		GroupAttribute:            "memberOf",
		GroupObjectClassAttribute: "group",
		TenantAttribute:           "department",
		ReferralServer:            "openldap",
		LdapServer:                "openldapref",
		LdapUser:                  "testuser",
		LdapUserGroupsDN:          []string{"CN=Domain Users,CN=Users,DC=pensando,DC=io", "CN=Users,CN=Builtin,DC=pensando,DC=io"},
		LdapUserPassword:          "G0pensando",
	}
}

func TestADAuthenticate(t *testing.T) {
	config := getADConfig()
	testAuthenticate(t, config)
}

func TestADIncorrectPasswordAuthentication(t *testing.T) {
	config := getADConfig()
	testIncorrectPasswordAuthentication(t, config)
}

func TestADIncorrectUserAuthentication(t *testing.T) {
	config := getADConfig()
	testIncorrectUserAuthentication(t, config)
}

func TestADIncorrectLdapAttributeMapping(t *testing.T) {
	config := getADConfig()
	testIncorrectLdapAttributeMapping(t, config)
}

func TestADIncorrectBaseDN(t *testing.T) {
	config := getADConfig()
	testIncorrectBaseDN(t, config)
}

func TestADIncorrectBindPassword(t *testing.T) {
	config := getADConfig()
	testIncorrectBindPassword(t, config)
}
