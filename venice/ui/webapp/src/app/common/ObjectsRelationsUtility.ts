import { WorkloadWorkload, IWorkloadWorkload, IApiStatus, WorkloadWorkloadIntfSpec, IWorkloadAutoMsgWorkloadWatchHelper } from '@sdk/v1/models/generated/workload';
import { ClusterDistributedServiceCard, ClusterHost, IClusterDistributedServiceCardID, IClusterAutoMsgHostWatchHelper } from '@sdk/v1/models/generated/cluster';
import { Utility } from '@app/common/Utility';
import { SecuritySecurityGroup, SecurityNetworkSecurityPolicy } from '@sdk/v1/models/generated/security';
import { EventTypes } from './HttpEventUtility';


export interface SecuritygroupWorkloadPolicyTuple {
    securitygroup: SecuritySecurityGroup;
    workloads: WorkloadWorkload[];
    policies?: SecurityNetworkSecurityPolicy[];
}
export interface WorkloadDSCHostSecurityTuple {
    dscs: ClusterDistributedServiceCard[];
    workload: WorkloadWorkload;
    host: ClusterHost;
    securitygroups?: SecuritySecurityGroup[];
}

export interface DSCWorkloadHostTuple {
    dsc: ClusterDistributedServiceCard;
    workloads: WorkloadWorkload[];
    host: ClusterHost;
}

export interface HostWorkloadTuple {
    workloads: WorkloadWorkload[];
    host: ClusterHost;
}

export interface DSCWorkloadsTuple {
    workloads: WorkloadWorkload[];
    dsc: ClusterDistributedServiceCard;
}

export interface DSCnameToMacMap {
    [key: string]: string; // dsc.spec.id - dsc.meta.name
}

export interface DSCmacToNameMap {
    [key: string]: string; // dsc.meta.name- dsc.spec.id
}

export interface DSCsNameMacMap {
    nameToMacMap: DSCnameToMacMap;
    macToNameMap: DSCmacToNameMap;
}

export interface HandleWatchItemResult {
    hasChange: boolean;
    list: any[];
}

/**
 * This utility class includes helper funcitons to link objects
 *
 * A
 * nic.spec.ip-config.ip-address = nic.status.ip-config.ip-address //"172.16.0.3/24"  where spec and status match
 * nic.status.host = host.meta.name //ae-s6
 *
 * B
 * host.spec.smart-nics[i].mac-address = nic.status.primary-mac // "00ae.cd00.17d0"
 * host.spec.smart-nics[i].mac-address = host.status.admitted-smart-nics[i] //"00ae.cd00.17d0"  where spec and status match
 *
 * C.
 * app.status.attached-policies[i] = SGPolicy.meta.name // policy1  app.meta.name = iperfUdpApp // link to (D)
 *
 * D.
 * SecurityNetworkSecurityPolicy (SGPolicy)
   SGPolicy.spec.rules[i].apps[j]= app.meta.name  //iperfUdpApp   SGPolicy.meta.name = policy1
   SGPolicy.spec.rules[i].from-ip-addresses[i] // virtual IP  -  not real DSC's IP  //"10.100.0.101", "10.100.0.102"
   SGPolicy.spec.rules[i].to-ip-addresses[i]  //  virtual IP  -  not real DSC's IP  //"10.100.0.102", "10.100.0.103"  link to (E)
   SGPolicy.spec.attach-groups =  security-group // To F

   SGPolicy.status.rule-status[i] is
    (
      either
        security-group-X to security-group-Y
      or
        from-IP-range to to-IP-range
    )
    &&
    (
       protocal-port or app
    )

  SGPolicy.spec.rules[i] maps to SGPolicy.status.rule-status[i] // the status i-th "rule-hash" --> the i-rule
  rule-hash is log in firewall log
   fwlog record
   {
      "source": "50.141.173.181", //
      "destination": "34.14.21.65",
      "source-port": 2242,
      "destination-port": 3876,
      "protocol": "GRE",
      "action": "allow",
      "direction": "from-host",
      "rule-id": "4918",                // policy's rule
      "session-id": "92",
      "session-state": "flow_delete",0
      "reporter-id": "00ae.cd00.1146",  // link back to DSC  ID:naples-2, mac:00ae.cd00.1146
      "time": "2019-10-03T17:49:06.353077274Z"
    },

 *  E
 *  Workload (Workloads are like docker processes running in one VM (physical box). One Workload has m interfaces/DSCs  )
 *  Workload.spec.host-name = host.meta.name   // link to host (B), host links to physical DSC (A)
 *  Workload.spec.interfaces[i] are Virtual NIC.
 *  Workload.spec.interfaces[i].mac-address !=   NIC.meta.name and NIC.status.primary-mac // "00ae.cd00.50a0"
 *  Workload.spec.interfaces[i].ip-addresses[i] = policy.spec.rules[i].to-ip-addresses or from-ip-addresses // "10.100.0.103"
 *
 *  F. SecurityGroup (2019-11-06 understanding)
 *    SecurityGroup.spec.WorkloadSelector[i] = Workload.meta.labels?
 *    SecurityGroup.spec.MatchPrefixes[i] = other security groups (name)
 *    SecurityGroup.status.Workloads[i] = workloads names  // To E
 *    SecurityGroup.status.Policies = Policies names  // To D
 */
