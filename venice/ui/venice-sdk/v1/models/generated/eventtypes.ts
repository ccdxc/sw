import { EventsEvent_severity } from "./events";

interface EventType {
  Name: string,
  Severity: EventsEvent_severity,
  Desc: string,
}

export const categoryToEventType: { [cat: string]: string[] } = {
  cluster: [
    'AUDITING_FAILED',
    'AUTO_GENERATED_TLS_CERT',
    'ELECTION_CANCELLED',
    'ELECTION_NOTIFICATION_FAILED',
    'ELECTION_STARTED',
    'ELECTION_STOPPED',
    'HOST_SMART_NIC_SPEC_CONFLICT',
    'LEADER_CHANGED',
    'LEADER_ELECTED',
    'LEADER_LOST',
    'LOGIN_FAILED',
    'MODULE_CREATION_FAILED',
    'NIC_ADMITTED',
    'NIC_HEALTHY',
    'NIC_REJECTED',
    'NIC_UNHEALTHY',
    'NIC_UNREACHABLE',
    'NODE_DISJOINED',
    'NODE_HEALTHY',
    'NODE_JOINED',
    'NODE_UNREACHABLE',
    'QUORUM_MEMBER_ADD',
    'QUORUM_MEMBER_HEALTHY',
    'QUORUM_MEMBER_REMOVE',
    'QUORUM_MEMBER_UNHEALTHY',
    'QUORUM_UNHEALTHY',
    'UNSUPPORTED_QUORUM_SIZE',
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
    'NAPLES_SERVICE_STOPPED',
    'SERVICE_PENDING',
    'SERVICE_RUNNING',
    'SERVICE_STARTED',
    'SERVICE_STOPPED',
    'SERVICE_UNRESPONSIVE',
    'SYSTEM_COLDBOOT',
  ],
}

export const eventTypes: { [name: string]: EventType } = {
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
  'HOST_SMART_NIC_SPEC_CONFLICT' : {
      "Name": "HOST_SMART_NIC_SPEC_CONFLICT",
      "Severity": EventsEvent_severity.warn,
      "Desc": "The system has detected a conflict between the SmartNIC specifications of different Host objects",
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
  'NIC_ADMITTED' : {
      "Name": "NIC_ADMITTED",
      "Severity": EventsEvent_severity.info,
      "Desc": "NIC admitted to the cluster",
  },
  'NIC_HEALTHY' : {
      "Name": "NIC_HEALTHY",
      "Severity": EventsEvent_severity.info,
      "Desc": "NIC health transitioned to &#x27;true&#x27;",
  },
  'NIC_REJECTED' : {
      "Name": "NIC_REJECTED",
      "Severity": EventsEvent_severity.warn,
      "Desc": "A SmartNIC admission request was rejected",
  },
  'NIC_UNHEALTHY' : {
      "Name": "NIC_UNHEALTHY",
      "Severity": EventsEvent_severity.warn,
      "Desc": "NIC health transitioned to &#x27;false&#x27;",
  },
  'NIC_UNREACHABLE' : {
      "Name": "NIC_UNREACHABLE",
      "Severity": EventsEvent_severity.critical,
      "Desc": "NIC is unreachable",
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
};