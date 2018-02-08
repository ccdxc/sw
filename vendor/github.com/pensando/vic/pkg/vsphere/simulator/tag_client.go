package simulator

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"net/url"
)

const (
	cisBase       = "/rest/com/vmware/cis/"
	sessionPath   = cisBase + "session"
	tagListPath   = cisBase + "tagging/tag"
	tagPath       = cisBase + "tagging/tag/"
	tagAssocPath  = cisBase + "tagging/tag-association/"
	tagAssocMulti = cisBase + "tagging/tag-association"
)

// TagClient is a rest client for vCenter tags interface
type TagClient struct {
	ctx     context.Context
	token   string
	user    string
	pwd     string
	baseURL string
	c       http.Client
	tagIDs  map[string]string
}

// Tag defines a tag object
type Tag struct {
	CategoryID string   `json:"category_id"`
	Name       string   `json:"name"`
	ID         string   `json:"id"`
	UsedBy     []string `json:"used_by,omitempty"`
}

// AttachedObj specifies an object that is tagged
type AttachedObj struct {
	ID   string `json:"id"`
	Type string `json:"type"`
}

type cSpec struct {
	CategoryID  string `json:"category_id"`
	Description string `json:"description,omitempty"`
	Name        string `json:"name"`
}

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
		user:    u.User.Username(),
		pwd:     pwd,
		baseURL: u.Scheme + "://" + u.Host,
		tagIDs:  make(map[string]string),
	}
	return tc
}

// CreateSession creates a new cis session
func (tc *TagClient) CreateSession(ctx context.Context) error {
	// session
	var token struct {
		Value string `json:"value"`
	}
	tc.ctx = ctx
	req, err := http.NewRequest("POST", tc.baseURL+sessionPath, nil)
	if err != nil {
		return err
	}
	req = req.WithContext(tc.ctx)
	req.SetBasicAuth(tc.user, tc.pwd)
	resp, err := tc.c.Do(req)
	if err != nil {
		return err
	}

	err = readResp(resp, &token)
	if token.Value == "" {
		return fmt.Errorf("Failed to get session token")
	}

	tc.token = token.Value
	return nil
}

// DeleteSession deletes the existing session
func (tc *TagClient) DeleteSession() error {
	req, err := http.NewRequest("DELETE", tc.baseURL+sessionPath, nil)
	if err != nil {
		return err
	}
	req.SetBasicAuth(tc.user, tc.pwd)
	resp, err := tc.c.Do(req)
	if err != nil {
		return err
	}
	if resp.StatusCode != int(200) {
		return fmt.Errorf("Server responded with %d", resp.StatusCode)
	}

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
	req.CS.CategoryID = "blah"

	code, err := tc.restReq("POST", tagPath, &req, &resp)
	if code != 200 || err != nil {
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
	u := fmt.Sprintf("%sid:%s", tagPath, id)
	code, err := tc.restReq("PATCH", u, &req, nil)
	if code != 200 || err != nil {
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
	u := fmt.Sprintf("%sid:%s", tagPath, tagID)
	code, err := tc.restReq("DELETE", u, &req, nil)
	if code != 200 || err != nil {
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
	u := fmt.Sprintf("%sid:%s", tagPath, tagID)
	code, err := tc.restReq("GET", u, &req, &resp)
	if code != 200 || err != nil {
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
	u := fmt.Sprintf("%sid:%s", tagPath, tagID)
	code, err := tc.restReq("GET", u, &req, &resp)
	if code != 200 || err != nil {
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
	code, err := tc.restReq("GET", tagListPath, &req, &resp)
	if code != 200 || err != nil {
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
	req.Obj.Type = "VirtualMachine"

	tagID, found := tc.tagIDs[tag]
	if !found {
		return fmt.Errorf("Tag %s not found", tag)
	}
	u := fmt.Sprintf("%sid:%s?~action=%s", tagAssocPath, tagID, action)
	code, err := tc.restReq("POST", u, &req, nil)
	if code != 200 || err != nil {
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

	u := fmt.Sprintf("%sid:%s?~action=%s", tagAssocPath, tagID, "list-attached-objects")
	code, err := tc.restReq("POST", u, &req, &resp)
	if code != 200 || err != nil {
		return nil, fmt.Errorf("failed code: %d err: %v", code, err)
	}

	vms := make([]string, len(resp.Value))
	for ix, obj := range resp.Value {
		if obj.Type == "VirtualMachine" {
			vms[ix] = obj.ID
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
	u := fmt.Sprintf("%s?~action=list-attached-objects-on-tags", tagAssocMulti)
	code, err := tc.restReq("POST", u, &req, &resp)
	if code != 200 || err != nil {
		return nil, fmt.Errorf("failed code: %d err: %v", code, err)
	}

	return resp.Value, nil
}

func (tc *TagClient) restReq(method, url string, body, resp interface{}) (int, error) {
	var buf *bytes.Buffer
	content, err := json.Marshal(body)
	if err != nil {
		return 0, err
	}

	buf = bytes.NewBuffer(content)

	req, err := http.NewRequest(method, tc.baseURL+url, buf)
	if err != nil {
		return 0, err
	}
	req = req.WithContext(tc.ctx)
	req.Header.Set("vmware-api-session-id", tc.token)
	req.Header.Set("Accept", "application/json")
	req.Header.Set("Content-Type", "application/json")
	r, err := tc.c.Do(req)
	if err != nil {
		return 0, err
	}
	if r.StatusCode != int(200) {
		return r.StatusCode, fmt.Errorf("Server responded with %d", r.StatusCode)
	}

	if resp == nil {
		return 200, nil
	}

	defer r.Body.Close()
	b, err := ioutil.ReadAll(r.Body)
	if err != nil {
		return r.StatusCode, err
	}
	err = json.Unmarshal(b, resp)
	if err != nil {
		return r.StatusCode, err
	}

	return r.StatusCode, nil
}

func readResp(r *http.Response, data interface{}) error {
	if r.StatusCode != int(200) {
		return fmt.Errorf("Server responded with %d", r.StatusCode)
	}
	defer r.Body.Close()
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		return err
	}
	err = json.Unmarshal(body, data)
	if err != nil {
		return err
	}

	return nil
}
