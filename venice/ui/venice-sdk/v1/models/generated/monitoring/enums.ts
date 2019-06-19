/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */

// generate enum based on strings instead of numbers
// (see https://blog.rsuter.com/how-to-implement-an-enum-with-string-values-in-typescript/)
export enum ApiListWatchOptions_sort_order {
    'None' = "None",
    'ByName' = "ByName",
    'ByNameReverse' = "ByNameReverse",
    'ByVersion' = "ByVersion",
    'ByVersionReverse' = "ByVersionReverse",
    'ByCreationTime' = "ByCreationTime",
    'ByCreationTimeReverse' = "ByCreationTimeReverse",
    'ByModTime' = "ByModTime",
    'ByModTimeReverse' = "ByModTimeReverse",
}

export enum FieldsRequirement_operator {
    'equals' = "equals",
    'notEquals' = "notEquals",
    'in' = "in",
    'notIn' = "notIn",
    'gt' = "gt",
    'gte' = "gte",
    'lt' = "lt",
    'lte' = "lte",
}

export enum LabelsRequirement_operator {
    'equals' = "equals",
    'notEquals' = "notEquals",
    'in' = "in",
    'notIn' = "notIn",
}

export enum MonitoringAlertPolicySpec_severity {
    'INFO' = "INFO",
    'WARN' = "WARN",
    'CRITICAL' = "CRITICAL",
}

export enum MonitoringAlertSpec_state {
    'OPEN' = "OPEN",
    'RESOLVED' = "RESOLVED",
    'ACKNOWLEDGED' = "ACKNOWLEDGED",
}

export enum MonitoringAlertStatus_severity {
    'INFO' = "INFO",
    'WARN' = "WARN",
    'CRITICAL' = "CRITICAL",
}

export enum MonitoringAuthConfig_algo {
    'MD5' = "MD5",
    'SHA1' = "SHA1",
}

export enum MonitoringEventPolicySpec_format {
    'SYSLOG_BSD' = "SYSLOG_BSD",
    'SYSLOG_RFC5424' = "SYSLOG_RFC5424",
}

export enum MonitoringExternalCred_auth_type {
    'AUTHTYPE_NONE' = "AUTHTYPE_NONE",
    'AUTHTYPE_USERNAMEPASSWORD' = "AUTHTYPE_USERNAMEPASSWORD",
    'AUTHTYPE_TOKEN' = "AUTHTYPE_TOKEN",
    'AUTHTYPE_CERTS' = "AUTHTYPE_CERTS",
}

export enum MonitoringFlowExportPolicySpec_format {
    'Ipfix' = "Ipfix",
}

export enum MonitoringFwlogPolicySpec_format {
    'SYSLOG_BSD' = "SYSLOG_BSD",
    'SYSLOG_RFC5424' = "SYSLOG_RFC5424",
}

export enum MonitoringFwlogPolicySpec_filter {
    'FIREWALL_ACTION_NONE' = "FIREWALL_ACTION_NONE",
    'FIREWALL_ACTION_ALLOW' = "FIREWALL_ACTION_ALLOW",
    'FIREWALL_ACTION_DENY' = "FIREWALL_ACTION_DENY",
    'FIREWALL_ACTION_REJECT' = "FIREWALL_ACTION_REJECT",
    'FIREWALL_ACTION_ALL' = "FIREWALL_ACTION_ALL",
}

export enum MonitoringMatchedRequirement_operator {
    'equals' = "equals",
    'notEquals' = "notEquals",
    'in' = "in",
    'notIn' = "notIn",
    'gt' = "gt",
    'gte' = "gte",
    'lt' = "lt",
    'lte' = "lte",
}

export enum MonitoringMirrorCollector_type {
    'ERSPAN' = "ERSPAN",
}

export enum MonitoringMirrorSessionSpec_packet_filters {
    'ALL_PKTS' = "ALL_PKTS",
    'ALL_DROPS' = "ALL_DROPS",
    'NETWORK_POLICY_DROP' = "NETWORK_POLICY_DROP",
    'FIREWALL_POLICY_DROP' = "FIREWALL_POLICY_DROP",
}