export class ObjectsRelationsUtility {

    public static buildDSCsNameMacMap(naples: ReadonlyArray<ClusterDistributedServiceCard> | ClusterDistributedServiceCard[]): DSCsNameMacMap {
        const _myDSCnameToMacMap: DSCsNameMacMap = {
            nameToMacMap: {},
            macToNameMap: {}
        };

        for (const smartnic of naples) {
            if (smartnic.spec.id != null && smartnic.spec.id !== '') {
                _myDSCnameToMacMap.nameToMacMap[smartnic.spec.id] = smartnic.meta.name;
                _myDSCnameToMacMap.macToNameMap[smartnic.meta.name] = smartnic.spec.id;
            }
        }
        return _myDSCnameToMacMap;
    }
    /**
     * Build a map key=workload.meta.name, value = 'WorkloadDSCHostTuple' object
     *
     * workload -- 1:1 --> host
     * host -- 1:m --> DSCs
     * workload and security-groups are linked
     */
    public static buildWorkloadDscHostSecuritygroupMap(workloads: ReadonlyArray<WorkloadWorkload> | WorkloadWorkload[],
        naples: ReadonlyArray<ClusterDistributedServiceCard> | ClusterDistributedServiceCard[],
        hosts: ReadonlyArray<ClusterHost> | ClusterHost[],
        securitygroups: ReadonlyArray<SecuritySecurityGroup> | SecuritySecurityGroup[],
    ): { [workloadkey: string]: WorkloadDSCHostSecurityTuple; } {
        const workloadDSCHostTupleMap: { [workloadkey: string]: WorkloadDSCHostSecurityTuple; } = {};
        for (const workload of workloads) {
            const workloadSpecHostName = workload.spec['host-name'];
            const host: ClusterHost = this.getHostByMetaName(hosts, workloadSpecHostName);
            let nics: ClusterDistributedServiceCard[] = [];
            if (host) {
                nics = this.getDSCsByHost(naples, host);
            }
            let mysecurtiygrous: SecuritySecurityGroup[] = [];
            if (securitygroups) {
                mysecurtiygrous = this.getSecurityGroupsByWorkload(securitygroups, workload);
            }

            const newTuple: WorkloadDSCHostSecurityTuple = {
                workload: workload,
                host: host,
                dscs: nics,
                securitygroups: mysecurtiygrous
            };
            workloadDSCHostTupleMap[workload.meta.name] = newTuple;
        }
        return workloadDSCHostTupleMap;
    }

    /**
     * Build a map key=dsc.meta.name, value = 'DSCWorkloadHostTuple' object
     *
     * dsc -- 1:1 --> host
     * host -- 1:m --> workload
     *
     * e.g DSC page can use this api to find workloads
     * const dscWorkloadHostMap  = ObjectsRelationsUtility.buildDscWorkloadHostMap(this.dataObjects, this.naples, this.hostObjects );

     */
    public static buildDscWorkloadHostMap(workloads: ReadonlyArray<WorkloadWorkload> | WorkloadWorkload[],
        naples: ReadonlyArray<ClusterDistributedServiceCard> | ClusterDistributedServiceCard[],
        hosts: ReadonlyArray<ClusterHost> | ClusterHost[],
    ): { [dsckey: string]: DSCWorkloadHostTuple; } {
        const dscWorkloadHostMap: { [dsckey: string]: DSCWorkloadHostTuple; } = {};
        for (const dsc of naples) {
            const hostNameFromDsc = dsc.status.host;
            const host: ClusterHost = this.getHostByMetaName(hosts, hostNameFromDsc);
            const linkworkloads = this.findAllWorkloadsInHosts(workloads, hosts);
            const newTuple: DSCWorkloadHostTuple = {
                workloads: linkworkloads,
                host: host,
                dsc: dsc
            };
            dscWorkloadHostMap[dsc.meta.name] = newTuple;
        }
        return dscWorkloadHostMap;
    }

