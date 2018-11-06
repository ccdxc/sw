package generator

import (
	"strings"
	"unicode"

	"github.com/golang/protobuf/protoc-gen-go/descriptor"
)

// Delphi changes start here
//
// Delpi objects are wrappers around the actual message buffer objects that
// control access to the message buffer object and propagate changes without
// the need of the user to explicitly call SetObject().
//
// The original protobuf objects names are suffixed with and underscore(_)
//
// Things that work at the moment:
// 1. Messages with fields of scalar values
// 2. Messages with fields of message value
// 3. Messages with fields of arrays to scalar values (i.e. repeated)
//
// Things that don't work:
// - Everthing else, including Messages with fields of arrays to messages
//   values, and referring messages to different files.
//

// delphi is stored in the Generator struct.
type delphi struct {
	arrayWrappers map[string]*delphiArrayWrapper
	messages      map[string]*delphiMessage
}

// delphiMessage represents a protobuf message
//   name is the altered name of the message. ie. InterfaceSpec_
//   wrapper is a pointer to the Delphi wrapper object
//   fields is the list of the message's fields
type delphiMessage struct {
	name    string
	wrapper *delphiWrapper
	fields  []*delphiField
}

// delphiField represents a field in a protobuf wrapper
//   name is the name of the field
//   isWrapper is true if the *type* of the field is another delphi wrapper
//   isArray is true if the field is repeated, i.e. array
//   typeName is the type the field should be declare as in the delphi wrapper
//     struct
type delphiField struct {
	name      string
	isWrapper bool
	isArray   bool
	typeName  string
}

// delphiArrayWrapper represents a wrapper of a repeate type. e.g. StringArray
//   wrapperName is the name of the wrapper. e.g. StringArray
//   typeName is the name of the underlying type. e.g string
type delphiArrayWrapper struct {
	wrapperName string
	typeName    string
}

// delphiWrapper represents a wrapper around a protobuf message.
//   name is the name of the wrapper. e.g. InterfaceSpec
//   message is the name of the undeflying message. e.g. InterfaceSpec_
type delphiWrapper struct {
	name    string
	message string
}

func newDelphi() *delphi {
	return &delphi{
		arrayWrappers: make(map[string]*delphiArrayWrapper),
		messages:      make(map[string]*delphiMessage),
	}
}

// trim is used to get the wrapper name from the message name.
// e.g. InterfaceSpec_ -> InterfaceSpec
func trim(s string) string {
	if s[len(s)-1] == '_' {
		return s[:len(s)-1]
	} else {
		return s
	}
}

// upperFirst is used to convert scalar types to uppercase first when
// constructing wrapper around them.
// e.g. an array of string would be:
// stringArray -> StringArray
func upperFirst(s string) string {
	r := []rune(s)
	r[0] = unicode.ToUpper(r[0])
	return string(r)
}

// lowerFirst is mainly used to convert fields to private fields in the wrapper
// and anywhere else similar converstaion is needed.
func lowerFirst(s string) string {
	r := []rune(s)
	r[0] = unicode.ToLower(r[0])
	return string(r)
}

// isGenFile returns true if the file we are looking at is one of the files we
// need to generate code for
func (g *Generator) isGenFile(name string) bool {
	for _, genFile := range g.Request.FileToGenerate {
		if name == genFile {
			return true
		}
	}
	return false
}

// isMessage returns true if a field's type is a message
func (g *Generator) isMessage(field *descriptor.FieldDescriptorProto) bool {
	if field.TypeName != nil {
		fqn := strings.Split(*field.TypeName, ".")
		// Special case. The delphi.Meta should return false and be treated
		// as a scalar value as far as the wrapper is concerned. Ideally
		// the delphi.Meta should be removed from a being a field.
		if fqn[1] == "delphi" {
			return false
		}
	}
	return *field.Type == descriptor.FieldDescriptorProto_TYPE_MESSAGE
}

// idDelphiObj returns true if the message is a delphi obje. I.e. it has "Key"
// and "Meta" fields
func isDelphiObj(msg *delphiMessage) bool {
	for _, field := range msg.fields {
		if field.name == "Meta" {
			return true
		}
	}
	return false
}

// idDelphiObj returns true if the message is a delphi obje. I.e. it has "Meta"
// field but no "Key" field
func isSingleton(msg *delphiMessage) bool {
	var singleton = false
	for _, field := range msg.fields {
		if field.name == "Meta" {
			singleton = true
		}
		if field.name == "Key" {
			return false
		}
		if field.name == "key_or_handle" {
			return false
		}
	}
	return singleton
}

