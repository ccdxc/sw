/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { ApiInterface, IApiInterface } from './api-interface.model';

export interface IApiInterfaceSlice {
    'Values'?: Array<IApiInterface>;
}


export class ApiInterfaceSlice extends BaseModel implements IApiInterfaceSlice {
    'Values': Array<ApiInterface> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'Values': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return ApiInterfaceSlice.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return ApiInterfaceSlice.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (ApiInterfaceSlice.propInfo[prop] != null &&
                        ApiInterfaceSlice.propInfo[prop].default != null &&
                        ApiInterfaceSlice.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['Values'] = new Array<ApiInterface>();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values) {
            this.fillModelArray<ApiInterface>(this, 'Values', values['Values'], ApiInterface);
        } else {
            this['Values'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'Values': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<ApiInterface>('Values', this['Values'], ApiInterface);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this.fillModelArray<ApiInterface>(this, 'Values', this['Values'], ApiInterface);
        }
    }
}