    /**
     * Build a map key=host.meta.name, value = 'HostWorkloadTuple' object
     *
     * dsc -- 1:1 --> host
     * host -- 1:m --> workloads
     *
     * e.g Host page can use this api to find workloads
     * const hostWorkloadsTuple  = ObjectsRelationsUtility.buildHostWorkloadsMap(this.hosts this.workloads );
     *
     */
    public static buildHostWorkloadsMap(workloads: ReadonlyArray<WorkloadWorkload> | WorkloadWorkload[],
        hosts: ReadonlyArray<ClusterHost> | ClusterHost[],
    ): { [hostKey: string]: HostWorkloadTuple; } {
        const hostWorkloadsTuple: { [hostKey: string]: HostWorkloadTuple; } = {};
        for (const host of hosts) {
            const linkworkloads = this.findAssociatedWorkloadsByHost(workloads, host);
            const newTuple: HostWorkloadTuple = {
                workloads: linkworkloads,
                host: host
            };
            hostWorkloadsTuple[host.meta.name] = newTuple;
        }
        return hostWorkloadsTuple;
    }

    /**
     * Build a map key=dsc.meta.name, value = 'DSCWorkloadsTuple' object
     *
     * dsc -- 1:1 --> host
     * host -- 1:m --> workloads
     * @param workloads
     * @param naples
     */
    public static buildDscWorkloadsMaps(workloads: ReadonlyArray<WorkloadWorkload> | WorkloadWorkload[], naples: ReadonlyArray<ClusterDistributedServiceCard> | ClusterDistributedServiceCard[])
        : { [dscKey: string]: DSCWorkloadsTuple; } {
        const dscWorkloadsTuple: { [dscKey: string]: DSCWorkloadsTuple; } = {};
        for (const naple of naples) {
            const linkworkloads = this.findAssociatedWorkloadsByDSC(workloads, naple);
            const newTuple: DSCWorkloadsTuple = {
                workloads: linkworkloads,
                dsc: naple
            };
            dscWorkloadsTuple[naple.meta.name] = newTuple;
        }
        return dscWorkloadsTuple;
    }

    public static buildSecuitygroupWorkloadPolicyMap(securityGroups: ReadonlyArray<SecuritySecurityGroup> | SecuritySecurityGroup[], workloads: ReadonlyArray<WorkloadWorkload> | WorkloadWorkload[], securitypolicies?: ReadonlyArray<SecurityNetworkSecurityPolicy> | SecurityNetworkSecurityPolicy[])
        : { [securitygroupKey: string]: SecuritygroupWorkloadPolicyTuple; } {
        const securitygroupWorkloadPolicyTuple: { [securitygroupKey: string]: SecuritygroupWorkloadPolicyTuple; } = {};
        for (const securitygroup of securityGroups) {
            const inclduedworkloads = this.findAssociatedWorkloadsFromSecuritygroup(workloads, securitygroup);
            const policies = this.findIncludedSecuritypolicesFromSecuritygroup(securitypolicies, securitygroup);
            const newTuple: SecuritygroupWorkloadPolicyTuple = {
                workloads: inclduedworkloads,
                securitygroup: securitygroup,
                policies: policies
            };
            securitygroupWorkloadPolicyTuple[securitygroup.meta.name] = newTuple;
        }
        return securitygroupWorkloadPolicyTuple;
    }


    /**
     * Given a DSC/Naple, find all associated workloads
     *  dsc -- 1:1 --> host  -- 1..m -> workloads
     * @param workloads
     * @param naple
     */
    public static findAssociatedWorkloadsByDSC(workloads: ReadonlyArray<WorkloadWorkload> | WorkloadWorkload[], naple: ClusterDistributedServiceCard): WorkloadWorkload[] {
        const workloadWorkloads: WorkloadWorkload[] = [];
        for (const workload of workloads) {
            const hostname = naple.status.host;
            if (workload.spec['host-name'] === hostname) {
                workloadWorkloads.push(workload);
            }
        }
        return workloadWorkloads;
    }

