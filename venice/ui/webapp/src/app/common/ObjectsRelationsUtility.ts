import { WorkloadWorkload, IWorkloadWorkload, IApiStatus } from '@sdk/v1/models/generated/workload';
import { ClusterDistributedServiceCard, ClusterHost, IClusterDistributedServiceCardID } from '@sdk/v1/models/generated/cluster';
import { Utility } from '@app/common/Utility';

export interface WorkloadDSCHostTuple {
    dscs: ClusterDistributedServiceCard[];
    workload: WorkloadWorkload;
    host: ClusterHost;
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
      "session-state": "flow_delete",
      "reporter-id": "00ae.cd00.1146",  // link back to NIC  ID:naples-2, mac:00ae.cd00.1146
      "time": "2019-10-03T17:49:06.353077274Z"
    },

 *  E
 *  Workload (Workloads are like docker processes running in one VM (physical box). One Workload has m interfaces/DSCs  )
 *  Workload.spec.host-name = host.meta.name   // link to host (B), host links to physical DSC (A)
 *  Workload.spec.interfaces[i] are Virtual NIC.
 *  Workload.spec.interfaces[i].mac-address !=   NIC.meta.name and NIC.status.primary-mac // "00ae.cd00.50a0"
 *  Workload.spec.interfaces[i].ip-addresses[i] = policy.spec.rules[i].to-ip-addresses or from-ip-addresses // "10.100.0.103"
 *
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
     */
    public static buildWorkloadDscHostMap(workloads: ReadonlyArray<WorkloadWorkload> | WorkloadWorkload[],
        naples: ReadonlyArray<ClusterDistributedServiceCard> | ClusterDistributedServiceCard[],
        hosts: ReadonlyArray<ClusterHost> | ClusterHost[],
    ): { [workloadkey: string]: WorkloadDSCHostTuple; } {
        const workloadDSCHostTupleMap: { [workloadkey: string]: WorkloadDSCHostTuple; } = {};
        for (const workload of workloads) {
            const workloadSpecHostName = workload.spec['host-name'];
            const host: ClusterHost = this.getHostByMetaName(hosts, workloadSpecHostName);
            let nics: ClusterDistributedServiceCard[] = [];
            if (host) {
                nics = this.getDSCsByHost(naples, host);
            }

            const newTuple: WorkloadDSCHostTuple = {
                workload: workload,
                host: host,
                dscs: nics
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
    public static findAssociatedWorkloadsByHost(workloads: ReadonlyArray<WorkloadWorkload> | WorkloadWorkload[], host: ClusterHost  ): WorkloadWorkload[] {
        const workloadWorkloads: WorkloadWorkload[] = [];
        for (const workload of workloads) {
            const workloadSpecHostName = workload.spec['host-name'];
            if (host.meta.name === workloadSpecHostName) {
                workloadWorkloads.push(workload);
            }
        }
        return workloadWorkloads;
    }

    public static findAllWorkloadsInHosts (workloads: ReadonlyArray<WorkloadWorkload> | WorkloadWorkload[], hosts: ReadonlyArray<ClusterHost> | ClusterHost[] ): WorkloadWorkload[] {
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
                const hostId = hostSpecDsc.id ;
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
}
