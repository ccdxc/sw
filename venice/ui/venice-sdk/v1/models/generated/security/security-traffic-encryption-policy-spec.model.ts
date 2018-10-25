/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { SecurityTLSProtocolSpec, ISecurityTLSProtocolSpec } from './security-tls-protocol-spec.model';
import { SecurityIPsecProtocolSpec, ISecurityIPsecProtocolSpec } from './security-i-psec-protocol-spec.model';

export interface ISecurityTrafficEncryptionPolicySpec {
    'mode'?: string;
    'tls'?: ISecurityTLSProtocolSpec;
    'ipsec'?: ISecurityIPsecProtocolSpec;
    'key-rotation-interval-secs'?: number;
}


export class SecurityTrafficEncryptionPolicySpec extends BaseModel implements ISecurityTrafficEncryptionPolicySpec {
    'mode': string = null;
    'tls': SecurityTLSProtocolSpec = null;
    'ipsec': SecurityIPsecProtocolSpec = null;
    'key-rotation-interval-secs': number = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'mode': {
            type: 'string'
        },
        'tls': {
            type: 'object'
        },
        'ipsec': {
            type: 'object'
        },
        'key-rotation-interval-secs': {
            type: 'number'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return SecurityTrafficEncryptionPolicySpec.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (SecurityTrafficEncryptionPolicySpec.propInfo[prop] != null &&
                        SecurityTrafficEncryptionPolicySpec.propInfo[prop].default != null &&
                        SecurityTrafficEncryptionPolicySpec.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['tls'] = new SecurityTLSProtocolSpec();
        this['ipsec'] = new SecurityIPsecProtocolSpec();
        this.setValues(values);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['mode'] != null) {
            this['mode'] = values['mode'];
        } else if (fillDefaults && SecurityTrafficEncryptionPolicySpec.hasDefaultValue('mode')) {
            this['mode'] = SecurityTrafficEncryptionPolicySpec.propInfo['mode'].default;
        }
        if (values) {
            this['tls'].setValues(values['tls']);
        }
        if (values) {
            this['ipsec'].setValues(values['ipsec']);
        }
        if (values && values['key-rotation-interval-secs'] != null) {
            this['key-rotation-interval-secs'] = values['key-rotation-interval-secs'];
        } else if (fillDefaults && SecurityTrafficEncryptionPolicySpec.hasDefaultValue('key-rotation-interval-secs')) {
            this['key-rotation-interval-secs'] = SecurityTrafficEncryptionPolicySpec.propInfo['key-rotation-interval-secs'].default;
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'mode': new FormControl(this['mode']),
                'tls': this['tls'].$formGroup,
                'ipsec': this['ipsec'].$formGroup,
                'key-rotation-interval-secs': new FormControl(this['key-rotation-interval-secs']),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['mode'].setValue(this['mode']);
            this['tls'].setFormGroupValuesToBeModelValues();
            this['ipsec'].setFormGroupValuesToBeModelValues();
            this._formGroup.controls['key-rotation-interval-secs'].setValue(this['key-rotation-interval-secs']);
        }
    }
}