export enum MonitoringMirrorSessionStatus_oper_state {
    'NONE' = "NONE",
    'RUNNING' = "RUNNING",
    'STOPPED' = "STOPPED",
    'SCHEDULED' = "SCHEDULED",
    'ERR_NO_MIRROR_SESSION' = "ERR_NO_MIRROR_SESSION",
}

export enum MonitoringPrivacyConfig_algo {
    'DES56' = "DES56",
    'AES128' = "AES128",
}

export enum MonitoringSNMPTrapServer_version {
    'V2C' = "V2C",
    'V3' = "V3",
}

export enum MonitoringSyslogExport_format {
    'SYSLOG_BSD' = "SYSLOG_BSD",
    'SYSLOG_RFC5424' = "SYSLOG_RFC5424",
}

export enum MonitoringSyslogExportConfig_facility_override {
    'LOG_KERN' = "LOG_KERN",
    'LOG_USER' = "LOG_USER",
    'LOG_MAIL' = "LOG_MAIL",
    'LOG_DAEMON' = "LOG_DAEMON",
    'LOG_AUTH' = "LOG_AUTH",
    'LOG_SYSLOG' = "LOG_SYSLOG",
    'LOG_LPR' = "LOG_LPR",
    'LOG_NEWS' = "LOG_NEWS",
    'LOG_UUCP' = "LOG_UUCP",
    'LOG_CRON' = "LOG_CRON",
    'LOG_AUTHPRIV' = "LOG_AUTHPRIV",
    'LOG_FTP' = "LOG_FTP",
    'LOG_LOCAL0' = "LOG_LOCAL0",
    'LOG_LOCAL1' = "LOG_LOCAL1",
    'LOG_LOCAL2' = "LOG_LOCAL2",
    'LOG_LOCAL3' = "LOG_LOCAL3",
    'LOG_LOCAL4' = "LOG_LOCAL4",
    'LOG_LOCAL5' = "LOG_LOCAL5",
    'LOG_LOCAL6' = "LOG_LOCAL6",
    'LOG_LOCAL7' = "LOG_LOCAL7",
}

export enum MonitoringTechSupportNodeResult_status {
    'Scheduled' = "Scheduled",
    'Running' = "Running",
    'Completed' = "Completed",
    'Failed' = "Failed",
    'TimeOut' = "TimeOut",
}

export enum MonitoringTechSupportRequestStatus_status {
    'Scheduled' = "Scheduled",
    'Running' = "Running",
    'Completed' = "Completed",
    'Failed' = "Failed",
    'TimeOut' = "TimeOut",
}

export enum MonitoringTroubleshootingSessionStatus_state {
    'TS_RUNNING' = "TS_RUNNING",
    'TS_STOPPED' = "TS_STOPPED",
    'TS_SCHEDULED' = "TS_SCHEDULED",
}


export enum ApiListWatchOptions_sort_order_uihint {
    'ByCreationTime' = "By Creation Time",
    'ByCreationTimeReverse' = "By Creation Time Reverse",
    'ByModTime' = "By Modification Time",
    'ByModTimeReverse' = "By Modification Time Reverse",
    'ByName' = "By Name",
    'ByNameReverse' = "By Name Reverse",
    'ByVersion' = "By Version",
    'ByVersionReverse' = "By Version Reverse",
    'None' = "None",
}

export enum FieldsRequirement_operator_uihint {
    'gt' = "greater than",
    'gte' = "greater than or equals",
    'lt' = "less than",
    'lte' = "less than or equals",
    'notEquals' = "not equals",
    'notIn' = "not in",
}

export enum LabelsRequirement_operator_uihint {
    'notEquals' = "not equals",
    'notIn' = "not in",
}

export enum MonitoringAlertSpec_state_uihint {
    'ACKNOWLEDGED' = "Acknowledged",
    'OPEN' = "Open",
    'RESOLVED' = "Resolved",
}

