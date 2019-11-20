/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl, CustomFormGroup } from '../../../utils/validators';
import { BaseModel, PropInfoItem } from '../basemodel/base-model';

import { AuthPermission_actions,  AuthPermission_actions_uihint  } from './enums';

export interface IAuthPermission {
    'resource-tenant'?: string;
    'resource-group'?: string;
    'resource-kind'?: string;
    'resource-namespace'?: string;
    'resource-names'?: Array<string>;
    'actions': Array<AuthPermission_actions>;
}


export class AuthPermission extends BaseModel implements IAuthPermission {
    /** ResourceTenant is the tenant to which resource belongs. It will be automatically set to the tenant to which role object belongs. Exception are roles in "default" tenant.
Role in "default" tenant can include permissions for resources in other tenants. Specifying "_All_" will match all tenants. */
    'resource-tenant': string = null;
    /** ResourceGroup is grouping of resource types for which a permission is defined. It is empty for Search, Event, MetricsQuery and non-api server endpoint.
Specifying "_All_" will match all api groups including empty group for non-api server endpoints like those defined in ResrcKind enum. */
    'resource-group': string = null;
    /** ResourceKind is a resource kind for which permission is defined. It can be an API Server object kind or kinds defined in
ResrcKind enum. Specifying "_All_" will match all resource kinds. */
    'resource-kind': string = null;
    /** ResourceNamespace is a namespace to which a resource (API Server object) belongs. Default value is "_All_" which matches all namespaces. */
    'resource-namespace': string = null;
    /** ResourceNames identify specific objects on which this permission applies. If resource name is not specified in permission then
it implies all resources of the specified kind */
    'resource-names': Array<string> = null;
    /** Actions are actions on a resource that a permission allows */
    'actions': Array<AuthPermission_actions> = null;
    public static propInfo: { [prop in keyof IAuthPermission]: PropInfoItem } = {
        'resource-tenant': {
            description:  'ResourceTenant is the tenant to which resource belongs. It will be automatically set to the tenant to which role object belongs. Exception are roles in &quot;default&quot; tenant. Role in &quot;default&quot; tenant can include permissions for resources in other tenants. Specifying &quot;_All_&quot; will match all tenants.',
            required: false,
            type: 'string'
        },
        'resource-group': {
            description:  'ResourceGroup is grouping of resource types for which a permission is defined. It is empty for Search, Event, MetricsQuery and non-api server endpoint. Specifying &quot;_All_&quot; will match all api groups including empty group for non-api server endpoints like those defined in ResrcKind enum.',
            required: false,
            type: 'string'
        },
        'resource-kind': {
            description:  'ResourceKind is a resource kind for which permission is defined. It can be an API Server object kind or kinds defined in ResrcKind enum. Specifying &quot;_All_&quot; will match all resource kinds.',
            required: false,
            type: 'string'
        },
        'resource-namespace': {
            default: '_All_',
            description:  'ResourceNamespace is a namespace to which a resource (API Server object) belongs. Default value is &quot;_All_&quot; which matches all namespaces.',
            required: false,
            type: 'string'
        },
        'resource-names': {
            description:  'ResourceNames identify specific objects on which this permission applies. If resource name is not specified in permission then it implies all resources of the specified kind',
            required: false,
            type: 'Array<string>'
        },
        'actions': {
            enum: AuthPermission_actions_uihint,
            default: 'all-actions',
            description:  'Actions are actions on a resource that a permission allows',
            required: true,
            type: 'Array<string>'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return AuthPermission.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return AuthPermission.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (AuthPermission.propInfo[prop] != null &&
                        AuthPermission.propInfo[prop].default != null);
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['resource-names'] = new Array<string>();
        this['actions'] = new Array<AuthPermission_actions>();
        this._inputValue = values;
        this.setValues(values, setDefaults);
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
        } else {
            this['resource-tenant'] = null
        }
        if (values && values['resource-group'] != null) {
            this['resource-group'] = values['resource-group'];
        } else if (fillDefaults && AuthPermission.hasDefaultValue('resource-group')) {
            this['resource-group'] = AuthPermission.propInfo['resource-group'].default;
        } else {
            this['resource-group'] = null
        }
        if (values && values['resource-kind'] != null) {
            this['resource-kind'] = values['resource-kind'];
        } else if (fillDefaults && AuthPermission.hasDefaultValue('resource-kind')) {
            this['resource-kind'] = AuthPermission.propInfo['resource-kind'].default;
        } else {
            this['resource-kind'] = null
        }
        if (values && values['resource-namespace'] != null) {
            this['resource-namespace'] = values['resource-namespace'];
        } else if (fillDefaults && AuthPermission.hasDefaultValue('resource-namespace')) {
            this['resource-namespace'] = AuthPermission.propInfo['resource-namespace'].default;
        } else {
            this['resource-namespace'] = null
        }
        if (values && values['resource-names'] != null) {
            this['resource-names'] = values['resource-names'];
        } else if (fillDefaults && AuthPermission.hasDefaultValue('resource-names')) {
            this['resource-names'] = [ AuthPermission.propInfo['resource-names'].default];
        } else {
            this['resource-names'] = [];
        }
        if (values && values['actions'] != null) {
            this['actions'] = values['actions'];
        } else if (fillDefaults && AuthPermission.hasDefaultValue('actions')) {
            this['actions'] = [ AuthPermission.propInfo['actions'].default];
        } else {
            this['actions'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'resource-tenant': CustomFormControl(new FormControl(this['resource-tenant']), AuthPermission.propInfo['resource-tenant']),
                'resource-group': CustomFormControl(new FormControl(this['resource-group']), AuthPermission.propInfo['resource-group']),
                'resource-kind': CustomFormControl(new FormControl(this['resource-kind']), AuthPermission.propInfo['resource-kind']),
                'resource-namespace': CustomFormControl(new FormControl(this['resource-namespace']), AuthPermission.propInfo['resource-namespace']),
                'resource-names': CustomFormControl(new FormControl(this['resource-names']), AuthPermission.propInfo['resource-names']),
                'actions': CustomFormControl(new FormControl(this['actions']), AuthPermission.propInfo['actions']),
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

