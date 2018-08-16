/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { ClusterTenant, IClusterTenant } from './cluster-tenant.model';

export interface IClusterTenantList {
    'kind'?: string;
    'api-version'?: string;
    'resource-version'?: string;
    'Items'?: Array<IClusterTenant>;
}


export class ClusterTenantList extends BaseModel implements IClusterTenantList {
    'kind': string = null;
    'api-version': string = null;
    'resource-version': string = null;
    'Items': Array<ClusterTenant> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'kind': {
            type: 'string'
        },
        'api-version': {
            type: 'string'
        },
        'resource-version': {
            type: 'string'
        },
        'Items': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return ClusterTenantList.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (ClusterTenantList.propInfo[prop] != null &&
                        ClusterTenantList.propInfo[prop].default != null &&
                        ClusterTenantList.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['Items'] = new Array<ClusterTenant>();
        this.setValues(values);
    }

    /**
     * set the values. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any): void {
        if (values && values['kind'] != null) {
            this['kind'] = values['kind'];
        } else if (ClusterTenantList.hasDefaultValue('kind')) {
            this['kind'] = ClusterTenantList.propInfo['kind'].default;
        }
        if (values && values['api-version'] != null) {
            this['api-version'] = values['api-version'];
        } else if (ClusterTenantList.hasDefaultValue('api-version')) {
            this['api-version'] = ClusterTenantList.propInfo['api-version'].default;
        }
        if (values && values['resource-version'] != null) {
            this['resource-version'] = values['resource-version'];
        } else if (ClusterTenantList.hasDefaultValue('resource-version')) {
            this['resource-version'] = ClusterTenantList.propInfo['resource-version'].default;
        }
        if (values) {
            this.fillModelArray<ClusterTenant>(this, 'Items', values['Items'], ClusterTenant);
        }
    }




    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'kind': new FormControl(this['kind']),
                'api-version': new FormControl(this['api-version']),
                'resource-version': new FormControl(this['resource-version']),
                'Items': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<ClusterTenant>('Items', this['Items'], ClusterTenant);
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this._formGroup.controls['kind'].setValue(this['kind']);
            this._formGroup.controls['api-version'].setValue(this['api-version']);
            this._formGroup.controls['resource-version'].setValue(this['resource-version']);
            this.fillModelArray<ClusterTenant>(this, 'Items', this['Items'], ClusterTenant);
        }
    }
}

