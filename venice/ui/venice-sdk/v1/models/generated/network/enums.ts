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

export enum NetworkTLSServerPolicySpec_client_authentication {
    'Mandatory' = "Mandatory",
    'Optional' = "Optional",
    'None' = "None",
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
    NetworkTLSServerPolicySpec_client_authentication = NetworkTLSServerPolicySpec_client_authentication;

    ApiListWatchOptions_sort_order_uihint = ApiListWatchOptions_sort_order_uihint;
}
