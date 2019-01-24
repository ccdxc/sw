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
import { MonitoringTechSupportRequest } from './monitoring';
import { EventsEvent } from './events';
import { AuditEvent } from './audit';
import { NetworkNetwork } from './network';
import { NetworkService } from './network';
import { NetworkLbPolicy } from './network';
import { RolloutRollout } from './rollout';
import { SecuritySecurityGroup } from './security';
import { SecuritySGPolicy } from './security';
import { SecurityApp } from './security';
import { SecurityFirewallProfile } from './security';
import { SecurityCertificate } from './security';
import { SecurityTrafficEncryptionPolicy } from './security';
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
    "TechSupportRequest" : new MonitoringTechSupportRequest(),
    "Event" : new EventsEvent(),
    "AuditEvent" : new AuditEvent(),
  },
  "Network" : {
    "Network" : new NetworkNetwork(),
    "Service" : new NetworkService(),
    "LbPolicy" : new NetworkLbPolicy(),
  },
  "Rollout" : {
    "Rollout" : new RolloutRollout(),
  },
  "Security" : {
    "SecurityGroup" : new SecuritySecurityGroup(),
    "SGPolicy" : new SecuritySGPolicy(),
    "App" : new SecurityApp(),
    "FirewallProfile" : new SecurityFirewallProfile(),
    "Certificate" : new SecurityCertificate(),
    "TrafficEncryptionPolicy" : new SecurityTrafficEncryptionPolicy(),
  },
  "Workload" : {
    "Endpoint" : new WorkloadEndpoint(),
    "Workload" : new WorkloadWorkload(),
  },
}