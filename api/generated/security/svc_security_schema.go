// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package securityApiServer is a auto generated package.
Input file: svc_security.proto
*/
package security

import (
	"github.com/pensando/sw/venice/utils/runtime"
)

var typesMapSvc_security = map[string]*runtime.Struct{

	"security.AppList": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"T": runtime.Field{Name: "T", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.TypeMeta"},

			"ListMeta": runtime.Field{Name: "ListMeta", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.ListMeta"},

			"Items": runtime.Field{Name: "Items", JSONTag: "", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "security.App"},
		},
	},
	"security.AppUserGrpList": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"T": runtime.Field{Name: "T", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.TypeMeta"},

			"ListMeta": runtime.Field{Name: "ListMeta", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.ListMeta"},

			"Items": runtime.Field{Name: "Items", JSONTag: "", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "security.AppUserGrp"},
		},
	},
	"security.AppUserList": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"T": runtime.Field{Name: "T", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.TypeMeta"},

			"ListMeta": runtime.Field{Name: "ListMeta", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.ListMeta"},

			"Items": runtime.Field{Name: "Items", JSONTag: "", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "security.AppUser"},
		},
	},
	"security.AutoMsgAppUserGrpWatchHelper": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"Events": runtime.Field{Name: "Events", JSONTag: "", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "security.AutoMsgAppUserGrpWatchHelper.WatchEvent"},
		},
	},
	"security.AutoMsgAppUserGrpWatchHelper.WatchEvent": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"Type": runtime.Field{Name: "Type", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"Object": runtime.Field{Name: "Object", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "security.AppUserGrp"},
		},
	},
	"security.AutoMsgAppUserWatchHelper": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"Events": runtime.Field{Name: "Events", JSONTag: "", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "security.AutoMsgAppUserWatchHelper.WatchEvent"},
		},
	},
	"security.AutoMsgAppUserWatchHelper.WatchEvent": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"Type": runtime.Field{Name: "Type", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"Object": runtime.Field{Name: "Object", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "security.AppUser"},
		},
	},
	"security.AutoMsgAppWatchHelper": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"Events": runtime.Field{Name: "Events", JSONTag: "", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "security.AutoMsgAppWatchHelper.WatchEvent"},
		},
	},
	"security.AutoMsgAppWatchHelper.WatchEvent": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"Type": runtime.Field{Name: "Type", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"Object": runtime.Field{Name: "Object", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "security.App"},
		},
	},
	"security.AutoMsgCertificateWatchHelper": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"Events": runtime.Field{Name: "Events", JSONTag: "", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "security.AutoMsgCertificateWatchHelper.WatchEvent"},
		},
	},
	"security.AutoMsgCertificateWatchHelper.WatchEvent": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"Type": runtime.Field{Name: "Type", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"Object": runtime.Field{Name: "Object", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "security.Certificate"},
		},
	},
	"security.AutoMsgSecurityGroupWatchHelper": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"Events": runtime.Field{Name: "Events", JSONTag: "", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "security.AutoMsgSecurityGroupWatchHelper.WatchEvent"},
		},
	},
	"security.AutoMsgSecurityGroupWatchHelper.WatchEvent": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"Type": runtime.Field{Name: "Type", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"Object": runtime.Field{Name: "Object", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "security.SecurityGroup"},
		},
	},
	"security.AutoMsgSgpolicyWatchHelper": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"Events": runtime.Field{Name: "Events", JSONTag: "", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "security.AutoMsgSgpolicyWatchHelper.WatchEvent"},
		},
	},
	"security.AutoMsgSgpolicyWatchHelper.WatchEvent": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"Type": runtime.Field{Name: "Type", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"Object": runtime.Field{Name: "Object", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "security.Sgpolicy"},
		},
	},
	"security.AutoMsgTrafficEncryptionPolicyWatchHelper": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"Events": runtime.Field{Name: "Events", JSONTag: "", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "security.AutoMsgTrafficEncryptionPolicyWatchHelper.WatchEvent"},
		},
	},
	"security.AutoMsgTrafficEncryptionPolicyWatchHelper.WatchEvent": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"Type": runtime.Field{Name: "Type", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"Object": runtime.Field{Name: "Object", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "security.TrafficEncryptionPolicy"},
		},
	},
	"security.CertificateList": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"T": runtime.Field{Name: "T", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.TypeMeta"},

			"ListMeta": runtime.Field{Name: "ListMeta", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.ListMeta"},

			"Items": runtime.Field{Name: "Items", JSONTag: "", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "security.Certificate"},
		},
	},
	"security.SecurityGroupList": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"T": runtime.Field{Name: "T", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.TypeMeta"},

			"ListMeta": runtime.Field{Name: "ListMeta", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.ListMeta"},

			"Items": runtime.Field{Name: "Items", JSONTag: "", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "security.SecurityGroup"},
		},
	},
	"security.SgpolicyList": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"T": runtime.Field{Name: "T", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.TypeMeta"},

			"ListMeta": runtime.Field{Name: "ListMeta", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.ListMeta"},

			"Items": runtime.Field{Name: "Items", JSONTag: "", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "security.Sgpolicy"},
		},
	},
	"security.TrafficEncryptionPolicyList": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"T": runtime.Field{Name: "T", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.TypeMeta"},

			"ListMeta": runtime.Field{Name: "ListMeta", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.ListMeta"},

			"Items": runtime.Field{Name: "Items", JSONTag: "", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "security.TrafficEncryptionPolicy"},
		},
	},
}

func init() {
	schema := runtime.GetDefaultScheme()
	schema.AddSchema(typesMapSvc_security)
}