// DelphiRename runs early on and rename messages by appending an underscore to
// their name
func (g *Generator) DelphiRename() {
	for _, file := range g.Request.ProtoFile {
		if g.isGenFile(*file.Name) {
			for _, desc := range file.MessageType {
				nestedTypes := make(map[string]struct{})
				for _, nested := range desc.NestedType {
					nestedTypes["."+*file.Package+"."+*desc.Name+"."+
						*nested.Name] = struct{}{}
				}
				oldDescName := *desc.Name
				newName := *desc.Name + "_"
				desc.Name = &newName
				// the type name is usually something like this:
				// .FILENAME.MESSAGENAME if they are a message
				// for nested fields the name is:
				// .FILENAME.MESSAGENAME.NESTEDTYPENAME
				for _, field := range desc.Field {
					if field.Type.String() == "TYPE_MESSAGE" {
						_, ok := nestedTypes[*field.TypeName]
						if ok == false {
							// Here we just rename:
							// .FILENAME.MESSAGENAME
							// to
							// .FILENAME.MESSAGENAME_
							// ignoring delphi.Meta
							if !strings.HasPrefix(*field.TypeName, ".delphi") {
								newTypeName := *field.TypeName + "_"
								field.TypeName = &newTypeName
							}
						} else {
							// Here we rename:
							// .FILENAME.MESSAGENAME.NESTEDTYPENAME
							// to
							// .FILENAME.MESSAGENAME_.NESTEDTYPENAME
							// ignoring delphi.Meta
							if !strings.HasPrefix(*field.TypeName, ".delphi") {
								newTypeName := strings.Replace(*field.TypeName,
									oldDescName, *desc.Name, -1)
								field.TypeName = &newTypeName
							}
						}
					}
				}
			}
		}
	}
}

// delphiAddArrayWrapper keeps track of what types we need to generate an
// array wrapper for
func (g *Generator) delphiAddArayWrapper(wrapperName, typeName string) {
	g.delphi.arrayWrappers[wrapperName+"Array"] = &delphiArrayWrapper{
		wrapperName: wrapperName,
		typeName:    typeName,
	}
}

func (g *Generator) delphiGenerateImports() {
	g.P(`import clientApi "github.com/pensando/sw/nic/delphi/gosdk/client_api"`)
}

// delphiGenerate first goes through the messages and extract information and
// fills in the g.delphi structure. Then it generates the wrappers.
func (g *Generator) delphiGenerate(file *FileDescriptor) {
	if !g.isGenFile(*file.Name) {
		return
	}

	g.delphi.messages = make(map[string]*delphiMessage)
	for _, desc := range file.desc {
		msg := new(delphiMessage)
		msg.name = *desc.Name
		msg.wrapper = &delphiWrapper{
			name:    trim(*desc.Name),
			message: *desc.Name,
		}

		for _, field := range desc.Field {
			f := new(delphiField)
			f.name = *field.Name
			var typeName string
			if g.isMessage(field) {
				f.isWrapper = true
				typeName, _ = g.GoType(desc, field)
				if isRepeated(field) {
					panic("Not supported yet!")
				}
				typeName = trim(typeName[1:])
			} else {
				if isRepeated(field) {
					f.isWrapper = true
					f.isArray = true
					arrWrp := new(delphiArrayWrapper)
					typeName, _ = g.GoType(desc, field)
					typeName = typeName[2:]
					arrWrp.wrapperName = upperFirst(typeName) + "Array"
					arrWrp.typeName = typeName
					typeName = arrWrp.wrapperName
					g.delphi.arrayWrappers[arrWrp.wrapperName] = arrWrp
				} else {
					f.isWrapper = false
					typeName, _ = g.GoType(desc, field)
				}
			}
			f.typeName = typeName

			msg.fields = append(msg.fields, f)
		}

		g.delphi.messages[msg.name] = msg
	}

	g.delphiGenerateInterfaceDecl()

	for _, msg := range g.delphi.messages {
		g.delphiGenerateMessage(msg)
	}

	for name := range g.delphi.arrayWrappers {
		g.delphiGenerateArrayWrapper(name)
	}
}

