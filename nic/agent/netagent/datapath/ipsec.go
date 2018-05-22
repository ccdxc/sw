package datapath

import (
	"context"
	"fmt"
	"net"
	"strings"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// ----------------------- IPSec Encrypt SA CRUDs ----------------------- //

// CreateIPSecSAEncrypt creates an IPSecSA encrypt rule in the datapath
func (hd *Datapath) CreateIPSecSAEncrypt(sa *netproto.IPSecSAEncrypt, ns *netproto.Namespace) error {
	localGwIP := net.ParseIP(strings.TrimSpace(sa.Spec.LocalGwIP))
	if len(localGwIP) == 0 {
		return fmt.Errorf("could not parse IP from {%v}", localGwIP)
	}
	remoteGwIP := net.ParseIP(strings.TrimSpace(sa.Spec.RemoteGwIP))
	if len(remoteGwIP) == 0 {
		return fmt.Errorf("could not parse IP from {%v}", remoteGwIP)
	}

	localGw := &halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(localGwIP),
		},
	}

	remoteGw := &halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(remoteGwIP),
		},
	}

	ipSecSAEncryptReqMsg := &halproto.IpsecSAEncryptRequestMsg{
		Request: []*halproto.IpsecSAEncrypt{
			{
				KeyOrHandle: &halproto.IpsecSAEncryptKeyHandle{
					KeyOrHandle: &halproto.IpsecSAEncryptKeyHandle_CbId{
						CbId: sa.Status.IPSecSAEncryptID,
					},
				},
				Protocol:                halproto.IpsecProtocol_IPSEC_PROTOCOL_ESP,
				AuthenticationAlgorithm: convertAuthAlgorithm(sa.Spec.AuthAlgo),
				AuthenticationKey: &halproto.Key{
					KeyInfo: &halproto.Key_Key{
						Key: []byte(sa.Spec.AuthKey),
					},
				},
				EncryptionAlgorithm: convertEncryptionAlgorithm(sa.Spec.EncryptAlgo),
				EncryptionKey: &halproto.Key{
					KeyInfo: &halproto.Key_Key{
						Key: []byte(sa.Spec.EncryptionKey),
					},
				},
				LocalGatewayIp:  localGw,
				RemoteGatewayIp: remoteGw,
				Spi:             sa.Spec.SPI,
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.IPSecclient.IpsecSAEncryptCreate(context.Background(), ipSecSAEncryptReqMsg)
		if err != nil {
			log.Errorf("Error creating IPSec Encrypt SA Rule. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)

			return ErrHALNotOK
		}
	} else {
		_, err := hd.Hal.IPSecclient.IpsecSAEncryptCreate(context.Background(), ipSecSAEncryptReqMsg)
		if err != nil {
			log.Errorf("Error creating IPSec Encrypt SA Rule. Err: %v", err)
			return err
		}
	}

	return nil
}

// UpdateIPSecSAEncrypt updates an IPSecSA encrypt rule in the datapath
func (hd *Datapath) UpdateIPSecSAEncrypt(sa *netproto.IPSecSAEncrypt, ns *netproto.Namespace) error {
	return nil
}

// DeleteIPSecSAEncrypt deletes an IPSecSA encrypt rule in the datapath
func (hd *Datapath) DeleteIPSecSAEncrypt(sa *netproto.IPSecSAEncrypt, ns *netproto.Namespace) error {
	return nil
}

// ----------------------- IPSec Encrypt SA CRUDs ----------------------- //

// CreateIPSecSADecrypt creates an IPSecSA decrypt rule in the datapath
func (hd *Datapath) CreateIPSecSADecrypt(sa *netproto.IPSecSADecrypt, ns *netproto.Namespace) error {
	ipSecSADecryptReqMsg := &halproto.IpsecSADecryptRequestMsg{
		Request: []*halproto.IpsecSADecrypt{
			{
				KeyOrHandle: &halproto.IpsecSADecryptKeyHandle{
					KeyOrHandle: &halproto.IpsecSADecryptKeyHandle_CbId{
						CbId: sa.Status.IPSecSADecryptID,
					},
				},
				Protocol:                halproto.IpsecProtocol_IPSEC_PROTOCOL_ESP,
				AuthenticationAlgorithm: convertAuthAlgorithm(sa.Spec.AuthAlgo),
				AuthenticationKey: &halproto.Key{
					KeyInfo: &halproto.Key_Key{
						Key: []byte(sa.Spec.AuthKey),
					},
				},
				DecryptionAlgorithm: convertEncryptionAlgorithm(sa.Spec.DecryptAlgo),
				DecryptionKey: &halproto.Key{
					KeyInfo: &halproto.Key_Key{
						Key: []byte(sa.Spec.DecryptionKey),
					},
				},
				RekeyDecAlgorithm: convertEncryptionAlgorithm(sa.Spec.RekeyDecryptAlgo),
				RekeyDecryptionKey: &halproto.Key{
					KeyInfo: &halproto.Key_Key{
						Key: []byte(sa.Spec.RekeyDecryptionKey),
					},
				},
				RekeyAuthenticationKey: &halproto.Key{
					KeyInfo: &halproto.Key_Key{
						Key: []byte(sa.Spec.RekeyAuthKey),
					},
				},

				Spi: sa.Spec.SPI,
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.IPSecclient.IpsecSADecryptCreate(context.Background(), ipSecSADecryptReqMsg)
		if err != nil {
			log.Errorf("Error creating IPSec Decrypt SA Rule. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)

			return ErrHALNotOK
		}
	} else {
		_, err := hd.Hal.IPSecclient.IpsecSADecryptCreate(context.Background(), ipSecSADecryptReqMsg)
		if err != nil {
			log.Errorf("Error creating IPSec Decrypt SA Rule. Err: %v", err)
			return err
		}
	}

	return nil
}

// UpdateIPSecSADecrypt updates an IPSecSA decrypt rule in the datapath
func (hd *Datapath) UpdateIPSecSADecrypt(sa *netproto.IPSecSADecrypt, ns *netproto.Namespace) error {
	return nil
}

// DeleteIPSecSADecrypt deletes an IPSecSA decrypt rule in the datapath
func (hd *Datapath) DeleteIPSecSADecrypt(sa *netproto.IPSecSADecrypt, ns *netproto.Namespace) error {
	return nil
}

// ----------------------- IPSec Policy CRUDs ----------------------- //

// CreateIPSecPolicy creates an IPSec Policy in the datapath
func (hd *Datapath) CreateIPSecPolicy(ipSec *netproto.IPSecPolicy, ns *netproto.Namespace, ipSecLUT map[string]*types.IPSecRuleRef) error {
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: ns.Status.NamespaceID,
		},
	}

	var ipSecRules []*halproto.IpsecRuleMatchSpec

	for _, r := range ipSec.Spec.Rules {
		// Match source and dest attributes
		ruleMatch, err := hd.convertMatchCriteria(r.Src, r.Dst)
		if err != nil {
			log.Errorf("Could not convert match criteria Err: %v", err)
			return err
		}

		// Populate esp info in the match selector.
		appInfo := []*halproto.RuleMatch_AppMatchInfo{
			{
				App: &halproto.RuleMatch_AppMatchInfo_EspInfo{
					EspInfo: &halproto.RuleMatch_ESPInfo{
						Spi: r.SPI,
					},
				},
			},
		}
		ruleMatch.AppMatch = appInfo

		// Lookup corresponding SA
		lookupKey := fmt.Sprintf("%s|%s", r.SAType, r.SAName)

		ipSecRuleRef, ok := ipSecLUT[lookupKey]
		if !ok {
			return fmt.Errorf("IPSec SA Rule not found. {%v}", r.SAName)
		}

		ipSecAction, err := hd.convertIPSecRuleAction(r.SAType, ipSecRuleRef.NamespaceID, ipSecRuleRef.RuleID)
		if err != nil {
			log.Errorf("Could not convert IPSec rule action. Rule: %v. Err: %v", r, err)
		}

		rule := &halproto.IpsecRuleMatchSpec{
			RuleId:   r.ID,
			Match:    ruleMatch,
			SaAction: ipSecAction,
		}
		ipSecRules = append(ipSecRules, rule)
	}

	ipSecPolicyReqMsg := &halproto.IpsecRuleRequestMsg{
		Request: []*halproto.IpsecRuleSpec{
			{
				KeyOrHandle: &halproto.IpsecRuleKeyHandle{
					KeyOrHandle: &halproto.IpsecRuleKeyHandle_RuleKey{
						RuleKey: &halproto.IPSecRuleKey{
							IpsecRuleId:    ipSec.Status.IPSecPolicyID,
							VrfKeyOrHandle: vrfKey,
						},
					},
				},
				VrfKeyHandle: vrfKey,
				Rules:        ipSecRules,
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.IPSecclient.IpsecRuleCreate(context.Background(), ipSecPolicyReqMsg)
		if err != nil {
			log.Errorf("EError creating IPSec Policy. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)

			return ErrHALNotOK
		}
	} else {
		_, err := hd.Hal.IPSecclient.IpsecRuleCreate(context.Background(), ipSecPolicyReqMsg)
		if err != nil {
			log.Errorf("Error creating IPSec Policy. Err: %v", err)
			return err
		}
	}
	return nil
}

// UpdateIPSecPolicy updates an IPSec Policy in the datapath
func (hd *Datapath) UpdateIPSecPolicy(ipSec *netproto.IPSecPolicy, ns *netproto.Namespace) error {
	return nil
}

// DeleteIPSecPolicy deletes an IPSec Policy in the datapath
func (hd *Datapath) DeleteIPSecPolicy(ipSec *netproto.IPSecPolicy, ns *netproto.Namespace) error {
	return nil
}

// convertAuthAlgorithm converts IPSec Authentication algorithm to hal format
func convertAuthAlgorithm(algorithm string) halproto.AuthenticationAlgorithm {
	switch algorithm {
	case "AES_GCM":
		return halproto.AuthenticationAlgorithm_AUTHENTICATION_AES_GCM
	case "AES_CCM":
		return halproto.AuthenticationAlgorithm_AUTHENTICATION_AES_CCM
	case "HMAC":
		return halproto.AuthenticationAlgorithm_AUTHENTICATION_HMAC
	case "AES_CBC_SHA":
		return halproto.AuthenticationAlgorithm_AUTHENTICATION_AES_CBC_SHA
	default:
		return halproto.AuthenticationAlgorithm_AUTHENTICATION_ALGORITHM_NONE
	}
}

// convertEncryptionAlgorithm converts IPSec Encryption algorithm to hal format
func convertEncryptionAlgorithm(algorithm string) halproto.EncryptionAlgorithm {
	switch algorithm {
	case "AES_GCM_128":
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_AES_GCM_128
	case "AES_GCM_256":
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_AES_GCM_256
	case "AES_CCM_128":
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_AES_CCM_128
	case "AES_CCM_192":
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_AES_CCM_192
	case "AES_CCM_256":
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_AES_CCM_256
	case "AES_CBC_128":
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_AES_CBC_128
	case "AES_CBC_192":
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_AES_CBC_192
	case "AES_CBC_256":
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_AES_CBC_256
	case "DES3":
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_DES3
	case "CHA_CHA":
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_CHA_CHA
	default:
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_NONE
	}
}

// convertIPSecRuleAction converts agent ipsec rule action hal format
func (hd *Datapath) convertIPSecRuleAction(saType string, vrfID, ruleID uint64) (*halproto.IpsecSAAction, error) {
	switch saType {
	case "ENCRYPT":
		encryptAction := &halproto.IpsecSAAction{
			SaActionType: halproto.IpsecSAActionType_IPSEC_SA_ACTION_TYPE_ENCRYPT,
			SaHandle: &halproto.IpsecSAAction_EncHandle{
				EncHandle: &halproto.IpsecSAEncryptKeyHandle{
					KeyOrHandle: &halproto.IpsecSAEncryptKeyHandle_CbId{
						CbId: ruleID,
					},
				},
			},
		}
		return encryptAction, nil
	case "DECRYPT":
		decryptAction := &halproto.IpsecSAAction{
			SaActionType: halproto.IpsecSAActionType_IPSEC_SA_ACTION_TYPE_DECRYPT,
			SaHandle: &halproto.IpsecSAAction_DecHandle{
				DecHandle: &halproto.IpsecSADecryptKeyHandle{
					KeyOrHandle: &halproto.IpsecSADecryptKeyHandle_CbId{
						CbId: ruleID,
					},
				},
			},
		}
		return decryptAction, nil
	default:
		log.Errorf("Invalid IPSec Action type. %s", saType)
		return nil, ErrInvalidIPSecSAType
	}

}
