/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { AuthPermission_resource_kind,  AuthPermission_resource_kind_uihint  } from './enums';
import { AuthPermission_actions,  AuthPermission_actions_uihint  } from './enums';

export interface IAuthPermission {
    'resource-tenant'?: string;
    'resource-group'?: string;
    'resource-kind'?: AuthPermission_resource_kind;
    'resource-namespace'?: string;
    'resource-names'?: Array<string>;
    'actions'?: Array<AuthPermission_actions>;
}


export class AuthPermission extends BaseModel implements IAuthPermission {
    /** ResourceTenant is the tenant to which resource belongs. For tenant scoped roles it will be automatically set to the tenant
    to which role object belongs. For cluster roles, if specified will give permission for resource belonging to that tenant. */
    'resource-tenant': string = null;
    'resource-group': string = null;
    'resource-kind': AuthPermission_resource_kind = null;
    'resource-namespace': string = null;
    /** ResourceNames identify specific objects on which this permission applies. */
    'resource-names': Array<string> = null;
    'actions': Array<AuthPermission_actions> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'resource-tenant': {
            description:  'ResourceTenant is the tenant to which resource belongs. For tenant scoped roles it will be automatically set to the tenant to which role object belongs. For cluster roles, if specified will give permission for resource belonging to that tenant.',
            type: 'string'
        },
        'resource-group': {
            type: 'string'
        },
        'resource-kind': {
            enum: AuthPermission_resource_kind_uihint,
            default: 'AllResourceKinds',
            type: 'string'
        },
        'resource-namespace': {
            type: 'string'
        },
        'resource-names': {
            description:  'ResourceNames identify specific objects on which this permission applies.',
            type: 'Array<string>'
        },
        'actions': {
            enum: AuthPermission_actions_uihint,
            default: 'ALL_ACTIONS',
            type: 'Array<string>'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return AuthPermission.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (AuthPermission.propInfo[prop] != null &&
                        AuthPermission.propInfo[prop].default != null &&
                        AuthPermission.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['resource-names'] = new Array<string>();
        this['actions'] = new Array<AuthPermission_actions>();
        this.setValues(values);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['resource-tenant'] != null) {
            this['resource-tenant'] = values['resource-tenant'];
        } else if (fillDefaults && AuthPermission.hasDefaultValue('resource-tenant')) {
            this['resource-tenant'] = AuthPermission.propInfo['resource-tenant'].default;
        }
        if (values && values['resource-group'] != null) {
            this['resource-group'] = values['resource-group'];
        } else if (fillDefaults && AuthPermission.hasDefaultValue('resource-group')) {
            this['resource-group'] = AuthPermission.propInfo['resource-group'].default;
        }
        if (values && values['resource-kind'] != null) {
            this['resource-kind'] = values['resource-kind'];
        } else if (fillDefaults && AuthPermission.hasDefaultValue('resource-kind')) {
            this['resource-kind'] = <AuthPermission_resource_kind>  AuthPermission.propInfo['resource-kind'].default;
        }
        if (values && values['resource-namespace'] != null) {
            this['resource-namespace'] = values['resource-namespace'];
        } else if (fillDefaults && AuthPermission.hasDefaultValue('resource-namespace')) {
            this['resource-namespace'] = AuthPermission.propInfo['resource-namespace'].default;
        }
        if (values && values['resource-names'] != null) {
            this['resource-names'] = values['resource-names'];
        }
        if (values && values['actions'] != null) {
            this['actions'] = values['actions'];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'resource-tenant': new FormControl(this['resource-tenant']),
                'resource-group': new FormControl(this['resource-group']),
                'resource-kind': new FormControl(this['resource-kind'], [enumValidator(AuthPermission_resource_kind), ]),
                'resource-namespace': new FormControl(this['resource-namespace']),
                'resource-names': new FormControl(this['resource-names']),
                'actions': new FormControl(this['actions']),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['resource-tenant'].setValue(this['resource-tenant']);
            this._formGroup.controls['resource-group'].setValue(this['resource-group']);
            this._formGroup.controls['resource-kind'].setValue(this['resource-kind']);
            this._formGroup.controls['resource-namespace'].setValue(this['resource-namespace']);
            this._formGroup.controls['resource-names'].setValue(this['resource-names']);
            this._formGroup.controls['actions'].setValue(this['actions']);
        }
    }
}