// delphiWrapper is an interface that is implemented by all the wrappers and
// is used to propagate change notification to the root wrapper
func (g *Generator) delphiGenerateInterfaceDecl() {
	g.P("type delphiWrapper interface {")
	g.P("  bubbleSave()")
	g.P("}\n")
}

// this is the main function that generates all the wrappers
// (minus the array wrappers)
// TODO: FIXME: Break this down to smaller pieces
func (g *Generator) delphiGenerateMessage(msg *delphiMessage) {
	var keyField *delphiField
	for _, field := range msg.fields {
		if isDelphiObj(msg) && field.name == "Key" {
			keyField = field
		} else if isDelphiObj(msg) && field.name == "key_or_handle" {
			keyField = field
		}
	}

	// struct
	g.P("type " + msg.wrapper.name + " struct {")
	g.In()
	g.P("sdkClient  clientApi.Client")
	g.P("parent delphiWrapper")
	for _, field := range msg.fields {
		if field.name == "Meta" { // special case
			g.P(lowerFirst(field.name) + " " + field.typeName)
		} else if field.isWrapper {
			g.P(lowerFirst(field.name) + " *" + field.typeName)
		} else {
			g.P(lowerFirst(field.name) + " " + field.typeName)
		}
	}
	g.Out()
	g.P("}\n")

	// Getters & Setters
	for _, field := range msg.fields {
		var typeName string
		if field.isWrapper {
			typeName = "*" + field.typeName
		} else {
			typeName = field.typeName
		}
		g.P("func (o *" + msg.wrapper.name + ")Get" + field.name + "() " +
			typeName + "{")
		g.In()
		g.P("return o." + lowerFirst(field.name))
		g.Out()
		g.P("}\n")
		if !field.isWrapper {
			g.P("func (o *" + msg.wrapper.name + ")Set" + field.name + "(val " +
				typeName + ") {")
			g.In()
			g.P("o." + lowerFirst(field.name) + " = val")
			g.P("o.bubbleSave()")
			g.Out()
			g.P("}\n")
		}
	}

	// bubbleSave
	g.P("func (o *" + msg.wrapper.name + ") bubbleSave() {")
	g.P("  if o.parent != nil {")
	g.P("    o.parent.bubbleSave()")
	g.P("  } else {")
	g.P("    o.save()")
	g.P("  }")
	g.P("}\n")

	// save
	g.P("func (o *" + msg.wrapper.name + ") save() {")
	if isDelphiObj(msg) {
		g.P("  if o.GetKeyString() != \"\" {")
		g.P("    o.sdkClient.SetObject(o)")
		g.P("  }")
	}
	g.P("}\n")

	// Delete
	if isDelphiObj(msg) {
		g.P("func (o *" + msg.wrapper.name + ") Delete() {")
		g.In()
		g.P("o.sdkClient.DeleteObject(o) ")
		g.Out()
		g.P("}\n")
	}

	// New<NAME>
	g.P("func New" + msg.wrapper.name + "(sdkClient clientApi.Client) *" +
		msg.wrapper.name + " {")
	g.In()
	g.P("w := &" + msg.wrapper.name + "{}")
	g.P("w.sdkClient = sdkClient")
	for _, field := range msg.fields {
		if field.name == "Meta" { // special case
			g.P("w.meta = &delphi.ObjectMeta{")
			g.P("  Kind: \"" + msg.wrapper.name + "\",")
			g.P("}")
		} else if field.isWrapper {
			g.P("w." + lowerFirst(field.name) + " = " +
				"childNew" + field.typeName + "(w, sdkClient)")
		}
	}
	g.P("return w")
	g.Out()
	g.P("}\n")

	// New<Name>WithKey
	if keyField != nil {
		var intype string
		if keyField.isWrapper {
			intype = "*" + keyField.typeName
		} else {
			intype = keyField.typeName
		}
		g.P("func New" + msg.wrapper.name + "WithKey(sdkClient clientApi.Client, " +
			"key " + intype + ") *" + msg.wrapper.name + " {")
		g.P("  w := New" + msg.wrapper.name + "(sdkClient)")
		if keyField.isWrapper {
			g.P("  w." + keyField.name + " = childNew" + keyField.typeName +
				"WithValue(w, sdkClient, key)")
		} else {
			g.P("  w.SetKey(key)")
		}
		g.P("  return w")
		g.P("}\n")
	}

	// New<NAME>FromMessage
	if isDelphiObj(msg) {
		g.P("func " + "New" + msg.wrapper.name + "FromMessage(sdkClient clientApi.Client, msg *" +
			msg.name + ") *" + msg.wrapper.name + " {")
		g.P("  obj := new" + msg.wrapper.name + "FromMessage (msg)")
		g.P("  obj.sdkClient = sdkClient")
		g.P("  obj.meta = &delphi.ObjectMeta{")
		g.P("  Kind: \"" + msg.wrapper.name + "\",")
		g.P("}\n")
		g.P("  obj.bubbleSave()")
		g.P("  return obj")
		g.P("}\n")
	}

	// Get<Name>
	if keyField != nil {
		var intype string
		if keyField.isWrapper {
			intype = "*" + keyField.typeName
		} else {
			intype = keyField.typeName
		}
		g.P("func Get" + msg.wrapper.name + "(sdkClient clientApi.Client, key " +
			intype + ") *" + msg.wrapper.name + " {")
		if keyField.isWrapper {
			g.P("  lookupKey := key.GetProtoMsg().String()")
		} else {
			g.P("  lookupKey := fmt.Sprintf(\"%v\", key)")
		}
		g.P("  b := sdkClient.GetObject(\"" + msg.wrapper.name +
			"\", lookupKey)")
		g.P("  if b == nil {")
		g.P("    return nil")
		g.P("  }")
		g.P("  o, ok := b.(*" + msg.wrapper.name + ")")
		g.P("  if !ok {")
		g.P("    panic(\"Couldn't cast to " + msg.wrapper.name + "\")")
		g.P("  }")
		g.P("  return o")
		g.P("}\n")
	} else if isSingleton(msg) {
		g.P("func Get" + msg.wrapper.name + "(sdkClient clientApi.Client) *" +
			msg.wrapper.name + " {")
		g.P("  b := sdkClient.GetObject(\"" + msg.wrapper.name +
			"\", \"default\")")
		g.P("  if b == nil {")
		g.P("    return nil")
		g.P("  }")
		g.P("  o, ok := b.(*" + msg.wrapper.name + ")")
		g.P("  if !ok {")
		g.P("    panic(\"Couldn't cast to " + msg.wrapper.name + "\")")
		g.P("  }")
		g.P("  return o")
		g.P("}\n")
	}

	// childNew<NAME>
	g.P("func childNew" + msg.wrapper.name +
		"(parent delphiWrapper, sdkClient clientApi.Client) *" +
		msg.wrapper.name + "{")
	g.P("  w := New" + msg.wrapper.name + "(sdkClient)")
	g.P("  w.parent = parent")
	g.P("  return w")
	g.P("}\n")

	// childNew<NAME>WithValue
	g.P("func childNew" + msg.wrapper.name + "WithValue" +
		"(parent delphiWrapper, sdkClient clientApi.Client, value *" +
		msg.wrapper.name + ") *" +
		msg.wrapper.name + "{")
	g.P("  w := childNew" + msg.wrapper.name + "(parent, sdkClient)")
	for _, field := range msg.fields {
		if field.name == "Meta" { // special case, ignore
		} else if field.isWrapper {
			g.P("w." + lowerFirst(field.name) + " = " + "childNew" +
				field.typeName + "WithValue(w, sdkClient, value. " +
				lowerFirst(field.name) + ")")
		} else {
			g.P("w." + lowerFirst(field.name) + " = value." +
				lowerFirst(field.name))
		}
	}
	g.P("  return w")
	g.P("}\n")

	// GetProtoMsg
	g.P("func (o *" + msg.wrapper.name + ") GetProtoMsg() *" + msg.name + "{")
	g.In()
	g.P("if o == nil {")
	g.P("    return &" + msg.name + "{}")
	g.P("}\n")
	g.P("return &" + msg.name + "{")
	g.In()
	for _, field := range msg.fields {
		if field.isWrapper {
			g.P(CamelCase(field.name) + ": o." + lowerFirst(field.name) + ".GetProtoMsg(),")
		} else {
			g.P(CamelCase(field.name) + ": o." + lowerFirst(field.name) + ",")
		}
	}
	g.Out()
	g.P("}")
	g.Out()
	g.P("}\n")

	// GetMessage
	g.P("func (o *" + msg.wrapper.name + ") GetMessage() proto.Message {")
	g.P("return o.GetProtoMsg()")
	g.P("}\n")

	// // TriggerEvent
	if isDelphiObj(msg) {
		// GetKeyString
		if isSingleton(msg) {
			g.P("func (obj *" + msg.wrapper.name + ") GetKeyString() string {")
			g.P("  return \"default\"")
			g.P("}\n")
		} else {
			for _, field := range msg.fields {
				if field.name == "Key" || field.name == "key_or_handle" {
					g.P("func (obj *" + msg.wrapper.name +
						") GetKeyString() string {")
					if field.isWrapper {
						g.P("return obj." + lowerFirst(field.name) +
							".GetProtoMsg().String()")
					} else {
						g.P("return fmt.Sprintf(\"%v\", (obj." +
							lowerFirst(field.name) + "))")
					}
					g.P("}\n")
				}
			}
		}

		// TriggerEvent
		g.P("func (obj *" + msg.wrapper.name +
			") TriggerEvent(oldObj clientApi.BaseObject, " +
			"op delphi.ObjectOperation, rl []clientApi.BaseReactor) {")
		g.P("  for _, r := range rl {")
		g.P("    rctr, ok := r.(" + msg.wrapper.name + "Reactor)")
		g.P("    if ok == false {")
		g.P("      panic(\"Not a Reactor\")")
		g.P("    }")
		g.P("    if op == delphi.ObjectOperation_SetOp {")
		g.P("      if oldObj == nil {")
		g.P("        rctr.On" + msg.wrapper.name + "Create(obj)")
		g.P("      } else {")
		g.P("        rctr.On" + msg.wrapper.name + "Update(obj)")
		g.P("      }")
		g.P("    } else {")
		g.P("      rctr.On" + msg.wrapper.name + "Delete(obj)")
		g.P("    }")
		g.P("  }")
		g.P("}\n")

		// Reactor Interface Definition
		g.P("type " + msg.wrapper.name + "Reactor interface {")
		g.P("  On" + msg.wrapper.name + "Create(obj *" +
			msg.wrapper.name + ")")
		g.P("  On" + msg.wrapper.name + "Update(obj *" +
			msg.wrapper.name + ")")
		g.P("  On" + msg.wrapper.name + "Delete(obj *" +
			msg.wrapper.name + ")")
		g.P("}\n")

		// GetPath
		g.P("func (obj *" + msg.wrapper.name + ") GetPath() string {")
		g.P("  return \"" + msg.wrapper.name +
			"\" + \"|\" + obj.GetKeyString()")
		g.P("}\n")
	}

	// new<NAME>FromMessage
	g.P("func " + "new" + msg.wrapper.name + "FromMessage (msg *" +
		msg.name + ") *" + msg.wrapper.name + " {")
	g.P("  if msg == nil {")
	g.P("      return &" + msg.wrapper.name + "{}")
	g.P("  }\n")
	g.P("  return &" + msg.wrapper.name + "{")
	for _, field := range msg.fields {
		if field.isWrapper {
			g.P(lowerFirst(field.name) + ": new" + field.typeName +
				"FromMessage(msg." + CamelCase(field.name) + "),")
		} else {
			g.P(lowerFirst(field.name) + ": msg." + CamelCase(field.name) + ",")
		}
	}
	g.P("  }")
	g.P("}\n")

	// Factory Definition
	if isDelphiObj(msg) {
		g.P("func " + lowerFirst(msg.wrapper.name) +
			"Factory(sdkClient clientApi.Client, data []byte) " +
			"(clientApi.BaseObject, error) {")
		g.P("  var msg " + msg.name)
		g.P("  err := proto.Unmarshal(data, &msg)")
		g.P("  if err != nil {")
		g.P("    return nil, err")
		g.P("  }")
		g.P("  w := new" + msg.wrapper.name + "FromMessage(&msg)")
		g.P("  w.sdkClient = sdkClient")
		g.P("  return w, nil")
		g.P("}\n")

		g.init = append(g.init, "clientApi.RegisterFactory(\""+
			msg.wrapper.name+"\", "+lowerFirst(msg.wrapper.name)+"Factory)")
	}

	//<NAME>Mount
	if isDelphiObj(msg) {
		g.P("func " + msg.wrapper.name +
			"Mount(client clientApi.Client, mode delphi.MountMode) {")
		g.P("  client.MountKind(\"" + msg.wrapper.name + "\", mode)")
		g.P("}\n")
	}

	// <NAME>MountKey
	if isDelphiObj(msg) && !isSingleton(msg) {
		var keytype string
		if keyField.isWrapper {
			keytype = "*" + keyField.typeName
		} else {
			keytype = keyField.typeName
		}
		g.P("func " + msg.wrapper.name + "MountKey" +
			"(client clientApi.Client, key " + keytype +
			", mode delphi.MountMode) {")
		if keyField.isWrapper {
			g.P("keyString := key.GetProtoMsg().String()")
		} else {
			g.P("keyString := fmt.Sprintf(\"%v\", key)")
		}
		g.P("  client.MountKindKey(\"" + msg.wrapper.name + "\", keyString " +
			", mode)")
		g.P("}\n")
	}

	//<NAME>Watch
	if isDelphiObj(msg) {
		g.P("func " + msg.wrapper.name + "Watch(client clientApi.Client, reactor " +
			msg.wrapper.name + "Reactor) {")
		g.P("  client.WatchKind(\"" + msg.wrapper.name + "\", reactor)")
		g.P("}\n")
	}

	// Iterator
	if isDelphiObj(msg) {
		// Iterator type
		g.P("type " + msg.wrapper.name + "Iterator struct {")
		g.P("  objects []clientApi.BaseObject")
		g.P("  cur     int")
		g.P("}\n")
		// Iterator Next()
		g.P("func (i *" + msg.wrapper.name + "Iterator)Next() *" +
			msg.wrapper.name + " {")
		g.P("  if i.cur >= len(i.objects) {")
		g.P("    return nil")
		g.P("  }")
		g.P("  obj, ok := i.objects[i.cur].(*" + msg.wrapper.name + ")")
		g.P("  if !ok { panic(\"Cast error\") }")
		g.P("  i.cur++")
		g.P("  return obj")
		g.P("}\n")
		// List
		g.P("func " + msg.wrapper.name + "List(client clientApi.Client) *" +
			msg.wrapper.name + "Iterator {")
		g.P("  return &" + msg.wrapper.name + "Iterator {")
		g.P("    objects: client.List(\"" + msg.wrapper.name + "\"),")
		g.P("    cur: 0,")
		g.P("  }")
		g.P("}\n")
	}
}

