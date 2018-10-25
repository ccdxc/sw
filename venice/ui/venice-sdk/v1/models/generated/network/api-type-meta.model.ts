/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';


export interface IApiTypeMeta {
    'kind'?: string;
    'api-version'?: string;
}


export class ApiTypeMeta extends BaseModel implements IApiTypeMeta {
    /** Kind represents the type of the API object. */
    'kind': string = null;
    'api-version': string = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'kind': {
            description:  'Kind represents the type of the API object.',
            type: 'string'
        },
        'api-version': {
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return ApiTypeMeta.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (ApiTypeMeta.propInfo[prop] != null &&
                        ApiTypeMeta.propInfo[prop].default != null &&
                        ApiTypeMeta.propInfo[prop].default != '');
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
        if (values && values['kind'] != null) {
            this['kind'] = values['kind'];
        } else if (fillDefaults && ApiTypeMeta.hasDefaultValue('kind')) {
            this['kind'] = ApiTypeMeta.propInfo['kind'].default;
        }
        if (values && values['api-version'] != null) {
            this['api-version'] = values['api-version'];
        } else if (fillDefaults && ApiTypeMeta.hasDefaultValue('api-version')) {
            this['api-version'] = ApiTypeMeta.propInfo['api-version'].default;
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'kind': new FormControl(this['kind']),
                'api-version': new FormControl(this['api-version']),
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
        }
    }
}