    /**
     * Give a ClusterHost, find all the workloads that associated with this host
     * @param workloads
     * @param host
     */
    public static findAssociatedWorkloadsByHost(workloads: ReadonlyArray<WorkloadWorkload> | WorkloadWorkload[], host: ClusterHost): WorkloadWorkload[] {
        const workloadWorkloads: WorkloadWorkload[] = [];
        for (const workload of workloads) {
            const workloadSpecHostName = workload.spec['host-name'];
            if (host.meta.name === workloadSpecHostName) {
                workloadWorkloads.push(workload);
            }
        }
        return workloadWorkloads;
    }

    public static findAllWorkloadsInHosts(workloads: ReadonlyArray<WorkloadWorkload> | WorkloadWorkload[], hosts: ReadonlyArray<ClusterHost> | ClusterHost[]): WorkloadWorkload[] {
        const workloadWorkloads: WorkloadWorkload[] = [];
        for (const workload of workloads) {
            const workloadSpecHostName = workload.spec['host-name'];
            const host: ClusterHost = this.getHostByMetaName(hosts, workloadSpecHostName);
            if (host) {
                workloadWorkloads.push(workload);
            }
        }
        return workloadWorkloads;
    }



    public static getDSCsByHost(naples: ReadonlyArray<ClusterDistributedServiceCard> | ClusterDistributedServiceCard[], host: ClusterHost): ClusterDistributedServiceCard[] {
        if (!host) {
            return [];
        }
        const hostSpecDSClen = host.spec.dscs.length;
        const nics: ClusterDistributedServiceCard[] = [];
        for (let i = 0; i < hostSpecDSClen; i++) {
            const hostSpecDsc: IClusterDistributedServiceCardID = host.spec.dscs[i];
            if (hostSpecDsc['mac-address'] != null) {
                const macAddress = hostSpecDsc['mac-address'];
                const dsc = this.getDSCByMACaddress(naples, macAddress);
                if (dsc) {
                    nics.push(dsc);
                }
            } else if (hostSpecDsc.id != null) {
                const hostId = hostSpecDsc.id;
                const dsc = this.getDSCById(naples, hostId);
                if (dsc) {
                    nics.push(dsc);
                }
            }
        }
        return nics;
    }

    public static getHostByMetaName(hosts: ReadonlyArray<ClusterHost> | ClusterHost[], hostname: string): ClusterHost {
        const interfacesLength = hosts.length;
        for (let i = 0; i < interfacesLength; i++) {
            const host: ClusterHost = hosts[i];
            if (host.meta.name === hostname) {
                return host;
            }
        }
        return null;
    }

    public static getDSCById(naples: ReadonlyArray<ClusterDistributedServiceCard> | ClusterDistributedServiceCard[], hostId: string): ClusterDistributedServiceCard {
        const interfacesLength = naples.length;
        for (let i = 0; i < interfacesLength; i++) {
            const nicSpecId = naples[i].spec.id;
            if (nicSpecId === hostId) {
                return naples[i];
            }
        }
        return null;
    }

    public static getDSCByMACaddress(naples: ReadonlyArray<ClusterDistributedServiceCard> | ClusterDistributedServiceCard[], macAddress: string): ClusterDistributedServiceCard {
        const interfacesLength = naples.length;
        for (let i = 0; i < interfacesLength; i++) {
            const napleMac = naples[i].status['primary-mac'];
            if (napleMac === macAddress) {
                return naples[i];
            }
        }
        return null;
    }

    /**
     * As a user, I want find trace from ip-1 to ip-2
     *  1. ip1's workload,  workload to DSC
     *     ip2's workload,  workload to DSC
     *  2.
     */

