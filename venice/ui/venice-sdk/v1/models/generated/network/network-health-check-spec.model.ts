/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';


export interface INetworkHealthCheckSpec {
    'interval'?: number;
    'probes-per-interval'?: number;
    'probe-port-or-url'?: string;
    'max-timeouts'?: number;
    'declare-healthy-count'?: number;
}


export class NetworkHealthCheckSpec extends BaseModel implements INetworkHealthCheckSpec {
    'interval': number = null;
    'probes-per-interval': number = null;
    'probe-port-or-url': string = null;
    'max-timeouts': number = null;
    'declare-healthy-count': number = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'interval': {
            type: 'number'
        },
        'probes-per-interval': {
            type: 'number'
        },
        'probe-port-or-url': {
            type: 'string'
        },
        'max-timeouts': {
            type: 'number'
        },
        'declare-healthy-count': {
            type: 'number'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return NetworkHealthCheckSpec.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return NetworkHealthCheckSpec.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (NetworkHealthCheckSpec.propInfo[prop] != null &&
                        NetworkHealthCheckSpec.propInfo[prop].default != null &&
                        NetworkHealthCheckSpec.propInfo[prop].default != '');
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
        if (values && values['interval'] != null) {
            this['interval'] = values['interval'];
        } else if (fillDefaults && NetworkHealthCheckSpec.hasDefaultValue('interval')) {
            this['interval'] = NetworkHealthCheckSpec.propInfo['interval'].default;
        } else {
            this['interval'] = null
        }
        if (values && values['probes-per-interval'] != null) {
            this['probes-per-interval'] = values['probes-per-interval'];
        } else if (fillDefaults && NetworkHealthCheckSpec.hasDefaultValue('probes-per-interval')) {
            this['probes-per-interval'] = NetworkHealthCheckSpec.propInfo['probes-per-interval'].default;
        } else {
            this['probes-per-interval'] = null
        }
        if (values && values['probe-port-or-url'] != null) {
            this['probe-port-or-url'] = values['probe-port-or-url'];
        } else if (fillDefaults && NetworkHealthCheckSpec.hasDefaultValue('probe-port-or-url')) {
            this['probe-port-or-url'] = NetworkHealthCheckSpec.propInfo['probe-port-or-url'].default;
        } else {
            this['probe-port-or-url'] = null
        }
        if (values && values['max-timeouts'] != null) {
            this['max-timeouts'] = values['max-timeouts'];
        } else if (fillDefaults && NetworkHealthCheckSpec.hasDefaultValue('max-timeouts')) {
            this['max-timeouts'] = NetworkHealthCheckSpec.propInfo['max-timeouts'].default;
        } else {
            this['max-timeouts'] = null
        }
        if (values && values['declare-healthy-count'] != null) {
            this['declare-healthy-count'] = values['declare-healthy-count'];
        } else if (fillDefaults && NetworkHealthCheckSpec.hasDefaultValue('declare-healthy-count')) {
            this['declare-healthy-count'] = NetworkHealthCheckSpec.propInfo['declare-healthy-count'].default;
        } else {
            this['declare-healthy-count'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'interval': new FormControl(this['interval']),
                'probes-per-interval': new FormControl(this['probes-per-interval']),
                'probe-port-or-url': new FormControl(this['probe-port-or-url']),
                'max-timeouts': new FormControl(this['max-timeouts']),
                'declare-healthy-count': new FormControl(this['declare-healthy-count']),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['interval'].setValue(this['interval']);
            this._formGroup.controls['probes-per-interval'].setValue(this['probes-per-interval']);
            this._formGroup.controls['probe-port-or-url'].setValue(this['probe-port-or-url']);
            this._formGroup.controls['max-timeouts'].setValue(this['max-timeouts']);
            this._formGroup.controls['declare-healthy-count'].setValue(this['declare-healthy-count']);
        }
    }
}

