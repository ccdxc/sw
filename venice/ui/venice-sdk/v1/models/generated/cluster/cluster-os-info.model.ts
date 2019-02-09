/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl } from './validators';
import { BaseModel, PropInfoItem } from './base-model';


export interface IClusterOsInfo {
    'type'?: string;
    'kernel-relase'?: string;
    'kernel-version'?: string;
    'processor'?: string;
}


export class ClusterOsInfo extends BaseModel implements IClusterOsInfo {
    'type': string = null;
    'kernel-relase': string = null;
    'kernel-version': string = null;
    'processor': string = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'type': {
            type: 'string'
        },
        'kernel-relase': {
            type: 'string'
        },
        'kernel-version': {
            type: 'string'
        },
        'processor': {
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return ClusterOsInfo.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return ClusterOsInfo.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (ClusterOsInfo.propInfo[prop] != null &&
                        ClusterOsInfo.propInfo[prop].default != null &&
                        ClusterOsInfo.propInfo[prop].default != '');
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
        if (values && values['type'] != null) {
            this['type'] = values['type'];
        } else if (fillDefaults && ClusterOsInfo.hasDefaultValue('type')) {
            this['type'] = ClusterOsInfo.propInfo['type'].default;
        } else {
            this['type'] = null
        }
        if (values && values['kernel-relase'] != null) {
            this['kernel-relase'] = values['kernel-relase'];
        } else if (fillDefaults && ClusterOsInfo.hasDefaultValue('kernel-relase')) {
            this['kernel-relase'] = ClusterOsInfo.propInfo['kernel-relase'].default;
        } else {
            this['kernel-relase'] = null
        }
        if (values && values['kernel-version'] != null) {
            this['kernel-version'] = values['kernel-version'];
        } else if (fillDefaults && ClusterOsInfo.hasDefaultValue('kernel-version')) {
            this['kernel-version'] = ClusterOsInfo.propInfo['kernel-version'].default;
        } else {
            this['kernel-version'] = null
        }
        if (values && values['processor'] != null) {
            this['processor'] = values['processor'];
        } else if (fillDefaults && ClusterOsInfo.hasDefaultValue('processor')) {
            this['processor'] = ClusterOsInfo.propInfo['processor'].default;
        } else {
            this['processor'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'type': CustomFormControl(new FormControl(this['type']), ClusterOsInfo.propInfo['type'].description),
                'kernel-relase': CustomFormControl(new FormControl(this['kernel-relase']), ClusterOsInfo.propInfo['kernel-relase'].description),
                'kernel-version': CustomFormControl(new FormControl(this['kernel-version']), ClusterOsInfo.propInfo['kernel-version'].description),
                'processor': CustomFormControl(new FormControl(this['processor']), ClusterOsInfo.propInfo['processor'].description),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['type'].setValue(this['type']);
            this._formGroup.controls['kernel-relase'].setValue(this['kernel-relase']);
            this._formGroup.controls['kernel-version'].setValue(this['kernel-version']);
            this._formGroup.controls['processor'].setValue(this['processor']);
        }
    }
}

