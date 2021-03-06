/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl, CustomFormGroup } from '../../../utils/validators';
import { BaseModel, PropInfoItem } from '../basemodel/base-model';

import { ApiObjectMeta, IApiObjectMeta } from './api-object-meta.model';

export interface IClusterUpdateTLSConfigRequest {
    'kind'?: string;
    'api-version'?: string;
    'meta'?: IApiObjectMeta;
    'certs'?: string;
    'key'?: string;
    '_ui'?: any;
}


export class ClusterUpdateTLSConfigRequest extends BaseModel implements IClusterUpdateTLSConfigRequest {
    /** Field for holding arbitrary ui state */
    '_ui': any = {};
    'kind': string = null;
    'api-version': string = null;
    'meta': ApiObjectMeta = null;
    /** Certs is the pem encoded certificate bundle used for API Gateway TLS. */
    'certs': string = null;
    /** Key is the pem encoded private key used for API Gateway TLS. We support RSA or ECDSA. */
    'key': string = null;
    public static propInfo: { [prop in keyof IClusterUpdateTLSConfigRequest]: PropInfoItem } = {
        'kind': {
            required: false,
            type: 'string'
        },
        'api-version': {
            required: false,
            type: 'string'
        },
        'meta': {
            required: false,
            type: 'object'
        },
        'certs': {
            description:  `Certs is the pem encoded certificate bundle used for API Gateway TLS.`,
            required: false,
            type: 'string'
        },
        'key': {
            description:  `Key is the pem encoded private key used for API Gateway TLS. We support RSA or ECDSA.`,
            required: false,
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return ClusterUpdateTLSConfigRequest.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return ClusterUpdateTLSConfigRequest.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (ClusterUpdateTLSConfigRequest.propInfo[prop] != null &&
                        ClusterUpdateTLSConfigRequest.propInfo[prop].default != null);
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['meta'] = new ApiObjectMeta();
        this._inputValue = values;
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['_ui']) {
            this['_ui'] = values['_ui']
        }
        if (values && values['kind'] != null) {
            this['kind'] = values['kind'];
        } else if (fillDefaults && ClusterUpdateTLSConfigRequest.hasDefaultValue('kind')) {
            this['kind'] = ClusterUpdateTLSConfigRequest.propInfo['kind'].default;
        } else {
            this['kind'] = null
        }
        if (values && values['api-version'] != null) {
            this['api-version'] = values['api-version'];
        } else if (fillDefaults && ClusterUpdateTLSConfigRequest.hasDefaultValue('api-version')) {
            this['api-version'] = ClusterUpdateTLSConfigRequest.propInfo['api-version'].default;
        } else {
            this['api-version'] = null
        }
        if (values) {
            this['meta'].setValues(values['meta'], fillDefaults);
        } else {
            this['meta'].setValues(null, fillDefaults);
        }
        if (values && values['certs'] != null) {
            this['certs'] = values['certs'];
        } else if (fillDefaults && ClusterUpdateTLSConfigRequest.hasDefaultValue('certs')) {
            this['certs'] = ClusterUpdateTLSConfigRequest.propInfo['certs'].default;
        } else {
            this['certs'] = null
        }
        if (values && values['key'] != null) {
            this['key'] = values['key'];
        } else if (fillDefaults && ClusterUpdateTLSConfigRequest.hasDefaultValue('key')) {
            this['key'] = ClusterUpdateTLSConfigRequest.propInfo['key'].default;
        } else {
            this['key'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'kind': CustomFormControl(new FormControl(this['kind']), ClusterUpdateTLSConfigRequest.propInfo['kind']),
                'api-version': CustomFormControl(new FormControl(this['api-version']), ClusterUpdateTLSConfigRequest.propInfo['api-version']),
                'meta': CustomFormGroup(this['meta'].$formGroup, ClusterUpdateTLSConfigRequest.propInfo['meta'].required),
                'certs': CustomFormControl(new FormControl(this['certs']), ClusterUpdateTLSConfigRequest.propInfo['certs']),
                'key': CustomFormControl(new FormControl(this['key']), ClusterUpdateTLSConfigRequest.propInfo['key']),
            });
            // We force recalculation of controls under a form group
            Object.keys((this._formGroup.get('meta') as FormGroup).controls).forEach(field => {
                const control = this._formGroup.get('meta').get(field);
                control.updateValueAndValidity();
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
            this._formGroup.controls['certs'].setValue(this['certs']);
            this._formGroup.controls['key'].setValue(this['key']);
        }
    }
}