    /**
     * Given an ipAddress, find all the workloads
     * @param workloads
     * @param ipAddress
     */
    public static getWorkloadFromIPAddress(workloads: ReadonlyArray<WorkloadWorkload> | WorkloadWorkload[], ipAddress: string): WorkloadWorkload[] {
        const matchingWorkloads: WorkloadWorkload[] = [];
        for (const workload of workloads) {
            const interfaces: WorkloadWorkloadIntfSpec[] = workload.spec.interfaces;
            for (const wlInterface of interfaces) {
                const matchedIps = wlInterface['ip-addresses'].find((ip) => {
                    return (ip === ipAddress);
                });
                if (matchedIps && matchedIps.length > 0) {
                    matchingWorkloads.push(workload);
                }
            }
        }
        return matchingWorkloads;
    }

    /**
     * Give a workload, this API finds all connected workloads
     * @param workloads
     * @param givenWorkload
     */
    public static findPossibleConnectedWorkloadsFromWorkload(workloads: ReadonlyArray<WorkloadWorkload> | WorkloadWorkload[], givenWorkload: WorkloadWorkload): WorkloadWorkload[] {
        const workloadWorkloads: WorkloadWorkload[] = [];
        const interfaces: WorkloadWorkloadIntfSpec[] = givenWorkload.spec.interfaces;
        for (const wlInterface of interfaces) {
            const ips = wlInterface['ip-addresses'];
            for (const ip of ips) {
                const wls: WorkloadWorkload[] = this.getWorkloadFromIPAddress(workloads, ip);
                workloadWorkloads.concat(...wls);
            }
        }
        return workloadWorkloads;
    }

    /**
     * Given securitygroup, find associated workloads
     * Logic:  all securitygroup.spec['workload-selector'].requirements[i] .key = workload.meta.labels.key
     *                                                                     .value = workload.meta.labels.value
     * "workload-selector": {
            "requirements": [
              {
                "key": "env",
                "operator": "in",
                "values": [
                  "21"
                ]
              },
              {
                "key": "tag",
                "operator": "in",
                "values": [
                  "1"
                ]
              }
            ]
          }
     * worloads
       {
        "events": [
            {
            "type": "Created",
            "object": {
                "kind": "Workload",
                "api-version": "v1",
                "meta": {
                    "name": "wl1-a",
                    "labels": {
                        "env": "21",  // match
                        "tag": "1"
                    }
                }
            }
            },
            {
            "type": "Created",
            "object": {
                "kind": "Workload",
                "api-version": "v1",
                "meta": {
                    "name": "wl1",
                    "labels": {   // match
                        "env": "21",
                        "tag": "1"
                    }
                }
            }
            }
        ]
        }
     * @param workloads
     * @param securityGroup
     */
    public static findAssociatedWorkloadsFromSecuritygroup(workloads: ReadonlyArray<WorkloadWorkload> | WorkloadWorkload[], securityGroup: SecuritySecurityGroup): WorkloadWorkload[] {
        const workloadWorkloads: WorkloadWorkload[] = [];
        for (const workload of workloads) {
            /* let allRequirementMatch: boolean = true;
            for (const sgroupWorkloadSelectorRequirement of securityGroup.spec['workload-selector'].requirements) {
                const key = sgroupWorkloadSelectorRequirement.key;
                const values = sgroupWorkloadSelectorRequirement.values;
                const labelValue = workload.meta.labels[key];
                if (labelValue) {
                    if (!values.includes(labelValue)) {
                        allRequirementMatch = false;
                        break;
                    }
                } else {
                    allRequirementMatch = false;
                    break;
                }
            } */
            const allRequirementMatch = this.isSecurityGroupWorkloadLabelMatchWorkloadLabels(securityGroup, workload);
            if (allRequirementMatch) {
                workloadWorkloads.push(workload);
            }
        }
        return workloadWorkloads;
    }

    public static isSecurityGroupWorkloadLabelMatchWorkloadLabels(securityGroup: SecuritySecurityGroup, workload: WorkloadWorkload): boolean {
        let allRequirementMatch: boolean = true;
        for (const sgroupWorkloadSelectorRequirement of securityGroup.spec['workload-selector'].requirements) {
            const key = sgroupWorkloadSelectorRequirement.key;
            const values = sgroupWorkloadSelectorRequirement.values;
            const operator = sgroupWorkloadSelectorRequirement.operator;
            const labelValue = workload.meta.labels[key];
            if (labelValue) {
                if (!values.includes(labelValue)) { // TODO: add more check here. Here, it only handles operator is "equal"
                    allRequirementMatch = false;
                    break;
                }
            } else {
                allRequirementMatch = false;
                break;
            }
        }
        return allRequirementMatch;
    }

