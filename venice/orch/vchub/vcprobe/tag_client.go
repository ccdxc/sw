package vcprobe

import (
	"context"
	"fmt"
	"net/url"

	"github.com/pensando/sw/venice/utils/netutils"
)

const (
	cisBase         = "/rest/com/vmware/cis/"
	sessionPath     = cisBase + "session"
	tagListPath     = cisBase + "tagging/tag"
	tagPath         = cisBase + "tagging/tag/"
	tagIDPath       = tagPath + "id"
	tagAssocPath    = cisBase + "tagging/tag-association/"
	tagAssocMulti   = cisBase + "tagging/tag-association"
	defaultCategory = "dogsVsCats"
	vmObjType       = "VirtualMachine"
)

// TagClient is a rest client for vCenter tags interface
// Meant to be single threaded
type TagClient struct {
	ctx     context.Context
	user    string
	pwd     string
	baseURL string
	c       *netutils.HTTPClient
	tagIDs  map[string]string // tagname to uuid
}

// The following definitions are from the spec at
// https://code.vmware.com/apis/191/vsphere-automation

// Tag defines a tag object
type Tag struct {
	CategoryID string   `json:"category_id"`
	Name       string   `json:"name"`              // name of the tag
	ID         string   `json:"id"`                // uuid of the tag
	UsedBy     []string `json:"used_by,omitempty"` // not used by vcprobe
}

// AttachedObj specifies an object that is tagged
type AttachedObj struct {
	ID   string `json:"id"`   // uuid of the attached object
	Type string `json:"type"` // type of the attached object
}

// createSpec for a tag
type cSpec struct {
	CategoryID  string `json:"category_id"`
	Description string `json:"description,omitempty"`
	Name        string `json:"name"`
}

// update Spec for a tag
type uSpec struct {
	Description string `json:"description,omitempty"`
	Name        string `json:"name"`
}

// TagObjects specifies a list of objects that have the same tag
type TagObjects struct {
	ObjIDs []AttachedObj `json:"object_ids"`
	TagID  string        `json:"tag_id"`
}

// NewTagClient returns a client for the specified URL
func NewTagClient(u *url.URL) *TagClient {
	pwd, _ := u.User.Password()
	tc := &TagClient{
		c:       netutils.NewHTTPClient(),
		user:    u.User.Username(),
		pwd:     pwd,
		baseURL: u.Scheme + "://" + u.Host,
		tagIDs:  make(map[string]string),
	}
	return tc
}

// SetContext sets the context of the client
func (tc *TagClient) SetContext(ctx context.Context) {
	tc.ctx = ctx
	tc.c.WithContext(ctx)
}

// CreateSession creates a new cis session
// There is a single session for the client, based on
// the auth provided
func (tc *TagClient) CreateSession(ctx context.Context) error {
	// session
	var token struct {
		Value string `json:"value"`
	}
	tc.ctx = ctx
	tc.c.WithContext(ctx)
	tc.c.DeleteHeader("vmware-api-session-id")
	r, err := tc.c.BasicAuthReq("POST", tc.baseURL+sessionPath, tc.user, tc.pwd)
	if err != nil {
		return err
	}
	err = netutils.ReadHTTPResp(r, &token)
	if token.Value == "" {
		return fmt.Errorf("Failed to get session token")
	}

	tc.c.SetHeader("vmware-api-session-id", token.Value)
	return nil
}

// DeleteSession deletes the existing session
func (tc *TagClient) DeleteSession() error {
	var req struct{}
	_, err := tc.c.Req("DELETE", tc.baseURL+sessionPath, &req, nil)
	if err != nil {
		return err
	}
	tc.c.DeleteHeader("vmware-api-session-id")
	return nil
}

// CreateTag creates a new tag.
func (tc *TagClient) CreateTag(name string) error {
	var req struct {
		CS cSpec `json:"create_spec"`
	}
	var resp struct {
		Value string `json:"value"`
	}
	req.CS.Name = name
	req.CS.CategoryID = defaultCategory

	code, err := tc.c.Req("POST", tc.baseURL+tagPath, &req, &resp)
	if err != nil {
		return fmt.Errorf("createTag failed code: %d err: %v", code, err)
	}

	tc.tagIDs[name] = resp.Value

	return nil
}

// UpdateTag updates the name of a tag
func (tc *TagClient) UpdateTag(id, name string) error {
	var req struct {
		US uSpec `json:"update_spec"`
	}
	req.US.Name = name
	u := fmt.Sprintf("%s%s:%s", tc.baseURL, tagIDPath, id)
	code, err := tc.c.Req("PATCH", u, &req, nil)
	if err != nil {
		return fmt.Errorf("updateTag failed code: %d err: %v", code, err)
	}

	oldName := ""
	for k, v := range tc.tagIDs {
		if v == id {
			oldName = k
		}
	}

	delete(tc.tagIDs, oldName)
	tc.tagIDs[name] = id
	return nil
}

