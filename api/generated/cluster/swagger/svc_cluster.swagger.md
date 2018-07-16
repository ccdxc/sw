svc_cluster.proto
=================
**Version:** version not set

### /configs/cluster/v1/cluster
---
##### ***GET***
**Summary:** Retreives a list of Cluster objects

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.name | query | Name of the object, unique within a Namespace for scoped objects. | No | string |
| O.tenant | query | Tenant is global namespace isolation for various objects. This can be automatically filled in many cases based on the tenant a user, who created the object, belongs go. | No | string |
| O.namespace | query | Namespace of the object, for scoped objects. | No | string |
| O.resource-version | query | Resource version in the object store. This can only be set by the server. | No | string |
| O.uuid | query | UUID is the unique identifier for the object. This can only be set by the server. | No | string |
| O.creation-time | query | CreationTime is the creation time of Object. | No | dateTime |
| O.mod-time | query | ModTime is the Last Modification time of Object. | No | dateTime |
| O.self-link | query | SelfLink is a link to accessing this object. When stored in the KV store this is  the key in the kvstore and when the object is served from the API-GW it is the  URI path. Examples    - "/venice/tenants/tenants/tenant2" in the kvstore    - "/v1/tenants/tenants/tenant2" when served by API Gateway. | No | string |
| label-selector | query |  | No | string |
| field-selector | query |  | No | string |
| prefix-watch | query |  | No | boolean (boolean) |
| field-change-selector | query |  | No | [ string ] |
| from | query | from represents the start offset (zero based), used for pagination. results returned would be in the range [from ... from+max-results-1]. | No | integer |
| max-results | query | max. number of events to be fetched for the request. | No | integer |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterClusterList](#clusterclusterlist) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

### /configs/cluster/v1/cluster/{O.Name}
---
##### ***GET***
**Summary:** Retreives the Cluster object

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.Name | path |  | Yes | string |
| T.kind | query | Kind represents the type of the API object. | No | string |
| T.api-version | query | APIVersion defines the version of the API object. | No | string |
| meta.tenant | query | Tenant is global namespace isolation for various objects. This can be automatically filled in many cases based on the tenant a user, who created the object, belongs go. | No | string |
| meta.namespace | query | Namespace of the object, for scoped objects. | No | string |
| meta.resource-version | query | Resource version in the object store. This can only be set by the server. | No | string |
| meta.uuid | query | UUID is the unique identifier for the object. This can only be set by the server. | No | string |
| meta.creation-time | query | CreationTime is the creation time of Object. | No | dateTime |
| meta.mod-time | query | ModTime is the Last Modification time of Object. | No | dateTime |
| meta.self-link | query | SelfLink is a link to accessing this object. When stored in the KV store this is  the key in the kvstore and when the object is served from the API-GW it is the  URI path. Examples    - "/venice/tenants/tenants/tenant2" in the kvstore    - "/v1/tenants/tenants/tenant2" when served by API Gateway. | No | string |
| spec.quorum-nodes | query | QuorumNodes contains the list of hostnames for nodes configured to be quorum nodes in the cluster. | No | [ string ] |
| spec.virtual-ip | query | VirtualIP is the IP address for managing the cluster. It will be hosted by the winner of election between quorum nodes. | No | string |
| spec.ntp-servers | query | NTPServers contains the list of NTP servers for the cluster. | No | [ string ] |
| spec.dns-subdomain | query | DNSSubDomain is the DNS subdomain for the default tenant. | No | string |
| spec.auto-admit-nics | query | AutoAdmitNICs when enabled auto-admits NICs that are validated into Venice Cluster. When it is disabled, NICs validated by CMD are set to Pending state and it requires Manual approval to be admitted into the cluster. | No | boolean (boolean) |
| status.leader | query | Leader contains the node name of the cluster leader. | No | string |
| status.last-leader-transition-time | query | LastLeaderTransitionTime is when the leadership changed last time. | No | dateTime |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterCluster](#clustercluster) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

##### ***DELETE***
**Summary:** Deletes the Cluster object

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.Name | path |  | Yes | string |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterCluster](#clustercluster) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

##### ***PUT***
**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.Name | path |  | Yes | string |
| body | body |  | Yes | [clusterCluster](#clustercluster) |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterCluster](#clustercluster) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

### /configs/cluster/v1/hosts
---
##### ***GET***
**Summary:** Retreives a list of Host objects

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.name | query | Name of the object, unique within a Namespace for scoped objects. | No | string |
| O.tenant | query | Tenant is global namespace isolation for various objects. This can be automatically filled in many cases based on the tenant a user, who created the object, belongs go. | No | string |
| O.namespace | query | Namespace of the object, for scoped objects. | No | string |
| O.resource-version | query | Resource version in the object store. This can only be set by the server. | No | string |
| O.uuid | query | UUID is the unique identifier for the object. This can only be set by the server. | No | string |
| O.creation-time | query | CreationTime is the creation time of Object. | No | dateTime |
| O.mod-time | query | ModTime is the Last Modification time of Object. | No | dateTime |
| O.self-link | query | SelfLink is a link to accessing this object. When stored in the KV store this is  the key in the kvstore and when the object is served from the API-GW it is the  URI path. Examples    - "/venice/tenants/tenants/tenant2" in the kvstore    - "/v1/tenants/tenants/tenant2" when served by API Gateway. | No | string |
| label-selector | query |  | No | string |
| field-selector | query |  | No | string |
| prefix-watch | query |  | No | boolean (boolean) |
| field-change-selector | query |  | No | [ string ] |
| from | query | from represents the start offset (zero based), used for pagination. results returned would be in the range [from ... from+max-results-1]. | No | integer |
| max-results | query | max. number of events to be fetched for the request. | No | integer |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterHostList](#clusterhostlist) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

##### ***POST***
**Summary:** Creates a new Host object

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| body | body |  | Yes | [clusterHost](#clusterhost) |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterHost](#clusterhost) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

### /configs/cluster/v1/hosts/{O.Name}
---
##### ***GET***
**Summary:** Retreives the Host object

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.Name | path |  | Yes | string |
| T.kind | query | Kind represents the type of the API object. | No | string |
| T.api-version | query | APIVersion defines the version of the API object. | No | string |
| meta.tenant | query | Tenant is global namespace isolation for various objects. This can be automatically filled in many cases based on the tenant a user, who created the object, belongs go. | No | string |
| meta.namespace | query | Namespace of the object, for scoped objects. | No | string |
| meta.resource-version | query | Resource version in the object store. This can only be set by the server. | No | string |
| meta.uuid | query | UUID is the unique identifier for the object. This can only be set by the server. | No | string |
| meta.creation-time | query | CreationTime is the creation time of Object. | No | dateTime |
| meta.mod-time | query | ModTime is the Last Modification time of Object. | No | dateTime |
| meta.self-link | query | SelfLink is a link to accessing this object. When stored in the KV store this is  the key in the kvstore and when the object is served from the API-GW it is the  URI path. Examples    - "/venice/tenants/tenants/tenant2" in the kvstore    - "/v1/tenants/tenants/tenant2" when served by API Gateway. | No | string |
| status.type | query | Type of the host. | No | string |
| status.operating-system | query | Operating system info - name of OS with version, eg: "Linux 3.10.0.xyz". | No | string |
| status.orchestrator | query | Orchestrator is the name of associated Compute controller (like VCenter) managing this host. | No | string |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterHost](#clusterhost) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

##### ***DELETE***
**Summary:** Deletes the Host object

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.Name | path |  | Yes | string |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterHost](#clusterhost) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

##### ***PUT***
**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.Name | path |  | Yes | string |
| body | body |  | Yes | [clusterHost](#clusterhost) |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterHost](#clusterhost) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

### /configs/cluster/v1/nodes
---
##### ***GET***
**Summary:** Retreives a list of Node objects

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.name | query | Name of the object, unique within a Namespace for scoped objects. | No | string |
| O.tenant | query | Tenant is global namespace isolation for various objects. This can be automatically filled in many cases based on the tenant a user, who created the object, belongs go. | No | string |
| O.namespace | query | Namespace of the object, for scoped objects. | No | string |
| O.resource-version | query | Resource version in the object store. This can only be set by the server. | No | string |
| O.uuid | query | UUID is the unique identifier for the object. This can only be set by the server. | No | string |
| O.creation-time | query | CreationTime is the creation time of Object. | No | dateTime |
| O.mod-time | query | ModTime is the Last Modification time of Object. | No | dateTime |
| O.self-link | query | SelfLink is a link to accessing this object. When stored in the KV store this is  the key in the kvstore and when the object is served from the API-GW it is the  URI path. Examples    - "/venice/tenants/tenants/tenant2" in the kvstore    - "/v1/tenants/tenants/tenant2" when served by API Gateway. | No | string |
| label-selector | query |  | No | string |
| field-selector | query |  | No | string |
| prefix-watch | query |  | No | boolean (boolean) |
| field-change-selector | query |  | No | [ string ] |
| from | query | from represents the start offset (zero based), used for pagination. results returned would be in the range [from ... from+max-results-1]. | No | integer |
| max-results | query | max. number of events to be fetched for the request. | No | integer |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterNodeList](#clusternodelist) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

##### ***POST***
**Summary:** Creates a new Node object

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| body | body |  | Yes | [clusterNode](#clusternode) |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterNode](#clusternode) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

### /configs/cluster/v1/nodes/{O.Name}
---
##### ***GET***
**Summary:** Retreives the Node object

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.Name | path |  | Yes | string |
| T.kind | query | Kind represents the type of the API object. | No | string |
| T.api-version | query | APIVersion defines the version of the API object. | No | string |
| meta.tenant | query | Tenant is global namespace isolation for various objects. This can be automatically filled in many cases based on the tenant a user, who created the object, belongs go. | No | string |
| meta.namespace | query | Namespace of the object, for scoped objects. | No | string |
| meta.resource-version | query | Resource version in the object store. This can only be set by the server. | No | string |
| meta.uuid | query | UUID is the unique identifier for the object. This can only be set by the server. | No | string |
| meta.creation-time | query | CreationTime is the creation time of Object. | No | dateTime |
| meta.mod-time | query | ModTime is the Last Modification time of Object. | No | dateTime |
| meta.self-link | query | SelfLink is a link to accessing this object. When stored in the KV store this is  the key in the kvstore and when the object is served from the API-GW it is the  URI path. Examples    - "/venice/tenants/tenants/tenant2" in the kvstore    - "/v1/tenants/tenants/tenant2" when served by API Gateway. | No | string |
| status.phase | query | Current lifecycle phase of the node. | No | string |
| status.quorum | query | Quorum node or not. | No | boolean (boolean) |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterNode](#clusternode) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

##### ***DELETE***
**Summary:** Deletes the Node object

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.Name | path |  | Yes | string |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterNode](#clusternode) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

##### ***PUT***
**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.Name | path |  | Yes | string |
| body | body |  | Yes | [clusterNode](#clusternode) |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterNode](#clusternode) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

### /configs/cluster/v1/smartnics
---
##### ***GET***
**Summary:** Retreives a list of SmartNIC objects

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.name | query | Name of the object, unique within a Namespace for scoped objects. | No | string |
| O.tenant | query | Tenant is global namespace isolation for various objects. This can be automatically filled in many cases based on the tenant a user, who created the object, belongs go. | No | string |
| O.namespace | query | Namespace of the object, for scoped objects. | No | string |
| O.resource-version | query | Resource version in the object store. This can only be set by the server. | No | string |
| O.uuid | query | UUID is the unique identifier for the object. This can only be set by the server. | No | string |
| O.creation-time | query | CreationTime is the creation time of Object. | No | dateTime |
| O.mod-time | query | ModTime is the Last Modification time of Object. | No | dateTime |
| O.self-link | query | SelfLink is a link to accessing this object. When stored in the KV store this is  the key in the kvstore and when the object is served from the API-GW it is the  URI path. Examples    - "/venice/tenants/tenants/tenant2" in the kvstore    - "/v1/tenants/tenants/tenant2" when served by API Gateway. | No | string |
| label-selector | query |  | No | string |
| field-selector | query |  | No | string |
| prefix-watch | query |  | No | boolean (boolean) |
| field-change-selector | query |  | No | [ string ] |
| from | query | from represents the start offset (zero based), used for pagination. results returned would be in the range [from ... from+max-results-1]. | No | integer |
| max-results | query | max. number of events to be fetched for the request. | No | integer |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterSmartNICList](#clustersmartniclist) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

##### ***POST***
**Summary:** Creates a new SmartNIC object

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| body | body |  | Yes | [clusterSmartNIC](#clustersmartnic) |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterSmartNIC](#clustersmartnic) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

### /configs/cluster/v1/smartnics/{O.Name}
---
##### ***GET***
**Summary:** Retreives the SmartNIC object

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.Name | path |  | Yes | string |
| T.kind | query | Kind represents the type of the API object. | No | string |
| T.api-version | query | APIVersion defines the version of the API object. | No | string |
| meta.tenant | query | Tenant is global namespace isolation for various objects. This can be automatically filled in many cases based on the tenant a user, who created the object, belongs go. | No | string |
| meta.namespace | query | Namespace of the object, for scoped objects. | No | string |
| meta.resource-version | query | Resource version in the object store. This can only be set by the server. | No | string |
| meta.uuid | query | UUID is the unique identifier for the object. This can only be set by the server. | No | string |
| meta.creation-time | query | CreationTime is the creation time of Object. | No | dateTime |
| meta.mod-time | query | ModTime is the Last Modification time of Object. | No | dateTime |
| meta.self-link | query | SelfLink is a link to accessing this object. When stored in the KV store this is  the key in the kvstore and when the object is served from the API-GW it is the  URI path. Examples    - "/venice/tenants/tenants/tenant2" in the kvstore    - "/v1/tenants/tenants/tenant2" when served by API Gateway. | No | string |
| spec.phase | query | Current phase of the SmartNIC. When auto-admission is enabled, Phase will be set to NIC_ADMITTED by CMD for validated NICs. When auto-admission is not enabled, Phase will be set to NIC_PENDING by CMD for validated NICs since it requires manual approval. To admit the NIC as a part of manual admission, user is expected to set the Phase to NIC_ADMITTED for the NICs that are in NIC_PENDING state. Note : Whitelist mode is not supported yet. | No | string |
| spec.mgmt-ip | query | Management IP address of the naples node. | No | string |
| spec.host-name | query | Host name. | No | string |
| status.serial-num | query | Serial number. | No | string |
| status.primary-mac-address | query | Primary MAC address, which is MAC address of the primary PF exposed by SmartNIC. | No | string |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterSmartNIC](#clustersmartnic) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

##### ***DELETE***
**Summary:** Deletes the SmartNIC object

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.Name | path |  | Yes | string |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterSmartNIC](#clustersmartnic) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

##### ***PUT***
**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.Name | path |  | Yes | string |
| body | body |  | Yes | [clusterSmartNIC](#clustersmartnic) |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterSmartNIC](#clustersmartnic) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

### /configs/cluster/v1/tenants
---
##### ***GET***
**Summary:** Retreives a list of Tenant objects

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.name | query | Name of the object, unique within a Namespace for scoped objects. | No | string |
| O.tenant | query | Tenant is global namespace isolation for various objects. This can be automatically filled in many cases based on the tenant a user, who created the object, belongs go. | No | string |
| O.namespace | query | Namespace of the object, for scoped objects. | No | string |
| O.resource-version | query | Resource version in the object store. This can only be set by the server. | No | string |
| O.uuid | query | UUID is the unique identifier for the object. This can only be set by the server. | No | string |
| O.creation-time | query | CreationTime is the creation time of Object. | No | dateTime |
| O.mod-time | query | ModTime is the Last Modification time of Object. | No | dateTime |
| O.self-link | query | SelfLink is a link to accessing this object. When stored in the KV store this is  the key in the kvstore and when the object is served from the API-GW it is the  URI path. Examples    - "/venice/tenants/tenants/tenant2" in the kvstore    - "/v1/tenants/tenants/tenant2" when served by API Gateway. | No | string |
| label-selector | query |  | No | string |
| field-selector | query |  | No | string |
| prefix-watch | query |  | No | boolean (boolean) |
| field-change-selector | query |  | No | [ string ] |
| from | query | from represents the start offset (zero based), used for pagination. results returned would be in the range [from ... from+max-results-1]. | No | integer |
| max-results | query | max. number of events to be fetched for the request. | No | integer |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterTenantList](#clustertenantlist) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

##### ***POST***
**Summary:** Creates a new Tenant object

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| body | body |  | Yes | [clusterTenant](#clustertenant) |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterTenant](#clustertenant) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

### /configs/cluster/v1/tenants/{O.Name}
---
##### ***GET***
**Summary:** Retreives the Tenant object

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.Name | path |  | Yes | string |
| T.kind | query | Kind represents the type of the API object. | No | string |
| T.api-version | query | APIVersion defines the version of the API object. | No | string |
| meta.tenant | query | Tenant is global namespace isolation for various objects. This can be automatically filled in many cases based on the tenant a user, who created the object, belongs go. | No | string |
| meta.namespace | query | Namespace of the object, for scoped objects. | No | string |
| meta.resource-version | query | Resource version in the object store. This can only be set by the server. | No | string |
| meta.uuid | query | UUID is the unique identifier for the object. This can only be set by the server. | No | string |
| meta.creation-time | query | CreationTime is the creation time of Object. | No | dateTime |
| meta.mod-time | query | ModTime is the Last Modification time of Object. | No | dateTime |
| meta.self-link | query | SelfLink is a link to accessing this object. When stored in the KV store this is  the key in the kvstore and when the object is served from the API-GW it is the  URI path. Examples    - "/venice/tenants/tenants/tenant2" in the kvstore    - "/v1/tenants/tenants/tenant2" when served by API Gateway. | No | string |
| spec.admin-user | query | Tenant admin user. | No | string |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterTenant](#clustertenant) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

##### ***DELETE***
**Summary:** Deletes the Tenant object

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.Name | path |  | Yes | string |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterTenant](#clustertenant) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

##### ***PUT***
**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.Name | path |  | Yes | string |
| body | body |  | Yes | [clusterTenant](#clustertenant) |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 |  | [clusterTenant](#clustertenant) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

### /configs/cluster/v1/watch/cluster
---
##### ***GET***
**Summary:** Watch for changes to Cluster objects

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.name | query | Name of the object, unique within a Namespace for scoped objects. | No | string |
| O.tenant | query | Tenant is global namespace isolation for various objects. This can be automatically filled in many cases based on the tenant a user, who created the object, belongs go. | No | string |
| O.namespace | query | Namespace of the object, for scoped objects. | No | string |
| O.resource-version | query | Resource version in the object store. This can only be set by the server. | No | string |
| O.uuid | query | UUID is the unique identifier for the object. This can only be set by the server. | No | string |
| O.creation-time | query | CreationTime is the creation time of Object. | No | dateTime |
| O.mod-time | query | ModTime is the Last Modification time of Object. | No | dateTime |
| O.self-link | query | SelfLink is a link to accessing this object. When stored in the KV store this is  the key in the kvstore and when the object is served from the API-GW it is the  URI path. Examples    - "/venice/tenants/tenants/tenant2" in the kvstore    - "/v1/tenants/tenants/tenant2" when served by API Gateway. | No | string |
| label-selector | query |  | No | string |
| field-selector | query |  | No | string |
| prefix-watch | query |  | No | boolean (boolean) |
| field-change-selector | query |  | No | [ string ] |
| from | query | from represents the start offset (zero based), used for pagination. results returned would be in the range [from ... from+max-results-1]. | No | integer |
| max-results | query | max. number of events to be fetched for the request. | No | integer |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 | (streaming responses) | [clusterAutoMsgClusterWatchHelper](#clusterautomsgclusterwatchhelper) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

### /configs/cluster/v1/watch/hosts
---
##### ***GET***
**Summary:** Watch for changes to Host objects

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.name | query | Name of the object, unique within a Namespace for scoped objects. | No | string |
| O.tenant | query | Tenant is global namespace isolation for various objects. This can be automatically filled in many cases based on the tenant a user, who created the object, belongs go. | No | string |
| O.namespace | query | Namespace of the object, for scoped objects. | No | string |
| O.resource-version | query | Resource version in the object store. This can only be set by the server. | No | string |
| O.uuid | query | UUID is the unique identifier for the object. This can only be set by the server. | No | string |
| O.creation-time | query | CreationTime is the creation time of Object. | No | dateTime |
| O.mod-time | query | ModTime is the Last Modification time of Object. | No | dateTime |
| O.self-link | query | SelfLink is a link to accessing this object. When stored in the KV store this is  the key in the kvstore and when the object is served from the API-GW it is the  URI path. Examples    - "/venice/tenants/tenants/tenant2" in the kvstore    - "/v1/tenants/tenants/tenant2" when served by API Gateway. | No | string |
| label-selector | query |  | No | string |
| field-selector | query |  | No | string |
| prefix-watch | query |  | No | boolean (boolean) |
| field-change-selector | query |  | No | [ string ] |
| from | query | from represents the start offset (zero based), used for pagination. results returned would be in the range [from ... from+max-results-1]. | No | integer |
| max-results | query | max. number of events to be fetched for the request. | No | integer |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 | (streaming responses) | [clusterAutoMsgHostWatchHelper](#clusterautomsghostwatchhelper) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

### /configs/cluster/v1/watch/nodes
---
##### ***GET***
**Summary:** Watch for changes to Node objects

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.name | query | Name of the object, unique within a Namespace for scoped objects. | No | string |
| O.tenant | query | Tenant is global namespace isolation for various objects. This can be automatically filled in many cases based on the tenant a user, who created the object, belongs go. | No | string |
| O.namespace | query | Namespace of the object, for scoped objects. | No | string |
| O.resource-version | query | Resource version in the object store. This can only be set by the server. | No | string |
| O.uuid | query | UUID is the unique identifier for the object. This can only be set by the server. | No | string |
| O.creation-time | query | CreationTime is the creation time of Object. | No | dateTime |
| O.mod-time | query | ModTime is the Last Modification time of Object. | No | dateTime |
| O.self-link | query | SelfLink is a link to accessing this object. When stored in the KV store this is  the key in the kvstore and when the object is served from the API-GW it is the  URI path. Examples    - "/venice/tenants/tenants/tenant2" in the kvstore    - "/v1/tenants/tenants/tenant2" when served by API Gateway. | No | string |
| label-selector | query |  | No | string |
| field-selector | query |  | No | string |
| prefix-watch | query |  | No | boolean (boolean) |
| field-change-selector | query |  | No | [ string ] |
| from | query | from represents the start offset (zero based), used for pagination. results returned would be in the range [from ... from+max-results-1]. | No | integer |
| max-results | query | max. number of events to be fetched for the request. | No | integer |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 | (streaming responses) | [clusterAutoMsgNodeWatchHelper](#clusterautomsgnodewatchhelper) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

### /configs/cluster/v1/watch/smartnics
---
##### ***GET***
**Summary:** Watch for changes to SmartNIC objects

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.name | query | Name of the object, unique within a Namespace for scoped objects. | No | string |
| O.tenant | query | Tenant is global namespace isolation for various objects. This can be automatically filled in many cases based on the tenant a user, who created the object, belongs go. | No | string |
| O.namespace | query | Namespace of the object, for scoped objects. | No | string |
| O.resource-version | query | Resource version in the object store. This can only be set by the server. | No | string |
| O.uuid | query | UUID is the unique identifier for the object. This can only be set by the server. | No | string |
| O.creation-time | query | CreationTime is the creation time of Object. | No | dateTime |
| O.mod-time | query | ModTime is the Last Modification time of Object. | No | dateTime |
| O.self-link | query | SelfLink is a link to accessing this object. When stored in the KV store this is  the key in the kvstore and when the object is served from the API-GW it is the  URI path. Examples    - "/venice/tenants/tenants/tenant2" in the kvstore    - "/v1/tenants/tenants/tenant2" when served by API Gateway. | No | string |
| label-selector | query |  | No | string |
| field-selector | query |  | No | string |
| prefix-watch | query |  | No | boolean (boolean) |
| field-change-selector | query |  | No | [ string ] |
| from | query | from represents the start offset (zero based), used for pagination. results returned would be in the range [from ... from+max-results-1]. | No | integer |
| max-results | query | max. number of events to be fetched for the request. | No | integer |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 | (streaming responses) | [clusterAutoMsgSmartNICWatchHelper](#clusterautomsgsmartnicwatchhelper) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

### /configs/cluster/v1/watch/tenants
---
##### ***GET***
**Summary:** Watch for changes to Tenant objects

**Parameters**

| Name | Located in | Description | Required | Schema |
| ---- | ---------- | ----------- | -------- | ---- |
| O.name | query | Name of the object, unique within a Namespace for scoped objects. | No | string |
| O.tenant | query | Tenant is global namespace isolation for various objects. This can be automatically filled in many cases based on the tenant a user, who created the object, belongs go. | No | string |
| O.namespace | query | Namespace of the object, for scoped objects. | No | string |
| O.resource-version | query | Resource version in the object store. This can only be set by the server. | No | string |
| O.uuid | query | UUID is the unique identifier for the object. This can only be set by the server. | No | string |
| O.creation-time | query | CreationTime is the creation time of Object. | No | dateTime |
| O.mod-time | query | ModTime is the Last Modification time of Object. | No | dateTime |
| O.self-link | query | SelfLink is a link to accessing this object. When stored in the KV store this is  the key in the kvstore and when the object is served from the API-GW it is the  URI path. Examples    - "/venice/tenants/tenants/tenant2" in the kvstore    - "/v1/tenants/tenants/tenant2" when served by API Gateway. | No | string |
| label-selector | query |  | No | string |
| field-selector | query |  | No | string |
| prefix-watch | query |  | No | boolean (boolean) |
| field-change-selector | query |  | No | [ string ] |
| from | query | from represents the start offset (zero based), used for pagination. results returned would be in the range [from ... from+max-results-1]. | No | integer |
| max-results | query | max. number of events to be fetched for the request. | No | integer |

**Responses**

| Code | Description | Schema |
| ---- | ----------- | ------ |
| 200 | (streaming responses) | [clusterAutoMsgTenantWatchHelper](#clusterautomsgtenantwatchhelper) |
| 400 | Bad request parameters | [apiStatus](#apistatus) |
| 401 | Unauthorized request | [apiStatus](#apistatus) |
| 409 | Conflict while processing request | [apiStatus](#apistatus) |
| 412 | Pre-condition failed | [apiStatus](#apistatus) |
| 500 | Internal server error | [apiStatus](#apistatus) |
| 501 | Request not implemented | [apiStatus](#apistatus) |

### Models
---

### apiListMeta  

ListMeta contains the metadata for list of objects.

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| resource-version | string | Resource version of object store at the time of list generation. | No |

### apiListWatchOptions  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| name | string |  | No |
| tenant | string |  | No |
| namespace | string |  | No |
| resource-version | string |  | No |
| uuid | string |  | No |
| labels | object |  | No |
| creation-time | dateTime |  | No |
| mod-time | dateTime |  | No |
| self-link | string |  | No |
| label-selector | string |  | No |
| field-selector | string |  | No |
| prefix-watch | boolean (boolean) |  | No |
| field-change-selector | [ string ] |  | No |
| from | integer |  | No |
| max-results | integer | max. number of events to be fetched for the request. | No |

### apiObjectMeta  

ObjectMeta contains metadata that all objects stored in kvstore must have.

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| name | string | Name of the object, unique within a Namespace for scoped objects. | No |
| tenant | string | Tenant is global namespace isolation for various objects. This can be automatically
filled in many cases based on the tenant a user, who created the object, belongs go. | No |
| namespace | string | Namespace of the object, for scoped objects. | No |
| resource-version | string | Resource version in the object store. This can only be set by the server. | No |
| uuid | string | UUID is the unique identifier for the object. This can only be set by the server. | No |
| labels | object | Labels are arbitrary (key,value) pairs associated with any object. | No |
| creation-time | dateTime |  | No |
| mod-time | dateTime |  | No |
| self-link | string | SelfLink is a link to accessing this object. When stored in the KV store this is
 the key in the kvstore and when the object is served from the API-GW it is the
 URI path. Examples
   - "/venice/tenants/tenants/tenant2" in the kvstore
   - "/v1/tenants/tenants/tenant2" when served by API Gateway. | No |

### apiObjectRef  

ObjectRef contains identifying information about an object.

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| tenant | string | Tenant of the object. | No |
| namespace | string | Namespace of the object, for scoped objects. | No |
| kind | string | Kind represents the type of the API object. | No |
| name | string | Name of the object, unique within a Namespace for scoped objects. | No |
| uri | string | URI is a link to accessing the referenced object. | No |

### apiStatus  

Status is returned for calls that dont return objects.

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| kind | string |  | No |
| api-version | string |  | No |
| result | [apiStatusResult](#apistatusresult) | Result contains the status of the operation, success or failure. | No |
| message | [ string ] | Message contains human readable form of the error. | No |
| code | integer | Code is the HTTP status code. | No |
| object-ref | [apiObjectRef](#apiobjectref) | Reference to the object (optional) for which this status is being sent. | No |

### apiStatusResult  

StatusResult contains possible statuses for a NIC.

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| Str | string |  | No |

### apiTimestamp  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| seconds | string (int64) |  | No |
| nanos | integer |  | No |

### apiTypeMeta  

TypeMeta contains the metadata about kind and version for all API objects.

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| kind | string | Kind represents the type of the API object. | No |
| api-version | string | APIVersion defines the version of the API object. | No |

### apiWatchEvent  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| type | string |  | No |
| object | [protobufAny](#protobufany) |  | No |

### apiWatchEventList  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| events | [ [apiWatchEvent](#apiwatchevent) ] |  | No |

### clusterAutoMsgClusterWatchHelper  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| Events | [ [clusterAutoMsgClusterWatchHelperWatchEvent](#clusterautomsgclusterwatchhelperwatchevent) ] |  | No |

### clusterAutoMsgClusterWatchHelperWatchEvent  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| Type | string |  | No |
| Object | [clusterCluster](#clustercluster) |  | No |

### clusterAutoMsgHostWatchHelper  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| Events | [ [clusterAutoMsgHostWatchHelperWatchEvent](#clusterautomsghostwatchhelperwatchevent) ] |  | No |

### clusterAutoMsgHostWatchHelperWatchEvent  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| Type | string |  | No |
| Object | [clusterHost](#clusterhost) |  | No |

### clusterAutoMsgNodeWatchHelper  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| Events | [ [clusterAutoMsgNodeWatchHelperWatchEvent](#clusterautomsgnodewatchhelperwatchevent) ] |  | No |

### clusterAutoMsgNodeWatchHelperWatchEvent  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| Type | string |  | No |
| Object | [clusterNode](#clusternode) |  | No |

### clusterAutoMsgSmartNICWatchHelper  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| Events | [ [clusterAutoMsgSmartNICWatchHelperWatchEvent](#clusterautomsgsmartnicwatchhelperwatchevent) ] |  | No |

### clusterAutoMsgSmartNICWatchHelperWatchEvent  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| Type | string |  | No |
| Object | [clusterSmartNIC](#clustersmartnic) |  | No |

### clusterAutoMsgTenantWatchHelper  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| Events | [ [clusterAutoMsgTenantWatchHelperWatchEvent](#clusterautomsgtenantwatchhelperwatchevent) ] |  | No |

### clusterAutoMsgTenantWatchHelperWatchEvent  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| Type | string |  | No |
| Object | [clusterTenant](#clustertenant) |  | No |

### clusterCluster  

Cluster represents a full cluster venice and workload nodes

Entity responsible & scenarios involved in managing this object:

     Create:
         o NetOps-admin
             - initial cluster creation
     Modify:
         o NetOps-admin
             - update spec attributes
         o CMD
             - update status attributes
     Delete:
         o NetOps-admin
             - TBD

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| kind | string |  | No |
| api-version | string |  | No |
| meta | [apiObjectMeta](#apiobjectmeta) |  | No |
| spec | [clusterClusterSpec](#clusterclusterspec) | Spec contains the configuration of the cluster. | No |
| status | [clusterClusterStatus](#clusterclusterstatus) | Status contains the current state of the cluster. | No |

### clusterClusterList  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| kind | string |  | No |
| api-version | string |  | No |
| resource-version | string |  | No |
| Items | [ [clusterCluster](#clustercluster) ] |  | No |

### clusterClusterSpec  

ClusterSpec contains the configuration of the cluster.

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| quorum-nodes | [ string ] | QuorumNodes contains the list of hostnames for nodes configured to be quorum
nodes in the cluster. | No |
| virtual-ip | string | VirtualIP is the IP address for managing the cluster. It will be hosted by
the winner of election between quorum nodes. | No |
| ntp-servers | [ string ] | NTPServers contains the list of NTP servers for the cluster. | No |
| dns-subdomain | string | DNSSubDomain is the DNS subdomain for the default tenant. | No |
| auto-admit-nics | boolean (boolean) | AutoAdmitNICs when enabled auto-admits NICs that are validated
into Venice Cluster. When it is disabled, NICs validated by CMD are
set to Pending state and it requires Manual approval to be admitted
into the cluster. | No |

### clusterClusterStatus  

ClusterStatus contains the current state of the Cluster.

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| leader | string | Leader contains the node name of the cluster leader. | No |
| last-leader-transition-time | dateTime |  | No |

### clusterHost  

Host represents a Baremetal or Hypervisor server.

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| kind | string |  | No |
| api-version | string |  | No |
| meta | [apiObjectMeta](#apiobjectmeta) |  | No |
| spec | [clusterHostSpec](#clusterhostspec) | Spec contains the configuration of the host. | No |
| status | [clusterHostStatus](#clusterhoststatus) | Status contains the current state of the host. | No |

### clusterHostIntfSpec  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| mac-addrs | [ string ] |  | No |

### clusterHostIntfStatus  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| ip-addrs | [ string ] |  | No |

### clusterHostList  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| kind | string |  | No |
| api-version | string |  | No |
| resource-version | string |  | No |
| Items | [ [clusterHost](#clusterhost) ] |  | No |

### clusterHostSpec  

HostSpec contains the configuration of the Host.

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| interfaces | object |  | No |

### clusterHostStatus  

HostStatus contains the current state of the Host.

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| type | string |  | No |
| operating-system | string |  | No |
| orchestrator | string | Orchestrator is the name of associated Compute controller
(like VCenter) managing this host. | No |
| interfaces | object |  | No |

### clusterNode  

Node is representation of a single Venice node in the system.

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| kind | string |  | No |
| api-version | string |  | No |
| meta | [apiObjectMeta](#apiobjectmeta) |  | No |
| spec | [clusterNodeSpec](#clusternodespec) | Spec contains the configuration of the node. | No |
| status | [clusterNodeStatus](#clusternodestatus) | Status contains the current state of the node. | No |

### clusterNodeCondition  

NodeCondition describes the state of a Node at a certain point.

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| type | string |  | No |
| status | string |  | No |
| last-transition-time | string |  | No |
| reason | string |  | No |
| message | string | A detailed message indicating details about the transition. | No |

### clusterNodeList  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| kind | string |  | No |
| api-version | string |  | No |
| resource-version | string |  | No |
| Items | [ [clusterNode](#clusternode) ] |  | No |

### clusterNodeSpec  

NodeSpec contains the configuration of the node.

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| clusterNodeSpec | object | NodeSpec contains the configuration of the node. |  |

### clusterNodeStatus  

NodeStatus contains the current state of the node.

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| phase | string | Current lifecycle phase of the node. | No |
| quorum | boolean (boolean) | Quorum node or not. | No |
| conditions | [ [clusterNodeCondition](#clusternodecondition) ] |  | No |

### clusterPortCondition  

PortCondition describes the state of a Port at a certain point.

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| type | string |  | No |
| status | string |  | No |
| last-transition-time | string |  | No |
| reason | string |  | No |
| message | string | A detailed message indicating details about the transition. | No |

### clusterPortSpec  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| mac-address | string |  | No |

### clusterPortStatus  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| mac-addrs | [ string ] |  | No |
| link-speed | string |  | No |
| conditions | [ [clusterPortCondition](#clusterportcondition) ] |  | No |

### clusterSmartNIC  

SmartNIC represents the Naples I/O subsystem

Entity responsible & scenarios involved in managing this object:

     Create:
         o CMD
             - created as part of NIC registration, Admittance
     Modify:
         o CMD
             - update spec attributes
             - update status attributes
     Delete:
         o CMD
             - aging out stale or rejected NICs (TBD)
         o NetOps, SecOps
             - Decomission a NIC (TBD)

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| kind | string |  | No |
| api-version | string |  | No |
| meta | [apiObjectMeta](#apiobjectmeta) |  | No |
| spec | [clusterSmartNICSpec](#clustersmartnicspec) | SmartNICSpec contains the configuration of the network adapter. | No |
| status | [clusterSmartNICStatus](#clustersmartnicstatus) | SmartNICStatus contains the current state of the network adapter. | No |

### clusterSmartNICCondition  

SmartNICCondition describes the state of a SmartNIC at a certain point.

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| type | string |  | No |
| status | string |  | No |
| last-transition-time | string |  | No |
| reason | string |  | No |
| message | string | A detailed message indicating details about the transition. | No |

### clusterSmartNICList  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| kind | string |  | No |
| api-version | string |  | No |
| resource-version | string |  | No |
| Items | [ [clusterSmartNIC](#clustersmartnic) ] |  | No |

### clusterSmartNICSpec  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| phase | string | Current phase of the SmartNIC.
When auto-admission is enabled, Phase will be set to NIC_ADMITTED
by CMD for validated NICs.
When auto-admission is not enabled, Phase will be set to NIC_PENDING
by CMD for validated NICs since it requires manual approval.
To admit the NIC as a part of manual admission, user is expected to
set the Phase to NIC_ADMITTED for the NICs that are in NIC_PENDING
state. Note : Whitelist mode is not supported yet. | No |
| mgmt-ip | string |  | No |
| host-name | string |  | No |
| ports | [ [clusterPortSpec](#clusterportspec) ] |  | No |

### clusterSmartNICStatus  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| conditions | [ [clusterSmartNICCondition](#clustersmartniccondition) ] |  | No |
| serial-num | string |  | No |
| primary-mac-address | string |  | No |
| ports | [ [clusterPortStatus](#clusterportstatus) ] |  | No |

### clusterTenant  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| kind | string |  | No |
| api-version | string |  | No |
| meta | [apiObjectMeta](#apiobjectmeta) |  | No |
| spec | [clusterTenantSpec](#clustertenantspec) | Spec contains the configuration of the tenant. | No |
| status | [clusterTenantStatus](#clustertenantstatus) | Status contains the current state of the tenant. | No |

### clusterTenantList  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| kind | string |  | No |
| api-version | string |  | No |
| resource-version | string |  | No |
| Items | [ [clusterTenant](#clustertenant) ] |  | No |

### clusterTenantSpec  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| admin-user | string |  | No |

### clusterTenantStatus  

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| clusterTenantStatus | object |  |  |

### protobufAny  

`Any` contains an arbitrary serialized protocol buffer message along with a
URL that describes the type of the serialized message.

Protobuf library provides support to pack/unpack Any values in the form
of utility functions or additional generated methods of the Any type.

Example 1: Pack and unpack a message in C++.

    Foo foo = ...;
    Any any;
    any.PackFrom(foo);
    ...
    if (any.UnpackTo(&foo)) {
      ...
    }

Example 2: Pack and unpack a message in Java.

    Foo foo = ...;
    Any any = Any.pack(foo);
    ...
    if (any.is(Foo.class)) {
      foo = any.unpack(Foo.class);
    }

 Example 3: Pack and unpack a message in Python.

    foo = Foo(...)
    any = Any()
    any.Pack(foo)
    ...
    if any.Is(Foo.DESCRIPTOR):
      any.Unpack(foo)
      ...

 Example 4: Pack and unpack a message in Go

     foo := &pb.Foo{...}
     any, err := ptypes.MarshalAny(foo)
     ...
     foo := &pb.Foo{}
     if err := ptypes.UnmarshalAny(any, foo); err != nil {
       ...
     }

The pack methods provided by protobuf library will by default use
'type.googleapis.com/full.type.name' as the type URL and the unpack
methods only use the fully qualified type name after the last '/'
in the type URL, for example "foo.bar.com/x/y.z" will yield type
name "y.z".


JSON
====
The JSON representation of an `Any` value uses the regular
representation of the deserialized, embedded message, with an
additional field `@type` which contains the type URL. Example:

    package google.profile;
    message Person {
      string first_name = 1;
      string last_name = 2;
    }

    {
      "@type": "type.googleapis.com/google.profile.Person",
      "firstName": <string>,
      "lastName": <string>
    }

If the embedded message type is well-known and has a custom JSON
representation, that representation will be embedded adding a field
`value` which holds the custom JSON in addition to the `@type`
field. Example (for message [google.protobuf.Duration][]):

    {
      "@type": "type.googleapis.com/google.protobuf.Duration",
      "value": "1.212s"
    }

| Name | Type | Description | Required |
| ---- | ---- | ----------- | -------- |
| type_url | string | A URL/resource name whose content describes the type of the
serialized protocol buffer message.

For URLs which use the scheme `http`, `https`, or no scheme, the
following restrictions and interpretations apply:

* If no scheme is provided, `https` is assumed.
* The last segment of the URL's path must represent the fully
  qualified name of the type (as in `path/google.protobuf.Duration`).
  The name should be in a canonical form (e.g., leading "." is
  not accepted).
* An HTTP GET on the URL must yield a [google.protobuf.Type][]
  value in binary format, or produce an error.
* Applications are allowed to cache lookup results based on the
  URL, or have them precompiled into a binary to avoid any
  lookup. Therefore, binary compatibility needs to be preserved
  on changes to types. (Use versioned type names to manage
  breaking changes.)

Schemes other than `http`, `https` (or the empty scheme) might be
used with implementation specific semantics. | No |
| value | byte | Must be a valid serialized protocol buffer of the above specified type. | No |