    /**
     * Give a securityGroup, find all assoicated security-policies
     * Logic:  securitypolicy.spec['attach-groups'][i] === securityGroup.meta.name
     *
     * @param securitypolicies
     * @param securitygroup
     */
    public static findIncludedSecuritypolicesFromSecuritygroup(securitypolicies: ReadonlyArray<SecurityNetworkSecurityPolicy> | SecurityNetworkSecurityPolicy[], securitygroup: SecuritySecurityGroup): SecurityNetworkSecurityPolicy[] {
        const securityNetworkSecurityPolicies: SecurityNetworkSecurityPolicy[] = [];
        for (const securitypolicy of securitypolicies) {
            const attachedGroups = securitypolicy.spec['attach-groups'];
            for (const attachedGroup of attachedGroups) {
                if (attachedGroup === securitygroup.meta.name) {
                    securityNetworkSecurityPolicies.push(securitypolicy);
                }
            }
        }
        return securityNetworkSecurityPolicies;
    }

    static getSecurityGroupsByWorkload(securitygroups: ReadonlyArray<SecuritySecurityGroup> | SecuritySecurityGroup[], workload: WorkloadWorkload): SecuritySecurityGroup[] {
        const linkedSecurityGroups: SecuritySecurityGroup[] = [];
        for (const securityGroup of securitygroups) {
            const allRequirementMatch = this.isSecurityGroupWorkloadLabelMatchWorkloadLabels(securityGroup, workload);
            if (allRequirementMatch) {
                linkedSecurityGroups.push(securityGroup);
            }
        }
        return linkedSecurityGroups;
    }


    /**
    *
    * @param veniceObjectWatchHelper
    * @param type
    */
    public static findTypedItemsFromWSResponse(veniceObjectWatchHelper: IWorkloadAutoMsgWorkloadWatchHelper | IClusterAutoMsgHostWatchHelper, type: EventTypes) {
        const typedItemList = [];
        if (!veniceObjectWatchHelper) {
            return [];
        }
        const events = veniceObjectWatchHelper.events;
        for (let i = 0; events && i < events.length; i++) {
            if (events[i].type === type && events[i].object) {
                typedItemList.push(events[i].object);
            }
        }
        return typedItemList;
    }

    public static handleAddedItemsFromWatch(addedItems: any[], dataList: any[], genFunc: (item: any) => any): HandleWatchItemResult {
        let needUpdate: boolean = false;
        for (let i = 0; i < addedItems.length; i++) {
            const index = dataList.findIndex((w) => w.meta.name === addedItems[i].meta.name);
            if (index < 0) {
                const workload = genFunc(addedItems[i]);  // genFunc is like: new WorkloadWorkload(addedItems[i]);
                dataList = dataList.concat(workload); // insert into workloadList
                needUpdate = true;
            }
        }
        const handleWatchItemResult: HandleWatchItemResult = {
            hasChange: needUpdate,
            list: dataList
        };
        return handleWatchItemResult;
    }

    public static handleUpdatedItemsFromWatch(updatedItems: any[], dataList: any[], genFunc: (item: any) => any): HandleWatchItemResult {
        let needUpdate: boolean = false;
        for (let i = 0; i < updatedItems.length; i++) {
            const index = dataList.findIndex((w) => w.meta.name === updatedItems[i].meta.name);
            if (index >= 0) {
                const workload = genFunc(updatedItems[i]);
                dataList[index] = workload;  // update to new value
                needUpdate = true;
            }
        }
        const handleWatchItemResult: HandleWatchItemResult = {
            hasChange: needUpdate,
            list: dataList
        };
        return handleWatchItemResult;
    }

    public static handleDeletedItemsFromWatch(deletedIems: any[], dataList: any[]): HandleWatchItemResult {
        let needUpdate: boolean = false;
        for (let i = 0; i < deletedIems.length; i++) {
            const index = dataList.findIndex((w) => w.meta.name === deletedIems[i].meta.name);
            if (index >= 0) {
                dataList.splice(index, 1);
                needUpdate = true;
            }
        }
        const handleWatchItemResult: HandleWatchItemResult = {
            hasChange: needUpdate,
            list: dataList
        };
        return handleWatchItemResult;
    }
}
