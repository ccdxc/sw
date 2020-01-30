import { EventsEvent_severity } from "./events";

interface EventType {
  Name: string,
  Severity: EventsEvent_severity,
  Desc: string,
}

export const categoryToEventType: { [cat: string]: string[] } = {
  : [
    'DISK_THRESHOLD_EXCEEDED',
  ],
  cluster: [
    'AUDITING_FAILED',
    'AUTO_GENERATED_TLS_CERT',
    'CLOCK_SYNC_FAILED',
    'CONFIG_RESTORED',
    'CONFIG_RESTORE_ABORTED',
    'CONFIG_RESTORE_FAILED',
    'DSC_ADMITTED',
    'DSC_DEADMITTED',
    'DSC_DECOMMISSIONED',
    'DSC_HEALTHY',
    'DSC_REJECTED',
    'DSC_UNHEALTHY',
    'DSC_UNREACHABLE',
    'ELECTION_CANCELLED',
    'ELECTION_NOTIFICATION_FAILED',
    'ELECTION_STARTED',
    'ELECTION_STOPPED',
    'HOST_DSC_SPEC_CONFLICT',
    'LEADER_CHANGED',
    'LEADER_ELECTED',
    'LEADER_LOST',
    'LOGIN_FAILED',
    'MODULE_CREATION_FAILED',
    'NODE_DISJOINED',
    'NODE_HEALTHY',
    'NODE_JOINED',
    'NODE_UNREACHABLE',
    'PASSWORD_CHANGED',
    'PASSWORD_RESET',
    'QUORUM_MEMBER_ADD',
    'QUORUM_MEMBER_HEALTHY',
    'QUORUM_MEMBER_REMOVE',
    'QUORUM_MEMBER_UNHEALTHY',
    'QUORUM_UNHEALTHY',
    'UNSUPPORTED_QUORUM_SIZE',
  ],
  config: [
    'CONFIG_FAIL',
  ],
  network: [
    'LINK_DOWN',
    'LINK_UP',
  ],
  rollout: [
    'ROLLOUT_FAILED',
    'ROLLOUT_STARTED',
    'ROLLOUT_SUCCESS',
    'ROLLOUT_SUSPENDED',
  ],
  system: [
    'NAPLES_CATTRIP_INTERRUPT',
    'NAPLES_FATAL_INTERRUPT',
    'NAPLES_OVER_TEMP',
    'NAPLES_OVER_TEMP_EXIT',
    'NAPLES_PANIC_EVENT',
    'NAPLES_POST_DIAG_FAILURE_EVENT',
    'NAPLES_SERVICE_STOPPED',
    'SERVICE_PENDING',
    'SERVICE_RUNNING',
    'SERVICE_STARTED',
    'SERVICE_STOPPED',
    'SERVICE_UNRESPONSIVE',
    'SYSTEM_COLDBOOT',
    'SYSTEM_RESOURCE_USAGE',
  ],
}

