import { AuthUser } from './auth';
import { AuthAuthenticationPolicy } from './auth';
import { AuthRole } from './auth';
import { AuthRoleBinding } from './auth';
import { ClusterCluster } from './cluster';
import { ClusterNode } from './cluster';
import { ClusterHost } from './cluster';
import { ClusterSmartNIC } from './cluster';
import { ClusterTenant } from './cluster';
import { MonitoringEventPolicy } from './monitoring';
import { MonitoringStatsPolicy } from './monitoring';
import { MonitoringFwlogPolicy } from './monitoring';
import { MonitoringFlowExportPolicy } from './monitoring';
import { MonitoringAlert } from './monitoring';
import { MonitoringAlertPolicy } from './monitoring';
import { MonitoringAlertDestination } from './monitoring';
import { MonitoringMirrorSession } from './monitoring';
import { MonitoringTroubleshootingSession } from './monitoring';
import { EventsEvent } from './events';
import { NetworkNetwork } from './network';
import { NetworkService } from './network';
import { NetworkLbPolicy } from './network';
import { SecuritySecurityGroup } from './security';
import { SecuritySGPolicy } from './security';
import { SecurityApp } from './security';
import { SecurityCertificate } from './security';
import { SecurityTrafficEncryptionPolicy } from './security';
import { StagingBuffer } from './staging';
import { WorkloadEndpoint } from './workload';
import { WorkloadWorkload } from './workload';

export const CategoryMapping  = {
  "Auth" : {
    "User" : new AuthUser(),
    "AuthenticationPolicy" : new AuthAuthenticationPolicy(),
    "Role" : new AuthRole(),
    "RoleBinding" : new AuthRoleBinding(),
  },
  "Cluster" : {
    "Cluster" : new ClusterCluster(),
    "Node" : new ClusterNode(),
    "Host" : new ClusterHost(),
    "SmartNIC" : new ClusterSmartNIC(),
    "Tenant" : new ClusterTenant(),
  },
  "Monitoring" : {
    "EventPolicy" : new MonitoringEventPolicy(),
    "StatsPolicy" : new MonitoringStatsPolicy(),
    "FwlogPolicy" : new MonitoringFwlogPolicy(),
    "FlowExportPolicy" : new MonitoringFlowExportPolicy(),
    "Alert" : new MonitoringAlert(),
    "AlertPolicy" : new MonitoringAlertPolicy(),
    "AlertDestination" : new MonitoringAlertDestination(),
    "MirrorSession" : new MonitoringMirrorSession(),
    "TroubleshootingSession" : new MonitoringTroubleshootingSession(),
    "Events" : new EventsEvent(),
  },
  "Network" : {
    "Network" : new NetworkNetwork(),
    "Service" : new NetworkService(),
    "LbPolicy" : new NetworkLbPolicy(),
  },
  "Security" : {
    "SecurityGroup" : new SecuritySecurityGroup(),
    "SGPolicy" : new SecuritySGPolicy(),
    "App" : new SecurityApp(),
    "Certificate" : new SecurityCertificate(),
    "TrafficEncryptionPolicy" : new SecurityTrafficEncryptionPolicy(),
  },
  "Staging" : {
    "Buffer" : new StagingBuffer(),
  },
  "Workload" : {
    "Endpoint" : new WorkloadEndpoint(),
    "Workload" : new WorkloadWorkload(),
  },
}