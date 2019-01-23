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

export enum LabelsRequirement_operator {
    'equals' = "equals",
    'notEquals' = "notEquals",
    'in' = "in",
    'notIn' = "notIn",
}

export enum SecurityALG_type {
    'ICMP' = "ICMP",
    'DNS' = "DNS",
    'FTP' = "FTP",
    'SunRPC' = "SunRPC",
    'MSRPC' = "MSRPC",
    'TFTP' = "TFTP",
    'RTSP' = "RTSP",
}

export enum SecurityCertificateSpec_usages {
    'Server' = "Server",
    'Client' = "Client",
    'TrustRoot' = "TrustRoot",
}

export enum SecurityCertificateStatus_validity {
    'Unknown' = "Unknown",
    'Valid' = "Valid",
    'Invalid' = "Invalid",
    'Expired' = "Expired",
}

export enum SecuritySGRule_action {
    'PERMIT' = "PERMIT",
    'DENY' = "DENY",
    'REJECT' = "REJECT",
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

export enum LabelsRequirement_operator_uihint {
    'notEquals' = "not equals",
    'notIn' = "not in",
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
    LabelsRequirement_operator = LabelsRequirement_operator;
    SecurityALG_type = SecurityALG_type;
    SecurityCertificateSpec_usages = SecurityCertificateSpec_usages;
    SecurityCertificateStatus_validity = SecurityCertificateStatus_validity;
    SecuritySGRule_action = SecuritySGRule_action;

    ApiListWatchOptions_sort_order_uihint = ApiListWatchOptions_sort_order_uihint;
    LabelsRequirement_operator_uihint = LabelsRequirement_operator_uihint;
}