func (g *Generator) delphiGenerateArrayWrapper(name string) {
	typeName := g.delphi.arrayWrappers[name].typeName

	// struct
	g.P("type " + name + " struct {")
	g.P("parent delphiWrapper")
	g.P("  values []" + typeName)
	g.P("}\n")

	// Append
	g.P("func (arr *" + name + ") Append(value " + typeName + ") {")
	g.P(" arr.values = append(arr.values, value)")
	g.P(" arr.parent.bubbleSave()")
	g.P("}\n")

	// Get
	g.P("func (arr *" + name + ") Get(pos int) " + typeName + " {")
	g.P("  return arr.values[pos]")
	g.P("}\n")

	// Length
	g.P("func (arr *" + name + ") Length() int {")
	g.P("  return len(arr.values)")
	g.P("}\n")

	// new<NAME>ArrayFromMessage
	g.P("func new" + name +
		"FromMessage (msg []" + typeName + ") *" + name + " {")
	g.P("  arr := new(" + name + ")")
	g.P("  arr.values = make([]" + typeName + ", len(msg))")
	g.P("  copy(arr.values, msg)")
	g.P("  return arr")
	g.P("}\n")

	// childNew<NAME>Array
	g.P("func childNew" + name + " (parent delphiWrapper, " +
		"sdkClient clientApi.Client) *" + name + " {")
	g.P("  arr := new(" + name + ")")
	g.P("  arr.values = make([]" + typeName + ", 0)")
	g.P("  arr.parent = parent")
	g.P("  return arr")
	g.P("}\n")

	// childNew<NAME>ArrayWithValue
	g.P("func childNew" + name + "WithValue(parent delphiWrapper, " +
		"sdkClient clientApi.Client, value *" + name + ") *" + name + " {")
	g.P("  arr := childNew" + name + "(parent, sdkClient)")
	g.P("  for _, v := range value.values {")
	g.P("    arr.values = append(arr.values, v)")
	g.P("  }")
	g.P("  return arr")
	g.P("}\n")

	//GetProtoMsg
	g.P("func (arr *" + name + ") GetProtoMsg() []" + typeName + "{")
	g.P("  v := make([]" + typeName + ", len(arr.values))")
	g.P("  copy(v, arr.values)")
	g.P("  return v")
	g.P("}\n")
}
