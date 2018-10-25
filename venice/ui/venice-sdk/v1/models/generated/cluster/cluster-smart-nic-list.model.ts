/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { ClusterSmartNIC, IClusterSmartNIC } from './cluster-smart-nic.model';

export interface IClusterSmartNICList {
    'kind'?: string;
    'api-version'?: string;
    'resource-version'?: string;
    'Items'?: Array<IClusterSmartNIC>;
}


export class ClusterSmartNICList extends BaseModel implements IClusterSmartNICList {
    'kind': string = null;
    'api-version': string = null;
    'resource-version': string = null;
    'Items': Array<ClusterSmartNIC> = null;
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
        return ClusterSmartNICList.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (ClusterSmartNICList.propInfo[prop] != null &&
                        ClusterSmartNICList.propInfo[prop].default != null &&
                        ClusterSmartNICList.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['Items'] = new Array<ClusterSmartNIC>();
        this.setValues(values);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['kind'] != null) {
            this['kind'] = values['kind'];
        } else if (fillDefaults && ClusterSmartNICList.hasDefaultValue('kind')) {
            this['kind'] = ClusterSmartNICList.propInfo['kind'].default;
        }
        if (values && values['api-version'] != null) {
            this['api-version'] = values['api-version'];
        } else if (fillDefaults && ClusterSmartNICList.hasDefaultValue('api-version')) {
            this['api-version'] = ClusterSmartNICList.propInfo['api-version'].default;
        }
        if (values && values['resource-version'] != null) {
            this['resource-version'] = values['resource-version'];
        } else if (fillDefaults && ClusterSmartNICList.hasDefaultValue('resource-version')) {
            this['resource-version'] = ClusterSmartNICList.propInfo['resource-version'].default;
        }
        if (values) {
            this.fillModelArray<ClusterSmartNIC>(this, 'Items', values['Items'], ClusterSmartNIC);
        }
        this.setFormGroupValuesToBeModelValues();
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
            this.fillFormArray<ClusterSmartNIC>('Items', this['Items'], ClusterSmartNIC);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['kind'].setValue(this['kind']);
            this._formGroup.controls['api-version'].setValue(this['api-version']);
            this._formGroup.controls['resource-version'].setValue(this['resource-version']);
            this.fillModelArray<ClusterSmartNIC>(this, 'Items', this['Items'], ClusterSmartNIC);
        }
    }
}

