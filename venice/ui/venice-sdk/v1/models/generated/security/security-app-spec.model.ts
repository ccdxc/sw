/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { SecurityProtoPort, ISecurityProtoPort } from './security-proto-port.model';
import { SecurityALG, ISecurityALG } from './security-alg.model';

export interface ISecurityAppSpec {
    'proto-ports'?: Array<ISecurityProtoPort>;
    'timeout'?: string;
    'alg'?: ISecurityALG;
}


export class SecurityAppSpec extends BaseModel implements ISecurityAppSpec {
    'proto-ports': Array<SecurityProtoPort> = null;
    /** Timeout specifies for how long the connection be kept before removing the flow entry, specified in string as '200s', '5m', etc. */
    'timeout': string = null;
    'alg': SecurityALG = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'proto-ports': {
            type: 'object'
        },
        'timeout': {
            description:  'Timeout specifies for how long the connection be kept before removing the flow entry, specified in string as &#x27;200s&#x27;, &#x27;5m&#x27;, etc.',
            hint:  '2h',
            type: 'string'
        },
        'alg': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return SecurityAppSpec.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return SecurityAppSpec.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (SecurityAppSpec.propInfo[prop] != null &&
                        SecurityAppSpec.propInfo[prop].default != null &&
                        SecurityAppSpec.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['proto-ports'] = new Array<SecurityProtoPort>();
        this['alg'] = new SecurityALG();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values) {
            this.fillModelArray<SecurityProtoPort>(this, 'proto-ports', values['proto-ports'], SecurityProtoPort);
        } else {
            this['proto-ports'] = [];
        }
        if (values && values['timeout'] != null) {
            this['timeout'] = values['timeout'];
        } else if (fillDefaults && SecurityAppSpec.hasDefaultValue('timeout')) {
            this['timeout'] = SecurityAppSpec.propInfo['timeout'].default;
        } else {
            this['timeout'] = null
        }
        if (values) {
            this['alg'].setValues(values['alg'], fillDefaults);
        } else {
            this['alg'].setValues(null, fillDefaults);
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'proto-ports': new FormArray([]),
                'timeout': new FormControl(this['timeout']),
                'alg': this['alg'].$formGroup,
            });
            // generate FormArray control elements
            this.fillFormArray<SecurityProtoPort>('proto-ports', this['proto-ports'], SecurityProtoPort);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this.fillModelArray<SecurityProtoPort>(this, 'proto-ports', this['proto-ports'], SecurityProtoPort);
            this._formGroup.controls['timeout'].setValue(this['timeout']);
            this['alg'].setFormGroupValuesToBeModelValues();
        }
    }
}