// DeleteTag deletes an existing tag
func (tc *TagClient) DeleteTag(name string) error {
	tagID, found := tc.tagIDs[name]
	if !found {
		return fmt.Errorf("tag %s not found", name)
	}

	var req struct{}
	u := fmt.Sprintf("%s%s:%s", tc.baseURL, tagIDPath, tagID)
	code, err := tc.c.Req("DELETE", u, &req, nil)
	if err != nil {
		return fmt.Errorf("deleteTag failed code: %d err: %v", code, err)
	}

	delete(tc.tagIDs, name)
	return nil
}

// GetTagByName gets the tagID matching the tag name
func (tc *TagClient) GetTagByName(name string) (string, error) {
	tagID, found := tc.tagIDs[name]
	if !found {
		return "", fmt.Errorf("tag %s not found", name)
	}

	var req struct{}
	var resp struct {
		Value *Tag `json:"value"`
	}
	u := fmt.Sprintf("%s%s:%s", tc.baseURL, tagIDPath, tagID)
	code, err := tc.c.Req("GET", u, &req, &resp)
	if err != nil {
		return "", fmt.Errorf("failed code: %d err: %v", code, err)
	}

	if resp.Value.Name != name {
		return "", fmt.Errorf("Exp %s, got %s", name, resp.Value.Name)
	}

	return resp.Value.ID, nil
}

// GetTag returns the name and category of the specified tagID
func (tc *TagClient) GetTag(tagID string) (string, string, error) {

	var req struct{}
	var resp struct {
		Value *Tag `json:"value"`
	}
	u := fmt.Sprintf("%s%s:%s", tc.baseURL, tagIDPath, tagID)
	code, err := tc.c.Req("GET", u, &req, &resp)
	if err != nil {
		return "", "", fmt.Errorf("failed code: %d err: %v", code, err)
	}

	if resp.Value.ID != tagID {
		return "", "", fmt.Errorf("Exp %s, got %s", tagID, resp.Value.ID)
	}

	return resp.Value.Name, resp.Value.CategoryID, nil
}

// ListTags returns a list of all tags
func (tc *TagClient) ListTags() ([]string, error) {
	var req struct{}
	var resp struct {
		Value []string `json:"value"`
	}
	code, err := tc.c.Req("GET", tc.baseURL+tagListPath, &req, &resp)
	if err != nil {
		return nil, fmt.Errorf("failed code: %d err: %v", code, err)
	}

	if len(resp.Value) != len(tc.tagIDs) {
		return resp.Value, fmt.Errorf("Exp %d tags, got %d", len(tc.tagIDs), len(resp.Value))
	}

	return resp.Value, nil
}

// Attach attaches a tag to the vm
func (tc *TagClient) Attach(tag, vmID string) error {
	return tc.action("attach", tag, vmID)
}

// Detach removes the tag from the vm
func (tc *TagClient) Detach(tag, vmID string) error {
	return tc.action("detach", tag, vmID)
}

func (tc *TagClient) action(action, tag, vmID string) error {
	var req struct {
		Obj AttachedObj `json:"object_id"`
	}
	req.Obj.ID = vmID
	req.Obj.Type = vmObjType

	tagID, found := tc.tagIDs[tag]
	if !found {
		return fmt.Errorf("Tag %s not found", tag)
	}
	u := fmt.Sprintf("%s%sid:%s?~action=%s", tc.baseURL, tagAssocPath, tagID, action)
	code, err := tc.c.Req("POST", u, &req, nil)
	if err != nil {
		return fmt.Errorf("failed code: %d err: %v", code, err)
	}

	return nil
}

// ListVMs returns a list of VMs that have the specified tag
func (tc *TagClient) ListVMs(tagID string) ([]string, error) {
	var req struct{}
	var resp struct {
		Value []AttachedObj `json:"value"`
	}

	u := fmt.Sprintf("%s%sid:%s?~action=%s", tc.baseURL, tagAssocPath, tagID, "list-attached-objects")
	code, err := tc.c.Req("POST", u, &req, &resp)
	if err != nil {
		return nil, fmt.Errorf("failed code: %d err: %v", code, err)
	}

	vms := make([]string, 0, len(resp.Value))
	for _, obj := range resp.Value {
		if obj.Type == vmObjType {
			vms = append(vms, obj.ID)
		}
	}

	return vms, nil
}

// ListVMsMulti returns a list of list of vms for each of the
// specified tags
func (tc *TagClient) ListVMsMulti(tagIDs []string) ([]TagObjects, error) {
	var req struct {
		TagIDs []string `json:"tag_ids"`
	}
	var resp struct {
		Value []TagObjects `json:"value"`
	}

	req.TagIDs = tagIDs
	u := fmt.Sprintf("%s%s?~action=list-attached-objects-on-tags", tc.baseURL, tagAssocMulti)
	code, err := tc.c.Req("POST", u, &req, &resp)
	if err != nil {
		return nil, fmt.Errorf("failed code: %d err: %v", code, err)
	}

	return resp.Value, nil
}
