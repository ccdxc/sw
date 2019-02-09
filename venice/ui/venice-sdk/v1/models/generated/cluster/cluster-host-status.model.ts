/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl } from './validators';
import { BaseModel, PropInfoItem } from './base-model';


export interface IClusterHostStatus {
    'admitted-smart-nics'?: Array<string>;
}


export class ClusterHostStatus extends BaseModel implements IClusterHostStatus {
    'admitted-smart-nics': Array<string> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'admitted-smart-nics': {
            type: 'Array<string>'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return ClusterHostStatus.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return ClusterHostStatus.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (ClusterHostStatus.propInfo[prop] != null &&
                        ClusterHostStatus.propInfo[prop].default != null &&
                        ClusterHostStatus.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['admitted-smart-nics'] = new Array<string>();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['admitted-smart-nics'] != null) {
            this['admitted-smart-nics'] = values['admitted-smart-nics'];
        } else if (fillDefaults && ClusterHostStatus.hasDefaultValue('admitted-smart-nics')) {
            this['admitted-smart-nics'] = [ ClusterHostStatus.propInfo['admitted-smart-nics'].default];
        } else {
            this['admitted-smart-nics'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'admitted-smart-nics': CustomFormControl(new FormControl(this['admitted-smart-nics']), ClusterHostStatus.propInfo['admitted-smart-nics'].description),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['admitted-smart-nics'].setValue(this['admitted-smart-nics']);
        }
    }
}

