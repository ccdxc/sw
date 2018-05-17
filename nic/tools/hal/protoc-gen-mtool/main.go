package main

import (
	"encoding/json"
	"io/ioutil"
	"log"
	"os"
	"strings"

	"github.com/gogo/protobuf/proto"
	google_protobuf "github.com/gogo/protobuf/protoc-gen-gogo/descriptor"
	"github.com/gogo/protobuf/protoc-gen-gogo/generator"
)

const veniceKeyTag = "venice:key"
const veniceRefTag = "venice:ref"

var msgDescMap = map[string]*google_protobuf.DescriptorProto{}
var keyObjMap = map[string]string{}
var objRefMap = map[string][]string{}

func updateMsgDescMap(msgType string, msgd *google_protobuf.DescriptorProto) {
	//log.Println("Adding " + msgType + " to msg descriptor map")
	msgDescMap[msgType] = msgd
}

func buildMsgDescMap(prefix string, msgd *google_protobuf.DescriptorProto) {
	updateMsgDescMap(prefix+"."+*msgd.Name, msgd)
	for _, nestedMsg := range msgd.NestedType {
		buildMsgDescMap(prefix+"."+*msgd.Name, nestedMsg)
	}
}

func msgDescMap2Str() (error, string) {
	log.Println("Message Type to Message Descriptor Map")
	b, err := json.MarshalIndent(msgDescMap, "", "  ")
	if err == nil {
		return nil, string(b)
	} else {
		return err, ""
	}
}

func updKey2ObjMap(keyType, msgName string) {
	keyObjMap[keyType] = msgName
}

func buildKey2ObjMap(pkgName string, msgd *google_protobuf.DescriptorProto) {
	for _, fieldd := range msgd.Field {
		if fieldd.Options == nil {
			continue
		}
		extnDescs, err := proto.ExtensionDescs(fieldd.Options)
		if err != nil {
			panic(err)
		}
		log.Println("Processing field", *fieldd.Name)
		for _, d := range extnDescs {
			if d.Name != "gogoproto.moretags" {
				continue
			}
			log.Println(fieldd.String())
			extnVal, err := proto.GetExtension(fieldd.Options, d)
			if err != nil {
				panic(err)
			}
			v, ok := extnVal.(*string)
			if !ok {
				panic(nil)
			}
			log.Println("Found gogoproto extn, name :", d.Name, ", val :", *v)
			if strings.Contains(*v, veniceKeyTag) {
				if fieldd.GetTypeName() != "" {
					updKey2ObjMap(fieldd.GetTypeName(), pkgName+"."+*msgd.Name)
				}
				break // there can only be one key attr
			}
		}
	}
}

func keyObjMap2String() (error, string) {
	log.Println("Message Key to Message Name Map")
	b, err := json.MarshalIndent(keyObjMap, "", "  ")
	if err == nil {
		return nil, string(b)
	} else {
		return err, ""
	}
}

func updObjRefMap(obj, refObj string) {
	// look for potential duplicates before adding
	for _, o := range objRefMap[obj] {
		if o == refObj {
			// dependency already exists
			return
		}
	}
	log.Println("Adding", obj, "to", refObj, "reference")
	objRefMap[obj] = append(objRefMap[obj], refObj)
}

func buildObjRefMap(msgName *string, msgd *google_protobuf.DescriptorProto) {
	var done bool

	for _, fieldd := range msgd.Field {
		done = false
		if fieldd.Options != nil {
			log.Println("Processing field", *fieldd.Name)
			extnDescs, err := proto.ExtensionDescs(fieldd.Options)
			if err != nil {
				panic(err)
			}
			for _, d := range extnDescs {
				if d.Name != "gogoproto.moretags" {
					continue
				}
				log.Println(fieldd.String())
				extnVal, err := proto.GetExtension(fieldd.Options, d)
				if err != nil {
					panic(err)
				}
				v, ok := extnVal.(*string)
				if !ok {
					panic(nil)
				}
				//log.Println(*v)
				if strings.Contains(*v, veniceRefTag) {
					if keyObjMap[fieldd.GetTypeName()] != "" {
						updObjRefMap(*msgName, keyObjMap[fieldd.GetTypeName()])
					} else {
						updObjRefMap(*msgName, fieldd.GetTypeName())
					}
					done = true
				} else if strings.Contains(*v, veniceKeyTag) {
					// for key fields we don't have to worry about references
					done = true
					break
				}
			}
		}

		// if we found tag of interest on this field, already stop analyzing
		// it further
		if done == false {
			if fieldd.Type.String() == "TYPE_MESSAGE" {
				log.Println(fieldd.String())
				// get this message's descriptor and recurse on it
				buildObjRefMap(msgName, msgDescMap[*fieldd.TypeName])
			}
		}
	}
}

func objRefMap2String() (error, string) {
	log.Println("Object Dependency Map")
	b, err := json.MarshalIndent(objRefMap, "", "  ")
	if err == nil {
		return nil, string(b)
	} else {
		return err, ""
	}
}

func main() {
	g := generator.New()
	if g != nil {
		log.Println("Created generator")
	}

	data, err := ioutil.ReadAll(os.Stdin)
	if err != nil {
		g.Error(err, "reading input")
	}

	if err := proto.Unmarshal(data, g.Request); err != nil {
		g.Error(err, "parsing input proto")
	}

	if len(g.Request.FileToGenerate) == 0 {
		g.Fail("no files to generate")
	}

	log.Println("Files to generate :", g.Request.FileToGenerate)

	// scan all objects and build a map from the message type to its
	// corresponding message descriptor
	log.Println("Building message descriptor map ...")
	for _, filed := range g.Request.ProtoFile {
		// skip all non HAL proto files
		if filed.Options == nil || filed.Options.GoPackage == nil || *filed.Options.GoPackage != "halproto" {
			continue
		}
		log.Println("Processing file", *filed.Name)
		for _, msgd := range filed.MessageType {
			log.Println("Processing message", *msgd.Name)
			buildMsgDescMap("."+*filed.Package, msgd)
		}
	}

	// scan all Spec objects and build key-type to msg map
	log.Println("Building key message map ...")
	for _, filed := range g.Request.ProtoFile {
		// skip all non HAL proto files
		if filed.Options == nil || filed.Options.GoPackage == nil || *filed.Options.GoPackage != "halproto" {
			continue
		}
		log.Println("Processing file", *filed.Name)
		for _, msgd := range filed.MessageType {
			// skip all non Spec object
			if strings.Contains(*msgd.Name, "Spec") == false {
				continue
			}
			log.Println("Processing message", *msgd.Name)
			buildKey2ObjMap("."+*filed.Package, msgd)
		}
	}

	// scan all messages and recursively analyze if any of the fields are
	// references to other messages
	log.Println("Building object dependency map ...")
	for _, filed := range g.Request.ProtoFile {
		// skip all non HAL proto files
		if filed.Options == nil || filed.Options.GoPackage == nil || *filed.Options.GoPackage != "halproto" {
			continue
		}
		log.Println("Processing file", *filed.Name)
		for _, msgd := range filed.MessageType {
			// skip all non Spec object
			if strings.Contains(*msgd.Name, "Spec") == false {
				continue
			}
			log.Println("Processing message", *msgd.Name)
			buildObjRefMap(msgd.Name, msgd)
		}
	}

	// dump all the gathered info for debugging
	//log.Println(msgDescMap2Str())
	log.Println(keyObjMap2String())
	log.Println(objRefMap2String())
}
