package ldap

import (
	"errors"
	"fmt"
	"strings"

	"gopkg.in/ldap.v2"

	"math"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	// PrimaryGroupID is the relative ID of primary group to which user belongs
	PrimaryGroupID = "primaryGroupID"
	// ObjectSid is the security identifier of the ldap entry
	ObjectSid = "objectSid"
)

var (
	// ErrIncorrectSIDFormat is thrown when there is an error in parsing binary SID attribute value
	ErrIncorrectSIDFormat = errors.New("incorrect SID format")
)

// getADPrimaryGroup returns primary group for AD user. It returns "" and error if not found
func (a *authenticator) getADPrimaryGroup(referral string, tlsOptions *auth.TLSOptions, entry *ldap.Entry) (string, error) {
	// this attribute is present in Active Directory user entry. It is RID for user's primary group
	primaryGroupID := entry.GetAttributeValue(PrimaryGroupID)
	// this attribute is present in Active Directory. It is user's SID
	rawObjectSID := entry.GetRawAttributeValue(ObjectSid)
	objectSID := SidToString(rawObjectSID)
	if primaryGroupID == "" || objectSID == "" {
		return "", ErrNoneOrMultipleGroupEntries
	}
	log.Debugf("user [%q], objectSID [%q], primaryGroupID [%q]", entry.DN, objectSID, primaryGroupID)
	// get primary group SID by replacing RID for user with RID for group
	var groupSID string
	toks := strings.Split(objectSID, "-")
	toks[len(toks)-1] = primaryGroupID
	groupSID = strings.Join(toks, "-")
	log.Debugf("user [%q], primary group SID [%q]", entry.DN, groupSID)
	var attributes = []string{
		a.ldapConfig.GetAttributeMapping().GetGroup(),
	}
	ldapURL, err := ParseLdapURL(referral, a.ldapConfig.BaseDN, SUB, a.primaryGroupADSearchFilter(groupSID))
	if err != nil {
		log.Errorf("Unable to parse ldap referral [%q] to search primary group [%q] for user [%q], Err: %v", referral, groupSID, entry.DN, err)
		return "", err
	}
	if ldapURL.Addr == "" {
		//no ldap host returned in referral
		log.Errorf("No hostname returned in referral [%q] while searching primary group [%q] for user [%q]", referral, groupSID, entry.DN)
		return "", ErrNoHostInLDAPReferral
	}
	log.Debugf("ldapURL.Addr: %q, referral: %q", ldapURL.Addr, referral)
	entries, err := a.search(ldapURL.Addr, tlsOptions, a.ldapConfig.BaseDN, ldapURL.Scope, ldap.DerefAlways,
		a.primaryGroupADSearchFilter(groupSID), attributes, func(referral string, conn connection, sr *ldap.SearchResult) (bool, error) {
			if len(sr.Entries) == 0 && len(sr.Referrals) > 0 {
				log.Infof("Referrals returned for group [%q] for search filter [%q]: %v", groupSID, a.primaryGroupADSearchFilter(groupSID), sr.Referrals)
				return true, nil
			}
			if len(sr.Entries) != 1 {
				log.Errorf("Group [%q] does not exist or too many entries [%q] returned", groupSID, len(sr.Entries))
				return false, ErrNoneOrMultipleGroupEntries
			}
			log.Debugf("Group entry [%q] successfully found using referral [%q]", sr.Entries[0].DN, referral)
			return false, nil
		})
	if err != nil {
		log.Errorf("LDAP search operation failed for group [%q]:  Err: %#v", groupSID, err)
		return "", err
	}
	if len(entries) != 1 {
		log.Errorf("LDAP search failed for group [%q], [%d] entries returned", groupSID, len(entries))
		return "", ErrNoneOrMultipleGroupEntries
	}
	log.Debugf("user [%q] belongs to primary group [%q]", entry.DN, entries[0].DN)
	return entries[0].DN, nil
}

func (a *authenticator) primaryGroupADSearchFilter(groupSID string) string {
	return fmt.Sprintf("(&(objectClass=%s)(%s=%s))", a.ldapConfig.AttributeMapping.GroupObjectClass, ObjectSid, groupSID)
}

// SidToString converts sid from binary form to string
// ref https://msdn.microsoft.com/en-us/library/ff632068.aspx
func SidToString(sid []byte) string {
	// sid[0] is the Revision
	if len(sid) == 0 || sid[0] != 1 {
		return ""
	}
	var sidBuilder strings.Builder
	sidBuilder.WriteString("S-1-")

	// The next byte specifies the numbers of sub authorities (number of dashes minus two)
	subAuthorityCount := int(sid[1])

	// IdentifierAuthority (6 bytes starting from the second) (big endian)
	var identifierAuthority uint64
	offset := 2
	size := 6
	for i := 0; i < size; i++ {
		identifierAuthority |= uint64(sid[offset+i]) << uint(8*(size-1-i))
	}
	if float64(identifierAuthority) < math.Pow(2, 32) {
		sidBuilder.WriteString(fmt.Sprintf("%d", identifierAuthority))
	} else {
		sidBuilder.WriteString(fmt.Sprintf("0x%X", identifierAuthority))
	}

	// Iterate all the SubAuthority (little-endian)
	offset = 8
	size = 4 // 32-bits (4 bytes) for each SubAuthority
	for i := 0; i < subAuthorityCount; i++ {
		var subAuthority uint64
		for j := 0; j < size; j++ {
			subAuthority |= uint64(sid[offset+j]) << uint(8*j)
		}
		sidBuilder.WriteString(fmt.Sprintf("-%d", subAuthority))
		offset += size
	}

	return sidBuilder.String()
}
