/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { FieldsRequirement, IFieldsRequirement } from './fields-requirement.model';

export interface IFieldsSelector {
    'requirements'?: Array<IFieldsRequirement>;
}


export class FieldsSelector extends BaseModel implements IFieldsSelector {
    /** Requirements are ANDed. */
    'requirements': Array<FieldsRequirement> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'requirements': {
            description:  'Requirements are ANDed.',
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return FieldsSelector.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return FieldsSelector.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (FieldsSelector.propInfo[prop] != null &&
                        FieldsSelector.propInfo[prop].default != null &&
                        FieldsSelector.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['requirements'] = new Array<FieldsRequirement>();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values) {
            this.fillModelArray<FieldsRequirement>(this, 'requirements', values['requirements'], FieldsRequirement);
        } else {
            this['requirements'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'requirements': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<FieldsRequirement>('requirements', this['requirements'], FieldsRequirement);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this.fillModelArray<FieldsRequirement>(this, 'requirements', this['requirements'], FieldsRequirement);
        }
    }
}

