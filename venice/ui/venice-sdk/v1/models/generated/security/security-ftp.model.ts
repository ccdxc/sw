/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl } from './validators';
import { BaseModel, PropInfoItem } from './base-model';


export interface ISecurityFtp {
    'allow-mismatch-ip-address'?: boolean;
}


export class SecurityFtp extends BaseModel implements ISecurityFtp {
    'allow-mismatch-ip-address': boolean = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'allow-mismatch-ip-address': {
            type: 'boolean'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return SecurityFtp.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return SecurityFtp.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (SecurityFtp.propInfo[prop] != null &&
                        SecurityFtp.propInfo[prop].default != null &&
                        SecurityFtp.propInfo[prop].default != '');
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
        if (values && values['allow-mismatch-ip-address'] != null) {
            this['allow-mismatch-ip-address'] = values['allow-mismatch-ip-address'];
        } else if (fillDefaults && SecurityFtp.hasDefaultValue('allow-mismatch-ip-address')) {
            this['allow-mismatch-ip-address'] = SecurityFtp.propInfo['allow-mismatch-ip-address'].default;
        } else {
            this['allow-mismatch-ip-address'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'allow-mismatch-ip-address': CustomFormControl(new FormControl(this['allow-mismatch-ip-address']), SecurityFtp.propInfo['allow-mismatch-ip-address'].description),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['allow-mismatch-ip-address'].setValue(this['allow-mismatch-ip-address']);
        }
    }
}

