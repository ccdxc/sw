/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { ApiObjectMeta, IApiObjectMeta } from './api-object-meta.model';
import { ClusterSmartNICSpec, IClusterSmartNICSpec } from './cluster-smart-nic-spec.model';
import { ClusterSmartNICStatus, IClusterSmartNICStatus } from './cluster-smart-nic-status.model';

export interface IClusterSmartNIC {
    'kind'?: string;
    'api-version'?: string;
    'meta'?: IApiObjectMeta;
    'spec'?: IClusterSmartNICSpec;
    'status'?: IClusterSmartNICStatus;
}


export class ClusterSmartNIC extends BaseModel implements IClusterSmartNIC {
    'kind': string = null;
    'api-version': string = null;
    'meta': ApiObjectMeta = null;
    /** SmartNICSpec contains the configuration of the network adapter. */
    'spec': ClusterSmartNICSpec = null;
    /** SmartNICStatus contains the current state of the network adapter. */
    'status': ClusterSmartNICStatus = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'kind': {
            type: 'string'
        },
        'api-version': {
            type: 'string'
        },
        'meta': {
            type: 'object'
        },
        'spec': {
            description:  'SmartNICSpec contains the configuration of the network adapter.',
            type: 'object'
        },
        'status': {
            description:  'SmartNICStatus contains the current state of the network adapter.',
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return ClusterSmartNIC.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (ClusterSmartNIC.propInfo[prop] != null &&
                        ClusterSmartNIC.propInfo[prop].default != null &&
                        ClusterSmartNIC.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['meta'] = new ApiObjectMeta();
        this['spec'] = new ClusterSmartNICSpec();
        this['status'] = new ClusterSmartNICStatus();
        this.setValues(values);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['kind'] != null) {
            this['kind'] = values['kind'];
        } else if (fillDefaults && ClusterSmartNIC.hasDefaultValue('kind')) {
            this['kind'] = ClusterSmartNIC.propInfo['kind'].default;
        }
        if (values && values['api-version'] != null) {
            this['api-version'] = values['api-version'];
        } else if (fillDefaults && ClusterSmartNIC.hasDefaultValue('api-version')) {
            this['api-version'] = ClusterSmartNIC.propInfo['api-version'].default;
        }
        if (values) {
            this['meta'].setValues(values['meta']);
        }
        if (values) {
            this['spec'].setValues(values['spec']);
        }
        if (values) {
            this['status'].setValues(values['status']);
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'kind': new FormControl(this['kind']),
                'api-version': new FormControl(this['api-version']),
                'meta': this['meta'].$formGroup,
                'spec': this['spec'].$formGroup,
                'status': this['status'].$formGroup,
            });
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
            this['meta'].setFormGroupValuesToBeModelValues();
            this['spec'].setFormGroupValuesToBeModelValues();
            this['status'].setFormGroupValuesToBeModelValues();
        }
    }
}

