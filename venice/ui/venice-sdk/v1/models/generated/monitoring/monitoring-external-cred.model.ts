/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { MonitoringExternalCred_auth_type,  MonitoringExternalCred_auth_type_uihint  } from './enums';

export interface IMonitoringExternalCred {
    'auth-type'?: MonitoringExternalCred_auth_type;
    'username'?: string;
    'password'?: string;
    'bearer-token'?: string;
    'cert-data'?: string;
    'key-data'?: string;
    'ca-data'?: string;
}


export class MonitoringExternalCred extends BaseModel implements IMonitoringExternalCred {
    'auth-type': MonitoringExternalCred_auth_type = null;
    'username': string = null;
    'password': string = null;
    'bearer-token': string = null;
    /** CertData holds PEM-encoded bytes (typically read from a client certificate file). */
    'cert-data': string = null;
    'key-data': string = null;
    /** CaData holds PEM-encoded bytes (typically read from a root certificates bundle). */
    'ca-data': string = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'auth-type': {
            enum: MonitoringExternalCred_auth_type_uihint,
            default: 'AUTHTYPE_NONE',
            type: 'string'
        },
        'username': {
            type: 'string'
        },
        'password': {
            type: 'string'
        },
        'bearer-token': {
            type: 'string'
        },
        'cert-data': {
            description:  'CertData holds PEM-encoded bytes (typically read from a client certificate file).',
            type: 'string'
        },
        'key-data': {
            type: 'string'
        },
        'ca-data': {
            description:  'CaData holds PEM-encoded bytes (typically read from a root certificates bundle).',
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringExternalCred.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringExternalCred.propInfo[prop] != null &&
                        MonitoringExternalCred.propInfo[prop].default != null &&
                        MonitoringExternalCred.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this.setValues(values);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['auth-type'] != null) {
            this['auth-type'] = values['auth-type'];
        } else if (fillDefaults && MonitoringExternalCred.hasDefaultValue('auth-type')) {
            this['auth-type'] = <MonitoringExternalCred_auth_type>  MonitoringExternalCred.propInfo['auth-type'].default;
        }
        if (values && values['username'] != null) {
            this['username'] = values['username'];
        } else if (fillDefaults && MonitoringExternalCred.hasDefaultValue('username')) {
            this['username'] = MonitoringExternalCred.propInfo['username'].default;
        }
        if (values && values['password'] != null) {
            this['password'] = values['password'];
        } else if (fillDefaults && MonitoringExternalCred.hasDefaultValue('password')) {
            this['password'] = MonitoringExternalCred.propInfo['password'].default;
        }
        if (values && values['bearer-token'] != null) {
            this['bearer-token'] = values['bearer-token'];
        } else if (fillDefaults && MonitoringExternalCred.hasDefaultValue('bearer-token')) {
            this['bearer-token'] = MonitoringExternalCred.propInfo['bearer-token'].default;
        }
        if (values && values['cert-data'] != null) {
            this['cert-data'] = values['cert-data'];
        } else if (fillDefaults && MonitoringExternalCred.hasDefaultValue('cert-data')) {
            this['cert-data'] = MonitoringExternalCred.propInfo['cert-data'].default;
        }
        if (values && values['key-data'] != null) {
            this['key-data'] = values['key-data'];
        } else if (fillDefaults && MonitoringExternalCred.hasDefaultValue('key-data')) {
            this['key-data'] = MonitoringExternalCred.propInfo['key-data'].default;
        }
        if (values && values['ca-data'] != null) {
            this['ca-data'] = values['ca-data'];
        } else if (fillDefaults && MonitoringExternalCred.hasDefaultValue('ca-data')) {
            this['ca-data'] = MonitoringExternalCred.propInfo['ca-data'].default;
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'auth-type': new FormControl(this['auth-type'], [enumValidator(MonitoringExternalCred_auth_type), ]),
                'username': new FormControl(this['username']),
                'password': new FormControl(this['password']),
                'bearer-token': new FormControl(this['bearer-token']),
                'cert-data': new FormControl(this['cert-data']),
                'key-data': new FormControl(this['key-data']),
                'ca-data': new FormControl(this['ca-data']),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['auth-type'].setValue(this['auth-type']);
            this._formGroup.controls['username'].setValue(this['username']);
            this._formGroup.controls['password'].setValue(this['password']);
            this._formGroup.controls['bearer-token'].setValue(this['bearer-token']);
            this._formGroup.controls['cert-data'].setValue(this['cert-data']);
            this._formGroup.controls['key-data'].setValue(this['key-data']);
            this._formGroup.controls['ca-data'].setValue(this['ca-data']);
        }
    }
}

