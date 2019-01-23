/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';


export interface IApiListMeta {
    'resource-version'?: string;
}


export class ApiListMeta extends BaseModel implements IApiListMeta {
    /** Resource version of object store at the time of list generation. */
    'resource-version': string = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'resource-version': {
            description:  'Resource version of object store at the time of list generation.',
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return ApiListMeta.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return ApiListMeta.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (ApiListMeta.propInfo[prop] != null &&
                        ApiListMeta.propInfo[prop].default != null &&
                        ApiListMeta.propInfo[prop].default != '');
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
        if (values && values['resource-version'] != null) {
            this['resource-version'] = values['resource-version'];
        } else if (fillDefaults && ApiListMeta.hasDefaultValue('resource-version')) {
            this['resource-version'] = ApiListMeta.propInfo['resource-version'].default;
        } else {
            this['resource-version'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'resource-version': new FormControl(this['resource-version']),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['resource-version'].setValue(this['resource-version']);
        }
    }
}

