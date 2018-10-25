/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';


export interface IApiAny {
    'type_url'?: string;
    'value'?: string;
}


export class ApiAny extends BaseModel implements IApiAny {
    'type_url': string = null;
    'value': string = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'type_url': {
            type: 'string'
        },
        'value': {
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return ApiAny.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (ApiAny.propInfo[prop] != null &&
                        ApiAny.propInfo[prop].default != null &&
                        ApiAny.propInfo[prop].default != '');
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
        if (values && values['type_url'] != null) {
            this['type_url'] = values['type_url'];
        } else if (fillDefaults && ApiAny.hasDefaultValue('type_url')) {
            this['type_url'] = ApiAny.propInfo['type_url'].default;
        }
        if (values && values['value'] != null) {
            this['value'] = values['value'];
        } else if (fillDefaults && ApiAny.hasDefaultValue('value')) {
            this['value'] = ApiAny.propInfo['value'].default;
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'type_url': new FormControl(this['type_url']),
                'value': new FormControl(this['value']),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['type_url'].setValue(this['type_url']);
            this._formGroup.controls['value'].setValue(this['value']);
        }
    }
}