export enum MonitoringEventPolicySpec_format_uihint {
    'SYSLOG_BSD' = "BSD",
    'SYSLOG_RFC5424' = "RFC5424",
}

export enum MonitoringExternalCred_auth_type_uihint {
    'AUTHTYPE_CERTS' = "Certs",
    'AUTHTYPE_NONE' = "None",
    'AUTHTYPE_TOKEN' = "Token",
    'AUTHTYPE_USERNAMEPASSWORD' = "Username/Password",
}

export enum MonitoringFwlogPolicySpec_format_uihint {
    'SYSLOG_BSD' = "BSD",
    'SYSLOG_RFC5424' = "RFC5424",
}

export enum MonitoringFwlogPolicySpec_filter_uihint {
    'FIREWALL_ACTION_ALL' = "All Logs",
    'FIREWALL_ACTION_ALLOW' = "Allow Logs",
    'FIREWALL_ACTION_DENY' = "Deny Logs",
    'FIREWALL_ACTION_NONE' = "No Logs",
    'FIREWALL_ACTION_REJECT' = "Reject Logs",
}

export enum MonitoringMatchedRequirement_operator_uihint {
    'gt' = "greater than",
    'gte' = "greater than or equals",
    'lt' = "less than",
    'lte' = "less than or equals",
    'notEquals' = "not equals",
    'notIn' = "not in",
}

export enum MonitoringMirrorCollector_type_uihint {
    'ERSPAN' = "ERSPAN",
}

export enum MonitoringMirrorSessionSpec_packet_filters_uihint {
    'ALL_DROPS' = "All Drops",
    'ALL_PKTS' = "All Packets",
    'FIREWALL_POLICY_DROP' = "Firewall Policy Drops",
    'NETWORK_POLICY_DROP' = "Network Policy Drops",
}

export enum MonitoringMirrorSessionStatus_oper_state_uihint {
    'ERR_NO_MIRROR_SESSION' = "Max Mirror Sessions Exceeded",
    'NONE' = "None",
    'RUNNING' = "Running",
    'SCHEDULED' = "Scheduled",
    'STOPPED' = "Stopped",
}

export enum MonitoringSyslogExport_format_uihint {
    'SYSLOG_BSD' = "BSD",
    'SYSLOG_RFC5424' = "RFC5424",
}

export enum MonitoringSyslogExportConfig_facility_override_uihint {
    'LOG_AUTH' = "Auth",
    'LOG_AUTHPRIV' = "Private Auth",
    'LOG_CRON' = "CRON",
    'LOG_DAEMON' = "Daemon",
    'LOG_FTP' = "FTP",
    'LOG_KERN' = "Kernel",
    'LOG_LOCAL0' = "local 0",
    'LOG_LOCAL1' = "local 1",
    'LOG_LOCAL2' = "local 2",
    'LOG_LOCAL3' = "local 3",
    'LOG_LOCAL4' = "local 4",
    'LOG_LOCAL5' = "local 5",
    'LOG_LOCAL6' = "local 6",
    'LOG_LOCAL7' = "local 7",
    'LOG_LPR' = "LPR",
    'LOG_MAIL' = "Mail",
    'LOG_NEWS' = "News",
    'LOG_SYSLOG' = "Syslog",
    'LOG_USER' = "User",
    'LOG_UUCP' = "UUCP",
}

export enum MonitoringTroubleshootingSessionStatus_state_uihint {
    'TS_RUNNING' = "Running",
    'TS_SCHEDULED' = "Scheduled",
    'TS_STOPPED' = "Stopped",
}




/**
 * bundle of all enums for databinding to options, radio-buttons etc.
 * usage in component:
 *   import { AllEnums, minValueValidator, maxValueValidator } from '../../models/webapi';
 *
 *   @Component({
 *       ...
 *   })
 *   export class xxxComponent implements OnInit {
 *       allEnums = AllEnums;
 *       ...
 *       ngOnInit() {
 *           this.allEnums = AllEnums.instance;
 *       }
 *   }
*/
export class AllEnums {
    private static _instance: AllEnums = new AllEnums();
    constructor() {
        if (AllEnums._instance) {
            throw new Error("Error: Instantiation failed: Use AllEnums.instance instead of new");
        }
        AllEnums._instance = this;
    }
    static get instance(): AllEnums {
        return AllEnums._instance;
    }

