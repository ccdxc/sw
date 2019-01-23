/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';


export interface IAuthLdapAttributeMapping {
    'user'?: string;
    'user-object-class'?: string;
    'tenant'?: string;
    'group'?: string;
    'group-object-class'?: string;
    'email'?: string;
    'fullname'?: string;
}


export class AuthLdapAttributeMapping extends BaseModel implements IAuthLdapAttributeMapping {
    'user': string = null;
    'user-object-class': string = null;
    'tenant': string = null;
    'group': string = null;
    'group-object-class': string = null;
    'email': string = null;
    'fullname': string = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'user': {
            type: 'string'
        },
        'user-object-class': {
            type: 'string'
        },
        'tenant': {
            type: 'string'
        },
        'group': {
            type: 'string'
        },
        'group-object-class': {
            type: 'string'
        },
        'email': {
            type: 'string'
        },
        'fullname': {
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return AuthLdapAttributeMapping.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return AuthLdapAttributeMapping.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (AuthLdapAttributeMapping.propInfo[prop] != null &&
                        AuthLdapAttributeMapping.propInfo[prop].default != null &&
                        AuthLdapAttributeMapping.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['user'] != null) {
            this['user'] = values['user'];
        } else if (fillDefaults && AuthLdapAttributeMapping.hasDefaultValue('user')) {
            this['user'] = AuthLdapAttributeMapping.propInfo['user'].default;
        } else {
            this['user'] = null
        }
        if (values && values['user-object-class'] != null) {
            this['user-object-class'] = values['user-object-class'];
        } else if (fillDefaults && AuthLdapAttributeMapping.hasDefaultValue('user-object-class')) {
            this['user-object-class'] = AuthLdapAttributeMapping.propInfo['user-object-class'].default;
        } else {
            this['user-object-class'] = null
        }
        if (values && values['tenant'] != null) {
            this['tenant'] = values['tenant'];
        } else if (fillDefaults && AuthLdapAttributeMapping.hasDefaultValue('tenant')) {
            this['tenant'] = AuthLdapAttributeMapping.propInfo['tenant'].default;
        } else {
            this['tenant'] = null
        }
        if (values && values['group'] != null) {
            this['group'] = values['group'];
        } else if (fillDefaults && AuthLdapAttributeMapping.hasDefaultValue('group')) {
            this['group'] = AuthLdapAttributeMapping.propInfo['group'].default;
        } else {
            this['group'] = null
        }
        if (values && values['group-object-class'] != null) {
            this['group-object-class'] = values['group-object-class'];
        } else if (fillDefaults && AuthLdapAttributeMapping.hasDefaultValue('group-object-class')) {
            this['group-object-class'] = AuthLdapAttributeMapping.propInfo['group-object-class'].default;
        } else {
            this['group-object-class'] = null
        }
        if (values && values['email'] != null) {
            this['email'] = values['email'];
        } else if (fillDefaults && AuthLdapAttributeMapping.hasDefaultValue('email')) {
            this['email'] = AuthLdapAttributeMapping.propInfo['email'].default;
        } else {
            this['email'] = null
        }
        if (values && values['fullname'] != null) {
            this['fullname'] = values['fullname'];
        } else if (fillDefaults && AuthLdapAttributeMapping.hasDefaultValue('fullname')) {
            this['fullname'] = AuthLdapAttributeMapping.propInfo['fullname'].default;
        } else {
            this['fullname'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'user': new FormControl(this['user']),
                'user-object-class': new FormControl(this['user-object-class']),
                'tenant': new FormControl(this['tenant']),
                'group': new FormControl(this['group']),
                'group-object-class': new FormControl(this['group-object-class']),
                'email': new FormControl(this['email']),
                'fullname': new FormControl(this['fullname']),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['user'].setValue(this['user']);
            this._formGroup.controls['user-object-class'].setValue(this['user-object-class']);
            this._formGroup.controls['tenant'].setValue(this['tenant']);
            this._formGroup.controls['group'].setValue(this['group']);
            this._formGroup.controls['group-object-class'].setValue(this['group-object-class']);
            this._formGroup.controls['email'].setValue(this['email']);
            this._formGroup.controls['fullname'].setValue(this['fullname']);
        }
    }
}

