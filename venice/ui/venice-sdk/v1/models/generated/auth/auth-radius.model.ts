/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { AuthRadiusServer, IAuthRadiusServer } from './auth-radius-server.model';

export interface IAuthRadius {
    'enabled'?: boolean;
    'nas-id'?: string;
    'servers'?: Array<IAuthRadiusServer>;
}


export class AuthRadius extends BaseModel implements IAuthRadius {
    'enabled': boolean = null;
    'nas-id': string = null;
    'servers': Array<AuthRadiusServer> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'enabled': {
            type: 'boolean'
        },
        'nas-id': {
            type: 'string'
        },
        'servers': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return AuthRadius.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (AuthRadius.propInfo[prop] != null &&
                        AuthRadius.propInfo[prop].default != null &&
                        AuthRadius.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['servers'] = new Array<AuthRadiusServer>();
        this.setValues(values);
    }

    /**
     * set the values. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any): void {
        if (values && values['enabled'] != null) {
            this['enabled'] = values['enabled'];
        } else if (AuthRadius.hasDefaultValue('enabled')) {
            this['enabled'] = AuthRadius.propInfo['enabled'].default;
        }
        if (values && values['nas-id'] != null) {
            this['nas-id'] = values['nas-id'];
        } else if (AuthRadius.hasDefaultValue('nas-id')) {
            this['nas-id'] = AuthRadius.propInfo['nas-id'].default;
        }
        if (values) {
            this.fillModelArray<AuthRadiusServer>(this, 'servers', values['servers'], AuthRadiusServer);
        }
    }




    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'enabled': new FormControl(this['enabled']),
                'nas-id': new FormControl(this['nas-id']),
                'servers': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<AuthRadiusServer>('servers', this['servers'], AuthRadiusServer);
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this._formGroup.controls['enabled'].setValue(this['enabled']);
            this._formGroup.controls['nas-id'].setValue(this['nas-id']);
            this.fillModelArray<AuthRadiusServer>(this, 'servers', this['servers'], AuthRadiusServer);
        }
    }
}

