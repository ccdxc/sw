/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';


export interface ISearchKindAggregation {
    'kinds'?: object;
}


export class SearchKindAggregation extends BaseModel implements ISearchKindAggregation {
    'kinds': object = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'kinds': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return SearchKindAggregation.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return SearchKindAggregation.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (SearchKindAggregation.propInfo[prop] != null &&
                        SearchKindAggregation.propInfo[prop].default != null &&
                        SearchKindAggregation.propInfo[prop].default != '');
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
        if (values && values['kinds'] != null) {
            this['kinds'] = values['kinds'];
        } else if (fillDefaults && SearchKindAggregation.hasDefaultValue('kinds')) {
            this['kinds'] = SearchKindAggregation.propInfo['kinds'].default;
        } else {
            this['kinds'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'kinds': new FormControl(this['kinds']),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['kinds'].setValue(this['kinds']);
        }
    }
}

