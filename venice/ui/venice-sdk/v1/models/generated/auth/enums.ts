/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */

// generate enum based on strings instead of numbers
// (see https://blog.rsuter.com/how-to-implement-an-enum-with-string-values-in-typescript/)
export enum AuthAuthenticators_authenticator_order {
    'LOCAL' = "LOCAL",
    'LDAP' = "LDAP",
    'RADIUS' = "RADIUS",
}

export enum AuthPermission_resource_kind {
    'AllResourceKinds' = "AllResourceKinds",
    'APIEndpoint' = "APIEndpoint",
    'Search' = "Search",
    'Event' = "Event",
    'Tenant' = "Tenant",
    'Cluster' = "Cluster",
    'Node' = "Node",
    'Host' = "Host",
    'SmartNIC' = "SmartNIC",
    'Rollout' = "Rollout",
    'Network' = "Network",
    'Service' = "Service",
    'LbPolicy' = "LbPolicy",
    'Workload' = "Workload",
    'Endpoint' = "Endpoint",
    'SecurityGroup' = "SecurityGroup",
    'SGPolicy' = "SGPolicy",
    'App' = "App",
    'TrafficEncryptionPolicy' = "TrafficEncryptionPolicy",
    'Certificate' = "Certificate",
    'Alert' = "Alert",
    'AlertPolicy' = "AlertPolicy",
    'AlertDestination' = "AlertDestination",
    'EventPolicy' = "EventPolicy",
    'StatsPolicy' = "StatsPolicy",
    'FwlogPolicy' = "FwlogPolicy",
    'FlowExportPolicy' = "FlowExportPolicy",
    'MirrorSession' = "MirrorSession",
    'User' = "User",
    'AuthenticationPolicy' = "AuthenticationPolicy",
    'Role' = "Role",
    'RoleBinding' = "RoleBinding",
}

export enum AuthPermission_actions {
    'ALL_ACTIONS' = "ALL_ACTIONS",
    'CREATE' = "CREATE",
    'READ' = "READ",
    'UPDATE' = "UPDATE",
    'DELETE' = "DELETE",
    'COMMIT' = "COMMIT",
}

export enum AuthRadiusServer_auth_method {
    'PAP' = "PAP",
    'PEAP_MSCHAPv2' = "PEAP_MSCHAPv2",
    'EAP_TTLS_PAP' = "EAP_TTLS_PAP",
}

export enum AuthUserSpec_type {
    'LOCAL' = "LOCAL",
    'EXTERNAL' = "EXTERNAL",
}

export enum AuthUserStatus_authenticators {
    'LOCAL' = "LOCAL",
    'LDAP' = "LDAP",
    'RADIUS' = "RADIUS",
}


export enum AuthAuthenticators_authenticator_order_uihint {
    'LOCAL' = "Local",
}

export enum AuthPermission_resource_kind_uihint {
    'APIEndpoint' = "API Endpoint",
    'Alert' = "Alert",
    'AlertDestination' = "AlertDestination",
    'AlertPolicy' = "AlertPolicy",
    'AllResourceKinds' = "All Resource Kinds",
    'App' = "App",
    'AuthenticationPolicy' = "AuthenticationPolicy",
    'Certificate' = "Certificate",
    'Cluster' = "Cluster",
    'Endpoint' = "Endpoint",
    'Event' = "Events",
    'EventPolicy' = "EventPolicy",
    'FlowExportPolicy' = "FlowExportPolicy",
    'FwlogPolicy' = "FwlogPolicy",
    'Host' = "Host",
    'LbPolicy' = "LBPolicy",
    'MirrorSession' = "MirrorSession",
    'Network' = "Network",
    'Node' = "Node",
    'Role' = "Role",
    'RoleBinding' = "RoleBinding",
    'Rollout' = "Rollout",
    'SGPolicy' = "SGPolicy",
    'Search' = "Search",
    'SecurityGroup' = "SecurityGroup",
    'Service' = "Service",
    'SmartNIC' = "SmartNIC",
    'StatsPolicy' = "StatsPolicy",
    'Tenant' = "Tenant",
    'TrafficEncryptionPolicy' = "EncryptionPolicy",
    'User' = "User",
    'Workload' = "Workload",
}

export enum AuthPermission_actions_uihint {
    'ALL_ACTIONS' = "All Actions",
    'COMMIT' = "Commit Config",
    'CREATE' = "Create",
    'DELETE' = "Delete",
    'READ' = "Read",
    'UPDATE' = "Update",
}

export enum AuthUserSpec_type_uihint {
    'EXTERNAL' = "External",
    'LOCAL' = "Local",
}

export enum AuthUserStatus_authenticators_uihint {
    'LOCAL' = "Local",
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

    AuthAuthenticators_authenticator_order = AuthAuthenticators_authenticator_order;
    AuthPermission_resource_kind = AuthPermission_resource_kind;
    AuthPermission_actions = AuthPermission_actions;
    AuthRadiusServer_auth_method = AuthRadiusServer_auth_method;
    AuthUserSpec_type = AuthUserSpec_type;
    AuthUserStatus_authenticators = AuthUserStatus_authenticators;

    AuthAuthenticators_authenticator_order_uihint = AuthAuthenticators_authenticator_order_uihint;
    AuthPermission_resource_kind_uihint = AuthPermission_resource_kind_uihint;
    AuthPermission_actions_uihint = AuthPermission_actions_uihint;
    AuthUserSpec_type_uihint = AuthUserSpec_type_uihint;
    AuthUserStatus_authenticators_uihint = AuthUserStatus_authenticators_uihint;
}
