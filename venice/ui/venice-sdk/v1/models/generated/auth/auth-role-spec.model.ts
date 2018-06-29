/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, EnumDef } from './base-model';

import { AuthPermission } from './auth-permission.model';

export interface IAuthRoleSpec {
    'permissions'?: Array<AuthPermission>;
}


export class AuthRoleSpec extends BaseModel implements IAuthRoleSpec {
    'permissions': Array<AuthPermission>;
    public static enumProperties: { [key: string] : EnumDef } = {
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['permissions'] = new Array<AuthPermission>();
        if (values) {
            this.setValues(values);
        }
    }

    /**
     * set the values.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any): void {
        if (values) {
            this.fillModelArray<AuthPermission>(this, 'permissions', values['permissions'], AuthPermission);
        }
    }

    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'permissions': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<AuthPermission>('permissions', this['permissions'], AuthPermission);
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this.fillModelArray<AuthPermission>(this, 'permissions', this['permissions'], AuthPermission);
        }
    }
}