export const eventTypes: { [name: string]: EventType } = {
  'DISK_THRESHOLD_EXCEEDED' : {
      "Name": "DISK_THRESHOLD_EXCEEDED",
      "Severity": EventsEvent_severity.critical,
      "Desc": "Disk threshold exceeded",
  },
  'AUDITING_FAILED' : {
      "Name": "AUDITING_FAILED",
      "Severity": EventsEvent_severity.critical,
      "Desc": "Writing of AuditEvent failed",
  },
  'AUTO_GENERATED_TLS_CERT' : {
      "Name": "AUTO_GENERATED_TLS_CERT",
      "Severity": EventsEvent_severity.warn,
      "Desc": "Auto generated certificate is being used for API Gateway TLS",
  },
  'CLOCK_SYNC_FAILED' : {
      "Name": "CLOCK_SYNC_FAILED",
      "Severity": EventsEvent_severity.warn,
      "Desc": "Node failed to synchronize clock",
  },
  'CONFIG_RESTORED' : {
      "Name": "CONFIG_RESTORED",
      "Severity": EventsEvent_severity.info,
      "Desc": "Configuration was restored",
  },
  'CONFIG_RESTORE_ABORTED' : {
      "Name": "CONFIG_RESTORE_ABORTED",
      "Severity": EventsEvent_severity.warn,
      "Desc": "Configuration restore operation was aborted",
  },
  'CONFIG_RESTORE_FAILED' : {
      "Name": "CONFIG_RESTORE_FAILED",
      "Severity": EventsEvent_severity.critical,
      "Desc": "Configuration restore operation failed",
  },
  'DSC_ADMITTED' : {
      "Name": "DSC_ADMITTED",
      "Severity": EventsEvent_severity.info,
      "Desc": "DSC admitted to the cluster",
  },
  'DSC_DEADMITTED' : {
      "Name": "DSC_DEADMITTED",
      "Severity": EventsEvent_severity.info,
      "Desc": "DSC de-admitted from the cluster",
  },
  'DSC_DECOMMISSIONED' : {
      "Name": "DSC_DECOMMISSIONED",
      "Severity": EventsEvent_severity.info,
      "Desc": "DSC decommissioned from the cluster",
  },
  'DSC_HEALTHY' : {
      "Name": "DSC_HEALTHY",
      "Severity": EventsEvent_severity.info,
      "Desc": "DSC is healthy",
  },
  'DSC_REJECTED' : {
      "Name": "DSC_REJECTED",
      "Severity": EventsEvent_severity.warn,
      "Desc": "DSC admission request was rejected",
  },
  'DSC_UNHEALTHY' : {
      "Name": "DSC_UNHEALTHY",
      "Severity": EventsEvent_severity.critical,
      "Desc": "DSC is unhealthy",
  },
  'DSC_UNREACHABLE' : {
      "Name": "DSC_UNREACHABLE",
      "Severity": EventsEvent_severity.critical,
      "Desc": "DSC is unreachable",
  },
  'ELECTION_CANCELLED' : {
      "Name": "ELECTION_CANCELLED",
      "Severity": EventsEvent_severity.warn,
      "Desc": "Leader election cancelled",
  },
  'ELECTION_NOTIFICATION_FAILED' : {
      "Name": "ELECTION_NOTIFICATION_FAILED",
      "Severity": EventsEvent_severity.warn,
      "Desc": "Failed to send leader election notification",
  },
  'ELECTION_STARTED' : {
      "Name": "ELECTION_STARTED",
      "Severity": EventsEvent_severity.info,
      "Desc": "Leader election started in the cluster",
  },
  'ELECTION_STOPPED' : {
      "Name": "ELECTION_STOPPED",
      "Severity": EventsEvent_severity.warn,
      "Desc": "Leader election stopped",
  },
  'HOST_DSC_SPEC_CONFLICT' : {
      "Name": "HOST_DSC_SPEC_CONFLICT",
      "Severity": EventsEvent_severity.warn,
      "Desc": "The system has detected a conflict between the DSC specifications of different Host objects",
  },
  'LEADER_CHANGED' : {
      "Name": "LEADER_CHANGED",
      "Severity": EventsEvent_severity.info,
      "Desc": "Leader changed in the election",
  },
  'LEADER_ELECTED' : {
      "Name": "LEADER_ELECTED",
      "Severity": EventsEvent_severity.info,
      "Desc": "Leader elected for the cluster",
  },
  'LEADER_LOST' : {
      "Name": "LEADER_LOST",
      "Severity": EventsEvent_severity.info,
      "Desc": "Node lost leadership during the election",
  },
  'LOGIN_FAILED' : {
      "Name": "LOGIN_FAILED",
      "Severity": EventsEvent_severity.info,
      "Desc": "User login failed",
  },
  'MODULE_CREATION_FAILED' : {
      "Name": "MODULE_CREATION_FAILED",
      "Severity": EventsEvent_severity.warn,
      "Desc": "Module creation for diagnostics failed",
  },
  'NODE_DISJOINED' : {
      "Name": "NODE_DISJOINED",
      "Severity": EventsEvent_severity.warn,
      "Desc": "Node disjoined from the cluster",
  },
  'NODE_HEALTHY' : {
      "Name": "NODE_HEALTHY",
      "Severity": EventsEvent_severity.info,
      "Desc": "Node is healthy",
  },
  'NODE_JOINED' : {
      "Name": "NODE_JOINED",
      "Severity": EventsEvent_severity.info,
      "Desc": "Node joined the cluster",
  },
  'NODE_UNREACHABLE' : {
      "Name": "NODE_UNREACHABLE",
      "Severity": EventsEvent_severity.critical,
      "Desc": "Node is unreachable",
  },
  'PASSWORD_CHANGED' : {
      "Name": "PASSWORD_CHANGED",
      "Severity": EventsEvent_severity.warn,
      "Desc": "Password changed",
  },
  'PASSWORD_RESET' : {
      "Name": "PASSWORD_RESET",
      "Severity": EventsEvent_severity.warn,
      "Desc": "Password reset",
  },
  'QUORUM_MEMBER_ADD' : {
      "Name": "QUORUM_MEMBER_ADD",
      "Severity": EventsEvent_severity.info,
      "Desc": "Added member to quorum",
  },
  'QUORUM_MEMBER_HEALTHY' : {
      "Name": "QUORUM_MEMBER_HEALTHY",
      "Severity": EventsEvent_severity.info,
      "Desc": "Quorum member is now healthy",
  },
  'QUORUM_MEMBER_REMOVE' : {
      "Name": "QUORUM_MEMBER_REMOVE",
      "Severity": EventsEvent_severity.info,
      "Desc": "Removed member from quorum",
  },
  'QUORUM_MEMBER_UNHEALTHY' : {
      "Name": "QUORUM_MEMBER_UNHEALTHY",
      "Severity": EventsEvent_severity.critical,
      "Desc": "Quorum member is now unhealthy",
  },
  'QUORUM_UNHEALTHY' : {
      "Name": "QUORUM_UNHEALTHY",
      "Severity": EventsEvent_severity.critical,
      "Desc": "Quorum does not have enough healthy members",
  },
  'UNSUPPORTED_QUORUM_SIZE' : {
      "Name": "UNSUPPORTED_QUORUM_SIZE",
      "Severity": EventsEvent_severity.critical,
      "Desc": "Quorum size is below supported minimum",
  },
  'CONFIG_FAIL' : {
      "Name": "CONFIG_FAIL",
      "Severity": EventsEvent_severity.warn,
      "Desc": "Configuration failed",
  },
  'LINK_DOWN' : {
      "Name": "LINK_DOWN",
      "Severity": EventsEvent_severity.warn,
      "Desc": "Port link status is down",
  },
  'LINK_UP' : {
      "Name": "LINK_UP",
      "Severity": EventsEvent_severity.info,
      "Desc": "Port is linked up",
  },
  'ROLLOUT_FAILED' : {
      "Name": "ROLLOUT_FAILED",
      "Severity": EventsEvent_severity.critical,
      "Desc": "Rollout failed",
  },
  'ROLLOUT_STARTED' : {
      "Name": "ROLLOUT_STARTED",
      "Severity": EventsEvent_severity.info,
      "Desc": "Rollout initiated",
  },
  'ROLLOUT_SUCCESS' : {
      "Name": "ROLLOUT_SUCCESS",
      "Severity": EventsEvent_severity.info,
      "Desc": "Rollout successfully completed",
  },
  'ROLLOUT_SUSPENDED' : {
      "Name": "ROLLOUT_SUSPENDED",
      "Severity": EventsEvent_severity.info,
      "Desc": "Rollout suspended",
  },
  'NAPLES_CATTRIP_INTERRUPT' : {
      "Name": "NAPLES_CATTRIP_INTERRUPT",
      "Severity": EventsEvent_severity.critical,
      "Desc": "System encountered cattrip resetting system",
  },
  'NAPLES_FATAL_INTERRUPT' : {
      "Name": "NAPLES_FATAL_INTERRUPT",
      "Severity": EventsEvent_severity.critical,
      "Desc": "Naples has a fatal interrupt",
  },
  'NAPLES_OVER_TEMP' : {
      "Name": "NAPLES_OVER_TEMP",
      "Severity": EventsEvent_severity.critical,
      "Desc": "System temperature is above threshold.",
  },
  'NAPLES_OVER_TEMP_EXIT' : {
      "Name": "NAPLES_OVER_TEMP_EXIT",
      "Severity": EventsEvent_severity.info,
      "Desc": "System temperature is below threshold.",
  },
  'NAPLES_PANIC_EVENT' : {
      "Name": "NAPLES_PANIC_EVENT",
      "Severity": EventsEvent_severity.critical,
      "Desc": "System panic on the previous boot",
  },
  'NAPLES_POST_DIAG_FAILURE_EVENT' : {
      "Name": "NAPLES_POST_DIAG_FAILURE_EVENT",
      "Severity": EventsEvent_severity.warn,
      "Desc": "System post diag test failed",
  },
  'NAPLES_SERVICE_STOPPED' : {
      "Name": "NAPLES_SERVICE_STOPPED",
      "Severity": EventsEvent_severity.critical,
      "Desc": "Naples service stopped",
  },
  'SERVICE_PENDING' : {
      "Name": "SERVICE_PENDING",
      "Severity": EventsEvent_severity.debug,
      "Desc": "Service pending",
  },
  'SERVICE_RUNNING' : {
      "Name": "SERVICE_RUNNING",
      "Severity": EventsEvent_severity.debug,
      "Desc": "Service running",
  },
  'SERVICE_STARTED' : {
      "Name": "SERVICE_STARTED",
      "Severity": EventsEvent_severity.debug,
      "Desc": "Service started",
  },
  'SERVICE_STOPPED' : {
      "Name": "SERVICE_STOPPED",
      "Severity": EventsEvent_severity.warn,
      "Desc": "Service stopped",
  },
  'SERVICE_UNRESPONSIVE' : {
      "Name": "SERVICE_UNRESPONSIVE",
      "Severity": EventsEvent_severity.critical,
      "Desc": "Service unresponsive due to lack of system resources",
  },
  'SYSTEM_COLDBOOT' : {
      "Name": "SYSTEM_COLDBOOT",
      "Severity": EventsEvent_severity.warn,
      "Desc": "System cold booted",
  },
  'SYSTEM_RESOURCE_USAGE' : {
      "Name": "SYSTEM_RESOURCE_USAGE",
      "Severity": EventsEvent_severity.warn,
      "Desc": "System resource usage is high",
  },
};