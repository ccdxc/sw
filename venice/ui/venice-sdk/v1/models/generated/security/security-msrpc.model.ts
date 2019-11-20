/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl, CustomFormGroup } from '../../../utils/validators';
import { BaseModel, PropInfoItem } from '../basemodel/base-model';


export interface ISecurityMsrpc {
    'program-uuid'?: string;
    'timeout': string;
}


export class SecurityMsrpc extends BaseModel implements ISecurityMsrpc {
    /** MSRPC Program identifier */
    'program-uuid': string = null;
    /** Timeout for this program idshould be a valid time duration
 */
    'timeout': string = null;
    public static propInfo: { [prop in keyof ISecurityMsrpc]: PropInfoItem } = {
        'program-uuid': {
            description:  'MSRPC Program identifier',
            required: false,
            type: 'string'
        },
        'timeout': {
            description:  'Timeout for this program idshould be a valid time duration ',
            hint:  '2h',
            required: true,
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return SecurityMsrpc.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return SecurityMsrpc.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (SecurityMsrpc.propInfo[prop] != null &&
                        SecurityMsrpc.propInfo[prop].default != null);
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
        if (values && values['program-uuid'] != null) {
            this['program-uuid'] = values['program-uuid'];
        } else if (fillDefaults && SecurityMsrpc.hasDefaultValue('program-uuid')) {
            this['program-uuid'] = SecurityMsrpc.propInfo['program-uuid'].default;
        } else {
            this['program-uuid'] = null
        }
        if (values && values['timeout'] != null) {
            this['timeout'] = values['timeout'];
        } else if (fillDefaults && SecurityMsrpc.hasDefaultValue('timeout')) {
            this['timeout'] = SecurityMsrpc.propInfo['timeout'].default;
        } else {
            this['timeout'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'program-uuid': CustomFormControl(new FormControl(this['program-uuid']), SecurityMsrpc.propInfo['program-uuid']),
                'timeout': CustomFormControl(new FormControl(this['timeout'], [required, ]), SecurityMsrpc.propInfo['timeout']),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['program-uuid'].setValue(this['program-uuid']);
            this._formGroup.controls['timeout'].setValue(this['timeout']);
        }
    }
}

