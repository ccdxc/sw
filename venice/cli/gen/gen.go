package gen

import (
	"fmt"
	"sort"
	"strings"
)

var (
	info *Info
)

// some function definitions that are registered for every object kind

// RemoveObjOperFunc removes operational state from the object
type RemoveObjOperFunc func(obj interface{}) error

// RestFunc is a generic rest (post/put/delete) function
type RestFunc func(hostname, token string, obj interface{}) error

// RestGetFunc is a generic rest (post/put/get/delete) function
type RestGetFunc func(hostname, tenant, token string, obj interface{}) error

// ObjInfo is information kept per object
type objInfo struct {
	flags         map[string][]CliFlag   // map of oper (create/update/read/etc) to cli flags
	postFuncs     map[string]RestFunc    // map of version (v1, v2, etc) to rest function
	putFuncs      map[string]RestFunc    // map of version (v1, v2, etc) to rest function
	getFuncs      map[string]RestGetFunc // map of version (v1, v2, etc) to rest function
	deleteFuncs   map[string]RestFunc    // map of version (v1, v2, etc) to rest function
	removeObjOper RemoveObjOperFunc      // removes any oper object state
}

// Info is a stash of all generated information that is used to fullfil queries
type Info struct {
	svcName map[string]string
	objInfo map[string]objInfo
}

// CliFlag allows keeping track of information to generate CLI for various fields in an object
type CliFlag struct {
	ID     string
	Type   string
	Help   string
	Skip   bool
	Insert string
}

// GetInfo returns the global generated information
func GetInfo() *Info {
	if info == nil {
		info = &Info{
			svcName: make(map[string]string),
			objInfo: make(map[string]objInfo),
		}
	}
	return info
}

func (cf *Info) initSvc(svcObjName string) string {
	strs := strings.Split(svcObjName, ".")
	if len(strs) != 2 {
		panic(fmt.Sprintf("invalid objname format, should be pkg-name.message-name"))
	}
	objName := strings.ToLower(strs[1])
	if _, ok := cf.svcName[objName]; !ok {
		cf.svcName[objName] = svcObjName
		cf.objInfo[objName] = objInfo{
			flags:       make(map[string][]CliFlag),
			postFuncs:   make(map[string]RestFunc),
			getFuncs:    make(map[string]RestGetFunc),
			putFuncs:    make(map[string]RestFunc),
			deleteFuncs: make(map[string]RestFunc),
		}
	}
	return objName
}

// AddCliInfo is called to register cli information
func (cf *Info) AddCliInfo(svcObjName string, oper string, cliFlags []CliFlag) {
	objName := cf.initSvc(svcObjName)
	if _, ok := cf.objInfo[objName].flags[oper]; ok {
		panic(fmt.Sprintf("ojbname %s oper %s - cli flags already exists!!", objName, oper))
	}
	cf.objInfo[objName].flags[oper] = cliFlags
}

// AddRemoveObjOperFunc is called to register removeObjOper function
func (cf *Info) AddRemoveObjOperFunc(svcObjName string, removeObjOper RemoveObjOperFunc) {
	objName := cf.initSvc(svcObjName)
	newObjInfo := cf.objInfo[objName]
	newObjInfo.removeObjOper = removeObjOper
	cf.objInfo[objName] = newObjInfo
}

// AddRestPostFunc is called to register create function for a speific version of API
func (cf *Info) AddRestPostFunc(svcObjName string, version string, restFunc RestFunc) {
	objName := cf.initSvc(svcObjName)
	if _, ok := cf.objInfo[objName].postFuncs[version]; ok {
		panic(fmt.Sprintf("ojbname %s version %s - create func already exists!!", objName, version))
	}
	cf.objInfo[objName].postFuncs[version] = restFunc
}

// AddRestGetFunc is called to register create function for a speific version of API
func (cf *Info) AddRestGetFunc(svcObjName string, version string, restGetFunc RestGetFunc) {
	objName := cf.initSvc(svcObjName)
	if _, ok := cf.objInfo[objName].getFuncs[version]; ok {
		panic(fmt.Sprintf("ojbname %s version %s - create func already exists!!", objName, version))
	}
	cf.objInfo[objName].getFuncs[version] = restGetFunc
}

