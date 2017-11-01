package keymgr

/*
#include <stdio.h>
#include <stdlib.h>
#define CK_BYTE_PTR unsigned char *
#define CK_ULONG unsigned long long
#define CK_EC_KDF_TYPE CK_ULONG
#define CKD_NULL                 0x00000001UL
typedef struct CK_GCM_PARAMS {
  CK_BYTE_PTR pIv;
  CK_ULONG ulIvLen;
  CK_ULONG ulIvBits;
  CK_BYTE_PTR pAAD;
  CK_ULONG ulAADLen;
  CK_ULONG ulTagBits;
} CK_GCM_PARAMS;
#define CK_GCM_PARAMS_SIZE sizeof(CK_GCM_PARAMS)

typedef struct CK_ECDH1_DERIVE_PARAMS {
  CK_EC_KDF_TYPE kdf;
  CK_ULONG ulSharedDataLen;
  CK_BYTE_PTR pSharedData;
  CK_ULONG ulPublicDataLen;
  CK_BYTE_PTR pPublicData;
} CK_ECDH1_DERIVE_PARAMS;
#define CK_ECDH1_DERIVE_PARAMS_SIZE sizeof(CK_ECDH1_DERIVE_PARAMS)
*/
import "C"
import "unsafe"
import "github.com/miekg/pkcs11"

func getKeyTypeAttributeValue(kt KeyType) uint {
	switch {
	case isRSAKey(kt):
		return pkcs11.CKK_RSA
	case isECDSAKey(kt):
		return pkcs11.CKK_EC
	case isAESKey(kt):
		return pkcs11.CKK_AES
	default:
		return pkcs11.CKK_GENERIC_SECRET
	}
}

func ecdh1DeriveParamBytes(key []byte) (paramBytes []byte, free func()) {
	keyBytes := C.CBytes(key)
	free = func() {
		C.free(keyBytes)
	}

	params := C.CK_ECDH1_DERIVE_PARAMS{
		kdf:             C.CK_ULONG(C.CKD_NULL),
		ulSharedDataLen: C.CK_ULONG(0),
		pSharedData:     nil,
		ulPublicDataLen: C.CK_ULONG(len(key)),
		pPublicData:     C.CK_BYTE_PTR(keyBytes),
	}

	paramBytes = C.GoBytes(unsafe.Pointer(&params), C.CK_ECDH1_DERIVE_PARAMS_SIZE)

	return
}
