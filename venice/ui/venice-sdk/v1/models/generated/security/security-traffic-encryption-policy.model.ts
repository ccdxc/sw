/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { ApiObjectMeta, IApiObjectMeta } from './api-object-meta.model';
import { SecurityTrafficEncryptionPolicySpec, ISecurityTrafficEncryptionPolicySpec } from './security-traffic-encryption-policy-spec.model';
import { SecurityTrafficEncryptionPolicyStatus, ISecurityTrafficEncryptionPolicyStatus } from './security-traffic-encryption-policy-status.model';

export interface ISecurityTrafficEncryptionPolicy {
    'kind'?: string;
    'api-version'?: string;
    'meta'?: IApiObjectMeta;
    'spec'?: ISecurityTrafficEncryptionPolicySpec;
    'status'?: ISecurityTrafficEncryptionPolicyStatus;
}


export class SecurityTrafficEncryptionPolicy extends BaseModel implements ISecurityTrafficEncryptionPolicy {
    'kind': string = null;
    'api-version': string = null;
    'meta': ApiObjectMeta = null;
    /** Spec contains the configuration of the encryption policy. */
    'spec': SecurityTrafficEncryptionPolicySpec = null;
    /** Status contains the current state of the encryption policy. */
    'status': SecurityTrafficEncryptionPolicyStatus = null;
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
            description:  'Spec contains the configuration of the encryption policy.',
            type: 'object'
        },
        'status': {
            description:  'Status contains the current state of the encryption policy.',
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return SecurityTrafficEncryptionPolicy.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return SecurityTrafficEncryptionPolicy.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (SecurityTrafficEncryptionPolicy.propInfo[prop] != null &&
                        SecurityTrafficEncryptionPolicy.propInfo[prop].default != null &&
                        SecurityTrafficEncryptionPolicy.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['meta'] = new ApiObjectMeta();
        this['spec'] = new SecurityTrafficEncryptionPolicySpec();
        this['status'] = new SecurityTrafficEncryptionPolicyStatus();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['kind'] != null) {
            this['kind'] = values['kind'];
        } else if (fillDefaults && SecurityTrafficEncryptionPolicy.hasDefaultValue('kind')) {
            this['kind'] = SecurityTrafficEncryptionPolicy.propInfo['kind'].default;
        } else {
            this['kind'] = null
        }
        if (values && values['api-version'] != null) {
            this['api-version'] = values['api-version'];
        } else if (fillDefaults && SecurityTrafficEncryptionPolicy.hasDefaultValue('api-version')) {
            this['api-version'] = SecurityTrafficEncryptionPolicy.propInfo['api-version'].default;
        } else {
            this['api-version'] = null
        }
        if (values) {
            this['meta'].setValues(values['meta'], fillDefaults);
        } else {
            this['meta'].setValues(null, fillDefaults);
        }
        if (values) {
            this['spec'].setValues(values['spec'], fillDefaults);
        } else {
            this['spec'].setValues(null, fillDefaults);
        }
        if (values) {
            this['status'].setValues(values['status'], fillDefaults);
        } else {
            this['status'].setValues(null, fillDefaults);
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