// AddRestPutFunc is called to register create function for a speific version of API
func (cf *Info) AddRestPutFunc(svcObjName string, version string, restFunc RestFunc) {
	objName := cf.initSvc(svcObjName)
	if _, ok := cf.objInfo[objName].putFuncs[version]; ok {
		panic(fmt.Sprintf("ojbname %s version %s - create func already exists!!", objName, version))
	}
	cf.objInfo[objName].putFuncs[version] = restFunc
}

// AddRestDeleteFunc is called to register create function for a speific version of API
func (cf *Info) AddRestDeleteFunc(svcObjName string, version string, restFunc RestFunc) {
	objName := cf.initSvc(svcObjName)
	if _, ok := cf.objInfo[objName].deleteFuncs[version]; ok {
		panic(fmt.Sprintf("ojbname %s version %s - create func already exists!!", objName, version))
	}
	cf.objInfo[objName].deleteFuncs[version] = restFunc
}

// FindFlags gets all create flags (used for cli generation) for a given object
func (cf *Info) FindFlags(objName, oper string) ([]CliFlag, error) {
	if objInfo, ok := cf.objInfo[objName]; ok {
		if cliFlags, ok := objInfo.flags[oper]; ok {
			return cliFlags, nil
		}
	}
	return []CliFlag{}, fmt.Errorf("unable to find flags for '%s'", objName)
}

// FindSvcName gives back the fully qualified object and object list name
// The returned values can be used to instantiate new objects of known types
func (cf *Info) FindSvcName(objName string) (string, string, error) {
	if v, ok := cf.svcName[objName]; ok {
		return v, v + "List", nil
	}
	return "", "", fmt.Errorf("unable to find svcname for '%s'", objName)
}

// FindRemoveObjOperFunc returns function that can remove the oper state from a provide structure
// The returned function is used to clean up oper state from any put/post operation
func (cf *Info) FindRemoveObjOperFunc(objName string) (RemoveObjOperFunc, error) {
	if objInfo, ok := cf.objInfo[objName]; ok {
		return objInfo.removeObjOper, nil
	}
	return nil, fmt.Errorf("unable to find deleteObjOper function for '%s'", objName)
}

// FindRestGetFunc returns a function that can perform REST GET on an object
func (cf *Info) FindRestGetFunc(objName, version string) (RestGetFunc, error) {
	if objInfo, ok := cf.objInfo[objName]; ok {
		if getFunc, ok := objInfo.getFuncs[version]; ok {
			return getFunc, nil
		}
	}
	return nil, fmt.Errorf("unable to find restGet function for '%s'", objName)
}

// FindRestDeleteFunc returns a function that can perform REST Delete on an object
func (cf *Info) FindRestDeleteFunc(objName, version string) (RestFunc, error) {
	if objInfo, ok := cf.objInfo[objName]; ok {
		if deleteFunc, ok := objInfo.deleteFuncs[version]; ok {
			return deleteFunc, nil
		}
	}
	return nil, fmt.Errorf("unable to find restDelete function for '%s'", objName)
}

// FindRestPostFunc returns a function that can perform REST Post on an object
func (cf *Info) FindRestPostFunc(objName, version string) (RestFunc, error) {
	if objInfo, ok := cf.objInfo[objName]; ok {
		if postFunc, ok := objInfo.postFuncs[version]; ok {
			return postFunc, nil
		}
	}
	return nil, fmt.Errorf("unable to find restPost function for '%s'", objName)
}

// FindRestPutFunc returns a function that can perform REST Put on an object
func (cf *Info) FindRestPutFunc(objName, version string) (RestFunc, error) {
	if objInfo, ok := cf.objInfo[objName]; ok {
		if putFunc, ok := objInfo.putFuncs[version]; ok {
			return putFunc, nil
		}
	}
	return nil, fmt.Errorf("unable to find restPut function for '%s'", objName)
}

// GetAllKeys lists all registered object kinds, used to iterate over all valid objects
func (cf *Info) GetAllKeys() []string {
	keys := []string{}
	for key := range cf.objInfo {
		keys = append(keys, key)
	}

	sort.Strings(keys)
	return keys
}
