/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';


export interface IAuthRoleBindingSpec {
    'users'?: Array<string>;
    'user-groups'?: Array<string>;
    'role'?: string;
}


export class AuthRoleBindingSpec extends BaseModel implements IAuthRoleBindingSpec {
    'users': Array<string> = null;
    'user-groups': Array<string> = null;
    'role': string = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'users': {
            type: 'Array<string>'
        },
        'user-groups': {
            type: 'Array<string>'
        },
        'role': {
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return AuthRoleBindingSpec.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (AuthRoleBindingSpec.propInfo[prop] != null &&
                        AuthRoleBindingSpec.propInfo[prop].default != null &&
                        AuthRoleBindingSpec.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['users'] = new Array<string>();
        this['user-groups'] = new Array<string>();
        this.setValues(values);
    }

    /**
     * set the values. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any): void {
        if (values) {
            this.fillModelArray<string>(this, 'users', values['users']);
        }
        if (values) {
            this.fillModelArray<string>(this, 'user-groups', values['user-groups']);
        }
        if (values && values['role'] != null) {
            this['role'] = values['role'];
        } else if (AuthRoleBindingSpec.hasDefaultValue('role')) {
            this['role'] = AuthRoleBindingSpec.propInfo['role'].default;
        }
    }




    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'users': new FormArray([]),
                'user-groups': new FormArray([]),
                'role': new FormControl(this['role']),
            });
            // generate FormArray control elements
            this.fillFormArray<string>('users', this['users']);
            // generate FormArray control elements
            this.fillFormArray<string>('user-groups', this['user-groups']);
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this.fillModelArray<string>(this, 'users', this['users']);
            this.fillModelArray<string>(this, 'user-groups', this['user-groups']);
            this._formGroup.controls['role'].setValue(this['role']);
        }
    }
}