    ApiListWatchOptions_sort_order = ApiListWatchOptions_sort_order;
    FieldsRequirement_operator = FieldsRequirement_operator;
    LabelsRequirement_operator = LabelsRequirement_operator;
    MonitoringAlertPolicySpec_severity = MonitoringAlertPolicySpec_severity;
    MonitoringAlertSpec_state = MonitoringAlertSpec_state;
    MonitoringAlertStatus_severity = MonitoringAlertStatus_severity;
    MonitoringAuthConfig_algo = MonitoringAuthConfig_algo;
    MonitoringEventPolicySpec_format = MonitoringEventPolicySpec_format;
    MonitoringExternalCred_auth_type = MonitoringExternalCred_auth_type;
    MonitoringFlowExportPolicySpec_format = MonitoringFlowExportPolicySpec_format;
    MonitoringFwlogPolicySpec_format = MonitoringFwlogPolicySpec_format;
    MonitoringFwlogPolicySpec_filter = MonitoringFwlogPolicySpec_filter;
    MonitoringMatchedRequirement_operator = MonitoringMatchedRequirement_operator;
    MonitoringMirrorCollector_type = MonitoringMirrorCollector_type;
    MonitoringMirrorSessionSpec_packet_filters = MonitoringMirrorSessionSpec_packet_filters;
    MonitoringMirrorSessionStatus_oper_state = MonitoringMirrorSessionStatus_oper_state;
    MonitoringPrivacyConfig_algo = MonitoringPrivacyConfig_algo;
    MonitoringSNMPTrapServer_version = MonitoringSNMPTrapServer_version;
    MonitoringSyslogExport_format = MonitoringSyslogExport_format;
    MonitoringSyslogExportConfig_facility_override = MonitoringSyslogExportConfig_facility_override;
    MonitoringTechSupportNodeResult_status = MonitoringTechSupportNodeResult_status;
    MonitoringTechSupportRequestStatus_status = MonitoringTechSupportRequestStatus_status;
    MonitoringTroubleshootingSessionStatus_state = MonitoringTroubleshootingSessionStatus_state;

    ApiListWatchOptions_sort_order_uihint = ApiListWatchOptions_sort_order_uihint;
    FieldsRequirement_operator_uihint = FieldsRequirement_operator_uihint;
    LabelsRequirement_operator_uihint = LabelsRequirement_operator_uihint;
    MonitoringAlertSpec_state_uihint = MonitoringAlertSpec_state_uihint;
    MonitoringEventPolicySpec_format_uihint = MonitoringEventPolicySpec_format_uihint;
    MonitoringExternalCred_auth_type_uihint = MonitoringExternalCred_auth_type_uihint;
    MonitoringFwlogPolicySpec_format_uihint = MonitoringFwlogPolicySpec_format_uihint;
    MonitoringFwlogPolicySpec_filter_uihint = MonitoringFwlogPolicySpec_filter_uihint;
    MonitoringMatchedRequirement_operator_uihint = MonitoringMatchedRequirement_operator_uihint;
    MonitoringMirrorCollector_type_uihint = MonitoringMirrorCollector_type_uihint;
    MonitoringMirrorSessionSpec_packet_filters_uihint = MonitoringMirrorSessionSpec_packet_filters_uihint;
    MonitoringMirrorSessionStatus_oper_state_uihint = MonitoringMirrorSessionStatus_oper_state_uihint;
    MonitoringSyslogExport_format_uihint = MonitoringSyslogExport_format_uihint;
    MonitoringSyslogExportConfig_facility_override_uihint = MonitoringSyslogExportConfig_facility_override_uihint;
    MonitoringTroubleshootingSessionStatus_state_uihint = MonitoringTroubleshootingSessionStatus_state_uihint;
}
