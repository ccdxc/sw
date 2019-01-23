/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { ApiListMeta, IApiListMeta } from './api-list-meta.model';
import { ClusterHost, IClusterHost } from './cluster-host.model';

export interface IClusterHostList {
    'kind'?: string;
    'api-version'?: string;
    'list-meta'?: IApiListMeta;
    'items'?: Array<IClusterHost>;
}


export class ClusterHostList extends BaseModel implements IClusterHostList {
    'kind': string = null;
    'api-version': string = null;
    'list-meta': ApiListMeta = null;
    'items': Array<ClusterHost> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'kind': {
            type: 'string'
        },
        'api-version': {
            type: 'string'
        },
        'list-meta': {
            type: 'object'
        },
        'items': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return ClusterHostList.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return ClusterHostList.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (ClusterHostList.propInfo[prop] != null &&
                        ClusterHostList.propInfo[prop].default != null &&
                        ClusterHostList.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['list-meta'] = new ApiListMeta();
        this['items'] = new Array<ClusterHost>();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['kind'] != null) {
            this['kind'] = values['kind'];
        } else if (fillDefaults && ClusterHostList.hasDefaultValue('kind')) {
            this['kind'] = ClusterHostList.propInfo['kind'].default;
        } else {
            this['kind'] = null
        }
        if (values && values['api-version'] != null) {
            this['api-version'] = values['api-version'];
        } else if (fillDefaults && ClusterHostList.hasDefaultValue('api-version')) {
            this['api-version'] = ClusterHostList.propInfo['api-version'].default;
        } else {
            this['api-version'] = null
        }
        if (values) {
            this['list-meta'].setValues(values['list-meta'], fillDefaults);
        } else {
            this['list-meta'].setValues(null, fillDefaults);
        }
        if (values) {
            this.fillModelArray<ClusterHost>(this, 'items', values['items'], ClusterHost);
        } else {
            this['items'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'kind': new FormControl(this['kind']),
                'api-version': new FormControl(this['api-version']),
                'list-meta': this['list-meta'].$formGroup,
                'items': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<ClusterHost>('items', this['items'], ClusterHost);
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
            this['list-meta'].setFormGroupValuesToBeModelValues();
            this.fillModelArray<ClusterHost>(this, 'items', this['items'], ClusterHost);
        }
    }
}

