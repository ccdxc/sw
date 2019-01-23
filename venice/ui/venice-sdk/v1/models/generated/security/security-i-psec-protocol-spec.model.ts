/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';


export interface ISecurityIPsecProtocolSpec {
    'encryption-transform'?: string;
    'integrity-transform'?: string;
}


export class SecurityIPsecProtocolSpec extends BaseModel implements ISecurityIPsecProtocolSpec {
    'encryption-transform': string = null;
    'integrity-transform': string = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'encryption-transform': {
            type: 'string'
        },
        'integrity-transform': {
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return SecurityIPsecProtocolSpec.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return SecurityIPsecProtocolSpec.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (SecurityIPsecProtocolSpec.propInfo[prop] != null &&
                        SecurityIPsecProtocolSpec.propInfo[prop].default != null &&
                        SecurityIPsecProtocolSpec.propInfo[prop].default != '');
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
        if (values && values['encryption-transform'] != null) {
            this['encryption-transform'] = values['encryption-transform'];
        } else if (fillDefaults && SecurityIPsecProtocolSpec.hasDefaultValue('encryption-transform')) {
            this['encryption-transform'] = SecurityIPsecProtocolSpec.propInfo['encryption-transform'].default;
        } else {
            this['encryption-transform'] = null
        }
        if (values && values['integrity-transform'] != null) {
            this['integrity-transform'] = values['integrity-transform'];
        } else if (fillDefaults && SecurityIPsecProtocolSpec.hasDefaultValue('integrity-transform')) {
            this['integrity-transform'] = SecurityIPsecProtocolSpec.propInfo['integrity-transform'].default;
        } else {
            this['integrity-transform'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'encryption-transform': new FormControl(this['encryption-transform']),
                'integrity-transform': new FormControl(this['integrity-transform']),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['encryption-transform'].setValue(this['encryption-transform']);
            this._formGroup.controls['integrity-transform'].setValue(this['integrity-transform']);
        }
    }
}

