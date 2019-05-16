import { EventsEvent_severity } from "./events";

interface EventType {
  Name: string,
  Severity: EventsEvent_severity,
  Desc: string,
}

export const categoryToEventType: { [cat: string]: string[] } = {
  Cluster: [
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
    'NIC_HEALTH_UNKNOWN',
    'NIC_REJECTED',
    'NIC_UNHEALTHY',
    'NODE_DISJOINED',
    'NODE_JOINED',
  ],
  Network: [
    'LINK_DOWN',
    'LINK_UP',
  ],
  System: [
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
      "Severity": EventsEvent_severity.CRITICAL,
      "Desc": "Writing of AuditEvent failed",
  },
  'AUTO_GENERATED_TLS_CERT' : {
      "Name": "AUTO_GENERATED_TLS_CERT",
      "Severity": EventsEvent_severity.WARN,
      "Desc": "Auto generated certificate is being used for API Gateway TLS",
  },
  'ELECTION_CANCELLED' : {
      "Name": "ELECTION_CANCELLED",
      "Severity": EventsEvent_severity.WARN,
      "Desc": "Leader election cancelled",
  },
  'ELECTION_NOTIFICATION_FAILED' : {
      "Name": "ELECTION_NOTIFICATION_FAILED",
      "Severity": EventsEvent_severity.WARN,
      "Desc": "Failed to send leader election notification",
  },
  'ELECTION_STARTED' : {
      "Name": "ELECTION_STARTED",
      "Severity": EventsEvent_severity.INFO,
      "Desc": "Leader election started in the cluster",
  },
  'ELECTION_STOPPED' : {
      "Name": "ELECTION_STOPPED",
      "Severity": EventsEvent_severity.WARN,
      "Desc": "Leader election stopped",
  },
  'HOST_SMART_NIC_SPEC_CONFLICT' : {
      "Name": "HOST_SMART_NIC_SPEC_CONFLICT",
      "Severity": EventsEvent_severity.WARN,
      "Desc": "The system has detected a conflict between the SmartNIC specifications of different Host objects",
  },
  'LEADER_CHANGED' : {
      "Name": "LEADER_CHANGED",
      "Severity": EventsEvent_severity.INFO,
      "Desc": "Leader changed in the election",
  },
  'LEADER_ELECTED' : {
      "Name": "LEADER_ELECTED",
      "Severity": EventsEvent_severity.INFO,
      "Desc": "Leader elected for the cluster",
  },
  'LEADER_LOST' : {
      "Name": "LEADER_LOST",
      "Severity": EventsEvent_severity.INFO,
      "Desc": "Node lost leadership during the election",
  },
  'LOGIN_FAILED' : {
      "Name": "LOGIN_FAILED",
      "Severity": EventsEvent_severity.INFO,
      "Desc": "User login failed",
  },
  'MODULE_CREATION_FAILED' : {
      "Name": "MODULE_CREATION_FAILED",
      "Severity": EventsEvent_severity.WARN,
      "Desc": "Module creation for diagnostics failed",
  },
  'NIC_ADMITTED' : {
      "Name": "NIC_ADMITTED",
      "Severity": EventsEvent_severity.INFO,
      "Desc": "NIC admitted to the cluster",
  },
  'NIC_HEALTHY' : {
      "Name": "NIC_HEALTHY",
      "Severity": EventsEvent_severity.INFO,
      "Desc": "NIC health transitioned to &#x27;true&#x27;",
  },
  'NIC_HEALTH_UNKNOWN' : {
      "Name": "NIC_HEALTH_UNKNOWN",
      "Severity": EventsEvent_severity.WARN,
      "Desc": "NIC health transitioned to &#x27;unknown&#x27;",
  },
  'NIC_REJECTED' : {
      "Name": "NIC_REJECTED",
      "Severity": EventsEvent_severity.WARN,
      "Desc": "A SmartNIC admission request was rejected",
  },
  'NIC_UNHEALTHY' : {
      "Name": "NIC_UNHEALTHY",
      "Severity": EventsEvent_severity.WARN,
      "Desc": "NIC health transitioned to &#x27;false&#x27;",
  },
  'NODE_DISJOINED' : {
      "Name": "NODE_DISJOINED",
      "Severity": EventsEvent_severity.WARN,
      "Desc": "Node disjoined from the cluster",
  },
  'NODE_JOINED' : {
      "Name": "NODE_JOINED",
      "Severity": EventsEvent_severity.INFO,
      "Desc": "Node joined the cluster",
  },
  'LINK_DOWN' : {
      "Name": "LINK_DOWN",
      "Severity": EventsEvent_severity.WARN,
      "Desc": "Port link status is down",
  },
  'LINK_UP' : {
      "Name": "LINK_UP",
      "Severity": EventsEvent_severity.INFO,
      "Desc": "Port is linked up",
  },
  'NAPLES_SERVICE_STOPPED' : {
      "Name": "NAPLES_SERVICE_STOPPED",
      "Severity": EventsEvent_severity.CRITICAL,
      "Desc": "Naples service stopped",
  },
  'SERVICE_PENDING' : {
      "Name": "SERVICE_PENDING",
      "Severity": EventsEvent_severity.DEBUG,
      "Desc": "Service pending",
  },
  'SERVICE_RUNNING' : {
      "Name": "SERVICE_RUNNING",
      "Severity": EventsEvent_severity.DEBUG,
      "Desc": "Service running",
  },
  'SERVICE_STARTED' : {
      "Name": "SERVICE_STARTED",
      "Severity": EventsEvent_severity.DEBUG,
      "Desc": "Service started",
  },
  'SERVICE_STOPPED' : {
      "Name": "SERVICE_STOPPED",
      "Severity": EventsEvent_severity.WARN,
      "Desc": "Service stopped",
  },
  'SERVICE_UNRESPONSIVE' : {
      "Name": "SERVICE_UNRESPONSIVE",
      "Severity": EventsEvent_severity.CRITICAL,
      "Desc": "Service unresponsive due to lack of system resources",
  },
  'SYSTEM_COLDBOOT' : {
      "Name": "SYSTEM_COLDBOOT",
      "Severity": EventsEvent_severity.WARN,
      "Desc": "System cold booted",
  },
};