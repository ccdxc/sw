/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl, CustomFormGroup } from '../../../utils/validators';
import { BaseModel, PropInfoItem } from '../basemodel/base-model';

import { AuthLdapDomain, IAuthLdapDomain } from './auth-ldap-domain.model';

export interface IAuthLdap {
    'enabled'?: boolean;
    'domains'?: Array<IAuthLdapDomain>;
}


export class AuthLdap extends BaseModel implements IAuthLdap {
    /** Whether LDAP is enabled or not */
    'enabled': boolean = null;
    'domains': Array<AuthLdapDomain> = null;
    public static propInfo: { [prop in keyof IAuthLdap]: PropInfoItem } = {
        'enabled': {
            description:  'Whether LDAP is enabled or not',
            required: false,
            type: 'boolean'
        },
        'domains': {
            required: false,
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return AuthLdap.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return AuthLdap.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (AuthLdap.propInfo[prop] != null &&
                        AuthLdap.propInfo[prop].default != null);
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['domains'] = new Array<AuthLdapDomain>();
        this._inputValue = values;
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['enabled'] != null) {
            this['enabled'] = values['enabled'];
        } else if (fillDefaults && AuthLdap.hasDefaultValue('enabled')) {
            this['enabled'] = AuthLdap.propInfo['enabled'].default;
        } else {
            this['enabled'] = null
        }
        if (values) {
            this.fillModelArray<AuthLdapDomain>(this, 'domains', values['domains'], AuthLdapDomain);
        } else {
            this['domains'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'enabled': CustomFormControl(new FormControl(this['enabled']), AuthLdap.propInfo['enabled']),
                'domains': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<AuthLdapDomain>('domains', this['domains'], AuthLdapDomain);
            // We force recalculation of controls under a form group
            Object.keys((this._formGroup.get('domains') as FormGroup).controls).forEach(field => {
                const control = this._formGroup.get('domains').get(field);
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
            this._formGroup.controls['enabled'].setValue(this['enabled']);
            this.fillModelArray<AuthLdapDomain>(this, 'domains', this['domains'], AuthLdapDomain);
        }
    }
}

