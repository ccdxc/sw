/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */

// generate enum based on strings instead of numbers
// (see https://blog.rsuter.com/how-to-implement-an-enum-with-string-values-in-typescript/)
export enum ApiListWatchOptions_sort_order {
    'none' = "none",
    'by-name' = "by-name",
    'by-name-reverse' = "by-name-reverse",
    'by-version' = "by-version",
    'by-version-reverse' = "by-version-reverse",
    'by-creation-time' = "by-creation-time",
    'by-creation-time-reverse' = "by-creation-time-reverse",
    'by-mod-time' = "by-mod-time",
    'by-mod-time-reverse' = "by-mod-time-reverse",
}

export enum FwlogFwLog_action {
    'allow' = "allow",
    'deny' = "deny",
    'reject' = "reject",
    'implicit_deny' = "implicit_deny",
}

export enum FwlogFwLog_direction {
    'from-host' = "from-host",
    'from-uplink' = "from-uplink",
}

export enum FwlogFwLogQuery_actions {
    'allow' = "allow",
    'deny' = "deny",
    'reject' = "reject",
    'implicit_deny' = "implicit_deny",
}

export enum FwlogFwLogQuery_sort_order {
    'ascending' = "ascending",
    'descending' = "descending",
}


export enum ApiListWatchOptions_sort_order_uihint {
    'by-creation-time' = "By Creation Time",
    'by-creation-time-reverse' = "By Creation Time Reverse",
    'by-mod-time' = "By Modification Time",
    'by-mod-time-reverse' = "By Modification Time Reverse",
    'by-name' = "By Name",
    'by-name-reverse' = "By Name Reverse",
    'by-version' = "By Version",
    'by-version-reverse' = "By Version Reverse",
    'none' = "None",
}

export enum FwlogFwLog_action_uihint {
    'allow' = "Allow",
    'deny' = "Deny",
    'implicit_deny' = "Implicit-deny",
    'reject' = "Reject",
}

export enum FwlogFwLog_direction_uihint {
    'from-host' = "From Host",
    'from-uplink' = "From Uplink",
}

export enum FwlogFwLogQuery_actions_uihint {
    'allow' = "Allow",
    'deny' = "Deny",
    'implicit_deny' = "Implicit-deny",
    'reject' = "Reject",
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
    FwlogFwLog_action = FwlogFwLog_action;
    FwlogFwLog_direction = FwlogFwLog_direction;
    FwlogFwLogQuery_actions = FwlogFwLogQuery_actions;
    FwlogFwLogQuery_sort_order = FwlogFwLogQuery_sort_order;

    ApiListWatchOptions_sort_order_uihint = ApiListWatchOptions_sort_order_uihint;
    FwlogFwLog_action_uihint = FwlogFwLog_action_uihint;
    FwlogFwLog_direction_uihint = FwlogFwLog_direction_uihint;
    FwlogFwLogQuery_actions_uihint = FwlogFwLogQuery_actions_uihint;
}
