/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl, CustomFormGroup } from '../../../utils/validators';
import { BaseModel, PropInfoItem } from '../basemodel/base-model';


export interface IAuthUserPreferenceSpec {
    'options'?: string;
    '_ui'?: any;
}


export class AuthUserPreferenceSpec extends BaseModel implements IAuthUserPreferenceSpec {
    /** Field for holding arbitrary ui state */
    '_ui': any = {};
    'options': string = null;
    public static propInfo: { [prop in keyof IAuthUserPreferenceSpec]: PropInfoItem } = {
        'options': {
            required: false,
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return AuthUserPreferenceSpec.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return AuthUserPreferenceSpec.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (AuthUserPreferenceSpec.propInfo[prop] != null &&
                        AuthUserPreferenceSpec.propInfo[prop].default != null);
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this._inputValue = values;
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['_ui']) {
            this['_ui'] = values['_ui']
        }
        if (values && values['options'] != null) {
            this['options'] = values['options'];
        } else if (fillDefaults && AuthUserPreferenceSpec.hasDefaultValue('options')) {
            this['options'] = AuthUserPreferenceSpec.propInfo['options'].default;
        } else {
            this['options'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'options': CustomFormControl(new FormControl(this['options']), AuthUserPreferenceSpec.propInfo['options']),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['options'].setValue(this['options']);
        }
    }
}

